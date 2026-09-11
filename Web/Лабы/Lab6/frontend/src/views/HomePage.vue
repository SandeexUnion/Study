<template>
    <div>
        <h1>OneB</h1>
        <p>Jumps, kicks and ultra‑violence</p>
        <img src="/resources/photo_2024-11-16_03-01-19.jpg" alt="OneB game">
        <p>How simple movements can be fun</p>

        <h2 style="text-align: center;">Games from API:</h2>
        <ul style="max-width: 600px; margin: 0 auto;">
            <li v-for="game in games" :key="game.id">
                <strong>{{ game.name }}</strong> — {{ game.genre }}
            </li>
        </ul>

        <div class="nav-links">
            <router-link to="/content">Learn more</router-link>
            <router-link to="/register">Register</router-link>
            <router-link to="/login">Log In</router-link>
        </div>
    </div>
</template>

<script>
import api from '@/services/api';

export default {
    name: 'HomePage',
    data() {
        return {
            games: []
        }
    },
    async mounted() {
        try {
            const response = await api.get('/games');
            this.games = response.data;
        } catch (err) {
            console.error('Ошибка загрузки игр:', err);
        }
    }
}
</script>

<style scoped>
</style>