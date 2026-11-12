#pragma once
#include "IAttackStrategy.h"
#include <random>
#include <vector>

class RandomAttackStrategy : public IAttackStrategy
{
public:
    explicit RandomAttackStrategy(unsigned int seed = std::random_device{}());

    void Reset() override;
    CellPosition ChooseTarget() override;

private:
    std::mt19937 m_randomEngine;
    std::vector<CellPosition> m_remainingTargets;
};
