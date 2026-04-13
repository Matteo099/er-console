<template>
  <v-container fluid class="py-6">
    <v-row class="mb-2" align="center" justify="space-between">
      <v-col cols="12" md="8">
        <div class="text-h4">ESP32 Configuration</div>
        <div class="text-subtitle-1 text-medium-emphasis">
          Configure language, theme, and WiFi settings
        </div>
      </v-col>
      <v-col cols="12" md="4" class="d-flex justify-md-end ga-2">
        <v-btn prepend-icon="mdi-refresh" variant="tonal" :loading="configStore.loading" @click="loadConfig">
          Reload
        </v-btn>
        <v-btn
          prepend-icon="mdi-content-save"
          color="primary"
          :loading="configStore.saving"
          :disabled="!configStore.hasChanges"
          @click="saveConfig"
        >
          Save
        </v-btn>
      </v-col>
    </v-row>

    <v-row v-if="config">
      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-chip" />
            Device
          </v-card-title>
          <v-card-text>
            <v-text-field
              v-model="config.deviceName"
              label="Device Name"
              name="device-name"
              autocomplete="organization"
              variant="outlined"
            />
            <v-select
              v-model="config.ui.theme"
              :items="['dark', 'light']"
              label="UI Theme"
              name="ui-theme"
              autocomplete="off"
              variant="outlined"
            />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-wifi" />
            WiFi Connection
          </v-card-title>
          <v-card-text>
            <v-form @submit.prevent>
              <v-text-field
                v-model="config.wifi.ssid"
                label="SSID"
                name="wifi-ssid"
                autocomplete="username"
                variant="outlined"
              />
              <v-text-field
                v-model="config.wifi.password"
                label="Password"
                type="password"
                name="wifi-password"
                autocomplete="current-password"
                variant="outlined"
              />
            </v-form>
            <v-alert v-if="!config.wifi.ssid" type="info" variant="tonal" class="mt-2">
              No WiFi configured. Select a network from the list below.
            </v-alert>
            <div class="d-flex justify-end mt-3">
              <v-btn prepend-icon="mdi-wifi-refresh" variant="tonal" :loading="scanningWifi" @click="scanWifi">
                Scan WiFi
              </v-btn>
            </div>
            <v-data-table
              class="mt-3"
              :headers="wifiHeaders"
              :items="wifiNetworks"
              :loading="scanningWifi"
              density="compact"
            >
              <template #item.signal="{ item }">
                <v-chip size="x-small" :color="signalColor(item.signal)">
                  {{ item.signal ?? 0 }} dBm
                </v-chip>
              </template>
              <template #item.secure="{ item }">
                <v-icon :icon="item.secure ? 'mdi-lock-outline' : 'mdi-lock-open-variant-outline'" />
              </template>
              <template #item.actions="{ item }">
                <v-btn size="small" variant="text" prepend-icon="mdi-check" @click="selectWifi(item)">
                  Select
                </v-btn>
              </template>
            </v-data-table>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-map-marker" />
            Locale
          </v-card-title>
          <v-card-text>
            <v-select
              v-model="config.locale.language"
              :items="languageOptions"
              label="Language"
              name="locale-language"
              autocomplete="language"
              variant="outlined"
            />
            <v-text-field
              v-model="config.locale.timezone"
              label="Timezone"
              name="locale-timezone"
              readonly
              variant="outlined"
            />
            <v-text-field
              v-model="config.locale.dateFormat"
              label="Date Format"
              name="locale-date-format"
              readonly
              variant="outlined"
            />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-restore-alert" />
            Maintenance
          </v-card-title>
          <v-card-text>
            <div class="text-body-2 text-medium-emphasis">
              Restore device configuration to factory defaults.
            </div>
          </v-card-text>
          <v-card-actions>
            <v-btn variant="text" prepend-icon="mdi-restore" :disabled="!configStore.hasChanges" @click="resetChanges">
              Reset changes
            </v-btn>
            <v-spacer />
            <v-btn color="error" variant="tonal" prepend-icon="mdi-factory" @click="factoryDialog = true">
              Factory reset
            </v-btn>
          </v-card-actions>
        </v-card>
      </v-col>
    </v-row>

    <v-dialog v-model="wifiPasswordDialog" max-width="440">
      <v-card>
        <v-card-title>Connect to {{ selectedWifi?.ssid }}</v-card-title>
        <v-card-text>
          <v-text-field
            v-model="selectedWifiPassword"
            label="WiFi Password"
            type="password"
            name="selected-wifi-password"
            autocomplete="current-password"
            variant="outlined"
          />
        </v-card-text>
        <v-card-actions>
          <v-spacer />
          <v-btn variant="text" @click="wifiPasswordDialog = false">Cancel</v-btn>
          <v-btn color="primary" @click="confirmWifiSelection">Apply</v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>

    <v-dialog v-model="factoryDialog" max-width="460">
      <v-card>
        <v-card-title>Confirm Factory Reset</v-card-title>
        <v-card-text>
          This will restore configuration to factory defaults. Continue?
        </v-card-text>
        <v-card-actions>
          <v-spacer />
          <v-btn variant="text" @click="factoryDialog = false">Cancel</v-btn>
          <v-btn color="error" :loading="configStore.saving" @click="factoryReset">Confirm reset</v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>

    <v-alert v-if="configStore.error" type="error" variant="tonal" class="mt-4">
      {{ configStore.error }}
    </v-alert>
  </v-container>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import { configService, type WifiScanEntry } from '@/services'
import { useConfigStore } from '@/stores/configStore'
import { useNotificationStore } from '@/stores/notificationStore'

const configStore = useConfigStore()
const notificationStore = useNotificationStore()

const config = computed(() => configStore.config)
const scanningWifi = ref(false)
const wifiNetworks = ref<WifiScanEntry[]>([])
const wifiPasswordDialog = ref(false)
const selectedWifi = ref<WifiScanEntry | null>(null)
const selectedWifiPassword = ref('')
const factoryDialog = ref(false)

const languageOptions = ['it', 'en', 'es', 'fr', 'de']

const wifiHeaders = [
  { title: 'SSID', key: 'ssid' },
  { title: 'Signal', key: 'signal' },
  { title: 'Security', key: 'secure' },
  { title: 'Auth', key: 'authMode' },
  { title: '', key: 'actions', sortable: false },
]

const browserTimezone = Intl.DateTimeFormat().resolvedOptions().timeZone || 'UTC'
const browserDateFormat = new Intl.DateTimeFormat().format(new Date())

const signalColor = (signal?: number) => {
  const value = signal ?? -100
  if (value >= -60) return 'success'
  if (value >= -75) return 'warning'
  return 'error'
}

const loadConfig = async () => {
  await configStore.fetchConfig()
  if (!configStore.config) return

  configStore.config.locale.timezone = browserTimezone
  configStore.config.locale.dateFormat = browserDateFormat

  if (!configStore.config.wifi.ssid) {
    await scanWifi()
  }
}

const saveConfig = async () => {
  try {
    await configStore.saveConfig()
    notificationStore.success('Configuration saved successfully')
  } catch {
    notificationStore.error(configStore.error ?? 'Failed to save configuration')
  }
}

const resetChanges = () => {
  configStore.resetChanges()
  if (configStore.config) {
    configStore.config.locale.timezone = browserTimezone
    configStore.config.locale.dateFormat = browserDateFormat
  }
  notificationStore.info('Local changes reverted')
}

const scanWifi = async () => {
  scanningWifi.value = true
  try {
    const payload = await configService.scanWifiNetworks()
    if (Array.isArray(payload)) {
      wifiNetworks.value = payload
    } else {
      wifiNetworks.value = payload.networks ?? payload.data ?? []
    }
  } catch {
    wifiNetworks.value = []
    notificationStore.warning('WiFi scan unavailable from backend')
  } finally {
    scanningWifi.value = false
  }
}

const selectWifi = (network: WifiScanEntry) => {
  if (!configStore.config) return
  selectedWifi.value = network

  if (network.secure) {
    selectedWifiPassword.value = ''
    wifiPasswordDialog.value = true
    return
  }

  configStore.config.wifi.ssid = network.ssid
  configStore.config.wifi.password = ''
}

const confirmWifiSelection = () => {
  if (!configStore.config || !selectedWifi.value) return

  configStore.config.wifi.ssid = selectedWifi.value.ssid
  configStore.config.wifi.password = selectedWifiPassword.value
  wifiPasswordDialog.value = false
}

const factoryReset = async () => {
  try {
    await configStore.resetToDefaults()
    if (configStore.config) {
      configStore.config.locale.timezone = browserTimezone
      configStore.config.locale.dateFormat = browserDateFormat
    }
    notificationStore.success('Configuration reset to factory defaults')
    factoryDialog.value = false
    await scanWifi()
  } catch {
    notificationStore.error(configStore.error ?? 'Factory reset failed')
  }
}

onMounted(loadConfig)
</script>
