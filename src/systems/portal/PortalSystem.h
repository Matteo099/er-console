#pragma once

#include <WebServer.h>
#include <DNSServer.h>

#include "core/SystemService.h"
#include "PortalService.h"

class StatusController;
class ConfigController;
class FilesController;

class PortalSystem : public SystemService
{
public:
    PortalSystem();

    void init(void* params = nullptr) override;
    void update() override;

    bool isRunning() const { return running; }
    std::string portalUrl() const { return service.portalUrl(); }
    std::string wifiQrPayload() const { return service.wifiQrPayload(); }

private:
    void startPortal();
    void registerRoutes();
    void serveStaticOrFallback();
    void serveFile(const String& path, bool fallbackToIndex = false);
    bool clientAcceptsGzip();
    static String mimeTypeForPath(const String& path);

private:
    StorageSystem* storageSystem = nullptr;
    NetworkSystem* networkSystem = nullptr;
    PortalService service;
    WebServer server;
    DNSServer dnsServer;
    bool running = false;
    bool started = false;

    StatusController* statusController = nullptr;
    ConfigController* configController = nullptr;
    FilesController* filesController = nullptr;
};