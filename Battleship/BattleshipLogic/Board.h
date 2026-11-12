#pragma once
#include "GameRules.h"
#include "Ship.h"
#include <array>
#include <optional>
#include <vector>

class Board
{
public:
    void Reset();

    bool PlaceShip(ShipType type, ShipPlacement placement);
    AttackResult ReceiveAttack(CellPosition target);
    bool AreAllShipsSunk() const;

    std::optional<ShipType> GetShipTypeAt(CellPosition position) const;
    CellState GetCellState(CellPosition position, bool hideShips) const;

private:
    static constexpr uint16_t kNoShip = UINT16_MAX;

    bool HasShipAround(CellPosition position) const;

    struct Cell
    {
        uint16_t shipIndex = kNoShip;
        bool isAttacked = false;
    };

    std::array<std::array<Cell, kBoardSize>, kBoardSize> m_cells{};
    std::vector<Ship> m_ships;
};
