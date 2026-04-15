<template>
  <v-container fluid class="py-6">
    <v-row class="mb-2" align="center" justify="space-between">
      <v-col cols="12" md="8">
        <div class="text-h4">{{ t('erConsole.hardware.title') }}</div>
        <div class="text-subtitle-1 text-medium-emphasis">
          {{ t('erConsole.hardware.subtitle') }}
        </div>
      </v-col>
      <v-col cols="12" md="4" class="d-flex justify-md-end">
        <v-chip prepend-icon="mdi-lock-outline" color="primary" variant="tonal" size="large">
          {{ t('erConsole.hardware.readOnly') }}
        </v-chip>
      </v-col>
    </v-row>

    <v-row>
      <v-col cols="12" xl="8">
        <v-card elevation="3">
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-vector-polyline" />
            {{ t('erConsole.hardware.mapTitle') }}
          </v-card-title>
          <v-card-text>
            <div class="board-map-surface">
              <div class="board-stage">
                <img :src="boardImage" class="board-image" alt="ESP32-S3 DevKitC-1 board" />
              </div>
            </div>
          </v-card-text>
        </v-card>
      </v-col>

      <v-col cols="12" xl="4">
        <v-card elevation="2" class="mb-4">
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-sitemap-outline" />
            {{ t('erConsole.hardware.legendTitle') }}
          </v-card-title>
          <v-card-text>
            <v-chip v-for="bus in buses" :key="bus.name" size="small" class="mr-2 mb-2" :color="bus.color" variant="tonal">
              {{ bus.name }}
            </v-chip>
            <div class="text-body-2 text-medium-emphasis mt-3">
              {{ t('erConsole.hardware.legendDescription') }}
            </div>
          </v-card-text>
        </v-card>

        <v-card elevation="2">
          <v-card-title class="d-flex align-center ga-2">
            <v-icon icon="mdi-table" />
            {{ t('erConsole.hardware.connectionTableTitle') }}
          </v-card-title>
          <v-card-text>
            <v-table density="compact">
              <thead>
                <tr>
                  <th>{{ t('erConsole.hardware.columns.device') }}</th>
                  <th>{{ t('erConsole.hardware.columns.signal') }}</th>
                  <th>{{ t('erConsole.hardware.columns.gpio') }}</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="item in connectionRows" :key="`${item.device}-${item.signal}`">
                  <td>{{ item.device }}</td>
                  <td>{{ item.signal }}</td>
                  <td>{{ item.gpio }}</td>
                </tr>
              </tbody>
            </v-table>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script setup lang="ts">
import { useI18n } from 'vue-i18n'

const { t } = useI18n()
const boardImage = new URL('@/assets/images/esp32-s3-devkitc-1-n16r8v.svg', import.meta.url).href

const buses = [
  { name: 'SPI1', color: 'indigo' },
  { name: 'SPI (SD)', color: 'teal' },
  { name: 'I2S', color: 'amber' },
  { name: 'I2C', color: 'green' },
  { name: 'GPIO', color: 'pink' },
]

const connectionRows = [
  { device: 'ILI9341 TFT', signal: 'MOSI', gpio: '11' },
  { device: 'ILI9341 TFT', signal: 'MISO', gpio: '13' },
  { device: 'ILI9341 TFT', signal: 'SCLK', gpio: '12' },
  { device: 'ILI9341 TFT', signal: 'CS', gpio: '10' },
  { device: 'ILI9341 TFT', signal: 'DC', gpio: '9' },
  { device: 'ILI9341 TFT', signal: 'RST', gpio: '8' },
  { device: 'XPT2046 Touch', signal: 'CS', gpio: '7' },
  { device: 'XPT2046 Touch', signal: 'IRQ', gpio: '6' },
  { device: 'MicroSD', signal: 'CS', gpio: '5' },
  { device: 'MAX98357A', signal: 'DOUT', gpio: '16' },
  { device: 'MAX98357A', signal: 'BCLK', gpio: '18' },
  { device: 'MAX98357A', signal: 'LRCLK', gpio: '17' },
  { device: 'MAX98357A', signal: 'SD', gpio: '4' },
  { device: 'I2C Bus', signal: 'SDA', gpio: '2' },
  { device: 'I2C Bus', signal: 'SCL', gpio: '1' },
  { device: 'NeoPixel', signal: 'DATA', gpio: '48' },
]
</script>

<style scoped>
.board-map-surface {
  background: radial-gradient(circle at 50% 0%, #f0f9ff 0%, #e2e8f0 100%);
  border-radius: 14px;
  padding: 12px;
}

.board-stage {
  width: 100%;
  max-width: 1280px;
  margin: 0 auto;
}

.board-image {
  width: 100%;
  display: block;
  border-radius: 12px;
  box-shadow: 0 20px 44px rgba(15, 23, 42, 0.24);
}
</style>
