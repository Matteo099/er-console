#include "PortalSystem.h"

#include <WiFi.h>

#include "ConfigController.h"
#include "FilesController.h"
#include "StatusController.h"
#include "config/Assets.h"
#include "core/SystemEventBus.h"
#include "log/Logger.h"

PortalSystem::PortalSystem()
    : server(80)
{
}

void PortalSystem::init(void* params)
{
    Log::info(Log::PORTAL_TAG, "Initializing portal system");

    storageSystem = static_cast<StorageSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    networkSystem = static_cast<NetworkSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_NETWORK));
    service.setDependencies(storageSystem, networkSystem);

    if (!storageSystem || !storageSystem->isReady())
    {
        Log::warn(Log::PORTAL_TAG, "Storage unavailable, portal will stay disabled");
        running = false;
        return;
    }

    startPortal();
    running = started;
}

void PortalSystem::update()
{
    if (!running)
        return;

    dnsServer.processNextRequest();
    server.handleClient();
}

void PortalSystem::startPortal()
{
    if (started)
        return;

    const std::string apSsid = service.portalSsid();

    WiFi.mode(WIFI_AP_STA);
    bool apStarted = WiFi.softAP(apSsid.c_str());
    if (!apStarted)
    {
        Log::error(Log::PORTAL_TAG, "Failed to start access point");
        return;
    }

    dnsServer.start(53, "*", WiFi.softAPIP());

    registerRoutes();
    started = true;

    Log::info(Log::PORTAL_TAG, "Portal available at %s", service.portalUrl().c_str());
}

void PortalSystem::registerRoutes()
{
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

    server.onNotFound([this]() { serveStaticOrFallback(); });

    server.begin();
}

void PortalSystem::serveStaticOrFallback()
{
    const String uri = server.uri();
    if (uri == "/" || uri == "/index.html")
    {
        serveFile(PORTAL_INDEX_FILE);
        return;
    }

    String filePath = uri;
    if (!filePath.startsWith("/"))
        filePath = String("/") + filePath;

    if (service.storage() && service.storage()->exists(filePath.c_str()))
    {
        serveFile(filePath);
        return;
    }

    serveFile(PORTAL_INDEX_FILE);
}

void PortalSystem::serveFile(const String& path)
{
    if (!service.storage())
    {
        server.send(500, "text/plain", "Storage unavailable");
        return;
    }

    File file = service.storage()->openNative(path.c_str(), false);
    if (!file)
    {
        server.send(404, "text/plain", "Not found");
        return;
    }

    server.streamFile(file, mimeTypeForPath(path));
    file.close();
}

String PortalSystem::mimeTypeForPath(const String& path)
{
    if (path.endsWith(".html")) return "text/html";
    if (path.endsWith(".css")) return "text/css";
    if (path.endsWith(".js")) return "application/javascript";
    if (path.endsWith(".json")) return "application/json";
    if (path.endsWith(".svg")) return "image/svg+xml";
    if (path.endsWith(".png")) return "image/png";
    if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
    return "application/octet-stream";
}