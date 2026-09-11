<?php

namespace Database\Seeders;

use App\Models\User;
use Illuminate\Database\Seeder;

class UserSeeder extends Seeder
{
    public function run(): void
    {
        $users = [
            [
                'name' => 'Admin',
                'email' => 'admin@mail.com',
                'password' => bcrypt('password123'),
            ],
            [
                'name' => 'John',
                'email' => 'john@mail.com',
                'password' => bcrypt('qwerty'),
            ]
        ];

        foreach ($users as $user) {
            User::create($user);
        }
    }
}
