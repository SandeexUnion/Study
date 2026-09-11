<?php

use App\Http\Controllers\GameController;
use App\Http\Controllers\UserController;
use App\Http\Controllers\FeedbackController;
use Illuminate\Support\Facades\Route;

Route::get('/api/v1/games', [GameController::class, 'index']);
Route::get('/api/v1/games/{id}', [GameController::class, 'show']);
Route::post('/api/v1/games', [GameController::class, 'store']);
Route::put('/api/v1/games/{id}', [GameController::class, 'update']);
Route::delete('/api/v1/games/{id}', [GameController::class, 'destroy']);

Route::post('/api/v1/register', [UserController::class, 'register']);
Route::post('/api/v1/login', [UserController::class, 'login']);
Route::put('/api/v1/users/{id}', [UserController::class, 'update']);
Route::delete('/api/v1/users/{id}', [UserController::class, 'destroy']);
Route::get('/api/v1/users/{id}', [UserController::class, 'show']);

Route::get('/api/v1/feedbacks', [FeedbackController::class, 'index']);
Route::post('/api/v1/feedbacks', [FeedbackController::class, 'store']);
Route::put('/api/v1/feedbacks/{id}', [FeedbackController::class, 'update']);
Route::delete('/api/v1/feedbacks/{id}', [FeedbackController::class, 'destroy']);
