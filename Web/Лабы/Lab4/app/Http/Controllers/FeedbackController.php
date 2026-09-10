<?php

namespace App\Http\Controllers;

use App\Models\Feedback;
use Illuminate\Http\Request;

class FeedbackController extends Controller
{
        public function index()
    {
        $feedbacks = Feedback::with(['user', 'game'])->get();
        return response()->json($feedbacks);
    }
    public function store(Request $request)
    {
        $feedback = Feedback::create([
            'user_id' => $request->user_id,
            'game_id' => $request->game_id,
            'message' => $request->message,
        ]);

        return response()->json($feedback, 201);
    }
}
