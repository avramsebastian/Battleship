#include "Ship.h"

Ship::Ship(ShipType type)
    : m_type(type)
{
}

ShipType Ship::GetType() const
{
    return m_type;
}

void Ship::RegisterHit()
{
    ++m_hitCount;
}

bool Ship::IsSunk() const
{
    return m_hitCount >= GetShipLength(m_type);
}
