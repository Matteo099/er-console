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

private:
    PortalService& service;
};