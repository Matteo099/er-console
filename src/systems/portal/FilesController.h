#pragma once

#include <WebServer.h>
#include "PortalService.h"

class FilesController {
  public:
    explicit FilesController(PortalService& service);
    void registerRoutes(WebServer& server);

  private:
    void listFiles(WebServer& server);
    void deleteFile(WebServer& server);
    void moveFile(WebServer& server);
    void downloadFile(WebServer& server);
    void uploadFile(WebServer& server);
    static String guessMimeType(const String& path);
    static String normalizePath(const String& rawPath);

  private:
    PortalService& service;
    mutable File uploadTarget;
    mutable String uploadPath;
};