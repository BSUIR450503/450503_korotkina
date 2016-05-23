#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QRegExpValidator>

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

private slots:
    void loginButtonClicked();
    void sendMessageButtonClicked();
    void sendFileButtonClicked();
    void aboutButtonClicked();
    void onReadyRead();
    void onConnected();
    void onDisconnected();
};

#endif // MAINWINDOW_H
