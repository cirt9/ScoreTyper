#include "tcpconnection.h"

TcpConnection::TcpConnection(QObject * parent) : QObject(parent)
{
    qDebug() << "Connection created";
}

void TcpConnection::accept(qintptr descriptor)
{
    qDebug() << "Accepting connection " << descriptor;

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &TcpConnection::connected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpConnection::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpConnection::read);
    connect(socket, &QTcpSocket::stateChanged, this, &TcpConnection::stateChanged);
    connect(socket, static_cast<void (QTcpSocket::*) (QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this, &TcpConnection::error);

    if(!socket->setSocketDescriptor(descriptor))
    {
        qDebug() << "Could not accept connection";
        return;
    }

    qDebug() << "Connection " << descriptor << "accepted";

    emit started();
}

void TcpConnection::quit()
{
    qDebug() << "Connection quitting...";
    socket->disconnectFromHost();
}

void TcpConnection::connected()
{
    qDebug() << "Client connected";
}

void TcpConnection::disconnected()
{
    qDebug() << "Client disconnected";

    emit finished();
}

void TcpConnection::read()
{
    qDebug() << socket->readAll();
}

void TcpConnection::stateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "State changed" << state;
}

void TcpConnection::error(QAbstractSocket::SocketError error)
{
    qDebug() << "Error:" << error;
}
