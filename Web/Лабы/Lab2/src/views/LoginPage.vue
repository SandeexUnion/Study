<template>
    <div>
        <h1>Authorization</h1>
        
        <form @submit.prevent="handleLogin">
            <label for="login">Login or Email:</label><br>
            <input type="text" id="login" v-model="loginData.login" placeholder="Enter login or email" required><br><br>
            
            <label for="password">Password:</label><br>
            <input type="password" id="password" v-model="loginData.password" placeholder="Enter password" required><br><br>
            
            <button type="submit">Log In</button>
        </form>
        
        <div class="nav-links">
            <p>Don't have an account? <router-link to="/register">Register</router-link></p>
            <router-link to="/">Back to main</router-link>
        </div>
    </div>
</template>

<script>
export default {
    name: 'LoginPage',
    data() {
        return {
            loginData: {
                login: '',
                password: ''
            }
        }
    },
    methods: {
        handleLogin() {
            console.log('Вход:', this.loginData);
            alert('Вы вошли! (пока что только в консоли)');
        }
    }
}
</script>

<style scoped>
    h1 {
        color: #502c2c;
        text-align: center;
        font-size: 36px;
        margin-bottom: 10px;
    }

    form {
        max-width: 400px;
        margin: 20px auto;
        background: white;
        padding: 30px;
        border-radius: 10px;
        box-shadow: 0 4px 10px rgba(0,0,0,0.1);
    }

    input {
        width: 100%;
        padding: 10px;
        border: 1px solid #ddd;
        border-radius: 5px;
        font-size: 16px;
        box-sizing: border-box;
    }

    input:focus {
        border-color: #310404;
        outline: none;
        box-shadow: 0 0 5px rgba(52,152,219,0.3);
    }

    button {
        width: 100%;
        padding: 12px;
        background-color: #440606;
        color: white;
        border: none;
        border-radius: 5px;
        font-size: 18px;
        font-weight: bold;
        cursor: pointer;
        transition: background-color 0.3s;
    }

    button:hover {
        background-color: #610909;
    }

    .nav-links {
        text-align: center;
        margin-top: 30px;
    }

    .nav-links a {
        margin: 0 15px;
        padding: 10px 20px;
        background-color: #440606;
        color: white;
        border-radius: 5px;
        text-decoration: none;
    }

    .nav-links a:hover {
        background-color: #610909;
        text-decoration: none;
    }
</style>