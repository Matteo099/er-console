<template>
  <v-app>
    <v-navigation-drawer v-model="drawer" permanent>
      <v-list nav>
        <v-list-subheader>ER Console</v-list-subheader>
        <v-list-item prepend-icon="mdi-view-dashboard" title="Dashboard" to="/" />
      </v-list>

      <template #append>
        <v-divider />
        <v-list density="compact">
          <v-list-item
            :prepend-icon="portalStore.isWifiConnected ? 'mdi-wifi' : 'mdi-wifi-off'"
            :title="portalStore.isWifiConnected ? 'WiFi connected' : 'WiFi disconnected'"
          />
          <v-list-item
            prepend-icon="mdi-clock-outline"
            :title="portalStore.lastRefresh ? `Last refresh: ${lastRefreshText}` : 'Last refresh: n/a'"
          />
        </v-list>
      </template>
    </v-navigation-drawer>

    <v-app-bar>
      <v-app-bar-nav-icon @click="drawer = !drawer" />
      <v-app-bar-title>ER Console Dashboard</v-app-bar-title>
      <v-spacer />
      <v-btn
        prepend-icon="mdi-refresh"
        variant="tonal"
        :loading="refreshing"
        @click="refreshStatus"
      >
        Refresh status
      </v-btn>
    </v-app-bar>

    <v-main>
      <RouterView />
    </v-main>

    <v-snackbar
      v-for="notification in notificationStore.notifications"
      :key="notification.id"
      :model-value="true"
      location="top right"
      :timeout="notification.duration ?? 3000"
      @update:model-value="(value) => !value && notificationStore.removeNotification(notification.id)"
    >
      {{ notification.message }}
      <template #actions>
        <v-btn icon="mdi-close" variant="text" @click="notificationStore.removeNotification(notification.id)" />
      </template>
    </v-snackbar>
  </v-app>
</template>

<script setup lang="ts">
import { computed, ref } from 'vue'
import { useNotificationStore } from '@/stores/notificationStore'
import { usePortalStore } from '@/stores/portalStore'

const drawer = ref(true)
const refreshing = ref(false)

const portalStore = usePortalStore()
const notificationStore = useNotificationStore()

const lastRefreshText = computed(() => {
  return portalStore.lastRefresh?.toLocaleString() ?? 'n/a'
})

const refreshStatus = async () => {
  refreshing.value = true
  await portalStore.fetchStatus()

  if (portalStore.error) {
    notificationStore.error(portalStore.error)
  } else {
    notificationStore.success('Device status refreshed')
  }

  refreshing.value = false
}
</script>
