#pragma once
#include "GameEnums.h"

class IGameObserver
{
public:
    virtual ~IGameObserver() = default;

    virtual void OnGameStarted() = 0;

    virtual void OnAttackResolved(Side attacker, uint16_t row, uint16_t col, AttackResult result) = 0;

    virtual void OnShipSunk(Side owner, ShipType shipType) = 0;

    virtual void OnTurnChanged(Side currentSide) = 0;

    virtual void OnGameOver(Side winner) = 0;
};
