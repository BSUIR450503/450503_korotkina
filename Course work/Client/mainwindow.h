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

#define PORT 1234

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTcpSocket* socket;

    QFile* receivedFile;
    QString fileName;
    quint64 fileSize;
    QDataStream* writeToFile;

    bool fileReadingContinue;
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
