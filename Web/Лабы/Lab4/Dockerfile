FROM php:8.4-cli

RUN apt-get update && apt-get install -y \
    zip \
    unzip \
    git \
    curl \
    libonig-dev \
    libxml2-dev \
    libpng-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

RUN docker-php-ext-install \
    pdo_sqlite \
    pdo_mysql \
    mbstring \
    bcmath \
    exif \
    pcntl \
    gd

WORKDIR /var/www/html

COPY --from=composer:latest /usr/bin/composer /usr/bin/composer

COPY . /var/www/html

RUN chown -R www-data:www-data /var/www/html \
    && chmod -R 755 /var/www/html/storage \
    && chmod -R 755 /var/www/html/bootstrap/cache

RUN composer install --no-interaction --no-progress

EXPOSE 8000

CMD ["php", "artisan", "serve", "--host=0.0.0.0", "--port=8000"]
