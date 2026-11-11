import { createRequire } from 'node:module'
import { fileURLToPath } from 'node:url'
import semanticRelease from 'semantic-release'

const require = createRequire(import.meta.url)
const cwd = fileURLToPath(new URL('../..', import.meta.url))
const result = await semanticRelease({
  branches: ['main'],
  tagFormat: 'v${version}',
  plugins: [
    [require.resolve("@semantic-release/commit-analyzer"), {"preset": "conventionalcommits"}],
    [require.resolve("@semantic-release/release-notes-generator"), {"preset": "conventionalcommits"}],
    [require.resolve("@semantic-release/changelog"), {"changelogFile": "dist/CHANGELOG.md"}],
    [require.resolve("@semantic-release/exec"), {"prepareCmd": "python scripts/ci/build_bundle.py --channel stable --version ${nextRelease.version}"}],
    [require.resolve("@semantic-release/github"), {"assets": [{"path": "dist/*.bin", "label": "ESP32-S3 application firmware"}, {"path": "dist/*.zip"}, {"path": "dist/manifest.json"}, {"path": "dist/SHA256SUMS"}, {"path": "dist/CHANGELOG.md"}], "successComment": false, "failComment": false, "failTitle": false, "releasedLabels": false}],
  ],
}, { cwd })
if (result === false) console.log('No release is required.')
