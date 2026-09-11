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
    public function update(Request $request, $id)
    {
        $feedback = Feedback::find($id);

        if (!$feedback) {
            return response()->json(['message' => 'Feedback not found'], 404);
        }

        $feedback->update([
            'user_id' => $request->user_id,
            'game_id' => $request->game_id,
            'message' => $request->message,
        ]);

        return response()->json($feedback);
    }
    public function destroy($id)
    {
        $feedback = Feedback::find($id);

        if (!$feedback) {
            return response()->json(['message' => 'Feedback not found'], 404);
        }

        $feedback->delete();

        return response()->json(['message' => 'Feedback deleted successfully']);
    }
    public function show($id)
    {
        $feedback = Feedback::with(['user', 'game'])->find($id);

        if (!$feedback) {
            return response()->json(['message' => 'Feedback not found'], 404);
        }

        return response()->json($feedback);
    }
}
