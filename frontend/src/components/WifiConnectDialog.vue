<template>
  <v-dialog :model-value="modelValue" max-width="460" @update:model-value="emit('update:modelValue', $event)">
    <v-card>
      <v-card-title>{{ t('erConsole.config.wifiDialog.title', { ssid: network?.ssid ?? '' }) }}</v-card-title>
      <v-card-text>
        <v-list density="compact" lines="one" class="mb-3">
          <v-list-item :title="network?.ssid ?? t('erConsole.common.na')" :subtitle="t('erConsole.config.labels.ssid')" />
          <v-list-item :title="authLabel" :subtitle="t('erConsole.config.wifiDialog.authMode')" />
          <v-list-item :title="`${network?.signal ?? 0} dBm`" :subtitle="t('erConsole.config.table.signal')" />
        </v-list>

        <v-alert
          v-if="currentSsid && currentSsid !== network?.ssid"
          type="warning"
          variant="tonal"
          class="mb-3"
        >
          {{ t('erConsole.config.wifiDialog.disconnectFirstNotice', { ssid: currentSsid }) }}
        </v-alert>

        <v-alert v-if="!requiresPassword" type="info" variant="tonal" class="mb-3">
          {{ t('erConsole.config.wifiDialog.openNetworkInfo') }}
        </v-alert>

        <v-text-field
          v-else
          v-model="password"
          :label="t('erConsole.config.labels.wifiPassword')"
          type="password"
          name="wifi-dialog-password"
          autocomplete="current-password"
          variant="outlined"
          :hint="t('erConsole.config.wifiDialog.passwordHint')"
          persistent-hint
        />
      </v-card-text>
      <v-card-actions>
        <v-spacer />
        <v-btn variant="text" :disabled="loading" @click="emit('update:modelValue', false)">
          {{ t('erConsole.common.cancel') }}
        </v-btn>
        <v-btn color="primary" :loading="loading" :disabled="!canConnect" @click="submit">
          {{ t('erConsole.config.actions.connect') }}
        </v-btn>
      </v-card-actions>
    </v-card>
  </v-dialog>
</template>

<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { useI18n } from 'vue-i18n'
import type { WifiScanEntry } from '@/services'

interface WifiConnectPayload {
  ssid: string
  password: string
  authMode?: string
  requiresPassword: boolean
}

const props = defineProps<{
  modelValue: boolean
  network: WifiScanEntry | null
  currentSsid?: string
  loading?: boolean
}>()

const emit = defineEmits<{
  (e: 'update:modelValue', value: boolean): void
  (e: 'connect', payload: WifiConnectPayload): void
}>()

const { t } = useI18n()
const password = ref('')

const authMode = computed(() => (props.network?.authMode ?? '').trim())

const requiresPassword = computed(() => {
  if (!props.network) return false
  const auth = authMode.value.toLowerCase()
  if (!auth) return Boolean(props.network.secure)
  if (auth.includes('open') || auth.includes('none') || auth.includes('nopass')) return false
  return true
})

const authLabel = computed(() => {
  if (!props.network) return t('erConsole.common.na')
  const raw = authMode.value
  if (raw) return raw
  return props.network.secure ? t('erConsole.config.wifiDialog.secured') : t('erConsole.config.wifiDialog.open')
})

const canConnect = computed(() => {
  if (!props.network) return false
  if (!requiresPassword.value) return true
  return password.value.trim().length >= 8
})

const submit = () => {
  if (!props.network || !canConnect.value) return
  emit('connect', {
    ssid: props.network.ssid,
    password: password.value.trim(),
    authMode: props.network.authMode,
    requiresPassword: requiresPassword.value,
  })
}

watch(
  () => props.modelValue,
  (opened) => {
    if (!opened) {
      password.value = ''
    }
  }
)
</script>
