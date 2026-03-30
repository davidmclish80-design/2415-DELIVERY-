<template>
  <div class="app">
    <h1>Weather Station Dashboard</h1>

    <div class="nav-buttons">
      <button :class="{ active: page === 'live' }" @click="page = 'live'">Live</button>
      <button :class="{ active: page === 'analysis' }" @click="page = 'analysis'">Analysis</button>
    </div>

    <Live v-if="page === 'live'" />
    <Analysis v-else />
  </div>
</template>

<script setup>
import { ref } from 'vue'
import Live from './views/Live.vue'
import Analysis from './views/Analysis.vue'

const page = ref('live')
</script>

<style scoped>
:global(body) {
  margin: 0;
  font-family: Arial, sans-serif;
  background: #eef3f7;
  color: #111827;
}

.app {
  padding: 24px;
}

.nav-buttons {
  display: flex;
  gap: 12px;
  margin: 20px 0;
}

button {
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

button:hover {
  background: #dbeafe;
  border-color: #93c5fd;
}

button.active {
  background: #2563eb;
  color: white;
  border-color: #2563eb;
}
</style>