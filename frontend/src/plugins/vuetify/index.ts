// Vuetify
import { createVuetify } from 'vuetify'
import * as components from 'vuetify/components'
import * as directives from 'vuetify/directives'
import * as labsComponents from 'vuetify/labs/components'
import 'vuetify/styles'

/**
 * Portal Dark Theme
 * Matches the original ESP32 captive portal design with deep blues and cyan accents
 */
const portalDarkTheme = {
  dark: true,
  colors: {
    // Primary brand colors (cyan accent theme)
    primary: '#7dd3fc', // Light cyan
    'primary-light': '#e0f2fe', // Very light cyan
    'primary-dark': '#0c4a6e', // Dark cyan

    // Secondary colors (emerald green)
    secondary: '#34d399', // Emerald green
    'secondary-light': '#a7f3d0',
    'secondary-dark': '#059669',

    // Accent colors (amber)
    accent: '#fbbf24', // Amber
    'accent-light': '#fef3c7',
    'accent-dark': '#d97706',

    // Semantic colors
    error: '#fb7185', // Rose
    warning: '#f59e0b', // Amber
    success: '#34d399', // Emerald
    info: '#60a5fa', // Blue

    // Background and surface
    background: '#06111b', // Deep navy blue
    surface: '#0d1722', // Slightly lighter navy
    'surface-bright': '#1a2a3a',
    'surface-soft': '#0a1828',

    // Additional UI colors
    border: '#1e3a4c',
    outline: '#2d5a6f',
    'on-background': '#e2e8f0', // Light slate
    'on-surface': '#cbd5e1', // Light slate
    'on-primary': '#06111b', // Dark on light primary
    'on-secondary': '#06111b',
    'on-error': '#fff',
  },
}

const portalLightTheme = {
  dark: false,
  colors: {
    primary: '#0c4a6e',
    'primary-light': '#e0f2fe',
    'primary-dark': '#0a364f',

    secondary: '#0f766e',
    'secondary-light': '#99f6e4',
    'secondary-dark': '#115e59',

    accent: '#d97706',
    'accent-light': '#fef3c7',
    'accent-dark': '#92400e',

    error: '#be123c',
    warning: '#b45309',
    success: '#047857',
    info: '#1d4ed8',

    background: '#f5f8fc',
    surface: '#ffffff',
    'surface-bright': '#ffffff',
    'surface-soft': '#eef4fb',

    border: '#d7e3f1',
    outline: '#b7cadf',
    'on-background': '#0f172a',
    'on-surface': '#0f172a',
    'on-primary': '#ffffff',
    'on-secondary': '#ffffff',
    'on-error': '#ffffff',
  },
}

const vuetify = createVuetify({
  components: {
    ...components,
    ...labsComponents,
  },
  directives,
  theme: {
    defaultTheme: 'portalDarkTheme',
    themes: {
      portalDarkTheme,
      portalLightTheme,
    },
  },
})

export default vuetify