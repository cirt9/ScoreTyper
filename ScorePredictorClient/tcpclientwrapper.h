#ifndef TCPCLIENTWRAPPER_H
#define TCPCLIENTWRAPPER_H

#include <QHostAddress>
#include <tcpclient.h>

class TcpClientWrapper : public QObject
{
    Q_OBJECT

private:
    TcpClient * client;

public:
    explicit TcpClientWrapper(QObject * parent = nullptr);
    ~TcpClientWrapper();

    TcpClient * getClient() const;

signals:
    void connectToServer(const QHostAddress & address, quint16 port);
    void disconnectFromServer();
    void connected();
    void disconnected();
    void serverClosed();
    void serverNotFound();
    void connectionRefused();
    void networkError();
    void socketTimeoutError();
    void unidentifiedError();
    void sendData(const QVariantList & data);
};

#endif // TCPCLIENTWRAPPER_H
