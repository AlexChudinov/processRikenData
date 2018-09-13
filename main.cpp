#include "mainwindow.h"
#include "Base\BaseObject.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	MyInit myInit;
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
