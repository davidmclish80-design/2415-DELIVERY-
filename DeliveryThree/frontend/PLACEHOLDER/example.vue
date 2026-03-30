<template>
  <div class="app">
    <h1>Weather Station Dashboard</h1>
    <p>Frontend is running.</p>

    <div class="links">
      <button @click="page = 'live'">Live</button>
      <button @click="page = 'analysis'">Analysis</button>
    </div>

    <div v-if="page === 'live'">
      <h2>Live Page</h2>
      <p>This is where live readings will go.</p>
    </div>

    <div v-else>
      <h2>Analysis Page</h2>
      <p>This is where min, max, avg and graphs will go.</p>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'

const page = ref('live')
</script>

<style>
body {
  margin: 0;
  font-family: Arial, sans-serif;
  background: #1fbdd9;
}

.app {
  padding: 24px;
}

.links {
  margin: 16px 0;
  display: flex;
  gap: 12px;
}

button {
  padding: 10px 16px;
  border: none;
  background: #0b19d5;
  color: white;
  border-radius: 8px;
  cursor: pointer;
}
</style>