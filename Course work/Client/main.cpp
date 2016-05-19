#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setWindowIcon(QIcon(":/chat.png"));
    MainWindow window;
    window.show();

    return application.exec();
}
