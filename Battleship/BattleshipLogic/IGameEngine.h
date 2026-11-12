#pragma once
#include "BattleshipAPI.h"
#include "GameEnums.h"
#include "IAttackStrategy.h"
#include "IGameObserver.h"
#include "IShipPlacer.h"
#include <memory>
#include <optional>
#include <vector>

class IGameEngine
{
public:
    virtual ~IGameEngine() = default;

    virtual void AddObserver(IGameObserver* observer) = 0;
    virtual void RemoveObserver(IGameObserver* observer) = 0;

    virtual void StartNewGame() = 0;

    virtual AttackResult AttackCell(uint16_t row, uint16_t col) = 0;

    virtual CellState GetCellState(Side boardOwner, uint16_t row, uint16_t col) const = 0;
    virtual std::optional<ShipType> GetShipTypeAt(Side boardOwner, uint16_t row, uint16_t col) const = 0;

    virtual uint16_t GetBoardSize() const = 0;
    virtual std::vector<ShipType> GetFleet() const = 0;
    virtual GameState GetGameState() const = 0;
};

using IGameEnginePtr = std::shared_ptr<IGameEngine>;
using IShipPlacerPtr = std::unique_ptr<IShipPlacer>;
using IAttackStrategyPtr = std::unique_ptr<IAttackStrategy>;

BATTLESHIP_API IGameEnginePtr CreateGameEngine();
BATTLESHIP_API IGameEnginePtr CreateGameEngine(
    IShipPlacerPtr shipPlacer,
    IAttackStrategyPtr aiStrategy);
