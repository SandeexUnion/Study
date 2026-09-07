<template>
    <!-- Копируем ТОЛЬКО содержимое <body>, без <html>, <head>, <body> -->
    <div>
        <h1>OneB</h1>
        <p>Jumps, kicks and ultra‑violence</p>
        <img src="/resources/photo_2024-11-16_03-01-19.jpg" alt="">
        <p>How simple movements can be fun</p>

        <div class="nav-links">
            <!-- МЕНЯЕМ <a href=""> на <router-link to=""> -->
            <router-link to="/content">Learn more</router-link>
            <router-link to="/register">Register</router-link>
            <router-link to="/login">Log In</router-link>
        </div>
    </div>
</template>

<script>
export default {
    name: 'HomePage'
}
</script>

<style scoped>
    /* Сюда переносим стили для ЭТОЙ страницы */
    /* Берем из style.css только то, что относится к этой странице */
    
    /* Например, стили для h1 и p на этой странице: */
    h1 {
        color: #502c2c;
        text-align: center;
        font-size: 36px;
        margin-bottom: 10px;
    }

    p {
        font-size: 18px;
        line-height: 1.6;
        text-align: center;
    }

    img {
        max-width: 100%;
        height: auto;
        display: block;
        margin: 10px auto;
        border-radius: 8px;
        box-shadow: 0 4px 8px rgb(0, 0, 0);
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