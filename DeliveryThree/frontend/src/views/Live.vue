<template>
  <div>
    <h2>Live Readings</h2>
    <p>Latest weather station readings.</p>

    <div class="card-grid">
      <ReadingCard title="Temperature (DHT22)" :value="formatNumber(latest.temperature_dht_c) + ' °C'" />
      <ReadingCard title="Humidity" :value="formatNumber(latest.humidity_pct, 0) + ' %'" />
      <ReadingCard title="Heat Index" :value="formatNumber(latest.heat_index_c) + ' °C'" />
      <ReadingCard title="Pressure" :value="formatNumber(latest.pressure_pa, 0) + ' Pa'" />
      <ReadingCard title="Altitude" :value="formatNumber(latest.altitude_m) + ' m'" />
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
import { ref, onMounted, onUnmounted } from 'vue'
import { Chart, registerables } from 'chart.js'
import ReadingCard from '../components/ReadingCard.vue'

Chart.register(...registerables)

const latest = ref({
  temperature_dht_c: '--',
  humidity_pct: '--',
  heat_index_c: '--',
  pressure_pa: '--',
  altitude_m: '--',
  moisture_pct: '--',
  timestamp: '--'
})

const chartOneCanvas = ref(null)
const chartTwoCanvas = ref(null)

let chartOne = null
let chartTwo = null
let intervalId = null

// plain JS arrays only
let labels = []
let humiditySeries = []
let moistureSeries = []
let heatIndexSeries = []

const MAX_POINTS = 20
let pointCount = 0

function formatNumber(value, digits = 2) {
  if (value === '--' || value === null || value === undefined) return '--'
  return Number(value).toFixed(digits)
}

function formatTimestamp(ts) {
  if (ts === '--' || !ts) return '--'
  return new Date(ts * 1000).toLocaleString()
}

function pushPoint(row) {
  labels.push(`${pointCount * 3}s`)
  humiditySeries.push(Number(row.humidity_pct))
  moistureSeries.push(Number(row.moisture_pct))
  heatIndexSeries.push(Number(row.heat_index_c))
  pointCount++

  if (labels.length > MAX_POINTS) {
    labels.shift()
    humiditySeries.shift()
    moistureSeries.shift()
    heatIndexSeries.shift()
  }
}

function createCharts() {
  chartOne = new Chart(chartOneCanvas.value, {
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
  })

  chartTwo = new Chart(chartTwoCanvas.value, {
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
  })
}

function updateCharts() {
  if (!chartOne || !chartTwo) return

  chartOne.data.labels = [...labels]
  chartOne.data.datasets[0].data = [...humiditySeries]
  chartOne.data.datasets[1].data = [...moistureSeries]
  chartOne.update('none')

  chartTwo.data.labels = [...labels]
  chartTwo.data.datasets[0].data = [...heatIndexSeries]
  chartTwo.update('none')
}

async function fetchLatest() {
  try {
    const response = await fetch('http://localhost:8080/api/latest')
    const result = await response.json()

    if (result.status === 'Success') {
      const row = result.data
      latest.value = { ...row }

      pushPoint(row)
      updateCharts()
    } else {
      console.error(result.message)
    }
  } catch (error) {
    console.error('Error fetching latest data:', error)
  }
}

onMounted(async () => {
  createCharts()
  await fetchLatest()
  intervalId = setInterval(fetchLatest, 3000)
})

onUnmounted(() => {
  if (intervalId) clearInterval(intervalId)
  if (chartOne) chartOne.destroy()
  if (chartTwo) chartTwo.destroy()
})
</script>

<style scoped>
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