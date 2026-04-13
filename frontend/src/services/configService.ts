/**
 * Config Service
 * API calls for device configuration
 */

import requestJson from './api'
import type { DeviceConfig } from '@/models'

export interface WifiScanEntry {
  ssid: string
  signal?: number
  secure?: boolean
  authMode?: string
}

export const configService = {
  /**
   * Get current device configuration
   */
  getConfig(): Promise<DeviceConfig> {
    return requestJson('/api/config')
  },

  /**
   * Save device configuration
   */
  saveConfig(config: DeviceConfig): Promise<DeviceConfig> {
    return requestJson('/api/config', {
      method: 'PUT',
      body: JSON.stringify(config),
    })
  },

  /**
   * Update specific config section
   */
  async updateConfigSection<K extends keyof DeviceConfig>(
    section: K,
    value: DeviceConfig[K]
  ): Promise<DeviceConfig> {
    const config = await this.getConfig()
    config[section] = value
    return this.saveConfig(config)
  },

  /**
   * Reset configuration to defaults
   */
  resetConfig(): Promise<DeviceConfig> {
    return requestJson('/api/config', {
      method: 'POST',
    })
  },

  /**
   * Scan nearby WiFi networks
   */
  scanWifiNetworks(): Promise<WifiScanEntry[] | { networks?: WifiScanEntry[]; data?: WifiScanEntry[] }> {
    return requestJson('/api/wifi/scan')
  },
}

export default configService
