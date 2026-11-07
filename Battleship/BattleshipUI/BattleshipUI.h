#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BattleshipUI.h"

class BattleshipUI : public QMainWindow
{
    Q_OBJECT

public:
    BattleshipUI(QWidget *parent = nullptr);
    ~BattleshipUI();

private:
    Ui::BattleshipUIClass ui;
};

