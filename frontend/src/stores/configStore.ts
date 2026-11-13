/**
 * Config Store
 * Manages device configuration state
 */

import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { configService } from '@/services'
import type { DeviceConfig } from '@/models'

export const useConfigStore = defineStore('config', () => {
  // State
  const config = ref<DeviceConfig | null>(null)
  const originalConfig = ref<DeviceConfig | null>(null)
  const loading = ref(false)
  const saving = ref(false)
  const error = ref<string | null>(null)

  // Computed
  const hasChanges = computed(() => JSON.stringify(config.value) !== JSON.stringify(originalConfig.value))

  // Methods
  const fetchConfig = async () => {
    loading.value = true
    error.value = null

    try {
      config.value = await configService.getConfig()
      originalConfig.value = JSON.parse(JSON.stringify(config.value))
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Failed to fetch config'
      console.error('Error fetching config:', err)
    } finally {
      loading.value = false
    }
  }

  const saveConfig = async () => {
    if (!config.value) return

    saving.value = true
    error.value = null

    try {
      const saved = await configService.saveConfig(config.value)
      originalConfig.value = JSON.parse(JSON.stringify(saved))
      config.value = saved
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Failed to save config'
      console.error('Error saving config:', err)
      throw err
    } finally {
      saving.value = false
    }
  }

  const resetToDefaults = async () => {
    saving.value = true
    error.value = null

    try {
      const reset = await configService.resetConfig()
      config.value = reset
      originalConfig.value = JSON.parse(JSON.stringify(reset))
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Failed to reset configuration'
      console.error('Error resetting config:', err)
      throw err
    } finally {
      saving.value = false
    }
  }

  const resetChanges = () => {
    if (originalConfig.value) {
      config.value = JSON.parse(JSON.stringify(originalConfig.value))
    }
  }

  const clearError = () => {
    error.value = null
  }

  return {
    // State
    config,
    originalConfig,
    loading,
    saving,
    error,

    // Computed
    hasChanges,

    // Methods
    fetchConfig,
    saveConfig,
    resetToDefaults,
    resetChanges,
    clearError,
  }
})
