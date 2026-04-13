/**
 * API Service Base
 * Core HTTP client with error handling and common utilities
 */

export interface RequestOptions {
  headers?: Record<string, string>
  body?: any
  method?: 'GET' | 'POST' | 'PUT' | 'DELETE' | 'PATCH'
}

export class ApiError extends Error {
  constructor(
    public status: number,
    public statusText: string,
    message: string
  ) {
    super(message)
    this.name = 'ApiError'
  }
}

const jsonHeaders = {
  'Content-Type': 'application/json',
}

/**
 * Make a JSON request and handle response
 */
async function requestJson<T>(path: string, options: RequestOptions = {}): Promise<T> {
  const response = await fetch(path, {
    ...options,
    headers: {
      ...jsonHeaders,
      ...options.headers,
    },
  })

  const contentType = response.headers.get('content-type') || ''
  const payload = contentType.includes('application/json')
    ? await response.json()
    : await response.text()

  if (!response.ok) {
    const message = typeof payload === 'string' ? payload : payload?.error || response.statusText
    throw new ApiError(response.status, response.statusText, message)
  }

  return payload
}

/**
 * Upload file with progress tracking
 */
export async function uploadFile(
  path: string,
  file: File,
  onProgress?: (progress: number) => void
): Promise<string> {
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest()
    const formData = new FormData()
    formData.append('file', file, file.name)

    // Upload progress
    if (onProgress) {
      xhr.upload.addEventListener('progress', (e) => {
        if (e.lengthComputable) {
          const percentComplete = (e.loaded / e.total) * 100
          onProgress(percentComplete)
        }
      })
    }

    // Request complete
    xhr.addEventListener('load', () => {
      if (xhr.status >= 200 && xhr.status < 300) {
        resolve(xhr.responseText)
      } else {
        reject(new ApiError(xhr.status, xhr.statusText, xhr.responseText || 'Upload failed'))
      }
    })

    xhr.addEventListener('error', () => {
      reject(new ApiError(0, 'Network Error', 'Upload failed due to network error'))
    })

    xhr.addEventListener('abort', () => {
      reject(new ApiError(0, 'Aborted', 'Upload was cancelled'))
    })

    xhr.open('POST', `/api/files/upload?path=${encodeURIComponent(path)}`)
    xhr.send(formData)
  })
}

export default requestJson
