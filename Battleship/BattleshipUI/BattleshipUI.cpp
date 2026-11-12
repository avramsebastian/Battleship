#include "BattleshipUI.h"

#include <QtGui/QScreen>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>

namespace
{
    constexpr int kMenuWindowWidth = 480;
    constexpr int kMenuWindowHeight = 320;
    constexpr int kMenuButtonWidth = 180;
    constexpr int kMenuButtonHeight = 40;
    constexpr int kCellSize = 34;
}

BattleshipUI::BattleshipUI(QWidget* parent)
    : QMainWindow(parent)
    , m_engine(CreateGameEngine())
{
    ui.setupUi(this);

    m_pages = new QStackedWidget(this);
    m_pages->addWidget(CreateMenuPage());
    m_gamePage = CreateGamePage();
    m_pages->addWidget(m_gamePage);
    setCentralWidget(m_pages);

    resize(kMenuWindowWidth, kMenuWindowHeight);
    CenterOnScreen();
    m_engine->AddObserver(this);
}

BattleshipUI::~BattleshipUI()
{
    m_engine->RemoveObserver(this);
}

void BattleshipUI::OnGameStarted()
{
    if (m_boardSize == 0)
    {
        m_boardSize = m_engine->GetBoardSize();
        m_boardsArea->layout()->addWidget(CreateBoard(Side::Human, "Your board", m_playerLabel));
        m_boardsArea->layout()->addWidget(CreateBoard(Side::AI, "AI board", m_aiLabel));
    }

    RedrawBoard(Side::Human);
    RedrawBoard(Side::AI);
    m_playerLabel->setText("Pick a cell on the AI board.");
    m_aiLabel->clear();
    m_playAgainButton->hide();

    m_pages->setCurrentWidget(m_gamePage);
    m_pages->setMinimumSize(m_gamePage->sizeHint());
    adjustSize();
    CenterOnScreen();
}

void BattleshipUI::OnAttackResolved(Side attacker, uint16_t row, uint16_t col, AttackResult result)
{
    const Side attackedBoard = attacker == Side::Human ? Side::AI : Side::Human;
    PaintCell(attackedBoard, row, col);

    const QString who = attacker == Side::Human ? "You" : "The AI";
    const QString what = result == AttackResult::Miss ? "missed" : "hit";
    QLabel* label = attacker == Side::Human ? m_playerLabel : m_aiLabel;
    label->setText(QString("%1 %2 %3.").arg(who, what, CellName(row, col)));
}

void BattleshipUI::OnShipSunk(Side owner, ShipType shipType)
{
    RedrawBoard(owner);

    const QString message = owner == Side::AI
        ? QString("You sank the AI's %1!").arg(ShipName(shipType))
        : QString("The AI sank your %1!").arg(ShipName(shipType));
    QLabel* label = owner == Side::AI ? m_playerLabel : m_aiLabel;
    label->setText(label->text() + '\n' + message);
}

void BattleshipUI::OnTurnChanged(Side currentSide)
{
    const bool isHumanTurn = currentSide == Side::Human;
    m_turnLabel->setText(isHumanTurn ? "Your turn" : "AI's turn");
    m_aiBoard->setEnabled(isHumanTurn);
}

void BattleshipUI::OnGameOver(Side winner)
{
    RedrawBoard(Side::AI);
    m_aiBoard->setEnabled(false);
    m_turnLabel->setText(winner == Side::Human ? "You won!" : "The AI won!");
    m_playAgainButton->show();
}

QWidget* BattleshipUI::CreateMenuPage()
{
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);

    auto* startButton = new QPushButton("Start");
    auto* exitButton = new QPushButton("Exit");
    layout->addStretch();
    for (QPushButton* button : { startButton, exitButton })
    {
        button->setFixedWidth(kMenuButtonWidth);
        button->setMinimumHeight(kMenuButtonHeight);
        layout->addWidget(button, 0, Qt::AlignHCenter);
    }
    layout->addStretch();

    connect(startButton, &QPushButton::clicked, this, [this]() { m_engine->StartNewGame(); });
    connect(exitButton, &QPushButton::clicked, this, &QWidget::close);
    return page;
}

QWidget* BattleshipUI::CreateGamePage()
{
    auto* content = new QWidget;
    auto* layout = new QVBoxLayout(content);

    m_turnLabel = new QLabel;
    QFont turnFont = m_turnLabel->font();
    turnFont.setBold(true);
    turnFont.setPointSize(turnFont.pointSize() + 2);
    m_turnLabel->setFont(turnFont);

    m_playerLabel = new QLabel;
    m_aiLabel = new QLabel;
    for (QLabel* label : { m_playerLabel, m_aiLabel })
    {
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        label->setMinimumHeight(label->fontMetrics().lineSpacing() * 2);
    }

    m_playAgainButton = new QPushButton("Play again");
    auto* exitButton = new QPushButton("Exit");

    auto* topLayout = new QHBoxLayout;
    topLayout->addWidget(m_turnLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_playAgainButton);
    topLayout->addWidget(exitButton);

    m_boardsArea = new QWidget;
    auto* boardsLayout = new QHBoxLayout(m_boardsArea);
    boardsLayout->setContentsMargins(0, 0, 0, 0);
    boardsLayout->setSpacing(24);

    auto* legendLayout = new QHBoxLayout;
    legendLayout->setSpacing(16);
    for (const ShipType shipType : m_engine->GetFleet())
    {
        auto* swatch = new QLabel;
        swatch->setFixedSize(14, 14);
        swatch->setStyleSheet(QString("background-color: %1; border-radius: 3px;").arg(ShipColor(shipType).name()));
        legendLayout->addWidget(swatch);
        legendLayout->addWidget(new QLabel(ShipName(shipType)));
    }
    legendLayout->addStretch();

    layout->addLayout(topLayout);
    layout->addWidget(m_boardsArea);
    layout->addLayout(legendLayout);

    auto* page = new QWidget;
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(content, 0, Qt::AlignCenter);

    connect(m_playAgainButton, &QPushButton::clicked, this, [this]() { m_engine->StartNewGame(); });
    connect(exitButton, &QPushButton::clicked, this, &QWidget::close);
    return page;
}

QWidget* BattleshipUI::CreateBoard(Side boardOwner, const QString& title, QLabel* statusLabel)
{
    auto* box = new QGroupBox(title);
    auto* grid = new QGridLayout(box);
    grid->setSpacing(2);

    for (uint16_t index = 0; index < m_boardSize; ++index)
    {
        grid->addWidget(new QLabel(QString::number(index + 1)), 0, index + 1, Qt::AlignCenter);
        grid->addWidget(new QLabel(QString(QChar('A' + index))), index + 1, 0, Qt::AlignCenter);
    }

    for (uint16_t row = 0; row < m_boardSize; ++row)
    {
        for (uint16_t col = 0; col < m_boardSize; ++col)
        {
            auto* cell = new QPushButton;
            cell->setFixedSize(kCellSize, kCellSize);
            cell->setFocusPolicy(Qt::NoFocus);
            grid->addWidget(cell, row + 1, col + 1);
            m_cells[static_cast<size_t>(boardOwner)].push_back(cell);

            if (boardOwner == Side::Human)
            {
                cell->setAttribute(Qt::WA_TransparentForMouseEvents);
                continue;
            }
            connect(cell, &QPushButton::clicked, this, [this, row, col]() {
                if (m_engine->AttackCell(row, col) == AttackResult::InvalidAttack)
                {
                    m_playerLabel->setText(QString("Cell %1 was already attacked. Pick another one.").arg(CellName(row, col)));
                }
            });
        }
    }

    if (boardOwner == Side::AI)
    {
        m_aiBoard = box;
    }

    auto* column = new QWidget;
    auto* columnLayout = new QVBoxLayout(column);
    columnLayout->setContentsMargins(0, 0, 0, 0);
    columnLayout->addWidget(statusLabel);
    columnLayout->addWidget(box);
    return column;
}

void BattleshipUI::RedrawBoard(Side boardOwner)
{
    for (uint16_t row = 0; row < m_boardSize; ++row)
    {
        for (uint16_t col = 0; col < m_boardSize; ++col)
        {
            PaintCell(boardOwner, row, col);
        }
    }
}

void BattleshipUI::PaintCell(Side boardOwner, uint16_t row, uint16_t col)
{
    const CellState state = m_engine->GetCellState(boardOwner, row, col);
    const std::optional<ShipType> shipType = m_engine->GetShipTypeAt(boardOwner, row, col);

    QString background = "#2b6c96";
    QString border = "1px solid #1d3b50";
    QString text;

    if (shipType.has_value())
    {
        const QColor shipColor = ShipColor(*shipType);
        background = state == CellState::Sunk ? shipColor.darker(170).name() : shipColor.name();
        border = state == CellState::Hit ? "3px solid #ff3b30" : border;
    }
    else if (state == CellState::Hit)
    {
        background = "#d1452f";
    }
    else if (state == CellState::Miss)
    {
        background = "#9fb7c6";
        text = QChar(0x2022);
    }

    if (state == CellState::Hit || state == CellState::Sunk)
    {
        text = "X";
    }

    QPushButton* cell = m_cells[static_cast<size_t>(boardOwner)][row * m_boardSize + col];
    cell->setText(text);
    cell->setStyleSheet(QString(
        "QPushButton { background-color: %1; border: %2; border-radius: 3px; color: white; font-weight: bold; }"
        "QPushButton:hover { border: 2px solid #f0c040; }").arg(background, border));
}

void BattleshipUI::CenterOnScreen()
{
    QRect frame = frameGeometry();
    frame.moveCenter(screen()->availableGeometry().center());
    move(frame.topLeft());
}

QString BattleshipUI::CellName(uint16_t row, uint16_t col)
{
    return QString("%1%2").arg(QChar('A' + row)).arg(col + 1);
}

QString BattleshipUI::ShipName(ShipType shipType)
{
    switch (shipType)
    {
    case ShipType::Carrier:    return "carrier";
    case ShipType::Battleship: return "battleship";
    case ShipType::Cruiser:    return "cruiser";
    case ShipType::Submarine:  return "submarine";
    case ShipType::Destroyer:  return "destroyer";
    }
    return "ship";
}

QColor BattleshipUI::ShipColor(ShipType shipType)
{
    switch (shipType)
    {
    case ShipType::Carrier:    return QColor("#8e6bbf");
    case ShipType::Battleship: return QColor("#3f9e8f");
    case ShipType::Cruiser:    return QColor("#c9a227");
    case ShipType::Submarine:  return QColor("#d07a3a");
    case ShipType::Destroyer:  return QColor("#6aa84f");
    }
    return QColor("#5b6770");
}
