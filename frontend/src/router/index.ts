import { createRouter, createWebHistory } from 'vue-router'
import DashboardView from '@/views/DashboardView.vue'
import DeviceConfigView from '@/views/DeviceConfigView.vue'
import FileBrowserView from '@/views/FileBrowserView.vue'
import LogsAuditView from '@/views/LogsAuditView.vue'
import AboutView from '@/views/AboutView.vue'

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
    },
    {
      path: '/logs-audit',
      name: 'logs-audit',
      component: LogsAuditView,
    },
    {
      path: '/about',
      name: 'about',
      component: AboutView,
    },
  ],
})

export default router
