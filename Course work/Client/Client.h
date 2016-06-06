#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QRegExpValidator>
#include <QRegExp>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QFile>
#include <QFileDialog>
#include <QThread>

/* Номер порта соединения с сервером */
#define PORT 1234

namespace Ui {
class MainWindow;
}

class Client : public QMainWindow
{
    Q_OBJECT
public:
    explicit Client(QWidget *parent = 0);
    ~Client();

private:
    /* Указатель на форму главного окна */
    Ui::MainWindow *ui;
    /* Указатель на сокет для соединения с сервером */
    QTcpSocket* socket;

    /* Указатель на получаемый файл */
    QFile* receivedFile;
    /* Имя получаемого файла */
    QString fileName;\
    /* Размер получаемого файла */
    quint64 fileSize;
    /* Поток записи в файл */
    QDataStream* writeToFile;

    /* Флаг записи файл */
    bool fileReadingContinue;
    /* Количество байт, уже записанных в файл */
    long int bytesWrittenToFile;

private slots:
    void loginButtonClicked();
    void sendMessageButtonClicked();
    void sendFileButtonClicked();
    void roomButtonClicked();
    void aboutButtonClicked();
    void onReadyRead();
    void onConnected();
    void onDisconnected();
};

#endif // MAINWINDOW_H
