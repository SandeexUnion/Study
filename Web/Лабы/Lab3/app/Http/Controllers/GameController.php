<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class GameController extends Controller
{
    // Метод для списка всех игр
    public function index()
    {
        $games = [
            [
                'id' => 1,
                'name' => 'OneB',
                'genre' => 'Action / Platformer',
                'description' => 'Jumps, kicks and ultra-violence'
            ],
            [
                'id' => 2,
                'name' => 'MonsterHorror',
                'genre' => 'FriendSlop',
                'description' => 'Its your turn to scare monster'
            ],
            [
                'id' => 3,
                'name' => 'Paralogica',
                'genre' => 'Horror',
                'description' => 'Horror about mental illnes'
            ]
        ];

        return response()->json($games);
    }

    public function show($id)
    {
         $games = [
            [
                'id' => 1,
                'name' => 'OneB',
                'genre' => 'Action / Platformer',
                'description' => 'Jumps, kicks and ultra-violence'
            ],
            [
                'id' => 2,
                'name' => 'MonsterHorror',
                'genre' => 'FriendSlop',
                'description' => 'Its your turn to scare monster'
            ],
            [
                'id' => 3,
                'name' => 'Paralogica',
                'genre' => 'Horror',
                'description' => 'Horror about mental illnes'
            ]
        ];
        if (!isset($games[$id])) {
            return response()->json(['message' => 'Game not found'], 404);
        }

        return response()->json($games[$id]);
    }
}