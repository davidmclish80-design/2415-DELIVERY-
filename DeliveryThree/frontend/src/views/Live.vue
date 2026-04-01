<template>
  <div>
    <div class="live-header">
      <div>
        <h2>Live Readings</h2>
        <p>Latest weather station readings.</p>
      </div>

      <button class="unit-btn" @click="toggleUnits">
        {{ unitMode === 'metric'
          ? 'Use °F / hPa / cm'
          : 'Use °C / Pa / m' }}
      </button>
    </div>

    <div class="card-grid">
      <ReadingCard title="Temperature (DHT22)" :value="displayTemperature(latest.temperature_dht_c)" />
      <ReadingCard title="Humidity" :value="formatNumber(latest.humidity_pct, 0) + ' %'" />
      <ReadingCard title="Heat Index" :value="displayHeatIndex(latest.heat_index_c)" />
      <ReadingCard title="Pressure" :value="displayPressure(latest.pressure_pa)" />
      <ReadingCard title="Altitude" :value="displayAltitude(latest.altitude_m)" />
      <ReadingCard title="Moisture" :value="formatNumber(latest.moisture_pct, 0) + ' %'" />
    </div>

    <p class="timestamp">
      Timestamp: {{ formatTimestamp(latest.timestamp) }}
    </p>

    <div class="graph-box">
      <h3>Live Chart 1</h3>
      <p>Humidity and Moisture vs Time</p>
      <div class="chart-wrap">
        <canvas ref="chartOneCanvas"></canvas>
      </div>
    </div>

    <div class="graph-box">
      <h3>Live Chart 2</h3>
      <p>Heat Index vs Time</p>
      <div class="chart-wrap">
        <canvas ref="chartTwoCanvas"></canvas>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, markRaw, watch } from 'vue'
import { Chart, registerables } from 'chart.js'
import ReadingCard from '../components/ReadingCard.vue'

Chart.register(...registerables)

const latest = ref({
  temperature_dht_c: null,
  humidity_pct: null,
  heat_index_c: null,
  pressure_pa: null,
  altitude_m: null,
  moisture_pct: null,
  timestamp: null
})

const unitMode = ref('metric') // metric or converted

const chartOneCanvas = ref(null)
const chartTwoCanvas = ref(null)

let chartOne = null
let chartTwo = null
let intervalId = null

const MAX_POINTS = 20
let pointCount = 0

let labels = []
let humiditySeries = []
let moistureSeries = []
let heatIndexSeries = []

function toggleUnits() {
  unitMode.value = unitMode.value === 'metric' ? 'converted' : 'metric'
}

function formatNumber(value, digits = 2) {
  if (value === null || value === undefined || Number.isNaN(Number(value))) return '--'
  return Number(value).toFixed(digits)
}

function formatTimestamp(ts) {
  if (!ts) return '--'
  return new Date(Number(ts) * 1000).toLocaleString()
}

function toFahrenheit(celsius) {
  return (Number(celsius) * 9) / 5 + 32
}

function displayTemperature(value) {
  if (value === null || value === undefined) return '--'
  if (unitMode.value === 'metric') {
    return `${formatNumber(value)} °C`
  }
  return `${formatNumber(toFahrenheit(value))} °F`
}

function displayHeatIndex(value) {
  if (value === null || value === undefined) return '--'
  if (unitMode.value === 'metric') {
    return `${formatNumber(value)} °C`
  }
  return `${formatNumber(toFahrenheit(value))} °F`
}

function displayPressure(value) {
  if (value === null || value === undefined) return '--'
  if (unitMode.value === 'metric') {
    return `${formatNumber(value, 0)} Pa`
  }
  return `${formatNumber(Number(value) / 100, 2)} hPa`
}

function displayAltitude(value) {
  if (value === null || value === undefined) return '--'
  if (unitMode.value === 'metric') {
    return `${formatNumber(value)} m`
  }
  return `${formatNumber(Number(value) * 100, 2)} cm`
}

function appendAndTrim(label, humidity, moisture, heatIndex) {
  labels.push(label)
  humiditySeries.push(humidity)
  moistureSeries.push(moisture)
  heatIndexSeries.push(heatIndex)

  if (labels.length > MAX_POINTS) {
    labels.shift()
    humiditySeries.shift()
    moistureSeries.shift()
    heatIndexSeries.shift()
  }
}

function createCharts() {
  const ctx1 = chartOneCanvas.value?.getContext('2d')
  const ctx2 = chartTwoCanvas.value?.getContext('2d')

  if (!ctx1 || !ctx2) return

  chartOne = markRaw(new Chart(ctx1, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Humidity (%)',
          data: [],
          tension: 0.25,
          pointRadius: 2,
          borderWidth: 2
        },
        {
          label: 'Moisture (%)',
          data: [],
          tension: 0.25,
          pointRadius: 2,
          borderWidth: 2
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: {
        mode: 'index',
        intersect: false
      },
      plugins: {
        legend: {
          position: 'top'
        }
      },
      scales: {
        x: {
          ticks: {
            maxTicksLimit: 8,
            maxRotation: 0,
            minRotation: 0
          }
        },
        y: {
          beginAtZero: true
        }
      }
    }
  }))

  chartTwo = markRaw(new Chart(ctx2, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Heat Index (°C)',
          data: [],
          tension: 0.25,
          pointRadius: 2,
          borderWidth: 2
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: {
        mode: 'index',
        intersect: false
      },
      plugins: {
        legend: {
          position: 'top'
        }
      },
      scales: {
        x: {
          ticks: {
            maxTicksLimit: 8,
            maxRotation: 0,
            minRotation: 0
          }
        },
        y: {
          beginAtZero: false
        }
      }
    }
  }))
}

function updateCharts() {
  if (!chartOne || !chartTwo) return

  chartOne.data.labels = labels.slice()
  chartOne.data.datasets[0].data = humiditySeries.slice()
  chartOne.data.datasets[1].data = moistureSeries.slice()
  chartOne.update('none')

  const displayHeatIndexSeries =
    unitMode.value === 'metric'
      ? heatIndexSeries.slice()
      : heatIndexSeries.map(v => toFahrenheit(v))

  chartTwo.data.labels = labels.slice()
  chartTwo.data.datasets[0].label =
    unitMode.value === 'metric' ? 'Heat Index (°C)' : 'Heat Index (°F)'
  chartTwo.data.datasets[0].data = displayHeatIndexSeries
  chartTwo.update('none')
}

async function fetchLatest() {
  try {
    const response = await fetch('http://localhost:8080/api/latest')
    const result = await response.json()

    if (result.status !== 'Success' || !result.data) {
      console.error(result.message || 'No latest data returned')
      return
    }

    const row = result.data

    latest.value = {
      temperature_dht_c: Number(row.temperature_dht_c),
      humidity_pct: Number(row.humidity_pct),
      heat_index_c: Number(row.heat_index_c),
      pressure_pa: Number(row.pressure_pa),
      altitude_m: Number(row.altitude_m),
      moisture_pct: Number(row.moisture_pct),
      timestamp: Number(row.timestamp)
    }

    const label = `${pointCount * 3}s`
    appendAndTrim(
      label,
      Number(row.humidity_pct),
      Number(row.moisture_pct),
      Number(row.heat_index_c)
    )
    pointCount += 1

    updateCharts()
  } catch (error) {
    console.error('Error fetching latest data:', error)
  }
}

watch(unitMode, () => {
  updateCharts()
})

onMounted(async () => {
  createCharts()
  await fetchLatest()
  intervalId = setInterval(fetchLatest, 3000)
})

onUnmounted(() => {
  if (intervalId) clearInterval(intervalId)
  intervalId = null

  if (chartOne) {
    chartOne.destroy()
    chartOne = null
  }

  if (chartTwo) {
    chartTwo.destroy()
    chartTwo = null
  }
})
</script>

<style scoped>
.live-header {
  display: flex;
  justify-content: space-between;
  align-items: end;
  gap: 16px;
  flex-wrap: wrap;
}

.unit-btn {
  padding: 10px 18px;
  border: 1px solid #cbd5e1;
  border-radius: 10px;
  cursor: pointer;
  background: #ffffff;
  color: #1e3a8a;
  font-weight: 600;
  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.08);
  transition: 0.2s ease;
}

.unit-btn:hover {
  background: #dbeafe;
  border-color: #93c5fd;
}

.card-grid {
  display: flex;
  flex-wrap: wrap;
  gap: 16px;
  margin-top: 20px;
}

.timestamp {
  margin-top: 20px;
  font-weight: bold;
}

.graph-box {
  margin-top: 24px;
  background: white;
  border-radius: 12px;
  padding: 16px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
}

.chart-wrap {
  position: relative;
  width: 100%;
  height: 360px;
  overflow: hidden;
}

canvas {
  display: block;
  width: 100% !important;
  height: 100% !important;
}
</style>