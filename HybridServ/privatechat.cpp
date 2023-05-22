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
    if (SOCKET_STATE)
    {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out << qint64(0);
        out << qint8(PRIVATE);
        out << nickname;
        out << writeBuf;
        out.device()->seek(qint64(0));
        out << qint64(data.size() - sizeof(qint64));
        sock->write(data);
    }
    else
    {
        emit test1(nickname, writeBuf, PRIVATE);
    }
    ui->line_message->setText("");
    ui->textEdit1->setText(buff + '\n' + "Вы" + ": " + writeBuf);
}


void PrivateChat::on_line_message_returnPressed()
{
    ui->butt_sendMessage1->click();
}

