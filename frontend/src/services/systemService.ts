/**
 * System Service
 * API calls for system-level operations
 */

import requestJson from './api'

export interface SystemResponse {
  success: boolean
  message: string
}

export const systemService = {
  /**
   * Reboot the device
   */
  reboot(): Promise<SystemResponse> {
    return requestJson('/api/system/reboot', {
      method: 'POST',
    })
  },

  /**
   * Shutdown the device
   */
  shutdown(): Promise<SystemResponse> {
    return requestJson('/api/system/shutdown', {
      method: 'POST',
    })
  },

  /**
   * Get system logs
   */
  getLogs(lines: number = 100): Promise<string> {
    return requestJson(`/api/system/logs?lines=${lines}`)
  },

  /**
   * Clear logs
   */
  clearLogs(): Promise<SystemResponse> {
    return requestJson('/api/system/logs', {
      method: 'DELETE',
    })
  },
}

export default systemService
