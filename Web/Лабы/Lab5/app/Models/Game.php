<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class Game extends Model
{
    protected $fillable = [
        'name',
        'genre',
        'description'
    ];
    public function feedbacks()
    {
        return $this->hasMany(Feedback::class);
    }
}
