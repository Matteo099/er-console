<template>
  <v-container fluid class="py-6">
    <v-row class="mb-2" align="center" justify="space-between">
      <v-col cols="12" md="8">
        <div class="text-h4">Device Dashboard</div>
        <div class="text-subtitle-1 text-medium-emphasis">
          Live status and charts overview
        </div>
      </v-col>
      <v-col cols="12" md="4" class="d-flex justify-md-end">
        <v-btn prepend-icon="mdi-refresh" :loading="refreshing" @click="refreshStatus">Refresh status</v-btn>
      </v-col>
    </v-row>

    <v-row>
      <v-col cols="12" md="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-wifi" />
            WiFi
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="status?.wifi?.state ?? 'n/a'" subtitle="State" />
              <v-list-item :title="status?.wifi?.ssid || 'n/a'" subtitle="SSID" />
              <v-list-item :title="status?.wifi?.ip || 'n/a'" subtitle="IP" />
              <v-list-item :title="`${signalStrength}%`" subtitle="Signal" />
            </v-list>
            <v-progress-linear :model-value="signalStrength" class="mt-2" color="primary" />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" md="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-sd" />
            Storage
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="formatBytes(status?.storage?.usedBytes ?? 0)" subtitle="Used" />
              <v-list-item :title="formatBytes(status?.storage?.freeBytes ?? 0)" subtitle="Free" />
              <v-list-item :title="formatBytes(status?.storage?.totalBytes ?? 0)" subtitle="Total" />
            </v-list>
            <v-progress-linear :model-value="portalStore.storagePercentage" color="primary" class="mt-2" />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" md="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-memory" />
            Memory
          </v-card-title>
          <v-card-text class="d-flex flex-column align-center ga-3">
            <v-progress-circular :model-value="memoryUsagePercentage" :size="120" :width="12" color="secondary">
              {{ memoryUsagePercentage }}%
            </v-progress-circular>
            <div class="text-body-2 text-medium-emphasis">
              Heap in use
            </div>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" md="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-chip" />
            CPU / Uptime
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="`${cpuMhz} MHz`" subtitle="CPU" />
              <v-list-item :title="formatBytes(freeHeap)" subtitle="Free Heap" />
              <v-list-item :title="formatBytes(psramFree)" subtitle="Free PSRAM" />
              <v-list-item :title="formatUptime(uptimeSeconds)" subtitle="Uptime" />
            </v-list>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row>
      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-web" />
            Portal
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="status?.portal?.url || 'n/a'" subtitle="Portal URL" />
              <v-list-item :title="status?.portal?.ssid || 'n/a'" subtitle="Portal SSID" />
              <v-list-item :title="status?.portal?.ip || 'n/a'" subtitle="Portal IP" />
            </v-list>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-power" />
            Device Controls
          </v-card-title>
          <v-card-text class="text-medium-emphasis">
            Reboot or shutdown the ESP32 from this dashboard.
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

    <v-alert
      v-if="portalStore.error"
      type="error"
      variant="tonal"
      icon="mdi-alert-circle-outline"
      class="mt-4"
    >
      {{ portalStore.error }}
    </v-alert>
  </v-container>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import { useNotificationStore } from '@/stores/notificationStore'
import { usePortalStore } from '@/stores/portalStore'
import { statusService, systemService } from '@/services'

const portalStore = usePortalStore()
const notificationStore = useNotificationStore()

const refreshing = ref(false)
const rebooting = ref(false)
const shuttingDown = ref(false)

const status = computed(() => portalStore.status)

const signalStrength = computed(() => {
  const rssi = status.value?.wifi?.signal
  if (rssi === undefined || rssi === null) return 0
  return statusService.getSignalStrength(rssi)
})

const cpuMhz = computed(() => {
  const hardware = status.value?.hardware as
    | { cpuFreq?: number; cpuFrequencyMhz?: number }
    | undefined
  return hardware?.cpuFrequencyMhz ?? hardware?.cpuFreq ?? 0
})

const freeHeap = computed(() => {
  const hardware = status.value?.hardware as
    | { freeHeap?: number; freeRam?: number }
    | undefined
  return hardware?.freeHeap ?? hardware?.freeRam ?? 0
})

const psramFree = computed(() => {
  const hardware = status.value?.hardware as { psramFree?: number } | undefined
  return hardware?.psramFree ?? 0
})

const uptimeSeconds = computed(() => {
  const hardware = status.value?.hardware as
    | { uptime?: number; uptimeMs?: number }
    | undefined
  return hardware?.uptime ?? Math.floor((hardware?.uptimeMs ?? 0) / 1000)
})

const memoryUsagePercentage = computed(() => {
  const total = freeHeap.value + psramFree.value
  if (total <= 0) return 0

  const used = total - freeHeap.value
  return Math.min(100, Math.max(0, Math.round((used / total) * 100)))
})

const formatBytes = (bytes: number) => statusService.formatBytes(bytes)
const formatUptime = (seconds: number) => statusService.formatUptime(seconds)

const refreshStatus = async () => {
  refreshing.value = true

  await portalStore.fetchStatus()

  if (portalStore.error) {
    notificationStore.error(portalStore.error)
  } else {
    notificationStore.success('Dashboard status refreshed')
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

onMounted(refreshStatus)
</script>
