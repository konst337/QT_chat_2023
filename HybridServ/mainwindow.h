#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QUdpSocket>

#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include "PrivateChat.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void sendVars(QString nick,QString message, qint8 type);

private slots:
    void on_butt_hostServ_clicked();

    void showWindow();

    //udp
    void readSome();
    void sendSome(QString nick, QString message, qint8 type);

    //server
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerReadMany();

    void on_nick_textChanged(const QString &arg1);

    void on_nick_returnPressed();
    void on_butt_hostServ_2_clicked();
    void on_butt_sendMessage_clicked();
    void on_lineEdit_returnPressed();
    void on_butt_hostServ_3_clicked();

private:
    PrivateChat *pChat;
    Ui::MainWindow *ui;

    QUdpSocket *usock = 0;
    QHostAddress myIp;
    QMap<int, QTcpSocket*> SClients;
    QTcpServer *mTcpServer;
    int counter;
    int server_status;

    int port = 7755;
    QString nickname;
    //UDP message events
    qint8 UDP_SETUP = 0;
    qint8 UDP_USUAL_MESSAGE = 1;
    qint8 UDP_PERSON_ONLINE = 2;
    qint8 UDP_WHO_IS_ONLINE = 3;
    qint8 UDP_EXIT = 4;
    //TCP message events
    qint8 TCP_PRIVATE_MESSAGE = 7;
    // доделать
    qint8 TCP_NEW_CONN = 8; // SERVER_STATE = true;
    qint8 TCP_DISCONNECT = 9;


};
#endif // MAINWINDOW_H
