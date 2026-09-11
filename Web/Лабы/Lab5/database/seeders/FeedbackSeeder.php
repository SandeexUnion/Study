<?php

namespace Database\Seeders;

use App\Models\Feedback;
use Illuminate\Database\Seeder;

class FeedbackSeeder extends Seeder
{
    public function run(): void
    {
        $feedbacks = [
            [
                'user_id' => 1,
                'game_id' => 1,
                'message' => 'Отличная игра! Очень динамичная.',
            ],
            [
                'user_id' => 2,
                'game_id' => 2,
                'message' => 'Страшная, но интересная.',
            ]
        ];

        foreach ($feedbacks as $feedback) {
            Feedback::create($feedback);
        }
    }
}
