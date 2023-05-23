#include "privatechat.h"
#include "ui_privatechat.h"

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    // Создание TCP сокета
    sock = new QTcpSocket();
    connect(sock, SIGNAL(readyRead()),this,SLOT(readData()));
    connect(sock, SIGNAL(connected()), SLOT(Sconnected()));
    connect(sock, SIGNAL(disconnected()), SLOT(Sdisconnected()));
    connect(sock, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), SLOT(error()));


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

    foreach(int i, SClients.keys())
    {
        QTextStream os(SClients[i]);
        SClients[i]->close();
        SClients.remove(i);
    }
    SERVER_STATE = false;
    mTcpServer->close();

}

void PrivateChat::getVars(QString nick, QString text, qint8 type)
{
    if (type == INIT)
    {
        nickname = nick;
        myIp = text;
    }
//    else
//    {
//        QString buff = ui->textEdit1->toPlainText();
//        ui->textEdit1->setText(buff + '\n' + nick + ": " + text);
//    }
    int a = 0;
}

QByteArray PrivateChat::prepareData(QString var1, QString var2, qint8 type)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out << qint64(0);
    out << qint8(type);
    out << var1;
    out << var2;
    out.device()->seek(qint64(0));
    out << qint64(data.size() - sizeof(qint64));

    return data;
}


void PrivateChat::slotNewConnection() // server
{
    if(SERVER_STATE and !SClients.size())
    {
        QTcpSocket *temp = mTcpServer->nextPendingConnection();
        QHostAddress tempIp;
        tempIp.setAddress(temp->peerAddress().toIPv4Address());

        QString mText = "Клиент с адресом ";
        mText += tempIp.toString();
        mText += " отправил запрос на подключение. Принять?";
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Запрос на подключение", mText, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            int id = (int)temp->socketDescriptor();
            SClients[id] = temp;
            counter++;
            qDebug() << "AMOUNT OF USERS:" << counter << Qt::endl;
            connect(temp, &QTcpSocket::readyRead, this, &PrivateChat::slotServerReadMany);
            connect(temp, &QTcpSocket::disconnected, this, &PrivateChat::slotClientDisconnected);

            temp->write(prepareData(nickname,myIp,TCP_ACCETP)); // Отправка подтверждения клиенту

            ui->line_ip->setText(tempIp.toString());
            ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + "Соединение с пользователем установлено!");
            ui->butt_connect->setStyleSheet(ui->butt_connect->styleSheet() + "background-color: rgb(85, 170, 255); color: white;");
            ui->butt_connect->setText("оnline");
            ui->line_ip->setEnabled(false);
            qDebug() << "yes";
        }
        else
        {
            temp->write(prepareData(nickname,myIp,TCP_REJECT)); // Отправка отключения клиенту
            temp->disconnectFromHost();
            qDebug() << "no";
        }
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

    QString buff = ui->textEdit1->toPlainText();
    ui->textEdit1->setText(buff + '\n' + nick + ": " + text);

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

        SClients.clear();

        ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + "Пользователь отключился!");
        ui->butt_connect->setText("Connect");
        ui->butt_connect->setStyleSheet("background-color: rgb(193, 193, 193); color: rgb(0, 0, 0); border-radius: 10px;");
        ui->line_ip->setEnabled(true);
    }
}


void PrivateChat::on_butt_public_clicked()
{
    emit secondWindow();
    this->close();
}


void PrivateChat::on_butt_connect_clicked() // Подключение клиентом
{
    if (!SOCKET_STATE && !SClients.size()) // подключаемся клиентом
    {
        QString ip = ui->line_ip->text();
        sock->connectToHost(ip, 25252);
        sock->open(QIODevice::ReadWrite);
    }
    else if (SOCKET_STATE) // Отключение клиентом
    {
        sock->disconnectFromHost();
    }
    else if (SClients.size()) // Отключение сервером
    {
        SClients.last()->close();
        SClients.clear();
    }
}

void PrivateChat::Sconnected()
{
    ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + "Пользователь найден. Ожидание подтверждения...");
}

void PrivateChat::Sdisconnected()
{
    sock->close();
    SOCKET_STATE = false;
    ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + "Соединение закрыто!");
    ui->butt_connect->setStyleSheet("background-color: rgb(193, 193, 193); color: rgb(0, 0, 0); border-radius: 10px;");
    ui->line_ip->setEnabled(true);
    ui->butt_connect->setText("Connect");
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

    if (type == TCP_ACCETP)
    {
        SOCKET_STATE = true;
        ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + "Соединение установлено!");
        ui->butt_connect->setStyleSheet(ui->butt_connect->styleSheet() + "background-color: rgb(85, 170, 255); color: white;");
        ui->line_ip->setEnabled(false);
        ui->butt_connect->setText("оnline");
    }
    else if (type == TCP_REJECT)
    {
        ui->textEdit1->setText(ui->textEdit1->toPlainText() + '\n' + "Пользователь отклонил ваш запрос!");
    }
    else
    {
        QString buff = ui->textEdit1->toPlainText();
        ui->textEdit1->setText(buff + '\n' + nick + ": " + text);
    }

}

void PrivateChat::error()
{
    qDebug() << sock->errorString();
}

void PrivateChat::on_butt_sendMessage1_clicked() // Отправка личных сообщений
{
    QString buff = ui->textEdit1->toPlainText();
    QString writeBuf = ui->line_message->text();
    QByteArray data = prepareData(nickname, writeBuf, PRIVATE);

    if (SOCKET_STATE)
    {
        sock->write(data);
        ui->line_message->setText("");
        ui->textEdit1->setText(buff + '\n' + "Вы" + ": " + writeBuf);
    }
    else if (SClients.size())
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

