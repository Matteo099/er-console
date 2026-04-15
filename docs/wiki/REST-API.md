# REST API

This page documents the portal REST API exposed by firmware controllers.

## Base

- Host: portal URL from `/api/portal/url` (typically http://192.168.4.1/)
- Content type: JSON for most endpoints
- Logs endpoint returns plain text

## Status And System

### GET /api/status

Returns composed runtime status:

- `wifi` (state, ssid, ip, signal, message)
- `portal` (active, url, ssid, ip, qrPayload)
- `hardware` (freeHeap, psramFree, cpuFrequencyMhz, uptimeMs)
- `storage` (totalBytes, usedBytes, freeBytes)
- `battery` (supported, percent, voltage, current)
- `config` summary (deviceName, language, timezone, dateFormat)

### GET /api/wifi/scan

Returns nearby SSIDs:

```json
{
  "networks": [
    {
      "ssid": "MyNetwork",
      "signal": -58,
      "secure": true,
      "authMode": "wpa2"
    }
  ]
}
```

### POST /api/system/reboot

- Response: `{"success":true,"message":"Reboot scheduled"}`
- Device restarts shortly after response

### POST /api/system/shutdown

- Response: `{"success":true,"message":"Shutdown scheduled"}`
- Device enters deep sleep shortly after response

### GET /api/system/logs?lines=100

- Response body is plain text log output

### DELETE /api/system/logs

- Response: `{"success":true,"message":"Logs cleared"}`

## Config

### GET /api/config

Returns full device config object.

### PUT /api/config

Updates config from JSON payload. Expected sections:

- `deviceName`
- `wifi` (`ssid`, `password`)
- `locale` (`language`, `timezone`, `dateFormat`)
- `ui` (`theme`, `brightness`)
- `audio` (`volume`, `enabled`)

On success returns persisted config payload.

### POST /api/config

Resets config to firmware defaults and returns resulting config payload.

## Files

### GET /api/files?path=/.core

Returns directory listing:

```json
{
  "items": [
    {
      "path": "/.core/config/config.json",
      "name": "config.json",
      "directory": false,
      "size": 512
    }
  ],
  "path": "/.core"
}
```

### DELETE /api/files?path=/some/path

Deletes file or directory path recursively (backend policy dependent).

### POST /api/files/move

Payload:

```json
{
  "sourcePath": "/old/name.txt",
  "destinationPath": "/new/name.txt"
}
```

Success response:

```json
{
  "success": true,
  "message": "Path moved"
}
```

### POST /api/files/upload?path=/.core/new.bin

Multipart upload with field name `file`.

## Portal Info

### GET /api/portal/url

Returns current portal URL:

```json
{
  "url": "http://192.168.4.1/"
}
```

## Error Patterns

Common JSON error payloads include:

- `{"error":"invalid_json"}`
- `{"error":"missing_path"}`
- `{"error":"missing_paths"}`
- `{"error":"save_failed"}`
- `{"error":"delete_failed"}`
- `{"error":"move_failed"}`
