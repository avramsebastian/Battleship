#include "RandomAttackStrategy.h"
#include "GameRules.h"
#include <utility>

RandomAttackStrategy::RandomAttackStrategy(unsigned int seed)
    : m_randomEngine(seed)
{
    Reset();
}

void RandomAttackStrategy::Reset()
{
    m_remainingTargets.clear();
    for (uint16_t row = 0; row < kBoardSize; ++row)
    {
        for (uint16_t col = 0; col < kBoardSize; ++col)
        {
            m_remainingTargets.push_back({ row, col });
        }
    }
}

CellPosition RandomAttackStrategy::ChooseTarget()
{
    std::uniform_int_distribution<size_t> pickIndex(0, m_remainingTargets.size() - 1);
    const size_t index = pickIndex(m_randomEngine);

    const CellPosition target = m_remainingTargets[index];
    std::swap(m_remainingTargets[index], m_remainingTargets.back());
    m_remainingTargets.pop_back();
    return target;
}
