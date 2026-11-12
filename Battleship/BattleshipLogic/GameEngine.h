#pragma once
#include "Board.h"
#include "IAttackStrategy.h"
#include "IGameEngine.h"
#include "IShipPlacer.h"
#include <functional>
#include <memory>
#include <vector>

class GameEngine : public IGameEngine
{
public:
    GameEngine(std::unique_ptr<IShipPlacer> shipPlacer, std::unique_ptr<IAttackStrategy> aiStrategy);

    void AddObserver(IGameObserver* observer) override;
    void RemoveObserver(IGameObserver* observer) override;

    void StartNewGame() override;
    AttackResult AttackCell(uint16_t row, uint16_t col) override;

    CellState GetCellState(Side boardOwner, uint16_t row, uint16_t col) const override;
    std::optional<ShipType> GetShipTypeAt(Side boardOwner, uint16_t row, uint16_t col) const override;
    uint16_t GetBoardSize() const override;
    std::vector<ShipType> GetFleet() const override;
    GameState GetGameState() const override;

private:
    const Board& BoardOf(Side boardOwner) const;
    bool IsGameOver() const;
    void PlaceFleet(Board& board);
    AttackResult ExecuteAttack(Side attacker, Board& targetBoard, CellPosition target);
    void NotifyObservers(const std::function<void(IGameObserver&)>& notification);

    IShipPlacerPtr m_shipPlacer;
    IAttackStrategyPtr m_aiStrategy;
    Board m_humanBoard;
    Board m_aiBoard;
    GameState m_state = GameState::NotStarted;
    std::vector<IGameObserver*> m_observers;
};
