/**
 * Status Service
 * API calls for device status and metrics
 */

import requestJson from './api'
import type { DeviceStatus } from '@/models'

export const statusService = {
  /**
   * Get current device status
   */
  getStatus(): Promise<DeviceStatus> {
    return requestJson('/api/status')
  },

  /**
   * Format bytes to human-readable size
   */
  formatBytes(bytes: number): string {
    if (bytes === 0) return '0 Bytes'

    const k = 1024
    const sizes = ['Bytes', 'KB', 'MB', 'GB']
    const i = Math.floor(Math.log(bytes) / Math.log(k))

    return Math.round((bytes / Math.pow(k, i)) * 100) / 100 + ' ' + sizes[i]
  },

  /**
   * Format uptime in seconds to readable format
   */
  formatUptime(seconds: number): string {
    const hours = Math.floor(seconds / 3600)
    const minutes = Math.floor((seconds % 3600) / 60)
    const secs = seconds % 60

    const parts = []
    if (hours > 0) parts.push(`${hours}h`)
    if (minutes > 0) parts.push(`${minutes}m`)
    if (secs > 0 || parts.length === 0) parts.push(`${secs}s`)

    return parts.join(' ')
  },

  /**
   * Get WiFi signal strength as percentage
   */
  getSignalStrength(rssi: number): number {
    // RSSI typically ranges from -30 (excellent) to -90 (poor)
    return Math.min(100, Math.max(0, (rssi + 90) * 2))
  },
}

export default statusService
