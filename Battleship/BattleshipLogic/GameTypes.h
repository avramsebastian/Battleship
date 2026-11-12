#pragma once
#include "GameEnums.h"

struct CellPosition
{
    uint16_t row = 0;
    uint16_t col = 0;
};

struct ShipPlacement
{
    CellPosition start;
    Orientation orientation = Orientation::Horizontal;
};
