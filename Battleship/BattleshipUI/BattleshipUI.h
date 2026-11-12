#pragma once

#include <QtWidgets/QMainWindow>
#include <QtGui/QColor>
#include <array>
#include <vector>
#include "ui_BattleshipUI.h"
#include "IGameEngine.h"

class QLabel;
class QPushButton;
class QStackedWidget;

class BattleshipUI : public QMainWindow, public IGameObserver
{
    Q_OBJECT

public:
    BattleshipUI(QWidget* parent = nullptr);
    ~BattleshipUI();

    void OnGameStarted() override;
    void OnAttackResolved(Side attacker, uint16_t row, uint16_t col, AttackResult result) override;
    void OnShipSunk(Side owner, ShipType shipType) override;
    void OnTurnChanged(Side currentSide) override;
    void OnGameOver(Side winner) override;

private:
    QWidget* CreateMenuPage();
    QWidget* CreateGamePage();
    QWidget* CreateBoard(Side boardOwner, const QString& title, QLabel* statusLabel);

    void RedrawBoard(Side boardOwner);
    void PaintCell(Side boardOwner, uint16_t row, uint16_t col);
    void CenterOnScreen();

    static QString CellName(uint16_t row, uint16_t col);
    static QString ShipName(ShipType shipType);
    static QColor ShipColor(ShipType shipType);

    Ui::BattleshipUIClass ui;
    IGameEnginePtr m_engine;

    QStackedWidget* m_pages = nullptr;
    QWidget* m_gamePage = nullptr;
    QWidget* m_boardsArea = nullptr;
    QWidget* m_aiBoard = nullptr;
    QLabel* m_turnLabel = nullptr;
    QLabel* m_playerLabel = nullptr;
    QLabel* m_aiLabel = nullptr;
    QPushButton* m_playAgainButton = nullptr;

    uint16_t m_boardSize = 0;
    std::array<std::vector<QPushButton*>, 2> m_cells;
};
