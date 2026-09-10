<?php

namespace Database\Seeders;

use App\Models\Game;
use Illuminate\Database\Seeder;

class GameSeeder extends Seeder
{
    public function run(): void
    {
        $games = [
            [
                'name' => 'OneB',
                'genre' => 'Action / Platformer',
                'description' => 'Jumps, kicks and ultra-violence'
            ],
            [
                'name' => 'MonsterHorror',
                'genre' => 'FriendSlop',
                'description' => 'Its your turn to scare monster'
            ],
            [
                'name' => 'Paralogica',
                'genre' => 'Horror',
                'description' => 'Horror about mental illnes'
            ]
        ];

        foreach ($games as $game) {
            Game::create($game);
        }
    }
}
