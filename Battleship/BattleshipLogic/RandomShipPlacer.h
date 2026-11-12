#pragma once
#include "IShipPlacer.h"
#include <random>

class RandomShipPlacer : public IShipPlacer
{
public:
    explicit RandomShipPlacer(unsigned int seed = std::random_device{}());

    ShipPlacement ChoosePlacement(ShipType shipType) override;

private:
    std::mt19937 m_randomEngine;
};
