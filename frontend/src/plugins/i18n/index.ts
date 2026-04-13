import enMessages from './locales/en.json'
import itMessages from './locales/it.json'
import { createI18n } from 'vue-i18n'

export default createI18n({
    legacy: false,
    locale: 'it', // set locale
    fallbackLocale: 'en', // set fallback locale
    availableLocales: ['it', 'en'],
    globalInjection: true,
    messages: {
        en: enMessages,
        it: itMessages
    }
})
