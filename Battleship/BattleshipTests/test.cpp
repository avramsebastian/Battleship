#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "IGameEngine.h"
#include "IGameObserver.h"
#include "GameEnums.h"
#include "GameTypes.h"
#include "GameRules.h"
#include "IShipPlacer.h"
#include "IAttackStrategy.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::StrictMock;

// Deterministic ship placer for testing exact board layouts.
class FixedShipPlacer : public IShipPlacer
{
public:
    ShipPlacement ChoosePlacement(ShipType shipType) override
    {
        switch (shipType)
        {
        case ShipType::Carrier:    return { {0, 0}, Orientation::Horizontal }; // (0,0)..(0,4)
        case ShipType::Battleship: return { {2, 0}, Orientation::Horizontal }; // (2,0)..(2,3)
        case ShipType::Cruiser:    return { {4, 0}, Orientation::Horizontal }; // (4,0)..(4,2)
        case ShipType::Submarine:  return { {6, 0}, Orientation::Horizontal }; // (6,0)..(6,2)
        case ShipType::Destroyer:  return { {8, 0}, Orientation::Horizontal }; // (8,0)..(8,1)
        }
        return { {0, 0}, Orientation::Horizontal };
    }
};

// Deterministic AI attack strategy that attacks odd rows sequentially (rows 1,3,5,7,9 which never have ships).
class FixedAttackStrategy : public IAttackStrategy
{
public:
    void Reset() override
    {
        m_nextRow = 1;
        m_nextCol = 0;
    }

    CellPosition ChooseTarget() override
    {
        CellPosition target{ m_nextRow, m_nextCol };
        m_nextCol++;
        if (m_nextCol >= kBoardSize)
        {
            m_nextCol = 0;
            m_nextRow += 2; // only odd rows: 1, 3, 5, 7, 9
            if (m_nextRow >= kBoardSize)
            {
                m_nextRow = 1;
            }
        }
        return target;
    }

private:
    uint16_t m_nextRow = 1;
    uint16_t m_nextCol = 0;
};

// Mock Observer for verifying event notifications via GMock.
class MockGameObserver : public IGameObserver
{
public:
    MOCK_METHOD(void, OnGameStarted, (), (override));
    MOCK_METHOD(void, OnAttackResolved, (Side attacker, uint16_t row, uint16_t col, AttackResult result), (override));
    MOCK_METHOD(void, OnShipSunk, (Side owner, ShipType shipType), (override));
    MOCK_METHOD(void, OnTurnChanged, (Side currentSide), (override));
    MOCK_METHOD(void, OnGameOver, (Side winner), (override));
};

// Helper function to create engine with deterministic dependencies.
IGameEnginePtr CreateTestEngine()
{
    return CreateGameEngine(std::make_unique<FixedShipPlacer>(), std::make_unique<FixedAttackStrategy>());
}

// ============================================================================
// 1. Game Rules & Helper Tests
// ============================================================================
TEST(GameRulesTests, BoardSizeIsTen)
{
    EXPECT_EQ(kBoardSize, 10);
}

TEST(GameRulesTests, FleetContainsFiveShips)
{
    EXPECT_EQ(kFleet.size(), 5);
    EXPECT_EQ(kFleet[0], ShipType::Carrier);
    EXPECT_EQ(kFleet[1], ShipType::Battleship);
    EXPECT_EQ(kFleet[2], ShipType::Cruiser);
    EXPECT_EQ(kFleet[3], ShipType::Submarine);
    EXPECT_EQ(kFleet[4], ShipType::Destroyer);
}

TEST(GameRulesTests, ShipLengthsAreCorrect)
{
    EXPECT_EQ(GetShipLength(ShipType::Carrier), 5);
    EXPECT_EQ(GetShipLength(ShipType::Battleship), 4);
    EXPECT_EQ(GetShipLength(ShipType::Cruiser), 3);
    EXPECT_EQ(GetShipLength(ShipType::Submarine), 3);
    EXPECT_EQ(GetShipLength(ShipType::Destroyer), 2);
}

TEST(GameRulesTests, InsideBoardBoundaries)
{
    EXPECT_TRUE(IsInsideBoard({ 0, 0 }));
    EXPECT_TRUE(IsInsideBoard({ 9, 9 }));
    EXPECT_TRUE(IsInsideBoard({ 0, 9 }));
    EXPECT_TRUE(IsInsideBoard({ 9, 0 }));

    EXPECT_FALSE(IsInsideBoard({ 10, 0 }));
    EXPECT_FALSE(IsInsideBoard({ 0, 10 }));
    EXPECT_FALSE(IsInsideBoard({ 10, 10 }));
    EXPECT_FALSE(IsInsideBoard({ 100, 100 }));
}

TEST(GameRulesTests, OpponentSwitching)
{
    EXPECT_EQ(GetOpponent(Side::Human), Side::AI);
    EXPECT_EQ(GetOpponent(Side::AI), Side::Human);
}

// ============================================================================
// 2. Initial State Tests
// ============================================================================
TEST(GameEngineInitialTests, StateIsNotStartedBeforeStartNewGame)
{
    auto engine = CreateTestEngine();
    ASSERT_NE(engine, nullptr);

    EXPECT_EQ(engine->GetGameState(), GameState::NotStarted);
    EXPECT_EQ(engine->GetBoardSize(), 10);
    EXPECT_EQ(engine->GetFleet().size(), 5);
}

TEST(GameEngineInitialTests, AttackBeforeGameStartReturnsInvalid)
{
    auto engine = CreateTestEngine();
    EXPECT_EQ(engine->AttackCell(0, 0), AttackResult::InvalidAttack);
    EXPECT_EQ(engine->GetGameState(), GameState::NotStarted);
}

// ============================================================================
// 3. Game Start & Board Population Tests
// ============================================================================
TEST(GameEngineStartTests, StartNewGameTransitionsToHumanTurn)
{
    auto engine = CreateTestEngine();
    engine->StartNewGame();

    EXPECT_EQ(engine->GetGameState(), GameState::HumanTurn);
}

TEST(GameEngineStartTests, HumanFleetIsPlacedCorrectly)
{
    auto engine = CreateTestEngine();
    engine->StartNewGame();

    // With FixedShipPlacer, Carrier is at (0,0)..(0,4)
    for (uint16_t col = 0; col < 5; ++col)
    {
        EXPECT_EQ(engine->GetCellState(Side::Human, 0, col), CellState::Ship);
        EXPECT_EQ(engine->GetShipTypeAt(Side::Human, 0, col), ShipType::Carrier);
    }

    // Row 1 is empty buffer zone
    for (uint16_t col = 0; col < 10; ++col)
    {
        EXPECT_EQ(engine->GetCellState(Side::Human, 1, col), CellState::Empty);
    }

    // Battleship is at (2,0)..(2,3)
    for (uint16_t col = 0; col < 4; ++col)
    {
        EXPECT_EQ(engine->GetCellState(Side::Human, 2, col), CellState::Ship);
        EXPECT_EQ(engine->GetShipTypeAt(Side::Human, 2, col), ShipType::Battleship);
    }
}

TEST(GameEngineStartTests, AIShipsAreHiddenDuringGame)
{
    auto engine = CreateTestEngine();
    engine->StartNewGame();

    // AI ships must not be visible to human (returns Unknown)
    EXPECT_EQ(engine->GetCellState(Side::AI, 0, 0), CellState::Unknown);
    EXPECT_EQ(engine->GetCellState(Side::AI, 1, 0), CellState::Unknown);
    EXPECT_EQ(engine->GetShipTypeAt(Side::AI, 0, 0), std::nullopt);
}

// ============================================================================
// 4. Attack Mechanics Tests
// ============================================================================
TEST(GameEngineAttackTests, AttackEmptyCellReturnsMiss)
{
    auto engine = CreateTestEngine();
    engine->StartNewGame();

    // Row 1 has no ships with FixedShipPlacer
    AttackResult result = engine->AttackCell(1, 0);
    EXPECT_EQ(result, AttackResult::Miss);
    EXPECT_EQ(engine->GetCellState(Side::AI, 1, 0), CellState::Miss);
}

TEST(GameEngineAttackTests, AttackShipCellReturnsHit)
{
    auto engine = CreateTestEngine();
    engine->StartNewGame();

    // Destroyer is at (8,0) and (8,1)
    AttackResult result = engine->AttackCell(8, 0);
    EXPECT_EQ(result, AttackResult::Hit);
    EXPECT_EQ(engine->GetCellState(Side::AI, 8, 0), CellState::Hit);
}

TEST(GameEngineAttackTests, SinkingAllCellsOfShipReturnsSunk)
{
    auto engine = CreateTestEngine();
    engine->StartNewGame();

    // Destroyer length 2: (8,0) and (8,1)
    EXPECT_EQ(engine->AttackCell(8, 0), AttackResult::Hit);
    EXPECT_EQ(engine->AttackCell(8, 1), AttackResult::Sunk);

    // Both cells are now Sunk
    EXPECT_EQ(engine->GetCellState(Side::AI, 8, 0), CellState::Sunk);
    EXPECT_EQ(engine->GetCellState(Side::AI, 8, 1), CellState::Sunk);
    EXPECT_EQ(engine->GetShipTypeAt(Side::AI, 8, 0), ShipType::Destroyer);
}

TEST(GameEngineAttackTests, RepeatedAttackReturnsInvalid)
{
    auto engine = CreateTestEngine();
    engine->StartNewGame();

    EXPECT_EQ(engine->AttackCell(8, 0), AttackResult::Hit);
    EXPECT_EQ(engine->AttackCell(8, 0), AttackResult::InvalidAttack);
}

TEST(GameEngineAttackTests, OutOfBoundsAttackReturnsInvalid)
{
    auto engine = CreateTestEngine();
    engine->StartNewGame();

    EXPECT_EQ(engine->AttackCell(10, 0), AttackResult::InvalidAttack);
    EXPECT_EQ(engine->AttackCell(0, 10), AttackResult::InvalidAttack);
    EXPECT_EQ(engine->AttackCell(99, 99), AttackResult::InvalidAttack);
}

// ============================================================================
// 5. Turn Flow & Victory Condition Tests
// ============================================================================
TEST(GameEngineVictoryTests, SinkingEntireFleetTriggersHumanWon)
{
    auto engine = CreateTestEngine();
    engine->StartNewGame();

    // Sink Carrier (len 5): (0,0)..(0,4)
    for (uint16_t c = 0; c < 4; ++c) EXPECT_EQ(engine->AttackCell(0, c), AttackResult::Hit);
    EXPECT_EQ(engine->AttackCell(0, 4), AttackResult::Sunk);

    // Sink Battleship (len 4): (2,0)..(2,3)
    for (uint16_t c = 0; c < 3; ++c) EXPECT_EQ(engine->AttackCell(2, c), AttackResult::Hit);
    EXPECT_EQ(engine->AttackCell(2, 3), AttackResult::Sunk);

    // Sink Cruiser (len 3): (4,0)..(4,2)
    for (uint16_t c = 0; c < 2; ++c) EXPECT_EQ(engine->AttackCell(4, c), AttackResult::Hit);
    EXPECT_EQ(engine->AttackCell(4, 2), AttackResult::Sunk);

    // Sink Submarine (len 3): (6,0)..(6,2)
    for (uint16_t c = 0; c < 2; ++c) EXPECT_EQ(engine->AttackCell(6, c), AttackResult::Hit);
    EXPECT_EQ(engine->AttackCell(6, 2), AttackResult::Sunk);

    // Sink Destroyer (len 2): (8,0)..(8,1)
    EXPECT_EQ(engine->AttackCell(8, 0), AttackResult::Hit);
    EXPECT_EQ(engine->AttackCell(8, 1), AttackResult::Sunk);

    // All ships sunk -> HumanWon
    EXPECT_EQ(engine->GetGameState(), GameState::HumanWon);

    // Further attacks rejected after game over
    EXPECT_EQ(engine->AttackCell(1, 1), AttackResult::InvalidAttack);
}

// ============================================================================
// 6. GMock Observer Pattern Tests (Mandatory Requirement: 2p)
// ============================================================================
TEST(ObserverMockTests, OnGameStartedAndOnTurnChangedCalledOnStart)
{
    auto engine = CreateTestEngine();
    MockGameObserver mockObserver;
    engine->AddObserver(&mockObserver);

    EXPECT_CALL(mockObserver, OnGameStarted()).Times(1);
    EXPECT_CALL(mockObserver, OnTurnChanged(Side::Human)).Times(1);

    engine->StartNewGame();
}

TEST(ObserverMockTests, OnAttackResolvedDispatchesEventsForHumanAndAI)
{
    auto engine = CreateTestEngine();
    MockGameObserver mockObserver;
    engine->AddObserver(&mockObserver);

    EXPECT_CALL(mockObserver, OnGameStarted()).Times(1);
    EXPECT_CALL(mockObserver, OnTurnChanged(Side::Human)).Times(1);
    engine->StartNewGame();

    // When Human attacks (1, 0) [Miss]:
    // 1. Human attack resolved
    // 2. Turn changes to AI
    // 3. AI executes attack at (9, 0)
    // 4. Turn changes back to Human
    EXPECT_CALL(mockObserver, OnAttackResolved(Side::Human, 1, 0, AttackResult::Miss)).Times(1);
    EXPECT_CALL(mockObserver, OnTurnChanged(Side::AI)).Times(1);
    EXPECT_CALL(mockObserver, OnAttackResolved(Side::AI, 1, 0, _)).Times(1);
    EXPECT_CALL(mockObserver, OnTurnChanged(Side::Human)).Times(1);

    engine->AttackCell(1, 0);
}

TEST(ObserverMockTests, OnShipSunkEventFiredWhenShipDestroyed)
{
    auto engine = CreateTestEngine();
    MockGameObserver mockObserver;
    engine->AddObserver(&mockObserver);

    EXPECT_CALL(mockObserver, OnGameStarted()).Times(1);
    EXPECT_CALL(mockObserver, OnTurnChanged(_)).Times(::testing::AtLeast(1));
    engine->StartNewGame();

    // Hit first cell of Destroyer
    EXPECT_CALL(mockObserver, OnAttackResolved(Side::Human, 8, 0, AttackResult::Hit)).Times(1);
    EXPECT_CALL(mockObserver, OnAttackResolved(Side::AI, _, _, _)).Times(1);
    EXPECT_CALL(mockObserver, OnTurnChanged(_)).Times(::testing::AtLeast(1));
    engine->AttackCell(8, 0);

    // Hit second cell -> Ship Sunk event MUST fire with ShipType::Destroyer
    EXPECT_CALL(mockObserver, OnAttackResolved(Side::Human, 8, 1, AttackResult::Sunk)).Times(1);
    EXPECT_CALL(mockObserver, OnShipSunk(Side::AI, ShipType::Destroyer)).Times(1);
    EXPECT_CALL(mockObserver, OnAttackResolved(Side::AI, _, _, _)).Times(1);
    EXPECT_CALL(mockObserver, OnTurnChanged(_)).Times(::testing::AtLeast(1));

    engine->AttackCell(8, 1);
}

TEST(ObserverMockTests, OnGameOverEventFiredWhenVictoryOccurs)
{
    auto engine = CreateTestEngine();
    MockGameObserver mockObserver;
    engine->StartNewGame();

    // Sink 4 ships without observer active
    for (uint16_t c = 0; c < 5; ++c) engine->AttackCell(0, c); // Carrier
    for (uint16_t c = 0; c < 4; ++c) engine->AttackCell(2, c); // Battleship
    for (uint16_t c = 0; c < 3; ++c) engine->AttackCell(4, c); // Cruiser
    for (uint16_t c = 0; c < 3; ++c) engine->AttackCell(6, c); // Submarine
    engine->AttackCell(8, 0); // 1st cell of Destroyer

    // Now attach observer for the final winning blow
    engine->AddObserver(&mockObserver);

    EXPECT_CALL(mockObserver, OnAttackResolved(Side::Human, 8, 1, AttackResult::Sunk)).Times(1);
    EXPECT_CALL(mockObserver, OnShipSunk(Side::AI, ShipType::Destroyer)).Times(1);
    EXPECT_CALL(mockObserver, OnGameOver(Side::Human)).Times(1);

    engine->AttackCell(8, 1);
}

TEST(ObserverMockTests, RemovedObserverReceivesNoFurtherNotifications)
{
    auto engine = CreateTestEngine();
    MockGameObserver mockObserver;
    engine->AddObserver(&mockObserver);

    EXPECT_CALL(mockObserver, OnGameStarted()).Times(1);
    EXPECT_CALL(mockObserver, OnTurnChanged(_)).Times(::testing::AtLeast(1));
    engine->StartNewGame();

    // Unsubscribe observer
    engine->RemoveObserver(&mockObserver);

    // Subsequent actions should NOT invoke the mock observer
    EXPECT_CALL(mockObserver, OnAttackResolved(_, _, _, _)).Times(0);
    EXPECT_CALL(mockObserver, OnTurnChanged(_)).Times(0);

    engine->AttackCell(1, 0);
}

// ============================================================================
// Main entry point for Google Test + Google Mock
// ============================================================================
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
