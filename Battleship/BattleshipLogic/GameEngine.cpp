#include "GameEngine.h"
#include "RandomAttackStrategy.h"
#include "RandomShipPlacer.h"
#include <algorithm>

GameEngine::GameEngine(std::unique_ptr<IShipPlacer> shipPlacer, std::unique_ptr<IAttackStrategy> aiStrategy)
    : m_shipPlacer(std::move(shipPlacer))
    , m_aiStrategy(std::move(aiStrategy))
{
}

void GameEngine::AddObserver(IGameObserver* observer)
{
    if (observer != nullptr && std::ranges::find(m_observers, observer) == m_observers.end())
    {
        m_observers.push_back(observer);
    }
}

void GameEngine::RemoveObserver(IGameObserver* observer)
{
    std::erase(m_observers, observer);
}

void GameEngine::StartNewGame()
{
    m_humanBoard.Reset();
    m_aiBoard.Reset();
    PlaceFleet(m_humanBoard);
    PlaceFleet(m_aiBoard);
    m_aiStrategy->Reset();

    m_state = GameState::HumanTurn;
    NotifyObservers([](IGameObserver& observer) { observer.OnGameStarted(); });
    NotifyObservers([](IGameObserver& observer) { observer.OnTurnChanged(Side::Human); });
}

AttackResult GameEngine::AttackCell(uint16_t row, uint16_t col)
{
    if (m_state != GameState::HumanTurn)
    {
        return AttackResult::InvalidAttack;
    }

    const AttackResult result = ExecuteAttack(Side::Human, m_aiBoard, { row, col });
    if (m_state == GameState::AITurn)
    {
        ExecuteAttack(Side::AI, m_humanBoard, m_aiStrategy->ChooseTarget());
    }
    return result;
}

CellState GameEngine::GetCellState(Side boardOwner, uint16_t row, uint16_t col) const
{
    return BoardOf(boardOwner).GetCellState({ row, col }, boardOwner == Side::AI && !IsGameOver());
}

std::optional<ShipType> GameEngine::GetShipTypeAt(Side boardOwner, uint16_t row, uint16_t col) const
{
    const CellState state = GetCellState(boardOwner, row, col);
    const bool isShipVisible = state == CellState::Ship || state == CellState::Sunk
        || (state == CellState::Hit && (boardOwner == Side::Human || IsGameOver()));
    if (!isShipVisible)
    {
        return std::nullopt;
    }
    return BoardOf(boardOwner).GetShipTypeAt({ row, col });
}

uint16_t GameEngine::GetBoardSize() const
{
    return kBoardSize;
}

std::vector<ShipType> GameEngine::GetFleet() const
{
    return { kFleet.begin(), kFleet.end() };
}

GameState GameEngine::GetGameState() const
{
    return m_state;
}

const Board& GameEngine::BoardOf(Side boardOwner) const
{
    return boardOwner == Side::Human ? m_humanBoard : m_aiBoard;
}

bool GameEngine::IsGameOver() const
{
    return m_state == GameState::HumanWon || m_state == GameState::AIWon;
}

void GameEngine::PlaceFleet(Board& board)
{
    for (const ShipType shipType : kFleet)
    {
        while (!board.PlaceShip(shipType, m_shipPlacer->ChoosePlacement(shipType)))
        {
        }
    }
}

AttackResult GameEngine::ExecuteAttack(Side attacker, Board& targetBoard, CellPosition target)
{
    const AttackResult result = targetBoard.ReceiveAttack(target);
    if (result == AttackResult::InvalidAttack)
    {
        return result;
    }

    const Side defender = GetOpponent(attacker);
    NotifyObservers([&](IGameObserver& observer) {
        observer.OnAttackResolved(attacker, target.row, target.col, result);
    });

    if (result == AttackResult::Sunk)
    {
        const ShipType sunkShip = *targetBoard.GetShipTypeAt(target);
        NotifyObservers([&](IGameObserver& observer) { observer.OnShipSunk(defender, sunkShip); });
    }

    if (targetBoard.AreAllShipsSunk())
    {
        m_state = attacker == Side::Human ? GameState::HumanWon : GameState::AIWon;
        NotifyObservers([&](IGameObserver& observer) { observer.OnGameOver(attacker); });
        return result;
    }

    m_state = defender == Side::Human ? GameState::HumanTurn : GameState::AITurn;
    NotifyObservers([&](IGameObserver& observer) { observer.OnTurnChanged(defender); });
    return result;
}

void GameEngine::NotifyObservers(const std::function<void(IGameObserver&)>& notification)
{
    for (IGameObserver* observer : m_observers)
    {
        notification(*observer);
    }
}

IGameEnginePtr CreateGameEngine()
{
    return CreateGameEngine(std::make_unique<RandomShipPlacer>(), std::make_unique<RandomAttackStrategy>());
}

IGameEnginePtr CreateGameEngine(
    IShipPlacerPtr shipPlacer,
    IAttackStrategyPtr aiStrategy)
{
    return std::make_shared<GameEngine>(std::move(shipPlacer), std::move(aiStrategy));
}
