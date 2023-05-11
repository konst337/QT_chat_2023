#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QTcpSocket>
#include <QTcpServer>
#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <QByteArray>
#include <QDataStream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_butt_hostServ_clicked();
    void on_butt_connServ_clicked();

    //client
    void readData();
    void error();
    void Sdisconnected();
    void Sconnected();

    //server
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerReadMany();

    void on_butt_sendMessage_clicked();

private:
    //client
    Ui::MainWindow *ui;
    QTcpSocket *sock;

    //server
    QMap<int, QTcpSocket*> SClients;
    int counter;
    QTcpServer * mTcpServer;
    int server_status;
};
#endif // MAINWINDOW_H
