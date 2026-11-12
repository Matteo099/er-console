#include "PortalSystem.h"

#include <WiFi.h>

#include "ConfigController.h"
#include "FilesController.h"
#include "StatusController.h"
#include "config/Assets.h"
#include "config/Console.h"
#include "core/SystemEventBus.h"
#include "log/Logger.h"
#include "systems/input/InputSystem.h"
#include "drivers/bus/SPIBusLock.h"

namespace {
static String normalizePortalRequestPath(const String& uri) {
    if (uri == "/" || uri == "/index.html")
        return PORTAL_INDEX_FILE;

    String requestPath = uri;
    if (!requestPath.startsWith("/"))
        requestPath = String("/") + requestPath;

    if (requestPath.startsWith(PORTAL_DIR))
        return requestPath;

    return String(PORTAL_DIR) + requestPath;
}

static bool shouldFallbackToPortalIndex(const String& uri) {
    if (uri == "/" || uri == "/index.html")
        return true;

    // return uri.indexOf('.') < 0;
    // also treat known SPA routes (no extension)
    return uri.indexOf('.') < 0 && !uri.startsWith("/api/");
}

static bool isCaptivePortalProbe(const String& uri) {
    return uri == "/generate_204" || uri == "/hotspot-detect.html" || uri == "/ncsi.txt" ||
           uri == "/connecttest.txt" || uri == "/redirect";
}
}  // namespace

PortalSystem::PortalSystem() : server(80) {
}

void PortalSystem::init(void* params) {
    Log::info(Log::PORTAL_TAG, "Initializing portal system");

#if !WIFI_ENABLED || !PORTAL_ENABLED
    running = false;
    started = false;
    Log::warn(Log::PORTAL_TAG, "Portal disabled in Console.h (requires Wi-Fi and portal enabled)");
    return;
#endif

    storageSystem = static_cast<StorageSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    networkSystem = static_cast<NetworkSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_NETWORK));
    inputSystem = static_cast<InputSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_INPUT));
    service.setDependencies(storageSystem, networkSystem, inputSystem);

    if (!storageSystem || !storageSystem->isReady()) {
        Log::warn(Log::PORTAL_TAG, "Storage unavailable, portal will stay disabled");
        running = false;
        return;
    }

    startPortal();
    running = started;
}

void PortalSystem::update() {
    if (!running)
        return;

    dnsServer.processNextRequest();
    server.handleClient();
}

void PortalSystem::startPortal() {
    if (started)
        return;

    const std::string apSsid = service.portalSsid();

    WiFi.mode(WIFI_AP_STA);
    bool apStarted = WiFi.softAP(apSsid.c_str());
    if (!apStarted) {
        Log::error(Log::PORTAL_TAG, "Failed to start access point");
        return;
    }

    dnsServer.start(53, "*", WiFi.softAPIP());

    registerRoutes();
    started = true;

    Log::info(Log::PORTAL_TAG, "Portal available at %s", service.portalUrl().c_str());
}

void PortalSystem::registerRoutes() {
    statusController = new StatusController(service);
    configController = new ConfigController(service);
    filesController = new FilesController(service);

    statusController->registerRoutes(server);
    configController->registerRoutes(server);
    filesController->registerRoutes(server);

    server.on("/api/portal/url", HTTP_GET, [this]() {
        const std::string payload = std::string("{\"url\":\"") + service.portalUrl() + "\"}";
        server.send(200, "application/json", payload.c_str());
    });

    const char* headerKeys[] = {"Accept-Encoding"};
    server.collectHeaders(headerKeys, 1);

    server.onNotFound([this]() {
        serveStaticOrFallback();
    });

    server.begin();
}

void PortalSystem::serveStaticOrFallback() {
    const String uri = server.uri();
    Log::info(Log::PORTAL_TAG, "Received request for '%s'", uri.c_str());
    if (isCaptivePortalProbe(uri)) {
        server.send(204);
        return;
    }

    if (uri.startsWith("/api/")) {
        server.send(404, "application/json", "{\"error\":\"not_found\"}");
        return;
    }

    const String storagePath = normalizePortalRequestPath(uri);
    if (service.storage() && service.storage()->exists(storagePath.c_str())) {
        Log::info(Log::PORTAL_TAG, "Serving file '%s'", storagePath.c_str());
        serveFile(storagePath);
        return;
    }

    if (shouldFallbackToPortalIndex(uri)) {
        serveFile(PORTAL_INDEX_FILE, false);
        return;
    }

    Log::warn(Log::PORTAL_TAG, "File not found for request '%s'", uri.c_str());
    server.send(404, "text/plain", "Not found");
}

void PortalSystem::serveFile(const String& path, bool fallbackToIndex) {
    SPIBusLock::Guard guard;

    if (!service.storage()) {
        server.send(500, "text/plain", "Storage unavailable");
        return;
    }

    const bool clientWantsGzip = false;  // Disable gzip for now
    // const bool clientWantsGzip = clientAcceptsGzip();

    const String requestPath = path;
    const String storagePath = normalizePortalRequestPath(requestPath);
    String selectedPath = storagePath;
    bool usingGzip = false;

    // Only allow gzip for compressible assets
    bool isCompressible = storagePath.endsWith(".js") || storagePath.endsWith(".css") ||
                          storagePath.endsWith(".json") || storagePath.endsWith(".svg");

    if (clientWantsGzip && isCompressible) {
        const String gzipPath = storagePath + ".gz";

        if (service.storage()->exists(gzipPath.c_str())) {
            File test = service.storage()->openNative(gzipPath.c_str(), false);

            if (test && test.size() > 20) {
                uint8_t b1 = test.read();
                uint8_t b2 = test.read();
                test.close();

                // gzip magic header: 1F 8B
                if (b1 == 0x1F && b2 == 0x8B) {
                    selectedPath = gzipPath;
                    usingGzip = true;
                }
            }
        }
    }

    File file = service.storage()->openNative(selectedPath.c_str(), false);

    if (!file && usingGzip) {
        usingGzip = false;
        selectedPath = storagePath;
        file = service.storage()->openNative(selectedPath.c_str(), false);
    }

    if (!file) {
        if (fallbackToIndex && requestPath != PORTAL_INDEX_FILE) {
            serveFile(PORTAL_INDEX_FILE, false);
            return;
        }
        server.send(404, "text/plain", "Not found");
        return;
    }

    if (usingGzip) {
        server.sendHeader("Content-Encoding", "gzip");
        server.sendHeader("Vary", "Accept-Encoding");
    }

    server.streamFile(file, mimeTypeForPath(selectedPath));
    file.close();
}

bool PortalSystem::clientAcceptsGzip() {
    if (!server.hasHeader("Accept-Encoding"))
        return false;

    String acceptEncoding = server.header("Accept-Encoding");
    acceptEncoding.toLowerCase();
    return acceptEncoding.indexOf("gzip") >= 0;
}

String PortalSystem::mimeTypeForPath(const String& path) {
    String normalizedPath = path;
    if (normalizedPath.endsWith(".gz"))
        normalizedPath.remove(normalizedPath.length() - 3);

    if (normalizedPath.endsWith(".html"))
        return "text/html";
    if (normalizedPath.endsWith(".css"))
        return "text/css";
    if (normalizedPath.endsWith(".js"))
        return "application/javascript";
    if (normalizedPath.endsWith(".json"))
        return "application/json";
    if (normalizedPath.endsWith(".svg"))
        return "image/svg+xml";
    if (normalizedPath.endsWith(".png"))
        return "image/png";
    if (normalizedPath.endsWith(".jpg") || normalizedPath.endsWith(".jpeg"))
        return "image/jpeg";
    return "application/octet-stream";
}
