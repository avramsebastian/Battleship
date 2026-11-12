#include "Board.h"
#include <algorithm>
#include <stdexcept>

void Board::Reset()
{
    m_cells = {};
    m_ships.clear();
}

bool Board::PlaceShip(ShipType type, ShipPlacement placement)
{
    std::vector<CellPosition> shipCells;
    for (uint16_t offset = 0; offset < GetShipLength(type); ++offset)
    {
        CellPosition position = placement.start;
        if (placement.orientation == Orientation::Horizontal)
        {
            position.col += offset;
        }
        else
        {
            position.row += offset;
        }

        if (!IsInsideBoard(position) || HasShipAround(position))
        {
            return false;
        }
        shipCells.push_back(position);
    }

    const uint16_t shipIndex = static_cast<uint16_t>(m_ships.size());
    for (const CellPosition position : shipCells)
    {
        m_cells[position.row][position.col].shipIndex = shipIndex;
    }
    m_ships.emplace_back(type);
    return true;
}

AttackResult Board::ReceiveAttack(CellPosition target)
{
    if (!IsInsideBoard(target) || m_cells[target.row][target.col].isAttacked)
    {
        return AttackResult::InvalidAttack;
    }

    Cell& cell = m_cells[target.row][target.col];
    cell.isAttacked = true;
    if (cell.shipIndex == kNoShip)
    {
        return AttackResult::Miss;
    }

    Ship& ship = m_ships[cell.shipIndex];
    ship.RegisterHit();
    return ship.IsSunk() ? AttackResult::Sunk : AttackResult::Hit;
}

bool Board::AreAllShipsSunk() const
{
    return std::ranges::all_of(m_ships, &Ship::IsSunk);
}

std::optional<ShipType> Board::GetShipTypeAt(CellPosition position) const
{
    const uint16_t shipIndex = m_cells[position.row][position.col].shipIndex;
    if (shipIndex == kNoShip)
    {
        return std::nullopt;
    }
    return m_ships[shipIndex].GetType();
}

bool Board::HasShipAround(CellPosition position) const
{
    for (int row = position.row - 1; row <= position.row + 1; ++row)
    {
        for (int col = position.col - 1; col <= position.col + 1; ++col)
        {
            const CellPosition neighbor{ static_cast<uint16_t>(row), static_cast<uint16_t>(col) };
            if (IsInsideBoard(neighbor) && m_cells[neighbor.row][neighbor.col].shipIndex != kNoShip)
            {
                return true;
            }
        }
    }
    return false;
}

CellState Board::GetCellState(CellPosition position, bool hideShips) const
{
    if (!IsInsideBoard(position))
    {
        throw std::out_of_range("Cell position is outside the board");
    }

    const Cell& cell = m_cells[position.row][position.col];
    const bool hasShip = cell.shipIndex != kNoShip;

    if (cell.isAttacked && !hasShip)
    {
        return CellState::Miss;
    }
    if (cell.isAttacked)
    {
        return m_ships[cell.shipIndex].IsSunk() ? CellState::Sunk : CellState::Hit;
    }
    if (hideShips)
    {
        return CellState::Unknown;
    }
    return hasShip ? CellState::Ship : CellState::Empty;
}
