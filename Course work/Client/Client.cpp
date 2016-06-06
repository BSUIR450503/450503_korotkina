#include "Client.h"
#include "ui_mainwindow.h"

Client::Client(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    /* Инициализируем форму главного окна */
    ui->setupUi(this);

    setCentralWidget(ui->mainFrame);
    /* Устанавливаем начальным окном окно ввода логина и адреса сервера */
    ui->stackedWidget->setCurrentWidget(ui->loginPage);

    /* Создаем объект сокета */
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

    /* Регулярные выражения для фильтрации текста в полях логина и адреса сервера */
    QRegExp usernameRegExp("^[a-zA-Z]\\w+");
    ui->userNameLine->setValidator(new QRegExpValidator(usernameRegExp, this));

    QRegExp roomRegExp("^[0-9]{1,10}$");
    ui->roomLine->setValidator(new QRegExpValidator(roomRegExp, this));

    fileReadingContinue = false;
    bytesWrittenToFile = 0;
}

Client::~Client()
{
    delete ui;
}

void Client::loginButtonClicked()
{
    /* Получаем введенное имя сервера */
    QString serverName = ui->serverNameLine->text().trimmed();
    if (serverName.isEmpty()) {
        QMessageBox::information(NULL, "Warning",
                                 "Enter the server name or address.",
                                 QMessageBox::Ok);
        return;
    }

    /* Получаем введенное имя пользователя */
    QString username = ui->userNameLine->text().trimmed();
    if (username.isEmpty()) {
        QMessageBox::information(NULL, "Warning",
                                 "Enter your nickname.",
                                 QMessageBox::Ok);
        return;
    }

    /* Устанавливаем соединение сокета с сервером */
    socket->connectToHost(serverName, PORT);
}

void Client::sendMessageButtonClicked()
{
    /* Получаем текст введенного сообщения */
    QString message = ui->messageLine->text().trimmed();
    if (!message.isEmpty()) {
        /* Записываем сообщение в сокет с ключевым словом "message", которое сервер
         * распознает и отправит остальным пользователям в комнате */
        socket->write(QString("/message:" + message + "\n").toUtf8());
        ui->messageLine->clear();
        ui->messageLine->setFocus();
    }
}

void Client::sendFileButtonClicked()
{
    /* Записываем в сокет ключевое слово "file", тем самым подготовив сервер к ожиданию
     * получения файла */
    socket->write(QString("/file:\n").toUtf8());

    /* Получаем путь к отправляемому файлу */
    QString path = QFileDialog::getOpenFileName(this);

    QFile* file = new QFile(path);
    /* Открываем выбранный файл только для чтения */
    file->open(QFile::ReadOnly);

    /* Поток для чтения из файла */
    QDataStream readFile(file);

    QFileInfo fileInfo(file->fileName());
    QString fileName(fileInfo.fileName());

    /* Создаем массив байтов с данными о файле (имя и размер) */
    QByteArray metadata;
    QDataStream writeMeta(&metadata, QIODevice::WriteOnly);
    writeMeta.setVersion(QDataStream::Qt_5_6);

    writeMeta << quint64(file->size());
    writeMeta << fileName;

    /* Записываем данные о файле в сокет */
    socket->write(metadata);
    /* Ожидаем окончания записи */
    socket->waitForBytesWritten();

    /* Буфер байтов для отправляемого файла */
    char bytesBuffer[1024];
    bytesBuffer[1023] = '\0';

    /* Цикл продолжается до тех пор, пока файл не считается до конца */
    while(!readFile.atEnd()) {
        /* Считываем данные из файла в буфер побайтово (блоками по 1023 байта) */
        int bytesReaded = readFile.readRawData(bytesBuffer, sizeof(char)*1023);
        QByteArray fileData(bytesBuffer, sizeof(char)*bytesReaded);

        /* Записываем полученные байты в сокет */
        socket->flush();
        socket->write(fileData, sizeof(char)*bytesReaded);
        socket->waitForBytesWritten();
    }

    /* Уведомляем пользователья об успешной отправке файла */
    ui->chatField->append("File successfuly sended: " + fileName);
}

void Client::roomButtonClicked()
{
    /* Получаем номер комнаты, в которую решено перейти */
    QString room = ui->roomLine->text().trimmed();
    ui->chatField->clear();
    /* Оповещаем сервер о переходе в другую комнату с помощью ключевого слова "login" */
    socket->write(QString("/login:" + ui->userNameLine->text() + ":" + room + "\n").toUtf8());
    ui->messageLine->setFocus();
}

void Client::aboutButtonClicked()
{
    QMessageBox::about(this, "About", "Korotkina Julia, 450503\nMinsk, BSUIR, 2016");
}

/* Cлот активируется при соединении сокета с сервером */
void Client::onConnected()
{
    ui->chatField->clear();

    /* В качестве текущего окна устанавливаем окно чата */
    ui->stackedWidget->setCurrentWidget(ui->chatPage);
    /* Оповещаем сервер о входе клиента с помощью ключевого слово "login" */
    socket->write(QString("/login:" + ui->userNameLine->text() + ":0\n").toUtf8());
    ui->messageLine->setFocus();
}

/* Cлот активируется при отсоединении сокета от сервера */
void Client::onDisconnected()
{
    QMessageBox::information(NULL, "Warning",
                         "You was disconnected from the server", QMessageBox::Ok);
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}

/* Слот активируется, когда в сокете появляются данные для чтения */
void Client::onReadyRead()
{
    /* Регулярные выражения для фильтрации сообщений от сервера */
    QRegExp usersRegExp("^/users:(.*)$");
    QRegExp systemRegExp("^/system:(.*)$");
    QRegExp messageRegExp("^(.*):(.*)$");
    QRegExp fileRegExp("^/file:(.*)$");

    /* Если получение файла не продолжается */
    if(!fileReadingContinue) {
        while (socket->canReadLine()) {
            /* Считываем строку из сокета и сравниваем по ключевым словам */
            QString line = QString::fromUtf8(socket->readLine()).trimmed();

            if (usersRegExp.indexIn(line) != -1) {
                QStringList users = usersRegExp.cap(1).split(",");
                ui->userList->clear();
                foreach (QString user, users) {
                    /* Добавляем нового пользователя в список пользователей в комнате */
                    new QListWidgetItem(QIcon(":/Icons/User.png"), user, ui->userList);
                }
            }
            else if (systemRegExp.indexIn(line) != -1) {
                QString message = systemRegExp.cap(1);
                ui->chatField->append("<p color=\"gray\">" + message + "</p>\n");
            }
            else if (fileRegExp.indexIn(line) != -1) {
                /* Устанавливаем флаг чтения файла в единицу */
                fileReadingContinue = true;

                QString message = fileRegExp.cap(1);
                ui->chatField->append("<p color=\"gray\">" + message + "</p>\n");

                /* Ждем данных о файле от сервера */
                socket->waitForReadyRead(-1);
                QByteArray metadata;
                metadata = socket->readAll();

                QDataStream readMeta(&metadata, QIODevice::ReadWrite);
                readMeta.setVersion(QDataStream::Qt_5_6);

                /* Считываем размер и имя получаемого файла */
                readMeta >> fileSize;
                readMeta >> fileName;
                ui->chatField->append("File name: " + fileName);

                /* Создаем файл */
                receivedFile = new QFile(QString(fileName));
                receivedFile->open(QFile::WriteOnly);

                /* Поток для записи в файл */
                writeToFile = new QDataStream(receivedFile);
                writeToFile->setVersion(QDataStream::Qt_5_6);

                char bytesBuffer[1024];
                bytesBuffer[1023] = '\0';

                /* "Лишние байты" из полученных данных записываем в сам файл. Эти байты
                 * возникают из-за того, что скоростной сервер успевает записать в сокет
                 * больше байт, чем занимает информация о файле, а клиент считывает все
                 * данные из сокета, даже "лишние" */
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
    /* Если получение файла продолжается */
    else {
        long int bytesToWrite = fileSize;

        /* Получаем до тех пор, когда не получим весь файл до конца */
        if (bytesWrittenToFile < bytesToWrite) {
            QByteArray tmp = socket->read(bytesToWrite - bytesWrittenToFile);
            writeToFile->writeRawData(tmp.data(), tmp.size());
            bytesWrittenToFile += tmp.size();
        }
        /* Если файл получен до конца */
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
