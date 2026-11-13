import assert from 'node:assert/strict'
import { readFile } from 'node:fs/promises'
import test from 'node:test'
import ts from 'typescript'

// Exercise the portal's real mock transport and its write-only credential
// contract without a browser, network connection or additional test framework.
const source = await readFile(new URL('../src/services/mockServer.ts', import.meta.url), 'utf8')
const compiled = ts.transpileModule(source, {
  compilerOptions: { target: ts.ScriptTarget.ES2022, module: ts.ModuleKind.ESNext },
}).outputText
const { mockRequestJson: request } = await import(`data:text/javascript;base64,${Buffer.from(compiled).toString('base64')}`)

test('configuration reads redact the password and settings saves preserve it', async () => {
  const config = await request('/api/config')
  assert.equal(config.wifi.password, null)
  assert.equal(config.wifi.passwordConfigured, true)
  config.ui.theme = 'coral'
  const saved = await request('/api/config', { method: 'PUT', body: JSON.stringify(config) })
  assert.equal(saved.ui.theme, 'coral')
  assert.equal(saved.wifi.passwordConfigured, true)
  assert.equal(saved.wifi.password, null)
  // Legacy portal normalization defaults to an empty password, then overlays
  // the response. The null placeholder must survive and preserve credentials.
  const legacy = { ...saved, wifi: { ssid: '', password: '', ...saved.wifi } }
  const roundTrip = await request('/api/config', { method: 'PUT', body: JSON.stringify(legacy) })
  assert.equal(roundTrip.wifi.passwordConfigured, true)
  assert.equal(roundTrip.wifi.password, null)
})

test('explicit credential changes and open networks use write-only passwords', async () => {
  const config = await request('/api/config')
  config.wifi.password = ''
  const opened = await request('/api/config', { method: 'PUT', body: JSON.stringify(config) })
  assert.equal(opened.wifi.passwordConfigured, false)
  opened.wifi.password = 'test-only-password'
  const secured = await request('/api/config', { method: 'PUT', body: JSON.stringify(opened) })
  assert.equal(secured.wifi.passwordConfigured, true)
  assert.equal(secured.wifi.password, null)
  secured.wifi.ssid = 'Different open network'
  const different = await request('/api/config', { method: 'PUT', body: JSON.stringify(secured) })
  assert.equal(different.wifi.passwordConfigured, false)
})

test('device configuration is not represented by a mock SD file', async () => {
  const listing = await request('/api/files?path=/.core/config')
  assert.deepEqual(listing.entries, [])
})
