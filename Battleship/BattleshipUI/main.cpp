#include "BattleshipUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    BattleshipUI window;
    window.show();
    return app.exec();
}
