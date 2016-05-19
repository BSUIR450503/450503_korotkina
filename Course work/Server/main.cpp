#include <QCoreApplication>
#include <QtCore>
#include <server.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    Server server;
    return application.exec();
}
