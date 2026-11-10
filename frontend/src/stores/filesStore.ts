/**
 * Files Store
 * Manages file browser state
 */

import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { filesService } from '@/services'
import type { FileEntry, FileResponse } from '@/models'

export const useFilesStore = defineStore('files', () => {
  // State
  const currentPath = ref('/.core')
  const entries = ref<FileEntry[]>([])
  const loading = ref(false)
  const uploading = ref(false)
  const uploadProgress = ref(0)
  const error = ref<string | null>(null)

  // Computed
  const parentPath = computed(() => filesService.getParentPath(currentPath.value))
  const sortedEntries = computed(() => filesService.sortEntries(entries.value))
  const canGoUp = computed(() => currentPath.value !== '/')

  // Methods
  const loadFiles = async (path?: string) => {
    if (path) {
      currentPath.value = path
    }

    loading.value = true
    error.value = null

    try {
      const response: FileResponse = await filesService.listFiles(currentPath.value)
      entries.value = response.entries ?? response.items ?? []
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Failed to load files'
      console.error('Error loading files:', err)
    } finally {
      loading.value = false
    }
  }

  const navigateTo = async (path: string) => {
    await loadFiles(path)
  }

  const goUp = async () => {
    if (canGoUp.value) {
      await navigateTo(parentPath.value)
    }
  }

  const deleteFile = async (path: string) => {
    try {
      await filesService.deleteFile(path)
      await loadFiles()
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Failed to delete file'
      throw err
    }
  }

  const uploadFile = async (file: File) => {
    uploading.value = true
    uploadProgress.value = 0
    error.value = null

    try {
      await filesService.uploadFile(currentPath.value, file, (progress) => {
        uploadProgress.value = progress
      })
      await loadFiles()
    } catch (err) {
      error.value = err instanceof Error ? err.message : 'Upload failed'
      throw err
    } finally {
      uploading.value = false
      uploadProgress.value = 0
    }
  }

  const downloadFile = (path: string) => {
    filesService.downloadFile(path)
  }

  const clearError = () => {
    error.value = null
  }

  return {
    // State
    currentPath,
    entries,
    loading,
    uploading,
    uploadProgress,
    error,

    // Computed
    parentPath,
    sortedEntries,
    canGoUp,

    // Methods
    loadFiles,
    navigateTo,
    goUp,
    deleteFile,
    uploadFile,
    downloadFile,
    clearError,
  }
})
