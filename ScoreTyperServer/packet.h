#ifndef PACKET_H
#define PACKET_H

#include <QByteArray>
#include <QVariantList>
#include <QDataStream>

#include <QDebug>

class Packet
{
private:
    QVariantList data;
    QByteArray serializedData;
    bool corrupted;

    static const QVariant START_OF_PACKET;
    static const QVariant END_OF_PACKET;
    static const QVariant PACKET_ID_MIN;
    static const QVariant PACKET_ID_MAX;

    void serialize();
    void unserialize(QDataStream & in);
    void clean();

public:
    explicit Packet(const QVariantList & packetData);
    explicit Packet(QDataStream & in);
    ~Packet() {}

    void setSerializedData(const QVariantList & packetData);
    void setUnserializedData(QDataStream & in);

    QVariantList getUnserializedData() const;
    QByteArray getSerializedData() const;
    bool isCorrupted() const;

    static const QVariant PACKET_ID_LOGIN;
};

#endif // PACKET_H
