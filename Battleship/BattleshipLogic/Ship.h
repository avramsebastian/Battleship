#pragma once
#include "GameRules.h"

class Ship
{
public:
    explicit Ship(ShipType type);

    ShipType GetType() const;
    void RegisterHit();
    bool IsSunk() const;

private:
    ShipType m_type;
    uint16_t m_hitCount = 0;
};
