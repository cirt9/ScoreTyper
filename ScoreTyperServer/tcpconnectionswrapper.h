#ifndef TCPCONNECTIONSWRAPPER_H
#define TCPCONNECTIONSWRAPPER_H

#include <QObject>
#include <QThread>
#include <tcpconnections.h>

#include <QDebug>

class TcpConnectionsWrapper : public QObject
{
    Q_OBJECT

private:
    QThread * workerThread;
    TcpConnections * connectionPool;
    int numberOfConnections;

private slots:
    void connectionsIncreased();
    void connectionsDecreased();
    void terminate();

public:
    explicit TcpConnectionsWrapper(QObject * parent = nullptr);
    ~TcpConnectionsWrapper();

    int getNumberOfConnections() const;

public slots:
    void close();
    void connectionPending(qintptr descriptor, TcpConnectionsWrapper * pool);

signals:
    void finished();
    void pendingConnection(qintptr descriptor);
    void updated();
    void quit();
};

#endif // TCPCONNECTIONSWRAPPER_H
