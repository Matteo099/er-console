#pragma once

#include <WebServer.h>
#include "PortalService.h"

class ConfigController
{
public:
    explicit ConfigController(PortalService& service);
    void registerRoutes(WebServer& server);

private:
    void sendConfig(WebServer& server);
    void updateConfig(WebServer& server);

private:
    PortalService& service;
};