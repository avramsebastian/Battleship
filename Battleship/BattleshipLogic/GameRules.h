#pragma once
#include "GameTypes.h"
#include <array>
#include <stdexcept>

constexpr uint16_t kBoardSize = 10;

constexpr std::array<ShipType, 5> kFleet = {
    ShipType::Carrier,
    ShipType::Battleship,
    ShipType::Cruiser,
    ShipType::Submarine,
    ShipType::Destroyer
};

constexpr uint16_t GetShipLength(ShipType shipType)
{
    switch (shipType)
    {
    case ShipType::Carrier:    return 5;
    case ShipType::Battleship: return 4;
    case ShipType::Cruiser:    return 3;
    case ShipType::Submarine:  return 3;
    case ShipType::Destroyer:  return 2;
    }
    throw std::invalid_argument("Unknown ship type");
}

constexpr bool IsInsideBoard(CellPosition position)
{
    return position.row < kBoardSize && position.col < kBoardSize;
}

constexpr Side GetOpponent(Side side)
{
    return side == Side::Human ? Side::AI : Side::Human;
}
