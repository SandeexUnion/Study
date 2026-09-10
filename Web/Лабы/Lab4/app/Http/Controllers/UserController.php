<?php

namespace App\Http\Controllers;

use App\Models\User;
use Illuminate\Http\Request;

class UserController extends Controller
{
    public function register(Request $request)
    {
        $data = $request->all();
        $existing = User::where('email', $data['email'])->first();
        if ($existing) {
            return response()->json(['message' => 'Email already taken'], 400);
        }
        $user = User::create([
            'name' => $data['name'],
            'email' => $data['email'],
            'password' => bcrypt($data['password']),
        ]);

        return response()->json([
            'message' => 'Registration successful!',
            'user' => $user
        ], 201);
    }
    public function login(Request $request)
    {
        $data = $request->all();

        $user = User::where('email', $data['email'])->first();

        if (!$user) {
            return response()->json(['message' => 'User not found'], 404);
        }
        if (!password_verify($data['password'], $user->password)) {
            return response()->json(['message' => 'Wrong password'], 401);
        }

        return response()->json([
            'message' => 'Login successful!',
            'user' => $user,
            'token' => 'fake-jwt-token-' . $user->id
        ]);
    }
}
