#pragma once

#include <WebServer.h>
#include "PortalService.h"

class StatusController
{
public:
    explicit StatusController(PortalService& service);
    void registerRoutes(WebServer& server);

private:
    void sendStatus(WebServer& server);
    void scanWifi(WebServer& server);
    void getLogs(WebServer& server);
    void clearLogs(WebServer& server);
    void rebootSystem(WebServer& server);
    void shutdownSystem(WebServer& server);

private:
    PortalService& service;
};