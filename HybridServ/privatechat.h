#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <QByteArray>
#include <QDataStream>

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = nullptr);
    ~PrivateChat();

private:
    Ui::PrivateChat *ui;

    bool SOCKET_STATE = false;
    bool SERVER_STATE = false;

    QTcpSocket *sock = 0;
    QString myIp;
    QString nickname;


    //TCP message events
    qint8 INIT = 9;
    qint8 PRIVATE = 7;
//    qint8 TCP_PRIVATE_MESSAGE = 7;
    // доделать
//    qint8 TCP_NEW_CONN = 8; // SERVER_STATE = true;
//    qint8 TCP_DISCONNECT = 9;

    // SERVER
    QMap<int, QTcpSocket*> SClients;
    QTcpServer *mTcpServer;
    int counter;


signals:
    void secondWindow();
    void test1(QString nick, QString message, qint8 type);

private slots:
    void on_butt_public_clicked();

    //client
    void readData();
    void error();
    void Sdisconnected();
    void Sconnected();

    void getVars(QString nick, QString message, qint8 type);

    //server
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerReadMany();

    void on_butt_connect_clicked();
    void on_butt_sendMessage1_clicked();
    void on_line_message_returnPressed();
    void on_line_ip_returnPressed();
};

#endif // PRIVATECHAT_H
