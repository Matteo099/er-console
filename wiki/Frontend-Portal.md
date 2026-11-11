# Frontend Portal

## Tech Stack

- Vue 3
- Vite
- TypeScript
- Vuetify
- Vue I18n
- Pinia

## Local Development

From `frontend/`:

```bash
npm install
npm run dev
```

## Build for Device Deployment

```bash
npm run build
```

Bundle artifacts should be copied to SD card under:

- `/.core/portal`

## Run Without Firmware Backend

Create `frontend/.env.local` with:

```bash
VITE_LOCAL_SERVER=true
```

When enabled, frontend services use `mockServer` implementations for status, config, files, and system endpoints.

## Frontend Responsibilities

- Dashboard with device status snapshot
- Device configuration editing
- File browser and upload/delete/move actions
- Logs/audit views and maintenance actions
- Portal and network onboarding details

## API Client Pattern

- Shared request helper in `frontend/src/services/api.ts`
- Service-specific wrappers in `frontend/src/services/*Service.ts`
- Centralized TypeScript models in `frontend/src/models/index.ts`

## Localization

Locale strings are provided under `frontend/src/plugins/i18n/locales/` and currently include multiple languages (for example EN, IT, DE, ES, FR).
