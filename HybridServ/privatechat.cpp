#include "privatechat.h"
#include "ui_privatechat.h"

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);

    // Запуск TCP сервера
    mTcpServer = new QTcpServer(this);
    counter = 0;
    connect(mTcpServer, &QTcpServer::newConnection,this, &PrivateChat::slotNewConnection);
    if(!mTcpServer->listen(QHostAddress::Any, 25252))
    {
        qDebug() << "SERVER IS NOT STARTED!";
    }
    else
    {
        qDebug() << "SERVER IS STARTED!";
        SERVER_STATE = true;
    }

}

PrivateChat::~PrivateChat()
{
    delete ui;
}

void PrivateChat::getVars(QString nick, QString text, qint8 type)
{
    if (type == INIT)
    {
        nickname = nick;
        myIp = text;
    }
    else
    {
        QString buff = ui->textEdit1->toPlainText();
        ui->textEdit1->setText(buff + '\n' + nick + ": " + text);
    }
}

void PrivateChat::slotNewConnection() // server
{
    if(SERVER_STATE)
    {
        counter++;
        QTcpSocket *temp = mTcpServer->nextPendingConnection();
        QMessageBox msg;
        QHostAddress tempIp;
        tempIp.setAddress(temp->peerAddress().toIPv4Address());
        msg.setText(tempIp.toString()); // Подтверждение подключения
        msg.exec();
        int id = (int)temp->socketDescriptor();
        SClients[id] = temp;
        qDebug() << "AMOUNT OF USERS:" << counter << Qt::endl;
        connect(temp, &QTcpSocket::readyRead, this, &PrivateChat::slotServerReadMany);
        connect(temp, &QTcpSocket::disconnected, this, &PrivateChat::slotClientDisconnected);
        //        QMessageBox msg;
        msg.setText("Новое подключение!");
        msg.exec();

        ui->line_ip->setText(tempIp.toString());

        ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + "Client connected!");
        ui->butt_connect->setStyleSheet(ui->butt_connect->styleSheet() + "background-color: rgb(85, 170, 255); color: white;");
        //        emit sendVars(nick,text,type);
    }
}

void PrivateChat::slotServerReadMany() //serv read
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

    if (type == INIT)
    {
        nickname = nick;
        myIp = text;
    }
    else
    {
        QString buff = ui->textEdit1->toPlainText();
        ui->textEdit1->setText(buff + '\n' + nick + ": " + text);
    }
}



void PrivateChat::slotClientDisconnected() // server
{
    if(SERVER_STATE)
    {

        QTcpSocket *temp = (QTcpSocket*)sender();
        //        int id = (int)temp->socketDescriptor();
        counter --;
        qDebug() << "AMOUNT OF USERS:" << counter << Qt::endl;
        qDebug() << "USER HAS BEEN DISCONNECTED!";
        temp->write("disconneted");
        temp->close();

        ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + "Client disconnected from server!");
        ui->butt_connect->setStyleSheet("background-color: rgb(193, 193, 193); color: rgb(0, 0, 0); border-radius: 10px;");
    }
}



void PrivateChat::on_butt_public_clicked()
{
    emit secondWindow();
    this->close();
}


void PrivateChat::on_butt_connect_clicked() // Подключение клиентом
{
    if (!sock)
    {
        sock = new QTcpSocket();
        connect(sock, SIGNAL(readyRead()),this,SLOT(readData()));
        connect(sock, SIGNAL(connected()), SLOT(Sconnected()));
        connect(sock, SIGNAL(disconnected()), SLOT(Sdisconnected()));
        connect(sock, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), SLOT(error()));
    }
    if (!SOCKET_STATE)
    {
        QString ip = ui->line_ip->text();
        sock->connectToHost(ip, 25252);
        sock->open(QIODevice::ReadWrite);
    }
    else
    {
        sock->disconnectFromHost();
    }
}

void PrivateChat::Sconnected()
{
    SOCKET_STATE = true;
    ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + "Connected to server!");
    ui->butt_connect->setStyleSheet(ui->butt_connect->styleSheet() + "background-color: rgb(85, 170, 255); color: white;");
    ui->line_ip->setEnabled(false);
    ui->butt_connect->setText("оnline");
}

void PrivateChat::Sdisconnected()
{
    sock->close();
    SOCKET_STATE = false;
    ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + "Disconnected from server!");
    ui->butt_connect->setStyleSheet("background-color: rgb(193, 193, 193); color: rgb(0, 0, 0); border-radius: 10px;");
    ui->line_ip->setEnabled(true);
    ui->butt_connect->setText("оnline");
}

void PrivateChat::readData() // Чтение на стороне клиента
{
    QTcpSocket *temp = (QTcpSocket*)sender();
    QByteArray datagram;
    QDataStream in(&datagram, QIODevice::ReadOnly);

    datagram.resize(temp->readBufferSize());
    datagram = temp->readAll();

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

    QString buff = ui->textEdit1->toPlainText();
    ui->textEdit1->setText(buff + '\n' + nick + ": " + text);

}

void PrivateChat::error()
{
    QMessageBox msg;
    msg.setText(sock->errorString());
    msg.exec();
}

void PrivateChat::on_butt_sendMessage1_clicked() // Отправка личных сообщений
{
    QString buff = ui->textEdit1->toPlainText();
    QString writeBuf = ui->line_message->text();
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << qint64(0);
    out << qint8(PRIVATE);
    out << nickname;
    out << writeBuf;
    out.device()->seek(qint64(0));
    out << qint64(data.size() - sizeof(qint64));
    if (SOCKET_STATE)
    {
        sock->write(data);
        ui->line_message->setText("");
        ui->textEdit1->setText(buff + '\n' + "Вы" + ": " + writeBuf);
    }
    else if (SERVER_STATE) // Перепроверить !!!
    {
        SClients.first()->write(data);
        ui->line_message->setText("");
        ui->textEdit1->setText(buff + '\n' + "Вы" + ": " + writeBuf);
    }

}


void PrivateChat::on_line_message_returnPressed()
{
    ui->butt_sendMessage1->click();
}


void PrivateChat::on_line_ip_returnPressed()
{
    ui->butt_connect->click();
}

