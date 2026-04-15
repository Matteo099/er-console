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
        <v-list-item v-for="item in menuItems" :key="item.to" :prepend-icon="item.icon" :title="item.title"
          :to="item.to" />
      </v-list>

      <template #append>
        <v-divider />
        <v-list density="compact">
          <v-list-item :prepend-icon="portalStore.isWifiConnected ? 'mdi-wifi' : 'mdi-wifi-off'"
            :title="portalStore.isWifiConnected ? t('erConsole.app.status.wifiConnected') : t('erConsole.app.status.wifiDisconnected')" />
          <v-list-item prepend-icon="mdi-clock-outline"
            :title="portalStore.lastRefresh ? t('erConsole.app.status.lastRefresh', { value: lastRefreshText }) : t('erConsole.app.status.lastRefresh', { value: t('erConsole.common.na') })" />
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


      <v-menu open-on-hover>
        <template #activator="{ props }">
          <v-btn size="small" icon v-bind="props" :variant="isDarkTheme ? 'tonal' : 'flat'" class="mr-3"
            :color="isDarkTheme ? 'grey-darken-3' : '#F5F6FA'">
            <v-img :src="getLangImage(locale)" height="24" width="24" :alt="locale" contain />
          </v-btn>
        </template>
        <v-card>
          <v-list density="compact">
            <v-list-item
              v-for="loc in availableLocales"
              :key="`locale-${loc}`"
              :value="loc"
              :active="locale === loc"
              @click="locale = loc"
            >
              <template #prepend>
                <v-img :src="getLangImage(loc as LocaleCode)" height="24" width="24" :alt="loc" contain />
              </template>
              <v-list-item-title class="ml-2">{{ languageLabels[loc as LocaleCode] }}</v-list-item-title>
            </v-list-item>
          </v-list>
        </v-card>
      </v-menu>
      
      <v-menu open-on-hover>
        <template #activator="{ props }">
          <v-btn size="small" icon v-bind="props" :variant="isDarkTheme ? 'tonal' : 'flat'" class="mr-3"
            color="#F5F6FA">
            <v-icon size="24">{{ isDarkTheme ? 'mdi-white-balance-sunny' : 'mdi-weather-night' }}</v-icon>
          </v-btn>
        </template>
        <v-card>
          <v-list density="compact">
            <v-list-item :active="!isDarkTheme" prepend-icon="mdi-weather-sunny"
              :title="t('erConsole.app.actions.lightMode')" @click="setTheme('portalLightTheme')" />
            <v-list-item :active="isDarkTheme" prepend-icon="mdi-weather-night"
              :title="t('erConsole.app.actions.darkMode')" @click="setTheme('portalDarkTheme')" />
          </v-list>
        </v-card>
      </v-menu>

    </v-app-bar>

    <v-main>
      <RouterView />
    </v-main>

    <v-snackbar v-for="notification in notificationStore.notifications" :key="notification.id" :model-value="true"
      location="top right" :timeout="notification.duration ?? 3000"
      @update:model-value="(value) => !value && notificationStore.removeNotification(notification.id)">
      {{ notification.message }}
      <template #actions>
        <v-btn icon="mdi-close" variant="text" @click="notificationStore.removeNotification(notification.id)" />
      </template>
    </v-snackbar>
  </v-app>
</template>

<script setup lang="ts">
import { useNotificationStore } from '@/stores/notificationStore'
import { usePortalStore } from '@/stores/portalStore'
import { computed, ref, watch } from 'vue'
import { useI18n } from 'vue-i18n'
import { useTheme } from 'vuetify'

const drawer = ref(true)
const rail = ref(false)
const refreshing = ref(false)
const appVersion = import.meta.env.VITE_APP_VERSION || 'dev'
const logoImage = new URL('@/assets/images/logo.png', import.meta.url).href
const brandPositiveImage = new URL('@/assets/images/brand.png', import.meta.url).href
const brandNegativeImage = new URL('@/assets/images/brand_negative.png', import.meta.url).href
const flagEn = new URL('@/assets/images/en.svg', import.meta.url).href
const flagIt = new URL('@/assets/images/it.svg', import.meta.url).href
const flagEs = new URL('@/assets/images/es.svg', import.meta.url).href
const flagFr = new URL('@/assets/images/fr.svg', import.meta.url).href
const flagDe = new URL('@/assets/images/de.svg', import.meta.url).href

const theme = useTheme()
const { locale, t } = useI18n()

const availableLocales = ['it', 'en', 'es', 'fr', 'de'] as const
type LocaleCode = (typeof availableLocales)[number]
const languageLabels = {
  it: 'Italiano',
  en: 'English',
  es: 'Español',
  fr: 'Français',
  de: 'Deutsch',
} as const

const flagMap: Record<LocaleCode, string> = {
  it: flagIt,
  en: flagEn,
  es: flagEs,
  fr: flagFr,
  de: flagDe,
}

const getLangImage = (code: string) => {
  return flagMap[code as LocaleCode] ?? flagEn
}

watch(locale, (value) => {
  localStorage.setItem('er-console-locale', value)
})

const portalStore = usePortalStore()
const notificationStore = useNotificationStore()

const menuItems = computed(() => [
  { title: t('erConsole.app.menu.dashboard'), to: '/', icon: 'mdi-view-dashboard' },
  { title: t('erConsole.app.menu.config'), to: '/config', icon: 'mdi-cog-outline' },
  ...(portalStore.developerMode
  ? [
    { title: t('erConsole.app.menu.files'), to: '/files', icon: 'mdi-folder-open-outline' },
    { title: t('erConsole.app.menu.logsAudit'), to: '/logs-audit', icon: 'mdi-text-box-search-outline' },
    { title: t('erConsole.app.menu.esp32s3r8n16'), to: '/esp32s3r8n16', icon: 'mdi-memory' },
    ]
    : []),
  { title: t('erConsole.app.menu.about'), to: '/about', icon: 'mdi-information-outline' },
])

const isDarkTheme = computed(() => theme.global.current.value.dark)

const brandImage = computed(() =>
  isDarkTheme.value ? brandNegativeImage : brandPositiveImage
)

const brandFooterImage = computed(() =>
  isDarkTheme.value ? brandNegativeImage : brandPositiveImage
)

const lastRefreshText = computed(() => {
  return portalStore.lastRefresh?.toLocaleString(locale.value) ?? t('erConsole.common.na')
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

const setTheme = (themeName: 'portalLightTheme' | 'portalDarkTheme') => {
  theme.global.name.value = themeName
}
</script>
