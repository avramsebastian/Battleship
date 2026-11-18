#pragma once
#include "GameEnums.h"

class IShipPlacer
{
public:
    virtual ~IShipPlacer() = default;

    virtual void ChoosePlacement(ShipType shipType, uint16_t& row, uint16_t& col, Orientation& orientation) = 0;
};
