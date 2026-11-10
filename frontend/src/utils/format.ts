/**
 * Formatting Utilities
 * Common formatting functions for display values
 */

/**
 * Format bytes to human-readable string
 * @param bytes - Number of bytes
 * @returns Formatted string (e.g., "1.5 MB")
 */
export function formatBytes(bytes: number): string {
  if (bytes === 0) return '0 Bytes'

  const k = 1024
  const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB']
  const i = Math.floor(Math.log(bytes) / Math.log(k))
  const value = bytes / Math.pow(k, i)

  return Math.round(value * 100) / 100 + ' ' + sizes[i]
}

/**
 * Format seconds to human-readable uptime string
 * @param seconds - Number of seconds
 * @returns Formatted string (e.g., "2h 15m 30s")
 */
export function formatUptime(seconds: number): string {
  const hours = Math.floor(seconds / 3600)
  const minutes = Math.floor((seconds % 3600) / 60)
  const secs = seconds % 60

  const parts: string[] = []
  if (hours > 0) parts.push(`${hours}h`)
  if (minutes > 0) parts.push(`${minutes}m`)
  if (secs > 0 || parts.length === 0) parts.push(`${secs}s`)

  return parts.join(' ')
}

/**
 * Calculate WiFi signal strength percentage from RSSI
 * @param rssi - Received Signal Strength Indicator (typically -30 to -90)
 * @returns Percentage 0-100
 */
export function getWiFiSignalStrength(rssi: number): number {
  // RSSI typically ranges from -30 (excellent) to -90 (poor)
  // Map to 0-100% scale
  return Math.min(100, Math.max(0, (rssi + 90) * 2))
}

/**
 * Get WiFi signal quality description
 * @param rssi - Received Signal Strength Indicator
 * @returns Quality description
 */
export function getWiFiSignalQuality(rssi: number): 'Excellent' | 'Good' | 'Fair' | 'Poor' {
  if (rssi > -50) return 'Excellent'
  if (rssi > -60) return 'Good'
  if (rssi > -70) return 'Fair'
  return 'Poor'
}

/**
 * Format date to localized string
 * @param date - Date object or timestamp
 * @returns Formatted date string
 */
export function formatDate(date: Date | number): string {
  const d = typeof date === 'number' ? new Date(date) : date
  return d.toLocaleString()
}

/**
 * Format time to HH:MM:SS format
 * @param date - Date object or timestamp
 * @returns Formatted time string
 */
export function formatTime(date: Date | number): string {
  const d = typeof date === 'number' ? new Date(date) : date
  return d.toLocaleTimeString()
}

/**
 * Truncate string to maximum length with ellipsis
 * @param str - String to truncate
 * @param maxLength - Maximum length
 * @returns Truncated string
 */
export function truncate(str: string, maxLength: number = 50): string {
  if (str.length <= maxLength) return str
  return str.slice(0, maxLength - 3) + '...'
}

/**
 * Capitalize first letter of string
 * @param str - String to capitalize
 * @returns Capitalized string
 */
export function capitalize(str: string): string {
  return str.charAt(0).toUpperCase() + str.slice(1)
}

/**
 * Convert percentage to color indicator
 * @param percentage - Value 0-100
 * @returns Color name (success, warning, error)
 */
export function getPercentageColor(percentage: number): 'success' | 'warning' | 'error' {
  if (percentage < 60) return 'success'
  if (percentage < 80) return 'warning'
  return 'error'
}
