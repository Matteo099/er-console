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
              <div ref="canvasHost" class="board-stage">
                <canvas
                  ref="boardCanvas"
                  class="board-canvas"
                  @mousemove="onCanvasMove"
                  @mouseleave="hideTooltip"
                />
                <v-sheet
                  v-if="tooltip.visible"
                  elevation="8"
                  rounded="lg"
                  class="canvas-tooltip pa-2"
                  :style="{ left: `${tooltip.x}px`, top: `${tooltip.y}px` }"
                >
                  <div class="text-caption font-weight-bold">{{ tooltip.title }}</div>
                  <div class="text-caption">{{ tooltip.description }}</div>
                </v-sheet>
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
import { nextTick, onBeforeUnmount, onMounted, reactive, ref } from 'vue'
import { useI18n } from 'vue-i18n'

interface Point {
  x: number
  y: number
}

interface BoardPin {
  key: string
  value: string
  x: number
  y: number
  row: 'top' | 'bottom'
  color: string
  note: string
}

interface InteractiveWire {
  id: string
  title: string
  description: string
  color: string
  points: Point[]
}

const { t } = useI18n()

const LOGICAL_WIDTH = 1800
const LOGICAL_HEIGHT = 1200

const boardCanvas = ref<HTMLCanvasElement | null>(null)
const canvasHost = ref<HTMLDivElement | null>(null)
let resizeObserver: ResizeObserver | null = null

const tooltip = reactive({
  visible: false,
  x: 0,
  y: 0,
  title: '',
  description: '',
})

const leftPins = ['3V3', '3V3', 'RST', '4', '5', '6', '7', '15', '16', '17', '18', '8', '3', '46', '9', '10', '11', '12', '13', '14', '5Vi', 'GND']
const rightPins = ['GND', '43', '44', '1', '2', '42', '41', '40', '39', '38', '37', '36', '35', '0', '45', '48', '47', '21', '20', '19', 'GND', 'GND']

const pinUsageNotes: Record<string, string> = {
  '11': 'SPI1 MOSI -> TFT + MicroSD',
  '13': 'SPI1 MISO -> TFT + MicroSD',
  '12': 'SPI1 SCLK -> TFT + MicroSD',
  '10': 'TFT CS',
  '9': 'TFT DC',
  '8': 'TFT RST',
  '7': 'Touch CS (XPT2046)',
  '6': 'Touch IRQ (XPT2046)',
  '5': 'MicroSD CS',
  '16': 'I2S DOUT -> MAX98357A DIN',
  '18': 'I2S BCLK -> MAX98357A BCLK',
  '17': 'I2S LRCLK -> MAX98357A LRC',
  '4': 'I2S SHUTDOWN -> MAX98357A SD',
  '48': 'Internal NeoPixel data',
  '3V3': '3.3V output from ESP32',
  '5Vi': '5V input rail',
  GND: 'Common ground reference',
}

const boardRect = {
  x: 680,
  y: 120,
  width: 440,
  height: 880,
}

const leftPinX = boardRect.x - 10
const rightPinX = boardRect.x + boardRect.width + 10
const leftLabelX = boardRect.x - 110
const rightLabelX = boardRect.x + boardRect.width + 38
const rowStartY = 200
const rowSpacing = 30

const colorForPin = (value: string) => {
  if (value === 'GND') return '#64748b'
  if (value === '5Vi') return '#f97316'
  if (value === '3V3') return '#38bdf8'
  if (['11', '12', '13'].includes(value)) return '#4f46e5'
  if (['10', '9', '8'].includes(value)) return '#2563eb'
  if (['7', '6'].includes(value)) return '#6366f1'
  if (value === '5') return '#0f766e'
  if (['16', '17', '18', '4'].includes(value)) return '#d97706'
  if (value === '48') return '#c026d3'
  return '#cbd5e1'
}

const buildPins = (): BoardPin[] => {
  const pins: BoardPin[] = []
  leftPins.forEach((value, idx) => {
    pins.push({
      key: `left-${idx}-${value}`,
      value,
      x: leftPinX,
      y: rowStartY + idx * rowSpacing,
      row: 'bottom',
      color: colorForPin(value),
      note: pinUsageNotes[value] ?? 'Board pin (not used in this wiring)',
    })
  })

  rightPins.forEach((value, idx) => {
    pins.push({
      key: `right-${idx}-${value}`,
      value,
      x: rightPinX,
      y: rowStartY + idx * rowSpacing,
      row: 'top',
      color: colorForPin(value),
      note: pinUsageNotes[value] ?? 'Board pin (not used in this wiring)',
    })
  })

  return pins
}

const pins = buildPins()

const getPin = (value: string, row: 'top' | 'bottom' = 'bottom') => {
  return pins.find((pin) => pin.value === value && pin.row === row)
}

const p11 = getPin('11')!
const p13 = getPin('13')!
const p12 = getPin('12')!
const p10 = getPin('10')!
const p9 = getPin('9')!
const p8 = getPin('8')!
const p7 = getPin('7')!
const p6 = getPin('6')!
const p5 = getPin('5')!
const p16 = getPin('16')!
const p18 = getPin('18')!
const p17 = getPin('17')!
const p4 = getPin('4')!
const p3v3 = getPin('3V3')!
const p48 = getPin('48', 'top')!
const pGndLeft = getPin('GND', 'bottom')!
const pGndRight = getPin('GND', 'top')!

const wires: InteractiveWire[] = [
  {
    id: 'tft-mosi',
    title: 'TFT MOSI',
    description: 'SPI1 MOSI from GPIO 11 to TFT display',
    color: '#4f46e5',
    points: [{ x: 430, y: 400 }, { x: 620, y: 400 }, { x: 620, y: p11.y }, { x: p11.x, y: p11.y }],
  },
  {
    id: 'tft-miso',
    title: 'TFT MISO',
    description: 'SPI1 MISO from GPIO 13 to TFT display',
    color: '#4f46e5',
    points: [{ x: 430, y: 430 }, { x: 620, y: 430 }, { x: 620, y: p13.y }, { x: p13.x, y: p13.y }],
  },
  {
    id: 'tft-sclk',
    title: 'TFT SCLK',
    description: 'SPI1 clock from GPIO 12 to TFT display',
    color: '#4f46e5',
    points: [{ x: 430, y: 460 }, { x: 620, y: 460 }, { x: 620, y: p12.y }, { x: p12.x, y: p12.y }],
  },
  {
    id: 'tft-cs',
    title: 'TFT CS',
    description: 'Chip Select from GPIO 10 to ILI9341',
    color: '#2563eb',
    points: [{ x: 430, y: 490 }, { x: 620, y: 490 }, { x: 620, y: p10.y }, { x: p10.x, y: p10.y }],
  },
  {
    id: 'tft-dc',
    title: 'TFT DC',
    description: 'Data/Command line from GPIO 9 to ILI9341',
    color: '#2563eb',
    points: [{ x: 430, y: 520 }, { x: 620, y: 520 }, { x: 620, y: p9.y }, { x: p9.x, y: p9.y }],
  },
  {
    id: 'tft-rst',
    title: 'TFT RST',
    description: 'Reset line from GPIO 8 to ILI9341',
    color: '#2563eb',
    points: [{ x: 430, y: 550 }, { x: 620, y: 550 }, { x: 620, y: p8.y }, { x: p8.x, y: p8.y }],
  },
  {
    id: 'touch-cs',
    title: 'Touch CS',
    description: 'Touch controller chip select from GPIO 7',
    color: '#6366f1',
    points: [{ x: 430, y: 580 }, { x: 620, y: 580 }, { x: 620, y: p7.y }, { x: p7.x, y: p7.y }],
  },
  {
    id: 'touch-irq',
    title: 'Touch IRQ',
    description: 'Touch interrupt from GPIO 6',
    color: '#6366f1',
    points: [{ x: 430, y: 610 }, { x: 620, y: 610 }, { x: 620, y: p6.y }, { x: p6.x, y: p6.y }],
  },
  {
    id: 'sd-mosi',
    title: 'MicroSD MOSI',
    description: 'Shared SPI1 MOSI from GPIO 11',
    color: '#4f46e5',
    points: [{ x: 430, y: 760 }, { x: 620, y: 760 }, { x: 620, y: p11.y }, { x: p11.x, y: p11.y }],
  },
  {
    id: 'sd-miso',
    title: 'MicroSD MISO',
    description: 'Shared SPI1 MISO from GPIO 13',
    color: '#4f46e5',
    points: [{ x: 430, y: 790 }, { x: 620, y: 790 }, { x: 620, y: p13.y }, { x: p13.x, y: p13.y }],
  },
  {
    id: 'sd-sclk',
    title: 'MicroSD SCLK',
    description: 'Shared SPI1 clock from GPIO 12',
    color: '#4f46e5',
    points: [{ x: 430, y: 820 }, { x: 620, y: 820 }, { x: 620, y: p12.y }, { x: p12.x, y: p12.y }],
  },
  {
    id: 'sd-cs',
    title: 'MicroSD CS',
    description: 'Chip Select from GPIO 5 to MicroSD',
    color: '#0f766e',
    points: [{ x: 430, y: 850 }, { x: 620, y: 850 }, { x: 620, y: p5.y }, { x: p5.x, y: p5.y }],
  },
  {
    id: 'amp-dout',
    title: 'MAX98357A DOUT',
    description: 'I2S serial audio data from GPIO 16',
    color: '#d97706',
    points: [{ x: 1290, y: 370 }, { x: 1180, y: 370 }, { x: 1180, y: p16.y }, { x: p16.x, y: p16.y }],
  },
  {
    id: 'amp-bclk',
    title: 'MAX98357A BCLK',
    description: 'I2S bit clock from GPIO 18',
    color: '#d97706',
    points: [{ x: 1290, y: 400 }, { x: 1180, y: 400 }, { x: 1180, y: p18.y }, { x: p18.x, y: p18.y }],
  },
  {
    id: 'amp-lrclk',
    title: 'MAX98357A LRCLK',
    description: 'I2S LRCLK from GPIO 17',
    color: '#d97706',
    points: [{ x: 1290, y: 430 }, { x: 1180, y: 430 }, { x: 1180, y: p17.y }, { x: p17.x, y: p17.y }],
  },
  {
    id: 'amp-shutdown',
    title: 'MAX98357A SHUTDOWN',
    description: 'Amplifier shutdown control from GPIO 4',
    color: '#d97706',
    points: [{ x: 1290, y: 460 }, { x: 1180, y: 460 }, { x: 1180, y: p4.y }, { x: p4.x, y: p4.y }],
  },
  {
    id: 'power-5v-tft',
    title: '5V External -> TFT',
    description: 'External 5V powers TFT display',
    color: '#f97316',
    points: [{ x: 360, y: 110 }, { x: 620, y: 110 }, { x: 620, y: p10.y - 60 }, { x: p10.x, y: p10.y - 60 }],
  },
  {
    id: 'power-5v-amp',
    title: '5V External -> MAX98357A',
    description: 'External 5V powers audio amplifier',
    color: '#f97316',
    points: [{ x: 360, y: 130 }, { x: 1180, y: 130 }, { x: 1180, y: p16.y - 50 }, { x: p16.x, y: p16.y - 50 }],
  },
  {
    id: 'power-33v-sd',
    title: 'ESP32 3.3V -> MicroSD',
    description: 'MicroSD is powered from the ESP32 3.3V rail',
    color: '#38bdf8',
    points: [{ x: p3v3.x, y: p3v3.y }, { x: p3v3.x, y: 920 }, { x: 540, y: 920 }, { x: 540, y: 780 }],
  },
  {
    id: 'common-ground',
    title: 'Common Ground',
    description: 'Ground reference shared by ESP32, TFT, MicroSD, and MAX98357A',
    color: '#64748b',
    points: [{ x: 260, y: 980 }, { x: 1540, y: 980 }, { x: 1540, y: 560 }, { x: 1290, y: 560 }],
  },
]

const buses = [
  { name: 'SPI1', color: 'indigo' },
  { name: 'SPI (SD)', color: 'teal' },
  { name: 'I2S', color: 'amber' },
  { name: 'Power 5V', color: 'red' },
  { name: 'Power 3.3V', color: 'blue' },
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
  { device: 'TFT Display', signal: 'VCC', gpio: '5V external' },
  { device: 'TFT Display', signal: 'GND', gpio: 'common' },
  { device: 'MicroSD', signal: 'MOSI', gpio: '11' },
  { device: 'MicroSD', signal: 'MISO', gpio: '13' },
  { device: 'MicroSD', signal: 'SCLK', gpio: '12' },
  { device: 'MicroSD', signal: 'CS', gpio: '5' },
  { device: 'MicroSD', signal: 'VCC', gpio: '3.3V ESP32' },
  { device: 'MicroSD', signal: 'GND', gpio: 'common' },
  { device: 'MAX98357A', signal: 'DOUT', gpio: '16' },
  { device: 'MAX98357A', signal: 'BCLK', gpio: '18' },
  { device: 'MAX98357A', signal: 'LRCLK', gpio: '17' },
  { device: 'MAX98357A', signal: 'SD', gpio: '4' },
  { device: 'MAX98357A', signal: 'VCC', gpio: '5V external' },
  { device: 'MAX98357A', signal: 'GND', gpio: 'common' },
  { device: 'NeoPixel', signal: 'DATA', gpio: '48' },
  { device: 'Internal RGB LEDs', signal: 'R / G / B', gpio: 'board-internal' },
]

const drawRoundedRect = (
  ctx: CanvasRenderingContext2D,
  x: number,
  y: number,
  w: number,
  h: number,
  r: number,
  fill: string | CanvasGradient,
  stroke: string,
) => {
  ctx.beginPath()
  ctx.moveTo(x + r, y)
  ctx.lineTo(x + w - r, y)
  ctx.quadraticCurveTo(x + w, y, x + w, y + r)
  ctx.lineTo(x + w, y + h - r)
  ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h)
  ctx.lineTo(x + r, y + h)
  ctx.quadraticCurveTo(x, y + h, x, y + h - r)
  ctx.lineTo(x, y + r)
  ctx.quadraticCurveTo(x, y, x + r, y)
  ctx.closePath()
  ctx.fillStyle = fill
  ctx.fill()
  ctx.lineWidth = 2
  ctx.strokeStyle = stroke
  ctx.stroke()
}

const drawText = (
  ctx: CanvasRenderingContext2D,
  text: string,
  x: number,
  y: number,
  size = 14,
  color = '#0f172a',
  weight = '600',
) => {
  ctx.font = `${weight} ${size}px "Segoe UI", Arial, sans-serif`
  ctx.fillStyle = color
  ctx.fillText(text, x, y)
}

const drawWire = (ctx: CanvasRenderingContext2D, wire: InteractiveWire) => {
  if (wire.points.length < 2) return

  ctx.strokeStyle = wire.color
  ctx.lineWidth = wire.id === 'common-ground' ? 3 : 4
  if (wire.id === 'common-ground') {
    ctx.setLineDash([10, 10])
  } else {
    ctx.setLineDash([])
  }
  ctx.beginPath()
  const [first, ...rest] = wire.points
  if (!first) return
  ctx.moveTo(first.x, first.y)
  rest.forEach((point) => ctx.lineTo(point.x, point.y))
  ctx.stroke()
  ctx.setLineDash([])
}

const drawBoard = (ctx: CanvasRenderingContext2D) => {
  const gradient = ctx.createLinearGradient(boardRect.x, boardRect.y, boardRect.x + boardRect.width, boardRect.y + boardRect.height)
  gradient.addColorStop(0, '#111827')
  gradient.addColorStop(1, '#0f172a')
  drawRoundedRect(ctx, boardRect.x, boardRect.y, boardRect.width, boardRect.height, 30, gradient, '#334155')

  drawRoundedRect(ctx, boardRect.x + 130, boardRect.y + 60, 180, 80, 14, '#374151', '#6b7280')
  drawText(ctx, 'ESP32-S3', boardRect.x + 170, boardRect.y + 112, 22, '#e5e7eb', '800')
  drawText(ctx, 'DevKitC-1', boardRect.x + 160, boardRect.y + 136, 16, '#cbd5e1', '700')

  drawRoundedRect(ctx, boardRect.x + 154, boardRect.y + 210, 132, 170, 10, '#d1d5db', '#9ca3af')
  drawText(ctx, 'ESP32-S3', boardRect.x + 165, boardRect.y + 248, 13, '#6b7280', '700')
  drawText(ctx, 'WROOM', boardRect.x + 172, boardRect.y + 270, 13, '#6b7280', '700')

  drawRoundedRect(ctx, boardRect.x + 114, boardRect.y + 410, 68, 38, 8, '#111827', '#475569')
  drawRoundedRect(ctx, boardRect.x + 258, boardRect.y + 410, 68, 38, 8, '#111827', '#475569')
  drawText(ctx, 'BOOT', boardRect.x + 127, boardRect.y + 404, 10, '#e2e8f0', '700')
  drawText(ctx, 'RESET', boardRect.x + 272, boardRect.y + 404, 10, '#e2e8f0', '700')

  drawRoundedRect(ctx, boardRect.x + 110, boardRect.y + 700, 92, 54, 10, '#f3f4f6', '#9ca3af')
  drawRoundedRect(ctx, boardRect.x + 238, boardRect.y + 700, 92, 54, 10, '#f3f4f6', '#9ca3af')
  drawText(ctx, 'UART', boardRect.x + 134, boardRect.y + 730, 13, '#6b7280', '700')
  drawText(ctx, 'USB', boardRect.x + 267, boardRect.y + 730, 13, '#6b7280', '700')

  ctx.beginPath()
  ctx.arc(boardRect.x + 322, boardRect.y + 560, 24, 0, Math.PI * 2)
  ctx.fillStyle = '#701a75'
  ctx.fill()
  ctx.beginPath()
  ctx.arc(boardRect.x + 322, boardRect.y + 560, 10, 0, Math.PI * 2)
  ctx.fillStyle = '#fde047'
  ctx.fill()
  ctx.beginPath()
  ctx.arc(boardRect.x + 378, boardRect.y + 526, 8, 0, Math.PI * 2)
  ctx.fillStyle = '#ef4444'
  ctx.fill()
  ctx.beginPath()
  ctx.arc(boardRect.x + 406, boardRect.y + 560, 8, 0, Math.PI * 2)
  ctx.fillStyle = '#22c55e'
  ctx.fill()
  ctx.beginPath()
  ctx.arc(boardRect.x + 378, boardRect.y + 594, 8, 0, Math.PI * 2)
  ctx.fillStyle = '#3b82f6'
  ctx.fill()
  drawText(ctx, 'GPIO 48', boardRect.x + 272, boardRect.y + 628, 14, '#e5e7eb', '700')
}

const drawPins = (ctx: CanvasRenderingContext2D) => {
  pins.forEach((pin) => {
    const labelX = pin.x < boardRect.x ? leftLabelX : rightLabelX
    const textAlign = pin.x < boardRect.x ? 'right' : 'left'
    ctx.textAlign = textAlign as CanvasTextAlign
    drawText(ctx, pin.value, labelX, pin.y + 5, 11, '#111827', '700')
    ctx.beginPath()
    ctx.roundRect(pin.x - 12, pin.y - 8, 24, 16, 4)
    ctx.fillStyle = pin.color
    ctx.fill()
    ctx.lineWidth = 1.4
    ctx.strokeStyle = '#334155'
    ctx.stroke()
  })
  ctx.textAlign = 'left'
}

const drawHardwareBoxes = (ctx: CanvasRenderingContext2D) => {
  drawRoundedRect(ctx, 40, 60, 260, 140, 18, '#fee2e2', '#ef4444')
  drawText(ctx, '5V External', 68, 100, 24, '#7f1d1d', '800')
  drawText(ctx, 'Battery / PSU', 68, 128, 16, '#7f1d1d')
  drawText(ctx, '5V+ power rail', 68, 152, 16, '#7f1d1d')

  drawRoundedRect(ctx, 40, 250, 480, 360, 22, '#dbeafe', '#2563eb')
  drawText(ctx, 'TFT Display', 70, 296, 30, '#0f172a', '800')
  drawText(ctx, 'ILI9341 + XPT2046', 70, 326, 16, '#1e293b')
  drawText(ctx, 'SPI1: MOSI 11  MISO 13  SCLK 12', 70, 368, 18, '#1e293b')
  drawText(ctx, 'CS 10  DC 9  RST 8', 70, 398, 18, '#1e293b')
  drawText(ctx, 'Touch CS 7  IRQ 6', 70, 428, 18, '#1e293b')
  drawText(ctx, 'Power: 5V external + GND common', 70, 462, 18, '#1e293b')

  drawRoundedRect(ctx, 40, 650, 480, 210, 22, '#ccfbf1', '#0f766e')
  drawText(ctx, 'MicroSD', 70, 696, 30, '#0f172a', '800')
  drawText(ctx, 'SPI1: MOSI 11  MISO 13  SCLK 12', 70, 736, 18, '#1e293b')
  drawText(ctx, 'CS 5  |  Power: 3.3V ESP32 + GND common', 70, 768, 18, '#1e293b')

  drawRoundedRect(ctx, 1290, 250, 470, 280, 22, '#fef3c7', '#d97706')
  drawText(ctx, 'MAX98357A', 1320, 296, 30, '#0f172a', '800')
  drawText(ctx, 'I2S DOUT 16  BCLK 18  LRCLK 17', 1320, 336, 18, '#1e293b')
  drawText(ctx, 'SD 4  |  Power: 5V external', 1320, 368, 18, '#1e293b')
  drawText(ctx, 'GND common', 1320, 398, 18, '#1e293b')

  drawRoundedRect(ctx, 1290, 580, 470, 150, 22, '#ede9fe', '#7c3aed')
  drawText(ctx, 'Internal LEDs', 1320, 626, 28, '#0f172a', '800')
  drawText(ctx, 'NeoPixel GPIO 48 + 3 RGB indicators', 1320, 662, 18, '#1e293b')
}

const drawInternalLeds = (ctx: CanvasRenderingContext2D) => {
  ctx.beginPath()
  ctx.arc(boardRect.x + 280, boardRect.y + 585, 48, 0, Math.PI * 2)
  ctx.fillStyle = '#701a75'
  ctx.fill()
  ctx.lineWidth = 3
  ctx.strokeStyle = '#f0abfc'
  ctx.stroke()

  ctx.beginPath()
  ctx.arc(boardRect.x + 280, boardRect.y + 585, 18, 0, Math.PI * 2)
  ctx.fillStyle = '#fde047'
  ctx.fill()

  const leds = [
    { x: boardRect.x + 352, y: boardRect.y + 548, color: '#ef4444', label: 'R' },
    { x: boardRect.x + 378, y: boardRect.y + 585, color: '#22c55e', label: 'G' },
    { x: boardRect.x + 352, y: boardRect.y + 622, color: '#3b82f6', label: 'B' },
  ]

  leds.forEach((led) => {
    ctx.beginPath()
    ctx.arc(led.x, led.y, 12, 0, Math.PI * 2)
    ctx.fillStyle = led.color
    ctx.fill()
    drawText(ctx, led.label, led.x + 18, led.y + 4, 12, led.color, '800')
  })
}

const drawLegend = (ctx: CanvasRenderingContext2D) => {
  drawRoundedRect(ctx, 40, 1020, 1720, 90, 14, '#ffffff', '#e2e8f0')
  drawText(ctx, 'Hover pins or wires for details. The wiring is data-driven, so changing a connection only requires updating the arrays in this component.', 68, 1060, 15, '#334155', '600')
  drawText(ctx, 'Pins are labeled on both sides of the DevKit-style board; used nets are color coded by device/power rail.', 68, 1090, 15, '#334155', '600')
}

const renderCanvas = () => {
  if (!boardCanvas.value) return
  const canvas = boardCanvas.value
  const ctx = canvas.getContext('2d')
  if (!ctx) return

  ctx.clearRect(0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT)

  const bg = ctx.createLinearGradient(0, 0, 0, LOGICAL_HEIGHT)
  bg.addColorStop(0, '#f8fafc')
  bg.addColorStop(1, '#e2e8f0')
  ctx.fillStyle = bg
  ctx.fillRect(0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT)

  drawHardwareBoxes(ctx)
  drawBoard(ctx)
  wires.forEach((wire) => drawWire(ctx, wire))
  drawInternalLeds(ctx)
  drawPins(ctx)
  drawLegend(ctx)
}

const resizeCanvas = () => {
  if (!boardCanvas.value || !canvasHost.value) return

  const canvas = boardCanvas.value
  const hostWidth = canvasHost.value.clientWidth
  const cssWidth = Math.max(900, hostWidth)
  const cssHeight = Math.round((cssWidth * LOGICAL_HEIGHT) / LOGICAL_WIDTH)
  const dpr = window.devicePixelRatio || 1

  canvas.width = Math.floor(cssWidth * dpr)
  canvas.height = Math.floor(cssHeight * dpr)
  canvas.style.width = `${cssWidth}px`
  canvas.style.height = `${cssHeight}px`

  const ctx = canvas.getContext('2d')
  if (!ctx) return
  ctx.setTransform(dpr * (cssWidth / LOGICAL_WIDTH), 0, 0, dpr * (cssHeight / LOGICAL_HEIGHT), 0, 0)

  renderCanvas()
}

const getLogicalPointFromMouse = (event: MouseEvent): Point | null => {
  if (!boardCanvas.value) return null
  const rect = boardCanvas.value.getBoundingClientRect()
  if (rect.width === 0 || rect.height === 0) return null

  const x = ((event.clientX - rect.left) / rect.width) * LOGICAL_WIDTH
  const y = ((event.clientY - rect.top) / rect.height) * LOGICAL_HEIGHT
  return { x, y }
}

const distanceToSegment = (p: Point, a: Point, b: Point) => {
  const l2 = (b.x - a.x) ** 2 + (b.y - a.y) ** 2
  if (l2 === 0) return Math.hypot(p.x - a.x, p.y - a.y)
  let t = ((p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y)) / l2
  t = Math.max(0, Math.min(1, t))
  const projection = { x: a.x + t * (b.x - a.x), y: a.y + t * (b.y - a.y) }
  return Math.hypot(p.x - projection.x, p.y - projection.y)
}

const hitTestPin = (p: Point) => {
  return pins.find((pin) => Math.hypot(p.x - pin.x, p.y - pin.y) <= 12) ?? null
}

const hitTestWire = (p: Point) => {
  for (const wire of wires) {
    for (let i = 0; i < wire.points.length - 1; i += 1) {
      const a = wire.points[i]
      const b = wire.points[i + 1]
      if (!a || !b) continue
      if (distanceToSegment(p, a, b) <= 8) {
        return wire
      }
    }
  }
  return null
}

const onCanvasMove = (event: MouseEvent) => {
  const logicalPoint = getLogicalPointFromMouse(event)
  if (!logicalPoint || !boardCanvas.value) return

  const pin = hitTestPin(logicalPoint)
  const wire = pin ? null : hitTestWire(logicalPoint)

  if (!pin && !wire) {
    hideTooltip()
    boardCanvas.value.style.cursor = 'default'
    return
  }

  const rect = boardCanvas.value.getBoundingClientRect()
  tooltip.visible = true
  tooltip.x = event.clientX - rect.left + 16
  tooltip.y = event.clientY - rect.top + 16

  if (pin) {
    tooltip.title = `Pin ${pin.value}`
    tooltip.description = pin.note
  } else if (wire) {
    tooltip.title = wire.title
    tooltip.description = wire.description
  }

  boardCanvas.value.style.cursor = 'pointer'
}

const hideTooltip = () => {
  tooltip.visible = false
}

onMounted(async () => {
  await nextTick()
  resizeCanvas()
  if (canvasHost.value) {
    resizeObserver = new ResizeObserver(resizeCanvas)
    resizeObserver.observe(canvasHost.value)
  }
})

onBeforeUnmount(() => {
  if (resizeObserver) {
    resizeObserver.disconnect()
    resizeObserver = null
  }
})
</script>

<style scoped>
.board-map-surface {
  background: radial-gradient(circle at 50% 0%, #f0f9ff 0%, #e2e8f0 100%);
  border-radius: 14px;
  padding: 12px;
}

.board-stage {
  position: relative;
  width: 100%;
  margin: 0 auto;
  overflow: auto;
}

.board-canvas {
  display: block;
  max-width: none;
  border-radius: 14px;
  box-shadow: 0 20px 44px rgba(15, 23, 42, 0.24);
}

.canvas-tooltip {
  position: absolute;
  pointer-events: none;
  max-width: 320px;
  z-index: 3;
}
</style>
