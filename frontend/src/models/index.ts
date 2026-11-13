/**
 * Data Models and Interfaces
 * Central location for all TypeScript types and interfaces
 */

// ============================================================================
// Configuration Models
// ============================================================================

export interface WiFiConfig {
  ssid: string
  // Write-only: omit to preserve the secret; send an empty value for an open network.
  password?: string
  passwordConfigured?: boolean
}

export interface LocaleConfig {
  language: string
  timezone: string
  dateFormat: string
}

export interface UIConfig {
  theme: 'mint' | 'sky' | 'coral'
}

export interface DeviceConfig {
  deviceName: string
  wifi: WiFiConfig
  locale: LocaleConfig
  ui: UIConfig
  audio: {
    music: { enabled: boolean; volume: number }
    effects: { enabled: boolean; volume: number }
  }
}

// ============================================================================
// Status Models
// ============================================================================

export interface WiFiStatus {
  state: string
  ssid: string
  ip: string
  signal: number
}

export interface StorageInfo {
  usedBytes: number
  freeBytes: number
  totalBytes: number
}

export interface BatteryStatus {
  supported?: boolean
  available?: boolean
  overflow?: boolean
  sampleTimeMs?: number
  percent: number | string
  voltage: number | string
  current: number | string
  power?: string
  loadVoltage?: string
  shuntVoltage?: string
  voltageV?: number
  currentMa?: number
  powerMw?: number
  loadVoltageV?: number
  shuntVoltageMv?: number
}

export interface PortalStatus {
  url: string
  qrPayload: string
  ssid?: string
  ip?: string
}

export interface StatusConfigSummary {
  deviceName?: string
  language?: string
  timezone?: string
  dateFormat?: string
}

export interface HardwareInfo {
  board?: string
  cpuFreq?: number
  freeRam?: number
  totalRam?: number
  uptime?: number
  cpuFrequencyMhz?: number
  freeHeap?: number
  psramFree?: number
  uptimeMs?: number
}

export interface DeviceStatus {
  wifi: WiFiStatus
  storage: StorageInfo
  battery: BatteryStatus
  portal: PortalStatus
  hardware: HardwareInfo
  config?: StatusConfigSummary
}

// ============================================================================
// File Management Models
// ============================================================================

export interface FileEntry {
  name: string
  path: string
  directory: boolean
  size: number
  modified?: number
}

export interface FileResponse {
  entries?: FileEntry[]
  items?: FileEntry[]
  path: string
}

// ============================================================================
// API Response Models
// ============================================================================

export interface ApiResponse<T> {
  success: boolean
  data?: T
  error?: string
}

export interface UploadProgress {
  loaded: number
  total: number
  percent: number
}
