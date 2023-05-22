#include "privatechat.h"
#include "ui_privatechat.h"

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);



}

PrivateChat::~PrivateChat()
{
    delete ui;

}

void PrivateChat::getVars(QString vars, qint8 type)
{
   ip = vars;
   ui->textEdit1->setText(ip);
   if (type == 5)
   {
        ui->textEdit1->setText("connecting...");
   }
}


void PrivateChat::on_butt_public_clicked()
{
    emit secondWindow();
    this->close();
}


void PrivateChat::on_butt_connect_clicked()
{
    mTcpServer = new QTcpServer(this);
    counter = 0;

    connect(mTcpServer, &QTcpServer::newConnection,this, &PrivateChat::slotNewConnection);
    if(!mTcpServer->listen(QHostAddress::Any, 25252)) // ИЗМЕНИТЬ вфовщвофцвцфвоцфвцфовцфвдцводцвоводфцво
    {
        qDebug() << "SERVER IS NOT STARTED!";
    }
    else {
        qDebug() << "SERVER IS STARTED!";
        server_status = 1;

        QString buff = ui->textEdit1->toPlainText();
        ui->textEdit1->setText(buff + '\n' + "Запрос на секретный чат отправлен клиенту: ");
    }

    emit test1("lol",ui->line_ip->text(),5);

}

void PrivateChat::slotNewConnection()
{
    if(server_status == 1)
    {
        counter++;
        QTcpSocket *temp = mTcpServer->nextPendingConnection();
        int id = (int)temp->socketDescriptor();
        SClients[id] = temp;
        qDebug() << "AMOUNT OF USERS:" << counter << Qt::endl;
        connect(temp, &QTcpSocket::readyRead, this, &PrivateChat::slotServerReadMany);
        connect(temp, &QTcpSocket::disconnected, this, &PrivateChat::slotClientDisconnected);

    }
}

void PrivateChat::slotServerReadMany() //serv
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

    ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + buffer);
}

void PrivateChat::slotClientDisconnected()
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

void PrivateChat::Sconnected()
{
    QMessageBox msg;
    msg.setText("Connected to server!");
    msg.exec();

}

void PrivateChat::Sdisconnected()
{
    sock->close();
    QMessageBox msg;
    msg.setText("Disconnected from server!");
    msg.exec();
}

void PrivateChat::readData() // client
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
            ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + message);
            message.clear();
        }
    }

}

void PrivateChat::error()
{
    QMessageBox msg;
    msg.setText(sock->errorString());
    msg.exec();
}


