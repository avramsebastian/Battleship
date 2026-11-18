#pragma once
#include "IShipPlacer.h"
#include <random>

class RandomShipPlacer : public IShipPlacer
{
public:
    explicit RandomShipPlacer(unsigned int seed = std::random_device{}());

    void ChoosePlacement(ShipType shipType, uint16_t& row, uint16_t& col, Orientation& orientation) override;

private:
    std::mt19937 m_randomEngine;
};
