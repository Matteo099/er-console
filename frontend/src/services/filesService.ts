/**
 * Files Service
 * API calls for file and storage management
 */

import requestJson, { uploadFile } from './api'
import { isLocalServerEnabled, mockDownloadFile } from './mockServer'
import type { FileEntry, FileResponse } from '@/models'

export const filesService = {
  /**
   * List files in a directory
   */
  listFiles(path: string = '/.core'): Promise<FileResponse> {
    return requestJson(`/api/files?path=${encodeURIComponent(path)}`)
  },

  /**
   * Delete a file or directory
   */
  deleteFile(path: string): Promise<void> {
    return requestJson(`/api/files?path=${encodeURIComponent(path)}`, {
      method: 'DELETE',
    })
  },

  /**
   * Upload a file to a specific path
   */
  uploadFile(path: string, file: File, onProgress?: (progress: number) => void): Promise<string> {
    return uploadFile(path, file, onProgress)
  },

  /**
   * Move or rename a file/directory
   */
  moveFile(sourcePath: string, destinationPath: string): Promise<void> {
    return requestJson('/api/files/move', {
      method: 'POST',
      body: JSON.stringify({
        sourcePath,
        destinationPath,
      }),
    })
  },

  /**
   * Download a file (opens in browser or downloads)
   */
  downloadFile(path: string): void {
    if (isLocalServerEnabled()) {
      mockDownloadFile(path)
      return
    }

    const link = document.createElement('a')
    link.href = `/api/files/download?path=${encodeURIComponent(path)}`
    link.download = path.split('/').pop() || 'download'
    document.body.appendChild(link)
    link.click()
    document.body.removeChild(link)
  },

  /**
   * Check if path is a directory
   */
  isDirectory(entry: FileEntry): boolean {
    return entry.directory
  },

  /**
   * Get parent directory path
   */
  getParentPath(path: string): string {
    const parts = path.split('/').filter((p) => p.length > 0)
    if (parts.length <= 1) return '/'
    parts.pop()
    return '/' + parts.join('/')
  },

  /**
   * Sort entries (directories first, then alphabetically)
   */
  sortEntries(entries: FileEntry[]): FileEntry[] {
    return [...entries].sort((a, b) => {
      if (a.directory !== b.directory) {
        return a.directory ? -1 : 1
      }
      return a.name.localeCompare(b.name)
    })
  },
}

export default filesService
