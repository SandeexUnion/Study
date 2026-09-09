<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class UserController extends Controller
{
    public function register(Request $request)
    {
        $data = $request->all();

        return response()->json([
            'message' => 'Registration successful!',
            'user' => [
                'name' => $data['name'] ?? 'Unknown',
                'email' => $data['email'] ?? 'No email'
            ]
        ], 201);
    }

    public function login(Request $request)
    {
        $data = $request->all();

        if (empty($data['email']) || empty($data['password'])) {
            return response()->json([
                'message' => 'Email and password are required'
            ], 400);
        }

        return response()->json([
            'message' => 'Login successful!',
            'user' => [
                'email' => $data['email'],
                'token' => 'fake-jwt-token-12345'
            ]
        ]);
    }
}