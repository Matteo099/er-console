#include "FilesController.h"

#include <ArduinoJson.h>
#include "drivers/bus/SPIBusLock.h"

FilesController::FilesController(PortalService& service) : service(service) {
}

void FilesController::registerRoutes(WebServer& server) {
    server.on("/api/files", HTTP_GET, [&server, this]() {
        listFiles(server);
    });
    server.on("/api/files", HTTP_DELETE, [&server, this]() {
        deleteFile(server);
    });
    server.on("/api/files/move", HTTP_POST, [&server, this]() {
        moveFile(server);
    });
    server.on("/api/files/download", HTTP_GET, [&server, this]() {
        downloadFile(server);
    });
    server.on(
        "/api/files/upload", HTTP_POST,
        [&server, this]() {
            server.send(200, "application/json", "{\"ok\":true}");
        },
        [&server, this]() {
            uploadFile(server);
        });
}

void FilesController::listFiles(WebServer& server) {
    const String path = server.hasArg("path") ? server.arg("path") : String("/.core");
    const std::vector<StorageEntry> entries = service.listFiles(path.c_str());

    JsonDocument doc;
    JsonArray items = doc["items"].to<JsonArray>();
    for (const StorageEntry& entry : entries) {
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

void FilesController::deleteFile(WebServer& server) {
    if (!server.hasArg("path")) {
        server.send(400, "application/json", "{\"error\":\"missing_path\"}");
        return;
    }

    const String path = server.arg("path");
    if (!service.deletePath(path.c_str())) {
        server.send(500, "application/json", "{\"error\":\"delete_failed\"}");
        return;
    }

    server.send(200, "application/json", "{\"ok\":true}");
}

void FilesController::moveFile(WebServer& server) {
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
        server.send(400, "application/json", "{\"error\":\"invalid_json\"}");
        return;
    }

    const String sourcePath = doc["sourcePath"].as<String>();
    const String destinationPath = doc["destinationPath"].as<String>();
    if (sourcePath.isEmpty() || destinationPath.isEmpty()) {
        server.send(400, "application/json", "{\"error\":\"missing_paths\"}");
        return;
    }

    if (!service.movePath(sourcePath.c_str(), destinationPath.c_str())) {
        server.send(500, "application/json", "{\"error\":\"move_failed\"}");
        return;
    }

    server.send(200, "application/json", "{\"success\":true,\"message\":\"Path moved\"}");
}

void FilesController::downloadFile(WebServer& server) {
    SPIBusLock::Guard guard;

    if (!server.hasArg("path")) {
        server.send(400, "application/json", "{\"error\":\"missing_path\"}");
        return;
    }

    const String path = normalizePath(server.arg("path"));
    if (path.indexOf("..") >= 0) {
        server.send(400, "application/json", "{\"error\":\"invalid_path\"}");
        return;
    }

    if (!service.storage() || !service.storage()->isReady()) {
        server.send(500, "application/json", "{\"error\":\"storage_unavailable\"}");
        return;
    }

    if (!service.storage()->exists(path.c_str())) {
        server.send(404, "application/json", "{\"error\":\"not_found\"}");
        return;
    }

    File file = service.storage()->openNative(path.c_str(), false);
    if (!file || file.isDirectory()) {
        if (file)
            file.close();
        server.send(404, "application/json", "{\"error\":\"not_found\"}");
        return;
    }

    const int slashIndex = path.lastIndexOf('/');
    const String fileName = slashIndex >= 0 ? path.substring(slashIndex + 1) : path;
    server.sendHeader("Content-Disposition", String("attachment; filename=\"") + fileName + "\"");
    server.sendHeader("Cache-Control", "no-store");
    server.streamFile(file, guessMimeType(path));
    file.close();
}

void FilesController::uploadFile(WebServer& server) {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        SPIBusLock::Guard guard;
        uploadPath =
            normalizePath(server.hasArg("path") ? server.arg("path") : String("/.core/upload.bin"));
        uploadTarget = service.openUploadTarget(uploadPath.c_str(), true);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        SPIBusLock::Guard guard;
        if (uploadTarget)
            uploadTarget.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
        SPIBusLock::Guard guard;
        if (uploadTarget) {
            uploadTarget.flush();
            uploadTarget.close();
        }
        uploadTarget = File();
        uploadPath = String();
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        SPIBusLock::Guard guard;
        if (uploadTarget)
            uploadTarget.close();
        uploadTarget = File();
        uploadPath = String();
    }
}

String FilesController::guessMimeType(const String& path) {
    if (path.endsWith(".html") || path.endsWith(".htm"))
        return "text/html";
    if (path.endsWith(".css"))
        return "text/css";
    if (path.endsWith(".js") || path.endsWith(".mjs"))
        return "application/javascript";
    if (path.endsWith(".json"))
        return "application/json";
    if (path.endsWith(".png"))
        return "image/png";
    if (path.endsWith(".jpg") || path.endsWith(".jpeg"))
        return "image/jpeg";
    if (path.endsWith(".svg"))
        return "image/svg+xml";
    return "application/octet-stream";
}

String FilesController::normalizePath(const String& rawPath) {
    if (rawPath.length() == 0)
        return String("/.core/upload.bin");

    if (rawPath.startsWith("/"))
        return rawPath;

    return String("/") + rawPath;
}
