import enMessages from './locales/en.json'
import esMessages from './locales/es.json'
import frMessages from './locales/fr.json'
import deMessages from './locales/de.json'
import itMessages from './locales/it.json'
import { createI18n } from 'vue-i18n'

const SUPPORTED_LOCALES = ['it', 'en', 'es', 'fr', 'de'] as const
type SupportedLocale = (typeof SUPPORTED_LOCALES)[number]

const localeFromStorage = localStorage.getItem('er-console-locale') as SupportedLocale | null
const localeFromBrowser = (navigator.language || 'en').slice(0, 2) as SupportedLocale

const locale: SupportedLocale = localeFromStorage && SUPPORTED_LOCALES.includes(localeFromStorage)
    ? localeFromStorage
    : SUPPORTED_LOCALES.includes(localeFromBrowser)
        ? localeFromBrowser
        : 'it'

const i18n = createI18n({
    legacy: false,
    locale,
    fallbackLocale: 'en',
    availableLocales: [...SUPPORTED_LOCALES],
    globalInjection: true,
    messages: {
        en: enMessages,
        es: esMessages,
        fr: frMessages,
        de: deMessages,
        it: itMessages,
    },
})

i18n.global.locale.value && localStorage.setItem('er-console-locale', i18n.global.locale.value)

export default i18n
