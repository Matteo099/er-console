# Contributing

## Workflow

1. Create a branch from main
2. Keep firmware and frontend changes isolated when possible
3. Build and test locally before opening PR
4. Include screenshots for UI changes and logs for firmware fixes

## Local Checks

Firmware:

```bash
pio run
```

Frontend:

```bash
cd frontend
npm install
npm run lint
npm run build
```

## Coding Notes

- Preserve separation between game logic (Lua) and firmware internals (C++)
- Update firmware API contracts and frontend service calls together
- Prefer small, reviewable PRs by subsystem (portal, storage, network, scenes)

## Pull Request Checklist

- [ ] Build passes for firmware environment
- [ ] Frontend lint/build passes
- [ ] New or changed API endpoints documented in wiki
- [ ] Config/storage path changes validated on SD card layout
- [ ] If i18n text changed, all active locales updated
