#pragma once
#include "GameTypes.h"

class IShipPlacer
{
public:
    virtual ~IShipPlacer() = default;

    virtual ShipPlacement ChoosePlacement(ShipType shipType) = 0;
};
