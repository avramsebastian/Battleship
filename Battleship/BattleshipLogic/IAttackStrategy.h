#pragma once
#include "GameEnums.h"

class IAttackStrategy
{
public:
    virtual ~IAttackStrategy() = default;

    virtual void Reset() = 0;
    virtual void ChooseTarget(uint16_t& row, uint16_t& col) = 0;
};
