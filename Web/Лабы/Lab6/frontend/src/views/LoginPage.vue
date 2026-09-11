<template>
    <div>
        <h1>Authorization</h1>
        
        <form @submit.prevent="handleLogin">
            <label for="email">Email:</label><br>
            <input type="email" id="email" v-model="loginData.email" placeholder="Enter email" required><br><br>
            
            <label for="password">Password:</label><br>
            <input type="password" id="password" v-model="loginData.password" placeholder="Enter password" required><br><br>
            
            <button type="submit" :disabled="loading">
                {{ loading ? 'Sending...' : 'Log In' }}
            </button>
        </form>

        <p v-if="message" style="color: green; text-align: center;">{{ message }}</p>
        <p v-if="error" style="color: red; text-align: center;">{{ error }}</p>
        
        <div class="nav-links">
            <p>Don't have an account? <router-link to="/register">Register</router-link></p>
            <router-link to="/">Back to main</router-link>
        </div>
    </div>
</template>

<script>
import api from '@/services/api';

export default {
    name: 'LoginPage',
    data() {
        return {
            loginData: {
                email: '',
                password: ''
            },
            loading: false,
            message: '',
            error: ''
        }
    },
    methods: {
        async handleLogin() {
            this.loading = true;
            this.message = '';
            this.error = '';

            try {
                const response = await api.post('/login', this.loginData);
                this.message = response.data.message;
                if (response.data.token) {
                    localStorage.setItem('token', response.data.token);
                    localStorage.setItem('user', JSON.stringify(response.data.user));
                }
            } catch (err) {
                if (err.response) {
                    this.error = err.response.data.message || 'Ошибка входа';
                } else {
                    this.error = 'Сервер недоступен';
                }
            } finally {
                this.loading = false;
            }
        }
    }
}
</script>

<style scoped>
h1 { color: #502c2c; text-align: center; font-size: 36px; margin-bottom: 10px; }
form { max-width: 400px; margin: 20px auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); }
input { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px; font-size: 16px; box-sizing: border-box; }
button { width: 100%; padding: 12px; background-color: #440606; color: white; border: none; border-radius: 5px; font-size: 18px; font-weight: bold; cursor: pointer; }
button:disabled { background-color: #999; cursor: not-allowed; }
.nav-links { text-align: center; margin-top: 30px; }
.nav-links a { margin: 0 15px; padding: 10px 20px; background-color: #440606; color: white; border-radius: 5px; text-decoration: none; display: inline-block; }
</style>