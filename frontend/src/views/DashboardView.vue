<template>
  <v-container fluid class="py-6">
    <v-row class="mb-2" align="center" justify="space-between">
      <v-col cols="12" md="8">
        <div class="text-h4">{{ t('erConsole.dashboard.title') }}</div>
        <div class="text-subtitle-1 text-medium-emphasis">
          {{ t('erConsole.dashboard.subtitle') }}
        </div>
      </v-col>
      <v-col cols="12" md="4" class="d-flex justify-md-end">
        <v-btn prepend-icon="mdi-refresh" :loading="refreshing" @click="refreshStatus">{{ t('erConsole.app.actions.refreshStatus') }}</v-btn>
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
              <v-list-item :title="status?.wifi?.state ?? t('erConsole.common.na')" :subtitle="t('erConsole.dashboard.labels.state')" />
              <v-list-item :title="status?.wifi?.ssid || t('erConsole.common.na')" :subtitle="t('erConsole.dashboard.labels.ssid')" />
              <v-list-item :title="status?.wifi?.ip || t('erConsole.common.na')" :subtitle="t('erConsole.dashboard.labels.ip')" />
              <v-list-item :title="`${signalStrength}%`" :subtitle="t('erConsole.dashboard.labels.signal')" />
            </v-list>
            <v-progress-linear :model-value="signalStrength" class="mt-2" color="primary" />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" md="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-sd" />
            {{ t('erConsole.dashboard.sections.storage') }}
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="formatBytes(status?.storage?.usedBytes ?? 0)" :subtitle="t('erConsole.dashboard.labels.used')" />
              <v-list-item :title="formatBytes(status?.storage?.freeBytes ?? 0)" :subtitle="t('erConsole.dashboard.labels.free')" />
              <v-list-item :title="formatBytes(status?.storage?.totalBytes ?? 0)" :subtitle="t('erConsole.dashboard.labels.total')" />
            </v-list>
            <v-progress-linear :model-value="portalStore.storagePercentage" color="primary" class="mt-2" />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" md="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-memory" />
            {{ t('erConsole.dashboard.sections.memory') }}
          </v-card-title>
          <v-card-text class="d-flex flex-column align-center ga-3">
            <v-progress-circular :model-value="memoryUsagePercentage" :size="120" :width="12" color="secondary">
              {{ memoryUsagePercentage }}%
            </v-progress-circular>
            <div class="text-body-2 text-medium-emphasis">
              {{ t('erConsole.dashboard.labels.heapInUse') }}
            </div>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" md="6" lg="3">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-chip" />
            {{ t('erConsole.dashboard.sections.cpuUptime') }}
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="`${cpuMhz} MHz`" :subtitle="t('erConsole.dashboard.labels.cpu')" />
              <v-list-item :title="formatBytes(freeHeap)" :subtitle="t('erConsole.dashboard.labels.freeHeap')" />
              <v-list-item :title="formatBytes(psramFree)" :subtitle="t('erConsole.dashboard.labels.freePsram')" />
              <v-list-item :title="formatUptime(uptimeSeconds)" :subtitle="t('erConsole.dashboard.labels.uptime')" />
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
            {{ t('erConsole.dashboard.sections.portal') }}
          </v-card-title>
          <v-card-text>
            <v-list density="compact" lines="one">
              <v-list-item :title="status?.portal?.url || t('erConsole.common.na')" :subtitle="t('erConsole.dashboard.labels.portalUrl')" />
              <v-list-item :title="status?.portal?.ssid || t('erConsole.common.na')" :subtitle="t('erConsole.dashboard.labels.portalSsid')" />
              <v-list-item :title="status?.portal?.ip || t('erConsole.common.na')" :subtitle="t('erConsole.dashboard.labels.portalIp')" />
            </v-list>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="6">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-power" />
            {{ t('erConsole.dashboard.sections.deviceControls') }}
          </v-card-title>
          <v-card-text class="text-medium-emphasis">
            {{ t('erConsole.dashboard.deviceControlsDescription') }}
          </v-card-text>
          <v-card-actions>
            <v-btn prepend-icon="mdi-restart" color="warning" variant="tonal" :loading="rebooting" @click="rebootDevice">
              {{ t('erConsole.dashboard.actions.reboot') }}
            </v-btn>
            <v-btn
              prepend-icon="mdi-power"
              color="error"
              variant="tonal"
              :loading="shuttingDown"
              @click="shutdownDevice"
            >
              {{ t('erConsole.dashboard.actions.shutdown') }}
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
import { useI18n } from 'vue-i18n'
import { useNotificationStore } from '@/stores/notificationStore'
import { usePortalStore } from '@/stores/portalStore'
import { statusService, systemService } from '@/services'

const portalStore = usePortalStore()
const notificationStore = useNotificationStore()
const { t } = useI18n()

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
    notificationStore.success(t('erConsole.dashboard.notifications.statusRefreshed'))
  }

  refreshing.value = false
}

const rebootDevice = async () => {
  rebooting.value = true
  try {
    const response = await systemService.reboot()
    notificationStore.success(response.message || t('erConsole.dashboard.notifications.rebootSent'))
  } catch (error) {
    notificationStore.error(error instanceof Error ? error.message : t('erConsole.dashboard.errors.rebootFailed'))
  } finally {
    rebooting.value = false
  }
}

const shutdownDevice = async () => {
  shuttingDown.value = true
  try {
    const response = await systemService.shutdown()
    notificationStore.warning(response.message || t('erConsole.dashboard.notifications.shutdownSent'))
  } catch (error) {
    notificationStore.error(error instanceof Error ? error.message : t('erConsole.dashboard.errors.shutdownFailed'))
  } finally {
    shuttingDown.value = false
  }
}

onMounted(refreshStatus)
</script>
