# CI/CD and release process

The `Build and release` workflow runs on pushes to all branches and pull
requests. It does not run for tag pushes. The same build/package script is used
for development artifacts and stable release assets.

## Branch policy

| Event | Result |
| --- | --- |
| Push to a feature/development branch | Checks and downloadable development artifact |
| PR opened, updated, reopened or title edited | PR title/commit checks and development artifact |
| Merge/push to `main` with releasable commits | Checks, development build, then stable build and GitHub Release |
| Merge/push to `main` with only non-releasable changes | Checks and development artifact; no new tag/release |
| Supported skip instruction | No push/PR workflow for that commit |

Feature builds **do not create GitHub Releases or tags**. They are short-lived
GitHub Actions artifacts, retained for 14 days. If a public beta channel is
needed later, configure an explicit Semantic Release prerelease branch; ordinary
feature branches should not fill the Releases page.

Development versions look like `1.2.4-dev.123.2+sha.abcdef123456`: the highest
reachable stable tag, incremented by one patch, plus the run number, attempt
and actual checkout commit. Without an existing stable tag the base is `0.0.0`.
This identifies a build; it does not predict whether the next release will be a
minor or major update. PR artifacts identify the tested synthetic merge commit;
their branch metadata identifies the PR's source branch.

## Jobs and publication ordering

1. **Validate conventions and release tooling** installs locked tools, validates
   new commits and the PR title, tests real Semantic Release version calculation
   in isolated local Git repositories, tests distribution packaging and validates
   firmware/game translations.
2. **Build development bundle** installs the locked frontend and pinned
   PlatformIO toolchain, type-checks/builds the Vue portal, compiles the explicit
   non-debug firmware environment and verifies the complete distribution.
3. **Publish stable release**, only on `main` push and after both preceding jobs
   succeed, runs Semantic Release. It calculates the version and notes, generates
   `CHANGELOG.md`, rebuilds/verifies the bundle in the `prepare` phase, and only
   then creates the tag and GitHub Release with all assets.

A failed stable build or packaging check fails `prepare`: no new version tag or
release is published. A later GitHub upload failure is still reported as a failed
release job; publication across Git tags and the GitHub API is not transactional.
Investigate the failed run before retrying, especially if a tag already exists.

There are no bot version-bump commits back to `main`. The tag, compiled version,
manifest, portal metadata and source commit refer to the same checkout. Firmware
and frontend consume one version supplied at build time; committed package
versions and configuration files are not rewritten with `sed`.
Stable builds reject modified tracked sources. Local development builds record
`dirty: true` and append `.dirty` to their version metadata when sources differ
from the checkout commit.

`CHANGELOG.md` is generated per release and attached to that release, together
with its GitHub release notes. It is not automatically committed to the source
branch. The Releases page is the chronological changelog. Builds with only
`docs`/`ci`/`chore` changes do not force a version increment.

## Downloadable contents

Development artifacts and stable releases contain:

- `escaperiom-esp32-s3-devkitc-1-n16r8v-<version>.bin`: application firmware.
- `...-portal.zip`: compiled Vue assets, gzip counterparts and build metadata.
- `...-sd.zip`: tracked games, scripts, images, audio, hidden `.core` directory
  and the freshly compiled portal. Untracked local files, saves, old generated
  portal files and `sd/.core/config/*` are excluded. A configuration README
  explains the omission; credentials are never copied into the package.
- `...-symbols.zip`: matching ELF and build metadata for crash decoding.
- `manifest.json`: schema version, build version/channel, source SHA, board,
  source timestamp, partition-table hash, actual OTA-slot limit, counts and
  per-file SHA-256 checksums/sizes.
- `SHA256SUMS`: checksums of the distributable files and manifest.
- `CHANGELOG.md` on stable releases.

The manifest validates firmware against the actual board's two OTA partitions,
rather than the board JSON's total flash size. An incomplete frontend, missing
game entry point, bad ESP image, oversized firmware or checksum mismatch fails
packaging. `.core` contents are added explicitly in Python, so shell globs and
artifact hidden-file defaults cannot accidentally remove them.

The `.bin` is an **application image**, not a merged USB/factory flash image.
The manifest and checksums prepare the release format for the future OTA client;
this workflow does not implement OTA installation or cryptographic signing.
Copy SD assets deliberately when updating an existing card; bundled games are
the repository examples and can replace files with the same names. Existing
configuration, credentials and NVS statistics are not part of the archive.

## Local reproduction

Use Node from `.nvmrc`, Python 3.11+ locally (3.12 in CI), Git and the project
toolchain. The CI requirements pin PlatformIO; the platform and Lua revision are
pinned in `platformio.ini`. npm installations use both committed lockfiles.

```sh
npm ci
npm ci --prefix frontend
python -m pip install -r scripts/ci/requirements.txt
npm test
python -m unittest discover -s tests/ci -p 'test_*.py' -v
python scripts/ci/build_bundle.py --channel dev
```

Output is in ignored `dist/`. This command compiles and packages but does not
upload to a device or publish a release. It uses
`esp32-s3-devkitc-1-n16r8v`, independently of the IDE's default debug environment.
A local stable bundle can be built with `--channel stable --version 1.2.3`.

## Repository configuration and recovery

Enable Actions and permit the release job's `contents: write` token to publish
tags/releases. No personal access token is needed in workflow secrets. Validation
and feature builds receive read-only permissions. PR workflows use `pull_request`,
not `pull_request_target`; untrusted PR code does not receive the release token.
The workflow does not post bot comments on PRs/issues.

In a branch ruleset, require **Validate conventions and release tooling** and
**Build development bundle** before merging to `main`. Prefer squash merges with
a conventional PR title. Configuring branch protection is an administrator step
separate from committing this workflow; do not assume it is already enabled.

Concurrency cancels obsolete feature/PR runs. Main runs are serialized without
cancelling a running release. GitHub can replace an older pending main run with
a newer one; the latest checkout's release includes the accumulated commits.

Open **Actions > Build and release**, select the exact commit/run attempt and
inspect the failed job/step. Fix source or tooling on a feature branch, wait for
green checks, then merge. Hardware serial/UI tests require an ESP32 and are not
claimed by a green hosted-runner build. Keep the release ELF to decode field
crashes against the exact binary shipped.

References: [Semantic Release exec phases](https://github.com/semantic-release/exec),
[GitHub release assets plugin](https://github.com/semantic-release/github),
[Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).
