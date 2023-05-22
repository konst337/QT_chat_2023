#include "mainwindow.h"
#include "qobjectdefs.h"
#include "ui_mainwindow.h"
int port = 7755;
QString nickname;
qint8 USUAL_MESSAGE = 1;
qint8 PERSON_ONLINE = 2;
qint8 WHO_IS_ONLINE = 3;
qint8 SETUP = 0;
qint8 EXIT = 4;
qint8 CHAT = 5;

QHostAddress *myIp = new QHostAddress();

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    pChat = new PrivateChat();
    connect(pChat, &PrivateChat::secondWindow, this, &MainWindow::showWindow);

    connect(pChat, SIGNAL(test1(QString,QString,qint8)), this, SLOT(sendSome(QString,QString,qint8)));
//    connect(pChat, SLOT(getVars(QString)), this, SIGNAL(sendVars(QString)));
    connect(this, SIGNAL(sendVars(QString, qint8)), pChat, SLOT(getVars(QString, qint8)));
}


void MainWindow::showWindow()
{
    this->setGeometry(pChat->geometry());
    this->show();
}


MainWindow::~MainWindow()
{
    sendSome(nickname, "", qint8(EXIT));
    delete ui;
}

void MainWindow::readSome()
{
    QByteArray datagram;
    datagram.resize(usock->pendingDatagramSize());
    QHostAddress *address = new QHostAddress();
    usock->readDatagram(datagram.data(), datagram.size(), address);

    QDataStream in(&datagram, QIODevice::ReadOnly);

    qint64 size = -1;
    if(in.device()->size() > sizeof(qint64)) {
        in >> size;
    } else return;
    if (in.device()->size() - sizeof(qint64) < size) return;


    qint8 type = 0;
    in >> type;
    QString nick;
    in >> nick;
    QString text;
    in >> text;

    if (type == USUAL_MESSAGE)
    {
        if (myIp.isEqual(*address))
        {
            nick = "Вы";
        }
      QString buff = ui->textEdit->toPlainText();
      ui->textEdit->setText(buff + '\n' + nick + ": " + text);
    }
    else if (type == PERSON_ONLINE && !myIp.isEqual(*address))
    {
        QString buff = ui->textEdit->toPlainText();
        QHostAddress tempA;
        tempA.setAddress(address->toIPv4Address());
        ui->textEdit->setText(buff + '\n' + "Пользователь " + nick + " c адресом: " + tempA.toString() + " в сети");
    }
    else if (type == WHO_IS_ONLINE && !myIp.isEqual(*address))
    {
        sendSome(nickname,"",qint8(PERSON_ONLINE));
    }
    else if (type == SETUP && myIp.isNull())
    {
       QString buff = ui->textEdit->toPlainText();
       myIp.setAddress(address->toIPv4Address());
       ui->textEdit->setText("Вы в сети! Ваш IP: " + myIp.toString());
    }
    else if (type == EXIT)
    {
       QString buff = ui->textEdit->toPlainText();
       QHostAddress tempA;
       tempA.setAddress(address->toIPv4Address());
       ui->textEdit->setText(buff + '\n' + "Пользователь " + nick + " c адресом: " + tempA.toString() + " вышел из сети");
    }
    else if (type == CHAT)
    {
       pChat->setGeometry(this->geometry());
       pChat->show();
       emit sendVars(text, CHAT);
       this->close();
    }

}

void MainWindow::sendSome(QString nick, QString message, qint8 type)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << qint64(0);
    out << qint8(type);
    out << nick;
    out << message;
    out.device()->seek(qint64(0));
    out << qint64(data.size() - sizeof(qint64));
    usock->writeDatagram(data, QHostAddress::Broadcast, port);
//    ui->textEdit->setText(ui->textEdit->toPlainText() + '\n' + "Отправлен запрос " + QString::number(type));
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
    if (usock)
    {
        QString text = ui->lineEdit->text();
        sendSome(nickname, text, qint8(USUAL_MESSAGE));
    }

//    QString writeBuf = ui->lineEdit->text();
//    if (sock)
//    {
//        sock->write(writeBuf.toStdString().c_str());
//    }
//    else
//    {
//        SClients.first()->write(writeBuf.toStdString().c_str());
//    }
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



void MainWindow::on_nick_textChanged(const QString &arg1)
{

    nickname = ui->nick->text();

}


void MainWindow::on_nick_textEdited(const QString &arg1)
{


}


void MainWindow::on_nick_returnPressed()
{
 sendSome(nickname,"",qint8(WHO_IS_ONLINE));
}


void MainWindow::on_butt_hostServ_2_clicked() // go online
{
 if (!usock){
    usock = new QUdpSocket(this);
    usock->bind(QHostAddress::Any, port);
    connect(usock, SIGNAL(readyRead()), SLOT(readSome()));
    sendSome(nickname, "", qint8(SETUP));
    sendSome(nickname, "", qint8(PERSON_ONLINE));
    sendSome(nickname, "", qint8(WHO_IS_ONLINE));
    ui->nick->setReadOnly(true);

    ui->butt_hostServ_2->setStyleSheet(ui->butt_hostServ_2->styleSheet() + "background-color: rgb(85, 170, 255); color: white;");
    ui->butt_hostServ_2->setEnabled(false);
    ui->butt_hostServ_2->setText("Online");
 }
}


void MainWindow::on_lineEdit_returnPressed()
{
    if (usock)
    {
        QString text = ui->lineEdit->text();
        sendSome(nickname, text, qint8(USUAL_MESSAGE));
    }
    ui->lineEdit->setText("");
}


void MainWindow::on_butt_hostServ_3_clicked()
{
    emit sendVars(myIp.toString(), 8);

    pChat->setGeometry(this->geometry());
    pChat->show(); // Показываем окно
    this->close();

}

