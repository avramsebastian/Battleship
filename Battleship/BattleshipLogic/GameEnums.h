#pragma once
#include <cstdint>

enum class Side : uint8_t
{
    Human,
    AI
};

enum class CellState : uint8_t
{
    Unknown,
    Empty,
    Ship,
    Hit,
    Sunk,
    Miss
};

enum class AttackResult : uint8_t
{
    Miss,
    Hit,
    Sunk,
    InvalidAttack
};

enum class GameState : uint8_t
{
    NotStarted,
    HumanTurn,
    AITurn,
    HumanWon,
    AIWon
};

enum class ShipType : uint8_t
{
    Carrier,
    Battleship,
    Cruiser,
    Submarine,
    Destroyer
};

enum class Orientation : uint8_t
{
    Horizontal,
    Vertical
};
