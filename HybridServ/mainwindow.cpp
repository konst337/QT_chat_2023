#include "mainwindow.h"
#include "qobjectdefs.h"
#include "ui_mainwindow.h"



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
    connect(this, SIGNAL(sendVars(QString,QString, qint8)), pChat, SLOT(getVars(QString,QString, qint8)));

 }


void MainWindow::showWindow()
{
    this->setGeometry(pChat->geometry());
    this->show();
}


MainWindow::~MainWindow()
{
    sendSome(nickname, "", qint8(UDP_EXIT));
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

    if (type == UDP_USUAL_MESSAGE)
    {
        if (myIp.isEqual(*address))
        {
            nick = "Вы";
        }
      QString buff = ui->textEdit->toPlainText();
      ui->textEdit->setText(buff + '\n' + nick + ": " + text);
    }
    else if (type == UDP_PERSON_ONLINE && !myIp.isEqual(*address))
    {
        QString buff = ui->textEdit->toPlainText();
        QHostAddress tempA;
        tempA.setAddress(address->toIPv4Address());
        ui->textEdit->setText(buff + '\n' + "Пользователь " + nick + " c адресом: " + tempA.toString() + " в сети");
    }
    else if (type == UDP_WHO_IS_ONLINE && !myIp.isEqual(*address))
    {
        sendSome(nickname,"",qint8(UDP_PERSON_ONLINE));
    }
    else if (type == UDP_SETUP && myIp.isNull())
    {
       QString buff = ui->textEdit->toPlainText();
       myIp.setAddress(address->toIPv4Address());
       ui->textEdit->setText("Вы в сети! Ваш IP: " + myIp.toString());
    }
    else if (type == UDP_EXIT)
    {
       QString buff = ui->textEdit->toPlainText();
       QHostAddress tempA;
       tempA.setAddress(address->toIPv4Address());
       ui->textEdit->setText(buff + '\n' + "Пользователь " + nick + " c адресом: " + tempA.toString() + " вышел из сети");
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

    if (type == TCP_PRIVATE_MESSAGE)
    {
       SClients.first()->write(data);
    }
    else
    {
       usock->writeDatagram(data, QHostAddress::Broadcast, port);
    }

//    ui->textEdit->setText(ui->textEdit->toPlainText() + '\n' + "Отправлен запрос " + QString::number(type));
}


void MainWindow::on_butt_hostServ_clicked() // Host server
{
    mTcpServer = new QTcpServer(this);
    counter = 0;
    connect(mTcpServer, &QTcpServer::newConnection,this, &MainWindow::slotNewConnection);
    if(!mTcpServer->listen(QHostAddress::Any, 25252))
    {
        qDebug() << "SERVER IS NOT STARTED!";
    }
    else
    {
        qDebug() << "SERVER IS STARTED!";
        server_status = 1;
        ui->butt_hostServ->setStyleSheet(ui->butt_hostServ->styleSheet() + "background-color: rgb(85, 170, 255); color: white;");
        ui->butt_hostServ->setEnabled(false);
        ui->butt_hostServ->setText("TCP оnline");
    }
}

void MainWindow::slotNewConnection()
{

    if(server_status == 1)
    {
        counter++;
        QTcpSocket *temp = mTcpServer->nextPendingConnection();
        QMessageBox msg;
        msg.setText(temp->peerAddress().toString()); // Подтверждение подключения
        msg.exec();
        int id = (int)temp->socketDescriptor();
        SClients[id] = temp;
        qDebug() << "AMOUNT OF USERS:" << counter << Qt::endl;
        connect(temp, &QTcpSocket::readyRead, this, &MainWindow::slotServerReadMany);
        connect(temp, &QTcpSocket::disconnected, this, &MainWindow::slotClientDisconnected);
//        QMessageBox msg;
        msg.setText("Новое подключение!");
        msg.exec();

//        emit sendVars(nick,text,type);
    }
}

void MainWindow::slotServerReadMany() //serv read
{
    QTcpSocket *temp = (QTcpSocket*)sender();

    QByteArray datagram;
    datagram.resize(temp->readBufferSize());
    datagram = temp->readAll();

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

    emit sendVars(nick,text,type);
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


void MainWindow::on_nick_textChanged(const QString &arg1)
{
    nickname = ui->nick->text();
}


void MainWindow::on_nick_returnPressed()
{
    sendSome(nickname,"",qint8(UDP_WHO_IS_ONLINE));
}


void MainWindow::on_butt_hostServ_2_clicked() // go online
{
 if (!usock)
 {
    usock = new QUdpSocket(this);
    usock->bind(QHostAddress::Any, port);
    connect(usock, SIGNAL(readyRead()), SLOT(readSome()));
    sendSome(nickname, "", qint8(UDP_SETUP));
    sendSome(nickname, "", qint8(UDP_PERSON_ONLINE));
    sendSome(nickname, "", qint8(UDP_WHO_IS_ONLINE));
    ui->nick->setReadOnly(true);

    ui->butt_hostServ_2->setStyleSheet(ui->butt_hostServ_2->styleSheet() + "background-color: rgb(85, 170, 255); color: white;");
    ui->butt_hostServ_2->setText("Online");
 }
}


void MainWindow::on_lineEdit_returnPressed()
{
 ui->butt_sendMessage->click();
}

void MainWindow::on_butt_sendMessage_clicked()
{
    if (usock)
    {
    QString text = ui->lineEdit->text();
    sendSome(nickname, text, qint8(UDP_USUAL_MESSAGE));
    }
    ui->lineEdit->setText("");
}



void MainWindow::on_butt_hostServ_3_clicked()
{
    emit sendVars(nickname,myIp.toString(), 9);
    pChat->setGeometry(this->geometry());
    pChat->show(); // Показываем окно
    this->close();
}

