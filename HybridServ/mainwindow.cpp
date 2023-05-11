#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_butt_hostServ_clicked()
{
    mTcpServer = new QTcpServer(this);
    counter = 0;

    connect(mTcpServer, &QTcpServer::newConnection,this, &MainWindow::slotNewConnection);
    if(!mTcpServer->listen(QHostAddress::Any, 25252))
    {
        qDebug() << "SERVER IS NOT STARTED!";
    }
    else {
        qDebug() << "SERVER IS STARTED!";
        server_status = 1;
    }

}

void MainWindow::slotNewConnection()
{
    if(server_status == 1)
    {
        counter++;
        QTcpSocket *temp = mTcpServer->nextPendingConnection();
        int id = (int)temp->socketDescriptor();
        SClients[id] = temp;
        qDebug() << "AMOUNT OF USERS:" << counter << Qt::endl;
        connect(temp, &QTcpSocket::readyRead, this, &MainWindow::slotServerReadMany);
        connect(temp, &QTcpSocket::disconnected, this, &MainWindow::slotClientDisconnected);

    }
}

void MainWindow::slotServerReadMany() //serv
{
    QTcpSocket *temp = (QTcpSocket*)sender();
    //    int id = (int)temp->socketDescriptor();

    QByteArray buffer, buffer1;

    while(temp->bytesAvailable() > 0) //canReadLine
    {
        buffer1 = temp->readAll();
        buffer.append(buffer1);
    }

    if (buffer == "1")
    {
        temp->write("ping 1");

    }
    if (buffer == "2")
    {
        temp->write("ping 2");

    }

    ui->textEdit->setText(ui->textEdit->toPlainText() + '\n' + buffer);
}


void MainWindow::on_butt_sendMessage_clicked()
{
    QString writeBuf = ui->lineEdit->text();
    if (sock)
    {
        sock->write(writeBuf.toStdString().c_str());
    }
    else
    {
        SClients.first()->write(writeBuf.toStdString().c_str());
    }
    ui->lineEdit->setText("");
}

void MainWindow::slotClientDisconnected()
{

    if(server_status==1)
    {

        QTcpSocket *temp = (QTcpSocket*)sender();
        //        int id = (int)temp->socketDescriptor();
        counter --;
        qDebug() << "AMOUNT OF USERS:" << counter << Qt::endl;
        qDebug() << "USER HAS BEEN DISCONNECTED!";
        temp->write("disconneted");
        temp->close();
    }
}


void MainWindow::on_butt_connServ_clicked()
{
    sock = new QTcpSocket();
    sock->connectToHost("127.0.0.1",25252);
    sock->open(QIODevice::ReadWrite);
    connect(sock, SIGNAL(readyRead()),this,SLOT(readData()));
    connect(sock, SIGNAL(connected()), SLOT(Sconnected()));
    connect(sock, SIGNAL(disconnected()), SLOT(Sdisconnected()));
    connect(sock, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), SLOT(error()));

    QMessageBox msg;

}

void MainWindow::Sconnected()
{
    QMessageBox msg;
    msg.setText("Connected to server!");
    msg.exec();

}

void MainWindow::Sdisconnected()
{
    sock->close();
    QMessageBox msg;
    msg.setText("Disconnected from server!");
    msg.exec();
}

void MainWindow::error()
{
    QMessageBox msg;
    msg.setText(sock->errorString());
    msg.exec();
}

void MainWindow::readData() // client
{
    if (sock)
    {
        if(sock->isOpen())
        {
            QString message;

            while(sock->bytesAvailable())
            {
                QString buffer = sock->readAll();
                message.append(buffer);
            }
            ui->textEdit->setText(ui->textEdit->toPlainText() + '\n' + message);
            message.clear();
        }
    }

}


