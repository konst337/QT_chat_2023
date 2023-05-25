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
    int PORT = 25252;

    // коды событий TCP
    qint8 PRIVATE = 7;
    qint8 INIT = 9;
    qint8 TCP_ACCETP = 11;
    qint8 TCP_REJECT = 12;

    // сервер
    QMap<int, QTcpSocket*> SClients;
    QTcpServer *mTcpServer;
    int counter;

signals:
    void secondWindow();
private slots:
    void getVars(QString nick, QString message, qint8 type);
    QByteArray prepareData(QString var1, QString var2, qint8 type);

    // слоты клиента
    void readData();
    void error();
    void Sdisconnected();
    void Sconnected();

    // слоты сервера
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerReadMany();

    // слоты ui-объектов
    void on_butt_connect_clicked();
    void on_butt_sendMessage1_clicked();
    void on_line_message_returnPressed();
    void on_line_ip_returnPressed();
    void on_butt_public_clicked();

};

#endif // PRIVATECHAT_H
