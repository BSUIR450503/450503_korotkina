#include "MainWindow.h"
#include "ui_mainwindow.h"

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
    connect(ui->roomButton, SIGNAL(clicked()), this, SLOT(roomButtonClicked()));

    connect(ui->serverNameLine, SIGNAL(returnPressed()), this, SLOT(loginButtonClicked()));
    connect(ui->userNameLine, SIGNAL(returnPressed()), this, SLOT(loginButtonClicked()));
    connect(ui->messageLine, SIGNAL(returnPressed()), this, SLOT(sendMessageButtonClicked()));
    connect(ui->roomLine, SIGNAL(returnPressed()), this, SLOT(roomButtonClicked()));

    QRegExp usernameRegExp("^[a-zA-Z]\\w+");
    ui->userNameLine->setValidator(new QRegExpValidator(usernameRegExp, this));

    QRegExp roomRegExp("^[0-9]{1,10}$");
    ui->roomLine->setValidator(new QRegExpValidator(roomRegExp, this));

    fileReadingContinue = false;
    bytesWrittenToFile = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loginButtonClicked()
{
    QString serverName = ui->serverNameLine->text().trimmed();
    if (serverName.isEmpty()) {
        QMessageBox::information(NULL, "Warning",
                                 "Enter the server name or address.",
                                 QMessageBox::Ok);
        return;
    }

    QString username = ui->userNameLine->text().trimmed();
    if (username.isEmpty()) {
        QMessageBox::information(NULL, "Warning",
                                 "Enter your nickname.",
                                 QMessageBox::Ok);
        return;
    }

    socket->connectToHost(serverName, PORT);
}

void MainWindow::sendMessageButtonClicked()
{
    QString message = ui->messageLine->text().trimmed();
    if (!message.isEmpty()) {
        socket->write(QString("/message:" + message + "\n").toUtf8());
        ui->messageLine->clear();
        ui->messageLine->setFocus();
    }
}

void MainWindow::sendFileButtonClicked()
{
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

    char bytesBuffer[1024];
    bytesBuffer[1023] = '\0';

    while(!readFile.atEnd()) {
        int l = readFile.readRawData(bytesBuffer, sizeof(char)*1023);
        QByteArray fileData(bytesBuffer, sizeof(char)*l);

        socket->flush();
        socket->write(fileData, sizeof(char)*l);
        socket->waitForBytesWritten();
    }

    ui->chatField->append("File successfuly sended: " + fileName);
}

void MainWindow::roomButtonClicked()
{
    QString room = ui->roomLine->text().trimmed();
    ui->chatField->clear();
    socket->write(QString("/login:" + ui->userNameLine->text() + ":" + room + "\n").toUtf8());
    ui->messageLine->setFocus();
}

void MainWindow::aboutButtonClicked()
{
    QMessageBox::about(this, "About", "Korotkina Julia, 450503\nMinsk, BSUIR, 2016");
}

void MainWindow::onConnected()
{
    ui->chatField->clear();

    ui->stackedWidget->setCurrentWidget(ui->chatPage);
    socket->write(QString("/login:" + ui->userNameLine->text() + ":0\n").toUtf8());
    ui->messageLine->setFocus();
}

void MainWindow::onDisconnected()
{
    QMessageBox::information(NULL, "Warning",
                         "You was disconnected from the server", QMessageBox::Ok);
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}


void MainWindow::onReadyRead()
{
    QRegExp usersRegExp("^/users:(.*)$");
    QRegExp systemRegExp("^/system:(.*)$");
    QRegExp messageRegExp("^(.*):(.*)$");
    QRegExp fileRegExp("^/file:(.*)$");

    if(!fileReadingContinue) {
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
                fileReadingContinue = true;

                QString message = fileRegExp.cap(1);
                ui->chatField->append("<p color=\"gray\">" + message + "</p>\n");

                socket->waitForReadyRead(-1);
                QByteArray metadata;
                metadata = socket->readAll();

                QDataStream readMeta(&metadata, QIODevice::ReadWrite);
                readMeta.setVersion(QDataStream::Qt_5_6);

                readMeta >> fileSize;
                readMeta >> fileName;
                ui->chatField->append("File name: " + fileName);

                receivedFile = new QFile(QString(fileName));
                receivedFile->open(QFile::WriteOnly);

                writeToFile = new QDataStream(receivedFile);
                writeToFile->setVersion(QDataStream::Qt_5_6);

                char bytesBuffer[1024];
                bytesBuffer[1023] = '\0';

                while(!readMeta.atEnd()) {
                    int bytesReaded = readMeta.readRawData(bytesBuffer, sizeof(char)*1023);
                    bytesWrittenToFile += bytesReaded;
                    QByteArray temp(bytesBuffer, sizeof(char)*bytesReaded);
                    writeToFile->writeRawData(temp.data(), temp.size());
                }
            }
            else if (messageRegExp.indexIn(line) != -1) {
                QString user = messageRegExp.cap(1);
                QString message = messageRegExp.cap(2);
                ui->chatField->append("<p><b>" + user + "</b>: " + message + "</p>\n");
            }
        }
    }
    else {
        long int bytesToWrite = fileSize;

        if (bytesWrittenToFile < bytesToWrite) {
            QByteArray tmp = socket->read(bytesToWrite - bytesWrittenToFile);
            writeToFile->writeRawData(tmp.data(), tmp.size());
            bytesWrittenToFile += tmp.size();
        }
        if (bytesWrittenToFile == bytesToWrite) {
            ui->chatField->append("File successfuly saved: "
                                  + QDir::currentPath());
            fileReadingContinue = false;
            fileSize = 0;
            fileName = "";
            bytesWrittenToFile = 0;
            receivedFile->close();
        }
    }
}
