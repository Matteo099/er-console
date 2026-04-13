<template>
  <v-container fluid class="py-6">
    <v-card>
      <v-card-title class="d-flex align-center ga-2">
        <v-icon icon="mdi-information-outline" />
        About Device
      </v-card-title>
      <v-card-text>
        <v-list density="comfortable">
          <v-list-item title="Application" subtitle="ER Console Web UI" />
          <v-list-item :title="`Version ${appVersion}`" subtitle="Frontend build version" />
          <v-list-item title="Device" :subtitle="status?.config?.deviceName || 'Unknown'" />
          <v-list-item title="Portal URL" :subtitle="status?.portal?.url || 'n/a'" />
          <v-list-item title="Language" :subtitle="status?.config?.language || 'n/a'" />
          <v-list-item title="Timezone" :subtitle="status?.config?.timezone || 'n/a'" />
        </v-list>
      </v-card-text>
    </v-card>
  </v-container>
</template>

<script setup lang="ts">
import { computed, onMounted } from 'vue'
import { usePortalStore } from '@/stores/portalStore'

const appVersion = import.meta.env.VITE_APP_VERSION || 'dev'
const portalStore = usePortalStore()

const status = computed(() => portalStore.status)

onMounted(async () => {
  if (!portalStore.status) {
    await portalStore.fetchStatus()
  }
})
</script>
