/**
 * Portal Store
 * Manages device status and dashboard state
 */

import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { statusService } from '@/services'
import type { DeviceStatus } from '@/models'

export const usePortalStore = defineStore('portal', () => {
  // State
  const status = ref<DeviceStatus | null>(null)
  const loading = ref(false)
  const error = ref<string | null>(null)
  const lastRefresh = ref<Date | null>(null)

  // Computed
  const isWifiConnected = computed(() => status.value?.wifi.state === 'connected')
  const storagePercentage = computed(() => {
    if (!status.value?.storage) return 0
    const { usedBytes, totalBytes } = status.value.storage
    return totalBytes > 0 ? Math.round((usedBytes / totalBytes) * 100) : 0
  })

  // Methods
  const fetchStatus = async () => {
    loading.value = true
    error.value = null

    try {
      status.value = await statusService.getStatus()
      lastRefresh.value = new Date()
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Failed to fetch status'
      console.error('Error fetching status:', err)
    } finally {
      loading.value = false
    }
  }

  const clearError = () => {
    error.value = null
  }

  return {
    // State
    status,
    loading,
    error,
    lastRefresh,

    // Computed
    isWifiConnected,
    storagePercentage,

    // Methods
    fetchStatus,
    clearError,
  }
})
