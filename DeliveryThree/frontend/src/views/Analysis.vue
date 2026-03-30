<template>
  <div>
    <h2>Analysis</h2>
    <p>Summary values and charts will go here.</p>

    <div class="controls">
      <label>
        Start:
        <input type="datetime-local" v-model="startInput" />
      </label>

      <label>
        End:
        <input type="datetime-local" v-model="endInput" />
      </label>

      <button @click="fetchRange">Load Range</button>
    </div>

    <p><strong>Points returned:</strong> {{ rows.length }}</p>

    <div class="stats-grid">
      <div class="stat-box">
        <h3>Temperature</h3>
        <p>Min: {{ formatNumber(stats.temperature_min) }}</p>
        <p>Max: {{ formatNumber(stats.temperature_max) }}</p>
        <p>Avg: {{ formatNumber(stats.temperature_avg) }}</p>
      </div>

      <div class="stat-box">
        <h3>Humidity</h3>
        <p>Min: {{ formatNumber(stats.humidity_min) }}</p>
        <p>Max: {{ formatNumber(stats.humidity_max) }}</p>
        <p>Avg: {{ formatNumber(stats.humidity_avg) }}</p>
      </div>

      <div class="stat-box">
        <h3>Moisture</h3>
        <p>Min: {{ formatNumber(stats.moisture_min) }}</p>
        <p>Max: {{ formatNumber(stats.moisture_max) }}</p>
        <p>Avg: {{ formatNumber(stats.moisture_avg) }}</p>
      </div>
    </div>

    <div> 


    </div>
    <div class="graph-box">
        <h3>Graph 1</h3>
        <p>Temperature and Humidity vs Time</p>
        <div class="chart-wrap">
            <canvas ref="chartOneCanvas"></canvas>
        </div>
    </div>

    <div class="graph-box">
        <h3>Graph 2</h3>
        <p>Moisture and Heat Index vs Time</p>
        <div class="chart-wrap">
            <canvas ref="chartTwoCanvas"></canvas>
        </div>
    </div>

    <div class="graph-box">
        <h3>Returned Data Preview</h3>
        <div class="chart-wrap">
            <pre>{{ rows.slice(0, 5) }}</pre>
        </div>
     
    </div>
  </div>
</template>


<script setup>
import { ref, nextTick } from 'vue'
import { Chart, registerables } from 'chart.js'

Chart.register(...registerables)

const rows = ref([])
const stats = ref({
  temperature_min: '--',
  temperature_max: '--',
  temperature_avg: '--',
  humidity_min: '--',
  humidity_max: '--',
  humidity_avg: '--',
  moisture_min: '--',
  moisture_max: '--',
  moisture_avg: '--'
})

const startInput = ref('')
const endInput = ref('')

const chartOneCanvas = ref(null)
const chartTwoCanvas = ref(null)

let chartOne = null
let chartTwo = null

function toUnix(datetimeString) {
  return Math.floor(new Date(datetimeString).getTime() / 1000)
}

function formatNumber(value) {
  if (value === '--' || value === null || value === undefined) return '--'
  return Number(value).toFixed(2)
}

function formatTimestamp(ts) {
  const date = new Date(ts * 1000)
  return date.toLocaleTimeString([], {
    hour: '2-digit',
    minute: '2-digit'
  })
}

function destroyCharts() {
  if (chartOne) {
    chartOne.destroy()
    chartOne = null
  }
  if (chartTwo) {
    chartTwo.destroy()
    chartTwo = null
  }
}

function renderCharts() {
  destroyCharts()

  if (!rows.value.length) return

  const labels = rows.value.map(row => formatTimestamp(row.timestamp))
  const temperatureData = rows.value.map(row => row.temperature_dht_c)
  const humidityData = rows.value.map(row => row.humidity_pct)
  const moistureData = rows.value.map(row => row.moisture_pct)
  const heatIndexData = rows.value.map(row => row.heat_index_c)

  chartOne = new Chart(chartOneCanvas.value, {
    type: 'line',
    data: {
      labels,
      datasets: [
        {
          label: 'Temperature (°C)',
          data: temperatureData,
          tension: 0.25,
          pointRadius: 1,
          borderWidth: 2
        },
        {
          label: 'Humidity (%)',
          data: humidityData,
          tension: 0.25,
          pointRadius: 1,
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

  chartTwo = new Chart(chartTwoCanvas.value, {
    type: 'line',
    data: {
      labels,
      datasets: [
        {
          label: 'Moisture (%)',
          data: moistureData,
          tension: 0.25,
          pointRadius: 1,
          borderWidth: 2
        },
        {
          label: 'Heat Index (°C)',
          data: heatIndexData,
          tension: 0.25,
          pointRadius: 1,
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

async function fetchRange() {
  try {
    if (!startInput.value || !endInput.value) {
      alert('Please choose both start and end date/time')
      return
    }

    const start = toUnix(startInput.value)
    const end = toUnix(endInput.value)

    const [rangeResponse, statsResponse] = await Promise.all([
      fetch(`http://localhost:8080/api/range/${start}/${end}`),
      fetch(`http://localhost:8080/api/stats/${start}/${end}`)
    ])

    const rangeResult = await rangeResponse.json()
    const statsResult = await statsResponse.json()

    if (rangeResult.status === 'Success') {
      rows.value = rangeResult.data
    } else {
      rows.value = []
    }

    if (statsResult.status === 'Success') {
      stats.value = statsResult.data
    } else {
      stats.value = {
        temperature_min: '--',
        temperature_max: '--',
        temperature_avg: '--',
        humidity_min: '--',
        humidity_max: '--',
        humidity_avg: '--',
        moisture_min: '--',
        moisture_max: '--',
        moisture_avg: '--'
      }
    }

    await nextTick()
    renderCharts()
  } catch (error) {
    console.error('Error fetching analysis data:', error)
    rows.value = []
    destroyCharts()
  }
}
</script>

<style scoped>
:global(body) {
  margin: 0;
  font-family: Arial, sans-serif;
  background: #eef3f7;
  color: #111827;
}

.controls {
  display: flex;
  gap: 16px;
  align-items: end;
  flex-wrap: wrap;
  margin: 20px 0;
}

.stats-grid {
  display: flex;
  flex-wrap: wrap;
  gap: 16px;
  margin: 20px 0;
  color: #111827;
}

.stat-box,
.graph-box {
  background: #ffffff;
  border-radius: 12px;
  padding: 16px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  color: #111827;
}

.graph-box {
  margin-top: 24px;
}

.chart-wrap {
  position: relative;
  width: 100%;
  height: 460px;
  overflow: hidden;
}

canvas {
  display: block;
  width: 100% !important;
  height: 100% !important;
}

pre {
  white-space: pre-wrap;
  word-break: break-word;
  max-height: 300px;
  overflow: auto;
  background: #f7f7f7;
  color: #111827;
  padding: 12px;
  border-radius: 8px;
  border: 1px solid #dbe3ea;
}
</style>

