<template>
  <v-container fluid class="py-6">
    <v-row class="mb-2" align="center" justify="space-between">
      <v-col cols="12" md="8">
        <div class="text-h4">{{ t('erConsole.config.title') }}</div>
        <div class="text-subtitle-1 text-medium-emphasis">
          {{ t('erConsole.config.subtitle') }}
        </div>
      </v-col>
      <v-col cols="12" md="4" class="d-flex justify-md-end ga-2">
        <v-btn prepend-icon="mdi-refresh" variant="tonal" :loading="configStore.loading" @click="loadConfig">
          {{ t('erConsole.common.reload') }}
        </v-btn>
        <v-btn
          prepend-icon="mdi-content-save"
          color="primary"
          :loading="configStore.saving"
          :disabled="!configStore.hasChanges"
          @click="saveConfig"
        >
          {{ t('erConsole.common.save') }}
        </v-btn>
      </v-col>
    </v-row>

    <v-row v-if="config">
      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-chip" />
            {{ t('erConsole.config.sections.device') }}
          </v-card-title>
          <v-card-text>
            <v-text-field
              v-model="config.deviceName"
              :label="t('erConsole.config.labels.deviceName')"
              name="device-name"
              autocomplete="organization"
              variant="outlined"
            />
            <v-select
              v-model="config.ui.theme"
              :items="['dark', 'light']"
              :label="t('erConsole.config.labels.uiTheme')"
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
            <v-icon icon="mdi-flask-outline" />
            {{ t('erConsole.config.sections.developer') }}
          </v-card-title>
          <v-card-text>
            <div class="text-body-2 text-medium-emphasis mb-3">
              {{ t('erConsole.config.developerDescription') }}
            </div>
            <v-switch
              v-model="developerMode"
              color="warning"
              inset
              :label="t('erConsole.config.labels.developerMode')"
              hide-details
            />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-wifi" />
            {{ t('erConsole.config.sections.wifiConnection') }}
          </v-card-title>
          <v-card-text>
            <v-alert v-if="config.wifi.ssid" type="success" variant="tonal" class="mb-3">
              {{ t('erConsole.config.currentWifiConfigured', { ssid: config.wifi.ssid }) }}
            </v-alert>
            <v-alert v-else type="info" variant="tonal" class="mb-3">
              {{ t('erConsole.config.noWifiConfigured') }}
            </v-alert>
            <div class="d-flex justify-end mt-3">
              <v-btn prepend-icon="mdi-wifi-refresh" variant="tonal" :loading="scanningWifi" @click="scanWifi">
                {{ t('erConsole.config.actions.scanWifi') }}
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
                  {{ t('erConsole.config.actions.connect') }}
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
            {{ t('erConsole.config.sections.locale') }}
          </v-card-title>
          <v-card-text>
            <v-select
              v-model="config.locale.language"
              :items="languageOptions"
              :label="t('erConsole.config.labels.language')"
              name="locale-language"
              autocomplete="language"
              variant="outlined"
            />
            <v-text-field
              v-model="config.locale.timezone"
              :label="t('erConsole.config.labels.timezone')"
              name="locale-timezone"
              readonly
              variant="outlined"
            />
            <v-text-field
              v-model="config.locale.dateFormat"
              :label="t('erConsole.config.labels.dateFormat')"
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
            {{ t('erConsole.config.sections.maintenance') }}
          </v-card-title>
          <v-card-text>
            <div class="text-body-2 text-medium-emphasis">
              {{ t('erConsole.config.maintenanceDescription') }}
            </div>
          </v-card-text>
          <v-card-actions>
            <v-btn variant="text" prepend-icon="mdi-restore" :disabled="!configStore.hasChanges" @click="resetChanges">
              {{ t('erConsole.config.actions.resetChanges') }}
            </v-btn>
            <v-spacer />
            <v-btn color="error" variant="tonal" prepend-icon="mdi-factory" @click="factoryDialog = true">
              {{ t('erConsole.config.actions.factoryReset') }}
            </v-btn>
          </v-card-actions>
        </v-card>
      </v-col>
    </v-row>

    <WifiConnectDialog
      v-model="wifiConnectDialog"
      :network="selectedWifi"
      :current-ssid="config?.wifi?.ssid"
      :loading="connectingWifi"
      @connect="connectToWifi"
    />

    <v-dialog v-model="factoryDialog" max-width="460">
      <v-card>
        <v-card-title>{{ t('erConsole.config.confirmFactoryResetTitle') }}</v-card-title>
        <v-card-text>
          {{ t('erConsole.config.confirmFactoryResetText') }}
        </v-card-text>
        <v-card-actions>
          <v-spacer />
          <v-btn variant="text" @click="factoryDialog = false">{{ t('erConsole.common.cancel') }}</v-btn>
          <v-btn color="error" :loading="configStore.saving" @click="factoryReset">{{ t('erConsole.config.actions.confirmReset') }}</v-btn>
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
import { useI18n } from 'vue-i18n'
import WifiConnectDialog from '@/components/WifiConnectDialog.vue'
import { configService, type WifiScanEntry } from '@/services'
import { useConfigStore } from '@/stores/configStore'
import { useNotificationStore } from '@/stores/notificationStore'
import { usePortalStore } from '@/stores/portalStore'

interface WifiConnectPayload {
  ssid: string
  password: string
  authMode?: string
  requiresPassword: boolean
}

const configStore = useConfigStore()
const notificationStore = useNotificationStore()
const portalStore = usePortalStore()
const { t } = useI18n()

const config = computed(() => configStore.config)
const scanningWifi = ref(false)
const wifiNetworks = ref<WifiScanEntry[]>([])
const wifiConnectDialog = ref(false)
const selectedWifi = ref<WifiScanEntry | null>(null)
const connectingWifi = ref(false)
const factoryDialog = ref(false)

const developerMode = computed({
  get: () => portalStore.developerMode,
  set: (value: boolean) => portalStore.setDeveloperMode(value),
})

const languageOptions = ['it', 'en', 'es', 'fr', 'de']

const wifiHeaders = computed(() => [
  { title: t('erConsole.config.table.ssid'), key: 'ssid' },
  { title: t('erConsole.config.table.signal'), key: 'signal' },
  { title: t('erConsole.config.table.security'), key: 'secure' },
  { title: t('erConsole.config.table.auth'), key: 'authMode' },
  { title: '', key: 'actions', sortable: false },
])

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
    notificationStore.success(t('erConsole.config.notifications.saved'))
  } catch {
    notificationStore.error(configStore.error ?? t('erConsole.config.errors.saveFailed'))
  }
}

const resetChanges = () => {
  configStore.resetChanges()
  if (configStore.config) {
    configStore.config.locale.timezone = browserTimezone
    configStore.config.locale.dateFormat = browserDateFormat
  }
  notificationStore.info(t('erConsole.config.notifications.localChangesReverted'))
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
    notificationStore.warning(t('erConsole.config.errors.wifiScanUnavailable'))
  } finally {
    scanningWifi.value = false
  }
}

const selectWifi = (network: WifiScanEntry) => {
  selectedWifi.value = network
  wifiConnectDialog.value = true
}

const connectToWifi = async (payload: WifiConnectPayload) => {
  if (!configStore.config) return

  connectingWifi.value = true
  const targetSsid = payload.ssid.trim()
  const currentSsid = (configStore.config.wifi.ssid ?? '').trim()

  try {
    if (currentSsid && currentSsid !== targetSsid) {
      configStore.config.wifi.ssid = ''
      configStore.config.wifi.password = ''
      await configStore.saveConfig()
      notificationStore.info(t('erConsole.config.notifications.disconnectedCurrentNetwork', { ssid: currentSsid }))
    }

    configStore.config.wifi.ssid = targetSsid
    configStore.config.wifi.password = payload.requiresPassword ? payload.password : ''

    await configStore.saveConfig()
    notificationStore.success(t('erConsole.config.notifications.connectedToNetwork', { ssid: targetSsid }))
    wifiConnectDialog.value = false
    selectedWifi.value = null
  } catch {
    notificationStore.error(configStore.error ?? t('erConsole.config.errors.connectFailed'))
  } finally {
    connectingWifi.value = false
  }
}

const factoryReset = async () => {
  try {
    await configStore.resetToDefaults()
    if (configStore.config) {
      configStore.config.locale.timezone = browserTimezone
      configStore.config.locale.dateFormat = browserDateFormat
    }
    notificationStore.success(t('erConsole.config.notifications.factoryResetDone'))
    factoryDialog.value = false
    await scanWifi()
  } catch {
    notificationStore.error(configStore.error ?? t('erConsole.config.errors.factoryResetFailed'))
  }
}

onMounted(loadConfig)
</script>
