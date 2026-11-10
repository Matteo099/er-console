<template>
  <v-container fluid class="py-6">
    <v-card>
      <v-card-title class="d-flex align-center ga-2">
        <v-icon icon="mdi-information-outline" />
        {{ t('erConsole.about.title') }}
      </v-card-title>
      <v-card-text>
        <v-list density="comfortable">
          <v-list-item :title="t('erConsole.about.application')" subtitle="ER Console Web UI" />
          <v-list-item :title="t('erConsole.about.version', { value: appVersion })" :subtitle="t('erConsole.about.frontendBuildVersion')" />
          <v-list-item :title="t('erConsole.about.device')" :subtitle="status?.config?.deviceName || t('erConsole.common.unknown')" />
          <v-list-item :title="t('erConsole.about.portalUrl')" :subtitle="status?.portal?.url || t('erConsole.common.na')" />
          <v-list-item :title="t('erConsole.about.language')" :subtitle="status?.config?.language || t('erConsole.common.na')" />
          <v-list-item :title="t('erConsole.about.timezone')" :subtitle="status?.config?.timezone || t('erConsole.common.na')" />
        </v-list>
      </v-card-text>
    </v-card>
  </v-container>
</template>

<script setup lang="ts">
import { computed, onMounted } from 'vue'
import { useI18n } from 'vue-i18n'
import { usePortalStore } from '@/stores/portalStore'

const appVersion = import.meta.env.VITE_APP_VERSION || 'dev'
const portalStore = usePortalStore()
const { t } = useI18n()

const status = computed(() => portalStore.status)

onMounted(async () => {
  if (!portalStore.status) {
    await portalStore.fetchStatus()
  }
})
</script>
