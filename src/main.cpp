#include "epamk_mw.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    EPAMK_MW w;
    w.show();

    return a.exec();
}
