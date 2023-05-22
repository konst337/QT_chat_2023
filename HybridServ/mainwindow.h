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
    void sendVars(QString vars, qint8 type);

private slots:
    void on_butt_hostServ_clicked();
    void on_butt_connServ_clicked();

    void showWindow();

    //client
    void readData();
    void error();
    void Sdisconnected();
    void Sconnected();

    //udp
    void readSome();
    void sendSome(QString nick, QString message, qint8 type);

    //server
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerReadMany();

    void on_butt_sendMessage_clicked();

    void on_nick_textChanged(const QString &arg1);

    void on_nick_textEdited(const QString &arg1);

    void on_nick_returnPressed();

    void on_butt_hostServ_2_clicked();


    void on_lineEdit_returnPressed();

    void on_butt_hostServ_3_clicked();

private:
    PrivateChat *pChat;
    //client
    Ui::MainWindow *ui;
    QTcpSocket *sock;
    QUdpSocket *usock;
    QHostAddress myIp; // ip
    //server
    QMap<int, QTcpSocket*> SClients;
    int counter;
    QTcpServer *mTcpServer;
    int server_status;
};
#endif // MAINWINDOW_H
