#include "Server.h"
#include <QString>
#include <QRegExp>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>

Server::Server(QObject* parent) : QObject(parent) {
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    if (!server->listen(QHostAddress::Any, PORT)) {
        qDebug() << "Server is not started.";
    } else {
        qDebug() << "Server is started.";
    }
}

void Server::sendUserList() {
    QString line = "/users:" + clients.values().join(',') + "\n";
    sendToAll(line);
}

void Server::sendToAll(const QString& msg) {
    foreach (QTcpSocket* socket, clients.keys()) {
        socket->write(msg.toUtf8());
    }
}

void Server::onNewConnection() {
    QTcpSocket* socket = server->nextPendingConnection();
    qDebug() << "Client connected: " << socket->peerAddress().toString();

    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));

    clients.insert(socket, "");
}

void Server::onDisconnect() {
    QTcpSocket* socket = (QTcpSocket*)sender();
    qDebug() << "Client disconnected: " << socket->peerAddress().toString();

    QString username = clients.value(socket);
    sendToAll(QString("/system:" + username + " has left the chat.\n"));
    clients.remove(socket);
    sendUserList();
}

void Server::onReadyRead() {
    QRegExp loginRegExp("^/login:(.*)$");
    QRegExp messageRegExp("^/message:(.*)$");
    QRegExp fileRegExp("^/file:(.*)$");

    QTcpSocket* socket = (QTcpSocket*)sender();

    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();
        if (loginRegExp.indexIn(line) != -1) {
            QString user = loginRegExp.cap(1);
            clients[socket] = user;
            sendToAll(QString("/system:" + user + " has joined the chat.\n"));
            sendUserList();
            qDebug() << user << "logged in.";
        }
        else if (messageRegExp.indexIn(line) != -1) {
            QString user = clients.value(socket);
            QString msg = messageRegExp.cap(1);
            sendToAll(QString(user + ":" + msg + "\n"));
            qDebug() << "User:" << user;
            qDebug() << "Message:" << msg;
        }
        else if (fileRegExp.indexIn(line) != -1){
            QString user = clients.value(socket);

            foreach (QTcpSocket* tempSocket, clients.keys()) {
                if(tempSocket == socket)
                    continue;
                tempSocket->write(QString("/file:" + user + " sended file.\n").toUtf8());
            }

            qDebug() << user << "send file.";

            socket->waitForReadyRead(-1);
            QByteArray metadata;
            metadata = socket->readAll();

            QDataStream readMeta(&metadata, QIODevice::ReadOnly);
            readMeta.setVersion(QDataStream::Qt_5_6);

            foreach (QTcpSocket* tempSocket, clients.keys()) {
                if(tempSocket == socket)
                    continue;
                tempSocket->write(metadata);
                tempSocket->waitForBytesWritten();
            }

            quint64 fileSize;
            readMeta >> fileSize;

            long int bytesSended = 0;
            long int bytesToSend = fileSize;

            while (bytesSended < bytesToSend)
            {
                while(!socket->waitForReadyRead(-1));
                QByteArray tmp = socket->readAll();

                foreach (QTcpSocket* tempSocket, clients.keys()) {
                    if(tempSocket == socket)
                        continue;
                    tempSocket->write(tmp);
                    tempSocket->waitForBytesWritten();
                }

                bytesSended += tmp.size();
            }
            qDebug() << "File size: " << bytesSended;
            qDebug() << "File sended";
        }
        else {
            qDebug() << "Bad message from " << socket->peerAddress().toString();
        }
    }
}

