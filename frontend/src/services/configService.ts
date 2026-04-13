/**
 * Config Service
 * API calls for device configuration
 */

import requestJson from './api'
import type { DeviceConfig } from '@/models'

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
}

export default configService
