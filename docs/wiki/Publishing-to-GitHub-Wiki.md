# Publishing to GitHub Wiki

GitHub Wikis are stored in a separate git repository named:

- `<repo-name>.wiki.git`

## One-Time Setup

1. Ensure Wiki is enabled in repository settings
2. Clone wiki repository locally:

```bash
git clone https://github.com/<owner>/<repo>.wiki.git
```

3. Copy the markdown files from `docs/wiki/` into the cloned wiki folder
4. Copy the `docs/wiki/images/` folder too (required by UI visual pages)

## Required Page Names

For GitHub wiki home and sidebar behavior:

- `Home.md` is the landing page
- `_Sidebar.md` controls side navigation

These are already included in this project wiki draft.

## Suggested Publish Flow

```bash
cd <repo>.wiki.git
# copy files from main repo docs/wiki/* here
git add .
git commit -m "Initial project wiki"
git push
```

## Recommended Maintenance

- Update `REST-API.md` whenever endpoint contracts change
- Update `Project-Structure.md` when folders move
- Keep `Troubleshooting.md` aligned with common field issues
- Link PRs that introduce architecture-level changes
