#include <QRegExp>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QFile>
#include <QFileDialog>
#include <QThread>
#include "MainWindow.h"
#include "ui_Mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setCentralWidget(ui->mainFrame);
    ui->stackedWidget->setCurrentWidget(ui->loginPage);

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

    connect(ui->loginButton, SIGNAL(clicked()), this, SLOT(loginButtonClicked()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendMessageButtonClicked()));
    connect(ui->sendImageButton, SIGNAL(clicked()), this, SLOT(sendFileButtonClicked()));
    connect(ui->aboutButton, SIGNAL(clicked()), this, SLOT(aboutButtonClicked()));

    connect(ui->serverNameLine, SIGNAL(returnPressed()), this, SLOT(loginButtonClicked()));
    connect(ui->userNameLine, SIGNAL(returnPressed()), this, SLOT(loginButtonClicked()));
    connect(ui->messageLine, SIGNAL(returnPressed()), this, SLOT(sendMessageButtonClicked()));

    QRegExp regex("^[a-zA-Z]\\w+");
    ui->userNameLine->setValidator(new QRegExpValidator(regex, this));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::loginButtonClicked() {
    QString serverName = ui->serverNameLine->text().trimmed();
    if (serverName.isEmpty()) {
        QMessageBox::information(NULL, "Warning",
                                 "Enter the server name or address.",
                                 QMessageBox::Ok);
        return;
    }

    QString userName = ui->userNameLine->text().trimmed();
    if (userName.isEmpty()) {
        QMessageBox::information(NULL, "Warning",
                                 "Enter your nickname.",
                                 QMessageBox::Ok);
        return;
    }

    socket->connectToHost(serverName, PORT);
}

void MainWindow::sendMessageButtonClicked() {
    QString message = ui->messageLine->text().trimmed();
    if (!message.isEmpty()) {
        socket->write(QString("/message:" + message + "\n").toUtf8());
        ui->messageLine->clear();
        ui->messageLine->setFocus();
    }
}

void MainWindow::sendFileButtonClicked() {
    socket->write(QString("/file:\n").toUtf8());

    QString path = QFileDialog::getOpenFileName(this);
    QFile* file = new QFile(path);
    file->open(QFile::ReadOnly);

    QDataStream readFile(file);

    QFileInfo fileInfo(file->fileName());
    QString fileName(fileInfo.fileName());

    QByteArray metadata;
    QDataStream writeMeta(&metadata, QIODevice::WriteOnly);
    writeMeta.setVersion(QDataStream::Qt_5_6);

    writeMeta << quint64(file->size());
    writeMeta << fileName;

    socket->write(metadata);
    socket->waitForBytesWritten();

    char ch [1024];
    ch[1023] = '\0';

    while(!readFile.atEnd()) {
        int l = readFile.readRawData(ch, sizeof(char)*1023);
        QByteArray fileData(ch, sizeof(char)*l);

        socket->flush();
        socket->write(fileData, sizeof(char)*l);
        socket->waitForBytesWritten();
    }
    ui->chatField->append("File successfuly sended: " + fileName);
}

void MainWindow::aboutButtonClicked() {
    QMessageBox::about(this, "About", "Korotkina Julia, 450503\nMinsk, BSUIR, 2016");
}

void MainWindow::onConnected() {
    ui->chatField->clear();

    ui->stackedWidget->setCurrentWidget(ui->chatPage);
    socket->write(QString("/login:" + ui->userNameLine->text() + "\n").toUtf8());
    ui->messageLine->setFocus();
}

void MainWindow::onDisconnected() {
    QMessageBox::warning(NULL, "Warning",
                         "You was disconnected from the server", QMessageBox::Ok);
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}


void MainWindow::onReadyRead() {
    QRegExp usersRegExp("^/users:(.*)$");
    QRegExp systemRegExp("^/system:(.*)$");
    QRegExp messageRegExp("^(.*):(.*)$");
    QRegExp fileRegExp("^/file:(.*)$");

    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();

        if (usersRegExp.indexIn(line) != -1) {
            QStringList users = usersRegExp.cap(1).split(",");
            ui->userList->clear();
            foreach (QString user, users) {
                new QListWidgetItem(QIcon(":/Icons/User.png"), user, ui->userList);
            }
        }
        else if (systemRegExp.indexIn(line) != -1) {
            QString message = systemRegExp.cap(1);
            ui->chatField->append("<p color=\"gray\">" + message + "</p>\n");
        }
        else if (fileRegExp.indexIn(line) != -1){
            QString message = fileRegExp.cap(1);
            ui->chatField->append("<p color=\"gray\">" + message + "</p>\n");

            socket->waitForReadyRead(-1);
            QByteArray metadata;
            metadata = socket->readAll();

            QDataStream readMeta(&metadata, QIODevice::ReadWrite);
            readMeta.setVersion(QDataStream::Qt_5_6);

            quint64 fileSize;
            QString fileName;

            readMeta >> fileSize;
            readMeta >> fileName;
            ui->chatField->append("File name:" + fileName);

            QFile receivedFile(QString("D:/client/" + fileName));
            receivedFile.open(QFile::WriteOnly);
            QDataStream write(&receivedFile);
            write.setVersion(QDataStream::Qt_5_6);

            char ch [1024];
            ch[1023] = '\0';

            while(!readMeta.atEnd()) {
                int l = readMeta.readRawData(ch, sizeof(char)*1023);
                QByteArray temp(ch, sizeof(char)*l);
                write.writeRawData(temp.data(), temp.size());
            }

            long int bytesWritten = 0;
            long int bytesToWrite = fileSize;

            while (bytesWritten < bytesToWrite)
            {
                if(!socket->waitForReadyRead(10000))
                    break;
                QByteArray tmp = socket->readAll();
                write.writeRawData(tmp.data(), tmp.size());
                bytesWritten += tmp.size();
            }
            ui->chatField->append("File successfuly saved to:" + receivedFile.fileName());
            receivedFile.close();
        }
        else if (messageRegExp.indexIn(line) != -1) {
            QString user = messageRegExp.cap(1);
            QString message = messageRegExp.cap(2);
            ui->chatField->append("<p><b>" + user + "</b>: " + message + "</p>\n");
        }
    }
}
