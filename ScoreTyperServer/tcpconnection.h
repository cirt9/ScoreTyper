#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <QTcpSocket>

#include <QDebug>

class TcpConnection : public QObject
{
    Q_OBJECT

private:
    QTcpSocket * socket;

private slots:
    void read();
    void stateChanged(QAbstractSocket::SocketState state);
    void error(QAbstractSocket::SocketError error);
    void connected();
    void disconnected();

public:
    explicit TcpConnection(QObject * parent = nullptr);
    ~TcpConnection() {}

public slots:
    void accept(qintptr descriptor);
    void quit();

signals:
    void started();
    void finished();
};

#endif // TCPCONNECTION_H