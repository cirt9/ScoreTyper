#ifndef QUERY_H
#define QUERY_H

#include <dbconnection.h>
#include <QSqlQuery>
#include <QSharedPointer>
#include <../ScoreTyperClient/tournament.h>

#include <QDebug>

class Query : public QSqlQuery
{
public:
    Query(const QSqlDatabase & dbConnection);
    ~Query() {}

    bool findUserId(const QString & nickname);
    bool isUserRegistered(const QString & nickname);
    bool registerUser(const QString & nickname, const QString & password);
    bool isPasswordCorrect(const QString & nickname, const QString & password);
    bool getUserProfile(const QString & nickname);
    bool tournamentExists(const QString & tournamentName, unsigned int hostId);
    bool createTournament(const Tournament & tournament, unsigned int hostId, const QString & password);
    void findNewestTournamentsList(unsigned int hostId, const QDateTime & dateTime);
};

#endif // QUERY_H
