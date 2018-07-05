#ifndef PACKETPROCESSOR_H
#define PACKETPROCESSOR_H

#include <packet.h>
#include <dbconnection.h>
#include <query.h>
#include <QSharedPointer>
#include <../ScoreTyperClient/tournament.h>

namespace Server
{
    class PacketProcessor: public QObject
    {
        Q_OBJECT

    private:
        QSharedPointer<DbConnection> dbConnection;

        void registerUser(const QVariantList & userData);
        void loginUser(const QVariantList & userData);
        void manageUserProfileRequest(const QVariantList & userData);
        void manageTournamentCreationRequest(QVariantList & tournamentData);
        void manageTournamentsListRequest(const QVariantList & requestData);
        void manageFindingTournamentsRequest(const QVariantList & requestData);

    public:
        explicit PacketProcessor(QSharedPointer<DbConnection> connection, QObject * parent = nullptr);
        ~PacketProcessor() {}

    public slots:
        void processPacket(const Packet & packet);

    signals:
        void response(const QVariantList & data);
    };
}

#endif // PACKETPROCESSOR_H