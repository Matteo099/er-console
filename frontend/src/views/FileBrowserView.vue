<template>
  <v-container fluid class="py-6">
    <v-row class="mb-2" align="center" justify="space-between">
      <v-col cols="12" md="8">
        <div class="text-h4">File Browser</div>
        <div class="text-subtitle-1 text-medium-emphasis">
          Navigate, upload, delete, and move files in the ESP32 filesystem
        </div>
      </v-col>
      <v-col cols="12" md="4" class="d-flex justify-md-end ga-2">
        <v-btn prepend-icon="mdi-arrow-up" variant="tonal" :disabled="!filesStore.canGoUp" @click="goUp">
          Up
        </v-btn>
        <v-btn prepend-icon="mdi-refresh" variant="tonal" :loading="filesStore.loading" @click="reload">
          Reload
        </v-btn>
      </v-col>
    </v-row>

    <v-card class="mb-4">
      <v-card-text>
        <v-row align="center" class="ga-2">
          <v-col cols="12" md="6">
            <v-text-field v-model="filesStore.currentPath" label="Current path" variant="outlined" readonly />
          </v-col>
          <v-col cols="12" md="6">
            <v-file-input
              v-model="uploadSelection"
              label="Upload file"
              variant="outlined"
              prepend-icon="mdi-upload"
              :disabled="filesStore.uploading"
            />
          </v-col>
        </v-row>
      </v-card-text>
      <v-card-actions>
        <v-btn
          prepend-icon="mdi-upload"
          color="primary"
          :loading="filesStore.uploading"
          :disabled="!selectedUploadFile"
          @click="upload"
        >
          Upload
        </v-btn>
      </v-card-actions>
    </v-card>

    <v-card>
      <v-card-title class="d-flex align-center ga-2">
        <v-icon icon="mdi-folder-open-outline" />
        Files
      </v-card-title>
      <v-card-text>
        <v-data-table :headers="headers" :items="filesStore.sortedEntries" :loading="filesStore.loading">
          <template #item.directory="{ item }">
            <v-icon :icon="item.directory ? 'mdi-folder' : 'mdi-file-outline'" />
          </template>

          <template #item.name="{ item }">
            <v-btn variant="text" @click="openEntry(item)">{{ item.name }}</v-btn>
          </template>

          <template #item.size="{ item }">
            {{ item.directory ? '-' : statusService.formatBytes(item.size) }}
          </template>

          <template #item.actions="{ item }">
            <v-btn
              icon="mdi-file-move-outline"
              size="small"
              variant="text"
              @click="openMoveDialog(item.path)"
            />
            <v-btn
              v-if="!item.directory"
              icon="mdi-download"
              size="small"
              variant="text"
              @click="filesStore.downloadFile(item.path)"
            />
            <v-btn
              icon="mdi-delete-outline"
              size="small"
              variant="text"
              color="error"
              @click="remove(item.path)"
            />
          </template>
        </v-data-table>
      </v-card-text>
    </v-card>

    <v-dialog v-model="moveDialog" max-width="560">
      <v-card>
        <v-card-title>Move / Rename Entry</v-card-title>
        <v-card-text>
          <v-text-field v-model="moveSource" label="Source path" variant="outlined" readonly />
          <v-text-field v-model="moveDestination" label="Destination path" variant="outlined" />
          <div class="text-body-2 text-medium-emphasis">
            This action requires backend support for /api/files/move.
          </div>
        </v-card-text>
        <v-card-actions>
          <v-spacer />
          <v-btn variant="text" @click="moveDialog = false">Cancel</v-btn>
          <v-btn color="primary" :loading="moving" @click="moveEntry">Move</v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>

    <v-alert v-if="filesStore.error" type="error" variant="tonal" class="mt-4">
      {{ filesStore.error }}
    </v-alert>
  </v-container>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import { filesService, statusService } from '@/services'
import { useFilesStore } from '@/stores/filesStore'
import { useNotificationStore } from '@/stores/notificationStore'
import type { FileEntry } from '@/models'

const filesStore = useFilesStore()
const notificationStore = useNotificationStore()

const uploadSelection = ref<File[] | File | null>(null)
const moveDialog = ref(false)
const moveSource = ref('')
const moveDestination = ref('')
const moving = ref(false)

const selectedUploadFile = computed(() => {
  if (!uploadSelection.value) return null
  return Array.isArray(uploadSelection.value) ? uploadSelection.value[0] : uploadSelection.value
})

const headers = [
  { title: '', key: 'directory', sortable: false },
  { title: 'Name', key: 'name' },
  { title: 'Path', key: 'path' },
  { title: 'Size', key: 'size' },
  { title: 'Actions', key: 'actions', sortable: false },
]

const reload = async () => {
  await filesStore.loadFiles(filesStore.currentPath)
}

const goUp = async () => {
  await filesStore.goUp()
}

const openEntry = async (entry: FileEntry) => {
  if (entry.directory) {
    await filesStore.navigateTo(entry.path)
  }
}

const upload = async () => {
  const file = selectedUploadFile.value
  if (!file) return

  try {
    await filesStore.uploadFile(file)
    uploadSelection.value = null
    notificationStore.success('File uploaded')
  } catch {
    notificationStore.error(filesStore.error ?? 'Upload failed')
  }
}

const remove = async (path: string) => {
  try {
    await filesStore.deleteFile(path)
    notificationStore.success('Entry deleted')
  } catch {
    notificationStore.error(filesStore.error ?? 'Delete failed')
  }
}

const openMoveDialog = (path: string) => {
  moveSource.value = path
  moveDestination.value = path
  moveDialog.value = true
}

const moveEntry = async () => {
  moving.value = true
  try {
    await filesService.moveFile(moveSource.value, moveDestination.value)
    notificationStore.success('Entry moved')
    moveDialog.value = false
    await reload()
  } catch (error) {
    notificationStore.error(error instanceof Error ? error.message : 'Move is not available on backend')
  } finally {
    moving.value = false
  }
}

onMounted(reload)
</script>
