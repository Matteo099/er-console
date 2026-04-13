/**
 * Notification Store
 * Manages toast and notification messages
 */

import { defineStore } from 'pinia'
import { ref } from 'vue'

export interface Notification {
  id: string
  message: string
  type: 'success' | 'error' | 'warning' | 'info'
  duration?: number
}

export const useNotificationStore = defineStore('notification', () => {
  // State
  const notifications = ref<Notification[]>([])
  let notificationId = 0

  // Methods
  const addNotification = (
    message: string,
    type: Notification['type'] = 'info',
    duration = 3000
  ): string => {
    const id = `notification-${notificationId++}`
    const notification: Notification = {
      id,
      message,
      type,
      duration,
    }

    notifications.value.push(notification)

    if (duration > 0) {
      setTimeout(() => {
        removeNotification(id)
      }, duration)
    }

    return id
  }

  const removeNotification = (id: string) => {
    const index = notifications.value.findIndex((n) => n.id === id)
    if (index !== -1) {
      notifications.value.splice(index, 1)
    }
  }

  const success = (message: string, duration?: number) => {
    return addNotification(message, 'success', duration)
  }

  const error = (message: string, duration?: number) => {
    return addNotification(message, 'error', duration)
  }

  const warning = (message: string, duration?: number) => {
    return addNotification(message, 'warning', duration)
  }

  const info = (message: string, duration?: number) => {
    return addNotification(message, 'info', duration)
  }

  const clear = () => {
    notifications.value = []
  }

  return {
    // State
    notifications,

    // Methods
    addNotification,
    removeNotification,
    success,
    error,
    warning,
    info,
    clear,
  }
})
