<template>
  <v-container fluid class="py-6">
    <v-row class="mb-2" align="center" justify="space-between">
      <v-col cols="12" md="8">
        <div class="text-h4">Device Dashboard</div>
        <div class="text-subtitle-1 text-medium-emphasis">
          Live overview from status, config, and file APIs
        </div>
      </v-col>
      <v-col cols="12" md="4" class="d-flex justify-md-end">
        <v-btn prepend-icon="mdi-refresh" :loading="refreshing" @click="refreshAll">Refresh all</v-btn>
      </v-col>
    </v-row>

    <v-row>
      <v-col cols="12" sm="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-wifi" />
            WiFi
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="status?.wifi.state ?? 'n/a'" subtitle="State" />
              <v-list-item :title="status?.wifi.ssid || 'n/a'" subtitle="SSID" />
              <v-list-item :title="status?.wifi.ip || 'n/a'" subtitle="IP" />
              <v-list-item :title="`${signalStrength}%`" subtitle="Signal" />
            </v-list>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" sm="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-sd" />
            Storage
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="formatBytes(status?.storage.usedBytes ?? 0)" subtitle="Used" />
              <v-list-item :title="formatBytes(status?.storage.freeBytes ?? 0)" subtitle="Free" />
              <v-list-item :title="formatBytes(status?.storage.totalBytes ?? 0)" subtitle="Total" />
            </v-list>
            <v-progress-linear :model-value="portalStore.storagePercentage" color="primary" class="mt-2" />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" sm="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-battery" />
            Battery
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="`${status?.battery.percent ?? 0}%`" subtitle="Percent" />
              <v-list-item :title="`${status?.battery.voltage ?? 0} V`" subtitle="Voltage" />
              <v-list-item :title="`${status?.battery.current ?? 0} mA`" subtitle="Current" />
            </v-list>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" sm="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-memory" />
            Hardware
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="status?.hardware.board || 'n/a'" subtitle="Board" />
              <v-list-item :title="`${status?.hardware.cpuFreq ?? 0} MHz`" subtitle="CPU" />
              <v-list-item :title="formatBytes(status?.hardware.freeRam ?? 0)" subtitle="Free RAM" />
              <v-list-item :title="formatUptime(status?.hardware.uptime ?? 0)" subtitle="Uptime" />
            </v-list>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row>
      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-cog-outline" />
            Device Configuration
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="config?.deviceName || 'n/a'" subtitle="Device name" />
              <v-list-item :title="config?.wifi.ssid || 'n/a'" subtitle="Configured SSID" />
              <v-list-item :title="config?.locale.language || 'n/a'" subtitle="Language" />
              <v-list-item :title="config?.ui.theme || 'n/a'" subtitle="Theme" />
              <v-list-item :title="config?.portal.enabled ? 'Enabled' : 'Disabled'" subtitle="Portal" />
            </v-list>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-web" />
            Portal
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="status?.portal.url || 'n/a'" subtitle="Portal URL" />
              <v-list-item :title="status?.portal.qrPayload || 'n/a'" subtitle="QR payload" />
            </v-list>
          </v-card-text>
          <v-card-actions>
            <v-btn prepend-icon="mdi-restart" color="warning" variant="tonal" :loading="rebooting" @click="rebootDevice">
              Reboot
            </v-btn>
            <v-btn
              prepend-icon="mdi-power"
              color="error"
              variant="tonal"
              :loading="shuttingDown"
              @click="shutdownDevice"
            >
              Shutdown
            </v-btn>
          </v-card-actions>
        </v-card>
      </v-col>
    </v-row>

    <v-row>
      <v-col cols="12">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-folder-open-outline" />
            Files in {{ filesStore.currentPath }}
          </v-card-title>
          <v-card-text>
            <v-data-table :headers="fileHeaders" :items="filesStore.sortedEntries" :loading="filesStore.loading" density="comfortable" />
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-alert
      v-if="portalStore.error || configStore.error || filesStore.error"
      type="error"
      variant="tonal"
      icon="mdi-alert-circle-outline"
      class="mt-4"
    >
      {{ portalStore.error || configStore.error || filesStore.error }}
    </v-alert>
  </v-container>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import { useConfigStore } from '@/stores/configStore'
import { useFilesStore } from '@/stores/filesStore'
import { useNotificationStore } from '@/stores/notificationStore'
import { usePortalStore } from '@/stores/portalStore'
import { statusService, systemService } from '@/services'

const portalStore = usePortalStore()
const configStore = useConfigStore()
const filesStore = useFilesStore()
const notificationStore = useNotificationStore()

const refreshing = ref(false)
const rebooting = ref(false)
const shuttingDown = ref(false)

const status = computed(() => portalStore.status)
const config = computed(() => configStore.config)

const signalStrength = computed(() => {
  const rssi = status.value?.wifi.signal
  if (rssi === undefined || rssi === null) return 0
  return statusService.getSignalStrength(rssi)
})

const formatBytes = (bytes: number) => statusService.formatBytes(bytes)
const formatUptime = (seconds: number) => statusService.formatUptime(seconds)

const fileHeaders = [
  { title: 'Type', key: 'directory' },
  { title: 'Name', key: 'name' },
  { title: 'Path', key: 'path' },
  {
    title: 'Size',
    key: 'size',
    value: (item: { size: number; directory: boolean }) =>
      item.directory ? '-' : statusService.formatBytes(item.size),
  },
]

const refreshAll = async () => {
  refreshing.value = true

  await Promise.all([
    portalStore.fetchStatus(),
    configStore.fetchConfig(),
    filesStore.loadFiles(filesStore.currentPath),
  ])

  if (portalStore.error || configStore.error || filesStore.error) {
    notificationStore.error('One or more API calls failed while refreshing dashboard data')
  } else {
    notificationStore.success('Dashboard refreshed')
  }

  refreshing.value = false
}

const rebootDevice = async () => {
  rebooting.value = true
  try {
    const response = await systemService.reboot()
    notificationStore.success(response.message || 'Reboot command sent')
  } catch (error) {
    notificationStore.error(error instanceof Error ? error.message : 'Failed to reboot device')
  } finally {
    rebooting.value = false
  }
}

const shutdownDevice = async () => {
  shuttingDown.value = true
  try {
    const response = await systemService.shutdown()
    notificationStore.warning(response.message || 'Shutdown command sent')
  } catch (error) {
    notificationStore.error(error instanceof Error ? error.message : 'Failed to shutdown device')
  } finally {
    shuttingDown.value = false
  }
}

onMounted(refreshAll)
</script>
