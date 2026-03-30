<template>
  <div class="card">
    <h3>{{ title }}</h3>
    <p class="value">{{ value }}</p>
  </div>
</template>

<script setup>
defineProps({
  title: String,
  value: [String, Number]
})
</script>

<style scoped>
.card {
  background: white;
  border-radius: 12px;
  padding: 16px;
  min-width: 180px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
}

.value {
  font-size: 1.2rem;
  font-weight: bold;
}
</style>