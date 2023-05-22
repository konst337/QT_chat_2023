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

    QTcpSocket *sock;
        //server
    QMap<int, QTcpSocket*> SClients;
    int counter;
    QTcpServer *mTcpServer;
    int server_status;
    QString ip;

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

    //server
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerReadMany();

    void on_butt_connect_clicked();

    void getVars(QString vars, qint8 type);



};

#endif // PRIVATECHAT_H
