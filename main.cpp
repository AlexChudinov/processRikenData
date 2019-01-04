#include "mainwindow.h"
#include "Base/BaseObject.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    try{
        QApplication a(argc, argv);
        MyInit init;
        MainWindow w;
        w.show();
        return a.exec();
    }
    catch(std::exception& ex)
    {
        QMessageBox::warning(nullptr, "Exception", ex.what());
        return 1;
    }
}
