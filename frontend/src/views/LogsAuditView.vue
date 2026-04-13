<template>
  <v-container fluid class="py-6">
    <v-row class="mb-2" align="center" justify="space-between">
      <v-col cols="12" md="8">
        <div class="text-h4">Logs & Audit</div>
        <div class="text-subtitle-1 text-medium-emphasis">
          Inspect system logs and local audit events
        </div>
      </v-col>
      <v-col cols="12" md="4" class="d-flex justify-md-end ga-2">
        <v-btn prepend-icon="mdi-refresh" variant="tonal" :loading="logsLoading" @click="loadLogs">
          Refresh logs
        </v-btn>
        <v-btn prepend-icon="mdi-delete-sweep" variant="tonal" color="warning" @click="clearLogs">
          Clear logs
        </v-btn>
      </v-col>
    </v-row>

    <v-row>
      <v-col cols="12" lg="7">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-text-box-search-outline" />
            System Logs
          </v-card-title>
          <v-card-text>
            <v-row>
              <v-col cols="12" md="4">
                <v-select
                  v-model="logLines"
                  :items="[50, 100, 200, 500]"
                  label="Log lines"
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
              placeholder="No log data yet"
            />
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" lg="5">
        <v-card>
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-history" />
            Audit Trail (UI)
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
            <v-btn variant="text" prepend-icon="mdi-delete" @click="clearAudit">Clear audit</v-btn>
          </v-card-actions>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script setup lang="ts">
import { onMounted, ref, watch } from 'vue'
import { systemService } from '@/services'
import { useNotificationStore } from '@/stores/notificationStore'

interface AuditEvent {
  id: string
  action: string
  time: string
}

const notificationStore = useNotificationStore()

const logsLoading = ref(false)
const logLines = ref(100)
const logText = ref('')
const auditEvents = ref<AuditEvent[]>([])

const pushAudit = (action: string) => {
  auditEvents.value.unshift({
    id: `${Date.now()}-${Math.random().toString(36).slice(2, 8)}`,
    action,
    time: new Date().toLocaleString(),
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
    pushAudit(`Loaded ${logLines.value} lines from system logs`)
  } catch (error) {
    notificationStore.error(error instanceof Error ? error.message : 'Unable to load logs')
  } finally {
    logsLoading.value = false
  }
}

const clearLogs = async () => {
  try {
    await systemService.clearLogs()
    logText.value = ''
    pushAudit('Cleared system logs')
    notificationStore.success('Logs cleared')
  } catch (error) {
    notificationStore.error(error instanceof Error ? error.message : 'Unable to clear logs')
  }
}

watch(logLines, loadLogs)

onMounted(() => {
  loadAudit()
  loadLogs()
})
</script>
