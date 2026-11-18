#include "RandomShipPlacer.h"
#include "GameRules.h"

RandomShipPlacer::RandomShipPlacer(unsigned int seed)
    : m_randomEngine(seed)
{
}

void RandomShipPlacer::ChoosePlacement(ShipType shipType, uint16_t& row, uint16_t& col, Orientation& orientation)
{
    std::uniform_int_distribution<uint16_t> acrossShip(0, kBoardSize - 1);
    std::uniform_int_distribution<uint16_t> alongShip(0, kBoardSize - GetShipLength(shipType));

    const bool isHorizontal = m_randomEngine() % 2 == 0;
    orientation = isHorizontal ? Orientation::Horizontal : Orientation::Vertical;
    row = isHorizontal ? acrossShip(m_randomEngine) : alongShip(m_randomEngine);
    col = isHorizontal ? alongShip(m_randomEngine) : acrossShip(m_randomEngine);
}
