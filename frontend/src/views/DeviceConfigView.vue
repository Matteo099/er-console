<template>
  <v-container fluid class="py-6">
    <v-row class="mb-2" align="center" justify="space-between">
      <v-col cols="12" md="8">
        <div class="text-h4">ESP32 Configuration</div>
        <div class="text-subtitle-1 text-medium-emphasis">
          Configure language, theme, WiFi, portal, and device preferences
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
            <v-text-field v-model="config.deviceName" label="Device Name" variant="outlined" />
            <v-select
              v-model="config.ui.theme"
              :items="['dark', 'light']"
              label="UI Theme"
              variant="outlined"
            />
            <v-switch v-model="config.portal.enabled" label="Captive Portal Enabled" color="primary" />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-wifi" />
            WiFi
          </v-card-title>
          <v-card-text>
            <v-switch v-model="config.wifi.enabled" label="WiFi Enabled" color="primary" />
            <v-switch v-model="config.wifi.autoConnect" label="Auto Connect" color="primary" />
            <v-text-field v-model="config.wifi.ssid" label="SSID" variant="outlined" />
            <v-text-field
              v-model="config.wifi.password"
              label="Password"
              type="password"
              variant="outlined"
            />
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
            <v-text-field v-model="config.locale.language" label="Language" variant="outlined" />
            <v-text-field v-model="config.locale.timezone" label="Timezone" variant="outlined" />
            <v-text-field v-model="config.locale.dateFormat" label="Date Format" variant="outlined" />
            <v-row>
              <v-col cols="12" md="6">
                <v-text-field
                  v-model.number="config.locale.latitude"
                  type="number"
                  label="Latitude"
                  variant="outlined"
                />
              </v-col>
              <v-col cols="12" md="6">
                <v-text-field
                  v-model.number="config.locale.longitude"
                  type="number"
                  label="Longitude"
                  variant="outlined"
                />
              </v-col>
            </v-row>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-access-point" />
            Portal Access Point
          </v-card-title>
          <v-card-text>
            <v-text-field v-model="config.portal.apSsid" label="AP SSID" variant="outlined" />
            <v-text-field
              v-model="config.portal.apPassword"
              label="AP Password"
              type="password"
              variant="outlined"
            />
          </v-card-text>
          <v-card-actions>
            <v-btn variant="text" prepend-icon="mdi-restore" :disabled="!configStore.hasChanges" @click="resetChanges">
              Reset changes
            </v-btn>
          </v-card-actions>
        </v-card>
      </v-col>
    </v-row>

    <v-alert v-if="configStore.error" type="error" variant="tonal" class="mt-4">
      {{ configStore.error }}
    </v-alert>
  </v-container>
</template>

<script setup lang="ts">
import { computed, onMounted } from 'vue'
import { useConfigStore } from '@/stores/configStore'
import { useNotificationStore } from '@/stores/notificationStore'

const configStore = useConfigStore()
const notificationStore = useNotificationStore()

const config = computed(() => configStore.config)

const loadConfig = async () => {
  await configStore.fetchConfig()
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
  notificationStore.info('Local changes reverted')
}

onMounted(loadConfig)
</script>
