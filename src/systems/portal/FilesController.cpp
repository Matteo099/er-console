#include "FilesController.h"

#include <ArduinoJson.h>

FilesController::FilesController(PortalService& service)
    : service(service)
{
}

void FilesController::registerRoutes(WebServer& server)
{
    server.on("/api/files", HTTP_GET, [&server, this]() { listFiles(server); });
    server.on("/api/files", HTTP_DELETE, [&server, this]() { deleteFile(server); });
    server.on("/api/files/upload", HTTP_POST,
              [&server, this]() { server.send(200, "application/json", "{\"ok\":true}"); },
              [&server, this]() { uploadFile(server); });
}

void FilesController::listFiles(WebServer& server)
{
    const String path = server.hasArg("path") ? server.arg("path") : String("/.core");
    const std::vector<StorageEntry> entries = service.listFiles(path.c_str());

    DynamicJsonDocument doc(2048);
    JsonArray items = doc["items"].to<JsonArray>();
    for (const StorageEntry& entry : entries)
    {
        JsonObject item = items.add<JsonObject>();
        item["path"] = entry.path;
        item["name"] = entry.name;
        item["directory"] = entry.isDirectory;
        item["size"] = entry.size;
    }

    doc["path"] = path.c_str();

    String body;
    serializeJsonPretty(doc, body);
    server.send(200, "application/json", body.c_str());
}

void FilesController::deleteFile(WebServer& server)
{
    if (!server.hasArg("path"))
    {
        server.send(400, "application/json", "{\"error\":\"missing_path\"}");
        return;
    }

    const String path = server.arg("path");
    if (!service.deletePath(path.c_str()))
    {
        server.send(500, "application/json", "{\"error\":\"delete_failed\"}");
        return;
    }

    server.send(200, "application/json", "{\"ok\":true}");
}

void FilesController::uploadFile(WebServer& server)
{
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START)
    {
        uploadPath = normalizePath(server.hasArg("path") ? server.arg("path") : String("/.core/upload.bin"));
        uploadTarget = service.openUploadTarget(uploadPath.c_str(), true);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (uploadTarget)
            uploadTarget.write(upload.buf, upload.currentSize);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (uploadTarget)
        {
            uploadTarget.flush();
            uploadTarget.close();
        }
        uploadTarget = File();
        uploadPath = String();
    }
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
        if (uploadTarget)
            uploadTarget.close();
        uploadTarget = File();
        uploadPath = String();
    }
}

String FilesController::guessMimeType(const String& path)
{
    if (path.endsWith(".html") || path.endsWith(".htm")) return "text/html";
    if (path.endsWith(".css")) return "text/css";
    if (path.endsWith(".js") || path.endsWith(".mjs")) return "application/javascript";
    if (path.endsWith(".json")) return "application/json";
    if (path.endsWith(".png")) return "image/png";
    if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
    if (path.endsWith(".svg")) return "image/svg+xml";
    return "application/octet-stream";
}

String FilesController::normalizePath(const String& rawPath)
{
    if (rawPath.length() == 0)
        return String("/.core/upload.bin");

    if (rawPath.startsWith("/"))
        return rawPath;

    return String("/") + rawPath;
}