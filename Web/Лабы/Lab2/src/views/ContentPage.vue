<template>
    <div>
        <div class="container">
        <p>Just as an artist starts with white paper, a player starts with "white room":</p>
        <img src="/resources/tutorial.jpg" alt="">

        <p>But the movement isn’t limited to just the legs:</p>
        <img src="/resources/swing.jpg" alt=""> 

        <p>And what kind of game is it if it doesn’t offer any variety in platforming?</p>
        <img src="/resources/platform.jpg" alt="">

        <p>As they say, in violence we trust.</p>
        <img src="/resources/violence.jpg" alt="">

        <p>And who asked about the small number of genres?</p>
        <img src="/resources/horror.jpg" alt="">

        <p>Here some silly animations:</p>
        <img src="/resources/punch.gif" alt="">
        </div>
        <div class="nav-links">
            <router-link to ="/">Back</router-link>
        </div>

    </div>
</template>

<script>
    export default {
        name: 'ContentPage'
    }
</script>

<style scoped>
    /* Стили для этой страницы */
    .container {
        max-width: 800px;
        margin: 0 auto;
        padding: 20px;
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