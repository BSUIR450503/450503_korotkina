#include <QCoreApplication>
#include <QtCore>
#include <Server.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server server;
    return a.exec();
}
