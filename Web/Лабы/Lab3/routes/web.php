<?php

use App\Http\Controllers\GameController;
use App\Http\Controllers\UserController;
use Illuminate\Support\Facades\Route;

Route::get('/games', [GameController::class, 'index']);
Route::get('/game/{id}', [GameController::class, 'show']);

Route::post('/reg', [UserController::class,'register']);
Route::post('/login', [UserController::class,'login']);