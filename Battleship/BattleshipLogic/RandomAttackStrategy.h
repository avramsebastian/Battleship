#pragma once
#include "GameTypes.h"
#include "IAttackStrategy.h"
#include <random>
#include <vector>

class RandomAttackStrategy : public IAttackStrategy
{
public:
    explicit RandomAttackStrategy(unsigned int seed = std::random_device{}());

    void Reset() override;
    void ChooseTarget(uint16_t& row, uint16_t& col) override;

private:
    std::mt19937 m_randomEngine;
    std::vector<CellPosition> m_remainingTargets;
};
