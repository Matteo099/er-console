import { createRouter, createWebHistory } from 'vue-router'
import DashboardView from '@/views/DashboardView.vue'
import DeviceConfigView from '@/views/DeviceConfigView.vue'
import FileBrowserView from '@/views/FileBrowserView.vue'
import LogsAuditView from '@/views/LogsAuditView.vue'
import AboutView from '@/views/AboutView.vue'
import Esp32S3R8N16View from '@/views/Esp32S3R8N16View.vue'
import { usePortalStore } from '@/stores/portalStore'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'dashboard',
      component: DashboardView,
    },
    {
      path: '/config',
      name: 'config',
      component: DeviceConfigView,
    },
    {
      path: '/files',
      name: 'files',
      component: FileBrowserView,
      meta: { requiresDevMode: true },
    },
    {
      path: '/logs-audit',
      name: 'logs-audit',
      component: LogsAuditView,
      meta: { requiresDevMode: true },
    },
    {
      path: '/esp32s3r8n16',
      name: 'esp32s3r8n16',
      component: Esp32S3R8N16View,
      meta: { requiresDevMode: true },
    },
    {
      path: '/about',
      name: 'about',
      component: AboutView,
    },
  ],
})

router.beforeEach((to) => {
  const portalStore = usePortalStore()
  if (to.meta.requiresDevMode && !portalStore.developerMode) {
    return '/config'
  }

  return true
})

export default router
