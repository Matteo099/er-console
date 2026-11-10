/**
 * Portal Store
 * Manages device status and dashboard state
 */

import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { statusService } from '@/services'
import type { DeviceStatus } from '@/models'

export const usePortalStore = defineStore('portal', () => {
  const DEV_MODE_STORAGE_KEY = 'er-console-dev-mode'

  // State
  const status = ref<DeviceStatus | null>(null)
  const loading = ref(false)
  const error = ref<string | null>(null)
  const lastRefresh = ref<Date | null>(null)
  const developerMode = ref(localStorage.getItem(DEV_MODE_STORAGE_KEY) === '1')

  // Computed
  const isWifiConnected = computed(() => status.value?.wifi?.state === 'connected')
  const storagePercentage = computed(() => {
    const usedBytes = status.value?.storage?.usedBytes ?? 0
    const totalBytes = status.value?.storage?.totalBytes ?? 0
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

  const setDeveloperMode = (value: boolean) => {
    developerMode.value = value
    localStorage.setItem(DEV_MODE_STORAGE_KEY, value ? '1' : '0')
  }

  return {
    // State
    status,
    loading,
    error,
    lastRefresh,
    developerMode,

    // Computed
    isWifiConnected,
    storagePercentage,

    // Methods
    fetchStatus,
    clearError,
    setDeveloperMode,
  }
})
