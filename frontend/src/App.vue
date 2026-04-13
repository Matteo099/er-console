<template>
  <v-app>
    <v-navigation-drawer v-model="drawer" :rail="rail" permanent>
      <v-list nav>
        <v-list-item>
          <template #prepend>
            <v-img :src="logoImage" width="28" height="28" />
          </template>
          <v-img v-if="!rail" :src="brandImage" width="120" height="24" />
        </v-list-item>
        <v-divider class="my-2" />
        <v-list-item
          v-for="item in menuItems"
          :key="item.to"
          :prepend-icon="item.icon"
          :title="item.title"
          :to="item.to"
        />
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
          <v-list-item v-if="!rail">
            <template #prepend>
              <v-img :src="logoImage" width="20" height="20" />
            </template>
            <template #title>
              <v-img :src="brandFooterImage" width="100" height="20" />
            </template>
            <template #append>
              <v-chip size="x-small" variant="tonal">v{{ appVersion }}</v-chip>
            </template>
          </v-list-item>
        </v-list>
      </template>
    </v-navigation-drawer>

    <v-app-bar>
      <v-app-bar-nav-icon @click="toggleNav" />
      <v-app-bar-title />
      <v-spacer />
      <v-btn
        :prepend-icon="isDarkTheme ? 'mdi-weather-sunny' : 'mdi-weather-night'"
        variant="tonal"
        class="mr-2"
        @click="toggleTheme"
      >
        {{ isDarkTheme ? 'Light mode' : 'Dark mode' }}
      </v-btn>
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
import { useTheme } from 'vuetify'
import { useNotificationStore } from '@/stores/notificationStore'
import { usePortalStore } from '@/stores/portalStore'

const drawer = ref(true)
const rail = ref(false)
const refreshing = ref(false)
const appVersion = import.meta.env.VITE_APP_VERSION || 'dev'
const logoImage = new URL('@/assets/images/logo.png', import.meta.url).href
const brandPositiveImage = new URL('@/assets/images/brand.png', import.meta.url).href
const brandNegativeImage = new URL('@/assets/images/brand_negative.png', import.meta.url).href
const theme = useTheme()

const menuItems = [
  { title: 'Dashboard', to: '/', icon: 'mdi-view-dashboard' },
  { title: 'ESP32 Config', to: '/config', icon: 'mdi-cog-outline' },
  { title: 'File Browser', to: '/files', icon: 'mdi-folder-open-outline' },
  { title: 'Logs & Audit', to: '/logs-audit', icon: 'mdi-text-box-search-outline' },
  { title: 'About', to: '/about', icon: 'mdi-information-outline' },
]

const portalStore = usePortalStore()
const notificationStore = useNotificationStore()

const isDarkTheme = computed(() => theme.global.current.value.dark)

const brandImage = computed(() =>
  isDarkTheme.value ? brandNegativeImage : brandPositiveImage
)

const brandFooterImage = computed(() =>
  isDarkTheme.value ? brandNegativeImage : brandPositiveImage
)

const lastRefreshText = computed(() => {
  return portalStore.lastRefresh?.toLocaleString() ?? 'n/a'
})

const toggleNav = () => {
  if (drawer.value && !rail.value) {
    rail.value = true
    return
  }

  if (drawer.value && rail.value) {
    rail.value = false
    return
  }

  drawer.value = true
}

const toggleTheme = () => {
  theme.global.name.value = isDarkTheme.value ? 'portalLightTheme' : 'portalDarkTheme'
}

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
