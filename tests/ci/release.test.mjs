import { test } from 'node:test'
import assert from 'node:assert/strict'
import { mkdtemp, mkdir, writeFile, readFile, rm } from 'node:fs/promises'
import { tmpdir } from 'node:os'
import { join, resolve, dirname, basename } from 'node:path'
import { execFileSync } from 'node:child_process'
import { createRequire } from 'node:module'
import { pathToFileURL } from 'node:url'
import { PassThrough } from 'node:stream'
import semanticRelease from 'semantic-release'

const require = createRequire(import.meta.url)
for (const [message, expected] of [
  ['fix(audio): correct playback', '1.2.4'],
  ['feat(ui): add settings', '1.3.0'],
  ['feat(api)!: change the protocol\n\nBREAKING CHANGE: new API schema', '2.0.0'],
  ['docs: explain configuration', null],
]) {
  test(`Semantic Release: ${message.split('\n')[0]}`, () => fixture(message, expected, false))
}
test('A failed prepare step cannot create a version tag or call the publisher', () => fixture('fix: test failure', null, true))

async function fixture(message, expected, fail) {
  const root = await mkdtemp(join(tmpdir(), 'escaperiom-release-'))
  const remote = join(root, 'remote.git')
  const cwd = join(root, 'work')
  const git = (where, ...args) => execFileSync('git', ['-C', where, ...args], { encoding: 'utf8', stdio: ['ignore', 'pipe', 'pipe'] }).trim()
  try {
    await mkdir(cwd)
    execFileSync('git', ['init', '--bare', remote], { stdio: 'ignore' })
    git(remote, 'symbolic-ref', 'HEAD', 'refs/heads/main')
    git(cwd, 'init', '-b', 'main')
    git(cwd, 'config', 'user.email', 'ci@example.invalid')
    git(cwd, 'config', 'user.name', 'CI test')
    git(cwd, 'commit', '--allow-empty', '-m', 'feat: baseline')
    git(cwd, 'tag', 'v1.2.3')
    git(cwd, 'remote', 'add', 'origin', remote)
    git(cwd, 'push', '-u', 'origin', 'main', '--tags')
    git(cwd, 'commit', '--allow-empty', '-m', message)
    git(cwd, 'push', 'origin', 'main')
    const head = git(cwd, 'rev-parse', 'HEAD')
    const plugin = join(root, 'publisher.cjs')
    await writeFile(plugin, `
      const fs = require('node:fs');
      const path = require('node:path');
      exports.prepare = (config, context) => {
        if (config.fail) throw new Error('Simulated build failure');
        fs.writeFileSync(path.join(context.cwd, 'bundle.json'), JSON.stringify({version: context.nextRelease.version, commit: '${head}'}));
      };
      exports.publish = (config, context) => {
        const value = JSON.parse(fs.readFileSync(path.join(context.cwd, 'bundle.json')));
        if (value.version !== context.nextRelease.version) throw new Error('Version mismatch');
        fs.writeFileSync(path.join(context.cwd, 'published'), value.version);
        return {name: 'fixture', url: 'https://example.invalid/release'};
      };
    `)
    const quiet = new PassThrough()
    quiet.resume()
    const invoke = () => semanticRelease({
      branches: ['main'], repositoryUrl: pathToFileURL(remote).href, tagFormat: 'v${version}', ci: false,
      plugins: [
        [require.resolve('@semantic-release/commit-analyzer'), { preset: 'conventionalcommits' }],
        [require.resolve('@semantic-release/release-notes-generator'), { preset: 'conventionalcommits', linkCompare: false, linkReferences: false }],
        [plugin, { fail }],
      ],
    }, { cwd, stdout: quiet, stderr: quiet, env: { ...process.env, GITHUB_ACTIONS: '', CI: '', GIT_AUTHOR_NAME: 'CI test', GIT_AUTHOR_EMAIL: 'ci@example.invalid', GIT_COMMITTER_NAME: 'CI test', GIT_COMMITTER_EMAIL: 'ci@example.invalid' } })
    if (fail) {
      await assert.rejects(invoke, /Simulated build failure/)
      assert.equal(git(cwd, 'tag', '--list', 'v1.2.4'), '')
      await assert.rejects(readFile(join(cwd, 'published')))
    } else {
      const result = await invoke()
      if (expected) {
        assert.equal(result.nextRelease.version, expected)
        assert.equal(git(cwd, 'rev-parse', `v${expected}`), head)
        assert.equal(await readFile(join(cwd, 'published'), 'utf8'), expected)
      } else {
        assert.equal(result, false)
        await assert.rejects(readFile(join(cwd, 'published')))
      }
    }
  } finally {
    assert.equal(dirname(resolve(root)), resolve(tmpdir()))
    assert.ok(basename(root).startsWith('escaperiom-release-'))
    await rm(root, { recursive: true, force: true })
  }
}
