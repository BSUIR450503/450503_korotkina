#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QStringList>
#include <QMap>

/* Порт, на котором расположится сервер */
#define PORT 1234

/* Класс, описывающий пользователя чата */
class User
{
private:
    /* Имя пользователя */
    QString username;
    /* Номер комнаты, в которой пользователь находится */
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
    /* Метод для отсылки списка пользователей клиентам */
    void sendUserList(int);
    /* Метод для отсылки сообщений (любых) клиентам */
    void sendToRoom(const QString&, int);

private:
    /* Указатель на сервер */
    QTcpServer* server;
    /* Список подключенных клиентов с ключом-сокетом */
    QMap<QTcpSocket*, User*> clients;

public slots:
    void onNewConnection();
    void onDisconnect();
    void onReadyRead();
};

#endif // SERVER_H
