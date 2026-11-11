import { execFileSync, spawnSync } from 'node:child_process'
import { readFileSync } from 'node:fs'
import { fileURLToPath } from 'node:url'
import { resolve } from 'node:path'

const root = fileURLToPath(new URL('../..', import.meta.url))
process.chdir(root)

// Adopt the convention from this change onwards; do not rewrite legacy history.
const baseline = '8c3b294af7f3704f3c597e135fa0701d0bde2be4'
const git = (...args) => execFileSync('git', args, { encoding: 'utf8' }).trim()
const cli = fileURLToPath(new URL('./node_modules/@commitlint/cli/cli.js', import.meta.url))
const event = process.env.GITHUB_EVENT_PATH
  ? JSON.parse(readFileSync(process.env.GITHUB_EVENT_PATH, 'utf8')) : {}
let failed = false
function lint(message, label) {
  const result = spawnSync(process.execPath, [cli, '--verbose', '--config', resolve(root, 'scripts/ci/commitlint.config.cjs')], { input: message, encoding: 'utf8' })
  console.log(label, result.stdout, result.stderr)
  failed ||= result.status !== 0
}
if (event.pull_request) lint(event.pull_request.title, 'Pull request title')
const before = event.pull_request?.base.sha || event.before
const base = before && !/^0+$/.test(before) ? before : git('merge-base', 'origin/main', 'HEAD')
const revisions = git('rev-list', '--no-merges', `${base}..HEAD`, '--not', baseline)
for (const sha of revisions.split('\n').filter(Boolean)) lint(git('show', '-s', '--format=%B', sha), sha)
if (!revisions) console.log('No new non-merge commits to lint; legacy history is grandfathered.')
if (failed) process.exitCode = 1
