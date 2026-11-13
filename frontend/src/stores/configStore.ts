/**
 * Config Store
 * Manages device configuration state
 */

import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { configService } from '@/services'
import type { DeviceConfig } from '@/models'

const defaultConfig = (): DeviceConfig => ({
  deviceName: 'ESP32 Device',
  wifi: {
    ssid: '',
  },
  locale: {
    language: 'it-IT',
    timezone: 'Europe/Rome',
    dateFormat: 'DD/MM/YYYY',
  },
  ui: {
    theme: 'mint',
    compactMode: false,
  },
  audio: { music: { enabled: true, volume: 50 }, effects: { enabled: true, volume: 70 } },
})

const normalizeConfig = (value?: Partial<DeviceConfig> | null): DeviceConfig => {
  const defaults = defaultConfig()
  return {
    ...defaults,
    ...value,
    wifi: {
      ...defaults.wifi,
      ...value?.wifi,
    },
    locale: {
      ...defaults.locale,
      ...value?.locale,
      language: value?.locale?.language?.startsWith('en') ? 'en-GB' : 'it-IT',
    },
    ui: {
      ...defaults.ui,
      ...value?.ui,
      // The TFT maps legacy light/dark values to the default pastel palette.
      theme: value?.ui?.theme === 'sky' || value?.ui?.theme === 'coral' ? value.ui.theme : 'mint',
    },
    audio: {
      music: {
        enabled: value?.audio?.music?.enabled ?? value?.audio?.enabled ?? defaults.audio.music.enabled,
        volume: Math.max(0, Math.min(100, value?.audio?.music?.volume ?? value?.audio?.volume ?? defaults.audio.music.volume)),
      },
      effects: {
        enabled: value?.audio?.effects?.enabled ?? value?.audio?.enabled ?? defaults.audio.effects.enabled,
        volume: Math.max(0, Math.min(100, value?.audio?.effects?.volume ?? value?.audio?.volume ?? defaults.audio.effects.volume)),
      },
    },
  }
}

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
      config.value = normalizeConfig(await configService.getConfig())
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
      const saved = normalizeConfig(await configService.saveConfig(config.value))
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
      const reset = normalizeConfig(await configService.resetConfig())
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
