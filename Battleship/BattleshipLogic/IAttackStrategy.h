#pragma once
#include "GameTypes.h"

class IAttackStrategy
{
public:
    virtual ~IAttackStrategy() = default;

    virtual void Reset() = 0;
    virtual CellPosition ChooseTarget() = 0;
};
