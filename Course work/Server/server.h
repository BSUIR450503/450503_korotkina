#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QStringList>
#include <QMap>

#define PORT 1234

class User
{
private:
    QString username;
    int room;

public:
    User(QString, int);
    void setUsername(QString);
    void setRoom(int);
    QString getUsername();
    int getRoom();
};

class Server : QObject
{
    Q_OBJECT
public:
    explicit Server(QObject* parent = 0);
    void sendUserList(int);
    void sendToRoom(const QString&, int);

private:
    QTcpServer* server;
    QMap<QTcpSocket*, User*> clients;

public slots:
    void onNewConnection();
    void onDisconnect();
    void onReadyRead();
};

#endif // SERVER_H
