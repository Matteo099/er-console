#pragma once

#include <WebServer.h>
#include "PortalService.h"

class ConfigController
{
public:
    explicit ConfigController(PortalService& service);
    void registerRoutes(WebServer& server);

private:
    void sendConfigPayload(WebServer& server, const DeviceConfig& config);
    void sendConfig(WebServer& server);
    void updateConfig(WebServer& server);
    void resetConfig(WebServer& server);

private:
    PortalService& service;
};