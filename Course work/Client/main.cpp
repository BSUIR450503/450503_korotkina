#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setWindowIcon(QIcon(":/Icons/Icon.png"));

    MainWindow mainWindow;
    mainWindow.show();

    return application.exec();
}
