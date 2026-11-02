/**
 * Mock Server
 * In-memory API emulation used when VITE_LOCAL_SERVER is enabled.
 */

import type { DeviceConfig, DeviceStatus, FileEntry } from '@/models'
import type { RequestOptions } from './api'
import type { WifiScanEntry } from './configService'

const toBool = (value: string | boolean | undefined): boolean => {
  if (typeof value === 'boolean') return value
  if (!value) return false
  const normalized = String(value).trim().toLowerCase()
  return normalized === '1' || normalized === 'true' || normalized === 'yes' || normalized === 'on'
}

export const isLocalServerEnabled = (): boolean => toBool(import.meta.env.VITE_LOCAL_SERVER)

interface MockFileNode {
  path: string
  directory: boolean
  content?: string
  modified: number
}

interface MockState {
  config: DeviceConfig
  files: Map<string, MockFileNode>
  logs: string[]
  wifiScan: WifiScanEntry[]
}

const now = () => Date.now()

const defaultConfig = (): DeviceConfig => ({
  deviceName: 'ER Console (Mock)',
  wifi: {
    ssid: 'ER-DEV-NET',
    password: 'mockpassword',
  },
  locale: {
    language: 'en',
    timezone: 'UTC',
    dateFormat: 'YYYY-MM-DD',
  },
  ui: {
    theme: 'mint',
    compactMode: false,
  },
})

const initialFiles = (): Map<string, MockFileNode> => {
  const m = new Map<string, MockFileNode>()
  const addDir = (path: string) => m.set(path, { path, directory: true, modified: now() })
  const addFile = (path: string, content: string) =>
    m.set(path, { path, directory: false, content, modified: now() })

  addDir('/.core')
  addDir('/.core/portal')
  addDir('/.core/config')
  addDir('/.core/audio')
  addFile('/.core/config/config.json', JSON.stringify(defaultConfig(), null, 2))
  addFile('/.core/portal/index.html', '<!doctype html><html><body><h1>Mock Portal</h1></body></html>')
  addFile('/.core/audio/stage.wav', 'mock-binary')
  return m
}

const state: MockState = {
  config: defaultConfig(),
  files: initialFiles(),
  logs: [
    '[INFO] mock portal boot complete',
    '[INFO] wifi scanner initialized',
    '[INFO] storage mounted /.core',
  ],
  wifiScan: [
    { ssid: 'ER-DEV-NET', signal: -52, secure: true, authMode: 'WPA2-PSK' },
    { ssid: 'Office-5G', signal: -63, secure: true, authMode: 'WPA2/WPA3-PSK' },
    { ssid: 'Guest-WiFi', signal: -71, secure: false, authMode: 'OPEN' },
    { ssid: 'Lab-IoT', signal: -66, secure: true, authMode: 'WPA-PSK' },
  ],
}

const clone = <T>(value: T): T => JSON.parse(JSON.stringify(value))

const delay = (ms = 120) => new Promise((resolve) => setTimeout(resolve, ms))

const normalizePath = (path: string): string => {
  if (!path) return '/'
  const clean = path.startsWith('/') ? path : `/${path}`
  return clean.replace(/\/+$/, '') || '/'
}

const listFiles = (basePath: string): FileEntry[] => {
  const root = normalizePath(basePath)
  const prefix = root === '/' ? '/' : `${root}/`
  const result: FileEntry[] = []

  for (const node of state.files.values()) {
    if (node.path === root) continue
    if (!node.path.startsWith(prefix)) continue

    const remainder = node.path.slice(prefix.length)
    if (!remainder || remainder.includes('/')) continue

    result.push({
      name: remainder,
      path: node.path,
      directory: node.directory,
      size: node.directory ? 0 : node.content?.length ?? 0,
      modified: node.modified,
    })
  }

  return result.sort((a, b) => {
    if (a.directory !== b.directory) return a.directory ? -1 : 1
    return a.name.localeCompare(b.name)
  })
}

const deletePath = (inputPath: string): boolean => {
  const path = normalizePath(inputPath)
  if (!state.files.has(path)) return false

  for (const key of [...state.files.keys()]) {
    if (key === path || key.startsWith(`${path}/`)) {
      state.files.delete(key)
    }
  }

  return true
}

const movePath = (source: string, destination: string): boolean => {
  const from = normalizePath(source)
  const to = normalizePath(destination)
  const node = state.files.get(from)
  if (!node) return false
  if (state.files.has(to)) return false

  const moved: MockFileNode[] = []
  for (const value of state.files.values()) {
    if (value.path === from || value.path.startsWith(`${from}/`)) {
      const suffix = value.path.slice(from.length)
      moved.push({
        ...value,
        path: `${to}${suffix}`,
        modified: now(),
      })
    }
  }

  for (const value of moved) {
    const sourceKey = value.path.replace(to, from)
    state.files.delete(sourceKey)
  }
  for (const value of moved) {
    state.files.set(value.path, value)
  }

  return true
}

const statusFromConfig = (): DeviceStatus => {
  const connected = Boolean(state.config.wifi.ssid)
  const uptimeSeconds = 18_420
  const uptimeMs = uptimeSeconds * 1000
  const sampleAgeMs = 4_500
  const sampleTimeMs = uptimeMs - sampleAgeMs

  const voltageV = 3.86
  const currentMa = 210.4
  const powerMw = voltageV * currentMa
  const shuntVoltageMv = 21.8
  const loadVoltageV = voltageV + shuntVoltageMv / 1000

  return {
    wifi: {
      state: connected ? 'connected' : 'disconnected',
      ssid: state.config.wifi.ssid || '',
      ip: connected ? '192.168.1.120' : '0.0.0.0',
      signal: connected ? -58 : -95,
    },
    storage: {
      usedBytes: 42 * 1024 * 1024,
      freeBytes: 214 * 1024 * 1024,
      totalBytes: 256 * 1024 * 1024,
    },
    battery: {
      supported: true,
      available: true,
      overflow: false,
      sampleTimeMs,
      percent: 87,
      voltage: `${voltageV.toFixed(2)} V`,
      current: `${currentMa.toFixed(2)} mA`,
      power: `${powerMw.toFixed(2)} mW`,
      loadVoltage: `${loadVoltageV.toFixed(2)} V`,
      shuntVoltage: `${shuntVoltageMv.toFixed(2)} mV`,
      voltageV,
      currentMa,
      powerMw,
      loadVoltageV,
      shuntVoltageMv,
    },
    portal: {
      url: 'http://192.168.4.1/',
      qrPayload: 'WIFI:S:ER-Console-Setup;T:nopass;P:;H:false;;',
      ssid: 'ER-Console-Setup',
      ip: '192.168.4.1',
    },
    hardware: {
      board: 'ESP32-S3 (Mock)',
      cpuFrequencyMhz: 240,
      freeHeap: 2_200_000,
      psramFree: 7_500_000,
      uptime: uptimeSeconds,
      uptimeMs,
    },
    config: {
      deviceName: state.config.deviceName,
      language: state.config.locale.language,
      timezone: state.config.locale.timezone,
      dateFormat: state.config.locale.dateFormat,
    },
  }
}

const parseJsonBody = <T>(raw: unknown): T => {
  if (typeof raw === 'string') return JSON.parse(raw) as T
  return raw as T
}

export async function mockRequestJson<T>(path: string, options: RequestOptions = {}): Promise<T> {
  await delay()

  const method = options.method ?? 'GET'
  const url = new URL(path, 'http://local.mock')
  const pathname = url.pathname

  if (pathname === '/api/status' && method === 'GET') {
    return clone(statusFromConfig()) as T
  }

  if (pathname === '/api/config' && method === 'GET') {
    return clone(state.config) as T
  }

  if (pathname === '/api/config' && method === 'PUT') {
    const incoming = parseJsonBody<DeviceConfig>(options.body)
    state.config = clone(incoming)
    state.logs.unshift(`[INFO] config updated (wifi=${state.config.wifi.ssid || 'none'})`)
    return clone(state.config) as T
  }

  if (pathname === '/api/config' && method === 'POST') {
    state.config = defaultConfig()
    state.logs.unshift('[INFO] config reset to defaults')
    return clone(state.config) as T
  }

  if (pathname === '/api/wifi/scan' && method === 'GET') {
    return clone(state.wifiScan) as T
  }

  if (pathname === '/api/files' && method === 'GET') {
    const base = url.searchParams.get('path') ?? '/.core'
    return {
      path: normalizePath(base),
      entries: listFiles(base),
    } as T
  }

  if (pathname === '/api/files' && method === 'DELETE') {
    const target = url.searchParams.get('path')
    if (!target || !deletePath(target)) {
      throw new Error('delete_failed')
    }
    state.logs.unshift(`[INFO] deleted path ${normalizePath(target)}`)
    return undefined as T
  }

  if (pathname === '/api/files/move' && method === 'POST') {
    const payload = parseJsonBody<{ sourcePath: string; destinationPath: string }>(options.body)
    if (!payload?.sourcePath || !payload?.destinationPath || !movePath(payload.sourcePath, payload.destinationPath)) {
      throw new Error('move_failed')
    }
    state.logs.unshift(`[INFO] moved ${normalizePath(payload.sourcePath)} -> ${normalizePath(payload.destinationPath)}`)
    return undefined as T
  }

  if (pathname === '/api/system/reboot' && method === 'POST') {
    state.logs.unshift('[INFO] reboot requested (mock)')
    return { success: true, message: 'Mock reboot command accepted' } as T
  }

  if (pathname === '/api/system/shutdown' && method === 'POST') {
    state.logs.unshift('[INFO] shutdown requested (mock)')
    return { success: true, message: 'Mock shutdown command accepted' } as T
  }

  if (pathname === '/api/system/logs' && method === 'GET') {
    const lines = Number(url.searchParams.get('lines') ?? '100')
    return state.logs.slice(0, lines).join('\n') as T
  }

  if (pathname === '/api/system/logs' && method === 'DELETE') {
    state.logs = ['[INFO] logs cleared (mock)']
    return { success: true, message: 'Logs cleared' } as T
  }

  if (pathname === '/api/portal/url' && method === 'GET') {
    return { url: 'http://192.168.4.1/' } as T
  }

  throw new Error(`mock_not_implemented: ${method} ${pathname}`)
}

export async function mockUploadFile(
  path: string,
  file: File,
  onProgress?: (progress: number) => void
): Promise<string> {
  const targetDir = normalizePath(path || '/.core')
  const targetPath = normalizePath(`${targetDir}/${file.name}`)

  if (onProgress) onProgress(35)
  await delay(70)
  if (onProgress) onProgress(80)
  await delay(70)

  state.files.set(targetPath, {
    path: targetPath,
    directory: false,
    content: `mock-file:${file.name}:size=${file.size}`,
    modified: now(),
  })

  state.logs.unshift(`[INFO] uploaded ${targetPath}`)
  if (onProgress) onProgress(100)
  return JSON.stringify({ ok: true, path: targetPath })
}

export function mockDownloadFile(path: string): void {
  const normalized = normalizePath(path)
  const node = state.files.get(normalized)
  const content = node?.content ?? `mock download for ${normalized}`
  const blob = new Blob([content], { type: 'text/plain;charset=utf-8' })
  const href = URL.createObjectURL(blob)
  const link = document.createElement('a')
  link.href = href
  link.download = normalized.split('/').pop() || 'download.txt'
  document.body.appendChild(link)
  link.click()
  document.body.removeChild(link)
  URL.revokeObjectURL(href)
}
