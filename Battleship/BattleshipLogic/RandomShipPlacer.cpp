#include "RandomShipPlacer.h"
#include "GameRules.h"

RandomShipPlacer::RandomShipPlacer(unsigned int seed)
    : m_randomEngine(seed)
{
}

ShipPlacement RandomShipPlacer::ChoosePlacement(ShipType shipType)
{
    std::uniform_int_distribution<uint16_t> acrossShip(0, kBoardSize - 1);
    std::uniform_int_distribution<uint16_t> alongShip(0, kBoardSize - GetShipLength(shipType));

    const bool isHorizontal = m_randomEngine() % 2 == 0;
    if (isHorizontal)
    {
        return { { acrossShip(m_randomEngine), alongShip(m_randomEngine) }, Orientation::Horizontal };
    }
    return { { alongShip(m_randomEngine), acrossShip(m_randomEngine) }, Orientation::Vertical };
}
