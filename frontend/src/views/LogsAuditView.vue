<template>
  <v-container fluid class="py-6">
    <v-row class="mb-2" align="center" justify="space-between">
      <v-col cols="12" md="8">
        <div class="text-h4">{{ t('erConsole.logs.title') }}</div>
        <div class="text-subtitle-1 text-medium-emphasis">
          {{ t('erConsole.logs.subtitle') }}
        </div>
      </v-col>
      <v-col cols="12" md="4" class="d-flex justify-md-end ga-2">
        <v-btn prepend-icon="mdi-refresh" variant="tonal" :loading="logsLoading" @click="loadLogs">
          {{ t('erConsole.logs.actions.refreshLogs') }}
        </v-btn>
        <v-btn prepend-icon="mdi-delete-sweep" variant="tonal" color="warning" @click="clearLogs">
          {{ t('erConsole.logs.actions.clearLogs') }}
        </v-btn>
      </v-col>
    </v-row>

    <v-row>
      <v-col cols="12" lg="7">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-text-box-search-outline" />
            {{ t('erConsole.logs.sections.systemLogs') }}
          </v-card-title>
          <v-card-text>
            <v-row>
              <v-col cols="12" md="4">
                <v-select
                  v-model="logLines"
                  :items="[50, 100, 200, 500]"
                  :label="t('erConsole.logs.labels.logLines')"
                  variant="outlined"
                />
              </v-col>
            </v-row>
            <v-textarea
              v-model="logText"
              rows="16"
              variant="outlined"
              readonly
              auto-grow
              :placeholder="t('erConsole.logs.noLogDataYet')"
            />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="5">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-history" />
            {{ t('erConsole.logs.sections.auditTrail') }}
          </v-card-title>
          <v-card-text>
            <v-list lines="two">
              <v-list-item v-for="event in auditEvents" :key="event.id">
                <v-list-item-title>{{ event.action }}</v-list-item-title>
                <v-list-item-subtitle>{{ event.time }}</v-list-item-subtitle>
              </v-list-item>
            </v-list>
          </v-card-text>
          <v-card-actions>
            <v-btn variant="text" prepend-icon="mdi-delete" @click="clearAudit">{{ t('erConsole.logs.actions.clearAudit') }}</v-btn>
          </v-card-actions>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script setup lang="ts">
import { onMounted, ref, watch } from 'vue'
import { useI18n } from 'vue-i18n'
import { systemService } from '@/services'
import { useNotificationStore } from '@/stores/notificationStore'

interface AuditEvent {
  id: string
  action: string
  time: string
}

const notificationStore = useNotificationStore()
const { locale, t } = useI18n()

const logsLoading = ref(false)
const logLines = ref(100)
const logText = ref('')
const auditEvents = ref<AuditEvent[]>([])

const pushAudit = (action: string) => {
  auditEvents.value.unshift({
    id: `${Date.now()}-${Math.random().toString(36).slice(2, 8)}`,
    action,
    time: new Date().toLocaleString(locale.value),
  })
  auditEvents.value = auditEvents.value.slice(0, 100)
  localStorage.setItem('er-console-audit', JSON.stringify(auditEvents.value))
}

const loadAudit = () => {
  const data = localStorage.getItem('er-console-audit')
  if (!data) return

  try {
    auditEvents.value = JSON.parse(data)
  } catch {
    auditEvents.value = []
  }
}

const clearAudit = () => {
  auditEvents.value = []
  localStorage.removeItem('er-console-audit')
}

const loadLogs = async () => {
  logsLoading.value = true
  try {
    logText.value = await systemService.getLogs(logLines.value)
    pushAudit(t('erConsole.logs.audit.loadedLines', { count: logLines.value }))
  } catch (error) {
    notificationStore.error(error instanceof Error ? error.message : t('erConsole.logs.errors.unableToLoadLogs'))
  } finally {
    logsLoading.value = false
  }
}

const clearLogs = async () => {
  try {
    await systemService.clearLogs()
    logText.value = ''
    pushAudit(t('erConsole.logs.audit.clearedSystemLogs'))
    notificationStore.success(t('erConsole.logs.notifications.logsCleared'))
  } catch (error) {
    notificationStore.error(error instanceof Error ? error.message : t('erConsole.logs.errors.unableToClearLogs'))
  }
}

watch(logLines, loadLogs)

onMounted(() => {
  loadAudit()
  loadLogs()
})
</script>
