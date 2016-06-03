#include "Server.h"
#include <QString>
#include <QRegExp>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>

User::User(QString username, int room)
{
    this->username = username;
    this->room = room;
}

void User::setUsername(QString username)
{
    this->username = username;
}

QString User::getUsername()
{
    return username;
}

void User::setRoom(int room)
{
    this->room = room;
}

int User::getRoom()
{
    return room;
}

Server::Server(QObject* parent) : QObject(parent)
{
    server = new QTcpServer(this);

    connect(server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    if (!server->listen(QHostAddress::Any, PORT)) {
        qDebug() << "Server is not started.";
    } else {
        qDebug() << "Server is started.";
    }
}

void Server::sendUserList(int room)
{
    QString line = "/users:";
    foreach (QTcpSocket* socket, clients.keys()) {
        if(clients.value(socket)->getRoom() == room) {
            line.append(QString(clients.value(socket)->getUsername() + ','));
        }
    }
    line.chop(1);
    line.append("\n");
    sendToRoom(line, room);
}

void Server::sendToRoom(const QString& message, int room)
{
    foreach (QTcpSocket* socket, clients.keys()) {
        if(clients.value(socket)->getRoom() == room) {
            socket->write(message.toUtf8());
        }
    }
}

void Server::onNewConnection()
{
    QTcpSocket* socket = server->nextPendingConnection();
    User* user = new User("", 0);

    qDebug() << "Client connected: " << socket->peerAddress().toString();

    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));

    clients.insert(socket, user);
}

void Server::onDisconnect()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    qDebug() << "Client disconnected: " << socket->peerAddress().toString();

    QString username = clients.value(socket)->getUsername();
    int room = clients.value(socket)->getRoom();

    sendToRoom(QString("/system:" + username + " has left the chat.\n"), room);
    clients.remove(socket);
    sendUserList(room);
}

void Server::onReadyRead()
{
    QRegExp loginRegExp("^/login:(.*):(.*)$");
    QRegExp messageRegExp("^/message:(.*)$");
    QRegExp fileRegExp("^/file:(.*)$");

    QTcpSocket* socket = (QTcpSocket*)sender();

    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();

        if (loginRegExp.indexIn(line) != -1) {
            QString username = loginRegExp.cap(1);

            int newRoom = loginRegExp.cap(2).toInt();
            int oldRoom = clients.value(socket)->getRoom();

            clients.value(socket)->setUsername(username);
            clients.value(socket)->setRoom(newRoom);

            sendToRoom(QString("/system:" + username + " has joined the room #"
                               + QString::number(newRoom) + "\n"), newRoom);
            sendUserList(newRoom);
            sendUserList(oldRoom);

            qDebug() << username << "logged in room #" << newRoom;
        }
        else if (messageRegExp.indexIn(line) != -1) {
            QString user = clients.value(socket)->getUsername();
            int room = clients.value(socket)->getRoom();

            QString message = messageRegExp.cap(1);
            sendToRoom(QString(user + ":" + message + "\n"), room);

            qDebug() << "User:" << user << "in room" << room;
            qDebug() << "Message:" << message;
        }
        else if (fileRegExp.indexIn(line) != -1) {
            QString user = clients.value(socket)->getUsername();
            int room = clients.value(socket)->getRoom();

            foreach (QTcpSocket* tempSocket, clients.keys()) {
                if(clients.value(tempSocket)->getRoom() == room) {
                    if(tempSocket == socket)
                        continue;
                    tempSocket->write(QString("/file:" + user + " sended file.\n").toUtf8());
                }
            }

            qDebug() << user << "send file.";

            socket->waitForReadyRead(-1);
            QByteArray metadata;
            metadata = socket->readAll();

            QDataStream readMeta(&metadata, QIODevice::ReadOnly);
            readMeta.setVersion(QDataStream::Qt_5_6);

            foreach (QTcpSocket* tempSocket, clients.keys()) {
                if(clients.value(tempSocket)->getRoom() == room) {
                    if(tempSocket == socket)
                        continue;
                    tempSocket->write(metadata);
                    tempSocket->waitForBytesWritten();
                }
            }

            quint64 fileSize;
            readMeta >> fileSize;

            long int bytesSended = 0;
            long int bytesToSend = fileSize;

            while (bytesSended < bytesToSend)
            {
                while(!socket->waitForReadyRead(-1));
                QByteArray tmp = socket->read(bytesToSend - bytesSended);

                foreach (QTcpSocket* tempSocket, clients.keys()) {
                    if(clients.value(tempSocket)->getRoom() == room) {
                        if(tempSocket == socket)
                            continue;
                        tempSocket->write(tmp);
                        tempSocket->waitForBytesWritten();
                    }
                }
                bytesSended += tmp.size();
            }

            qDebug() << "File size: " << bytesSended;
            qDebug() << "File sended to the room" << room;
        }
        else {
            qDebug() << "Bad message from " << socket->peerAddress().toString();
        }
    }
}

