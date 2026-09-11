<?php

namespace App\Http\Controllers;

use App\Models\Game;
use Illuminate\Http\Request;

class GameController extends Controller
{
    public function index()
    {
        $games = Game::all();
        return response()->json($games);
    }
    public function show($id)
    {
        $game = Game::find($id);

        if (!$game) {
            return response()->json(['message' => 'Game not found'], 404);
        }

        return response()->json($game);
    }
    public function store(Request $request)
    {
        $game = Game::create([
            'name' => $request->name,
            'genre' => $request->genre,
            'description' => $request->description,
        ]);

        return response()->json($game, 201);
    }
    public function update(Request $request, $id)
    {
        $game = Game::find($id);

        if (!$game) {
            return response()->json(['message' => 'Game not found'], 404);
        }

        $game->update([
            'name' => $request->name,
            'genre' => $request->genre,
            'description' => $request->description,
        ]);

        return response()->json($game);
    }
    public function destroy($id)
    {
        $game = Game::find($id);

        if (!$game) {
            return response()->json(['message' => 'Game not found'], 404);
        }

        $game->delete();

        return response()->json(['message' => 'Game deleted successfully']);
    }
}
