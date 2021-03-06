#include "packetprocessor.h"

namespace Server
{
    const QString PacketProcessor::STARTING_MESSAGE_PATH = QString("data/starting_message.txt");
    const QString PacketProcessor::DEFAULT_AVATAR_PATH = QString("avatars/default_avatar.png");

    PacketProcessor::PacketProcessor(QSharedPointer<DbConnection> connection, QObject * parent) : QObject(parent)
    {
        dbConnection = connection;
    }

    void PacketProcessor::processPacket(const Packet & packet)
    {
        if(!dbConnection->isConnected())
            return;

        QVariantList data = packet.getUnserializedData();
        int packetId = data[0].toInt();
        data.removeFirst();

        switch(packetId)
        {
        case Packet::ID_DOWNLOAD_STARTING_MESSAGE: manageDownloadingStartingMessage(); break;
        case Packet::ID_REGISTER: registerUser(data); break;
        case Packet::ID_LOGIN: loginUser(data); break;
        case Packet::ID_DOWNLOAD_USER_PROFILE_INFO: manageDownloadingUserInfo(data); break;
        case Packet::ID_PULL_FINISHED_TOURNAMENTS: managePullingUserTournaments(data, false); break;
        case Packet::ID_PULL_ONGOING_TOURNAMENTS: managePullingUserTournaments(data, true); break;
        case Packet::ID_UPDATE_USER_PROFILE_DESCRIPTION: manageUpdatingUserProfileDescription(data); break;
        case Packet::ID_UPDATE_USER_PROFILE_AVATAR: manageUpdatingUserProfileAvatar(data); break;
        case Packet::ID_CREATE_TOURNAMENT: manageTournamentCreationRequest(data); break;
        case Packet::ID_PULL_TOURNAMENTS: managePullingTournaments(data); break;
        case Packet::ID_JOIN_TOURNAMENT: manageJoiningTournament(data); break;
        case Packet::ID_JOIN_TOURNAMENT_PASSWORD: manageJoiningTournamentWithPassword(data); break;
        case Packet::ID_DOWNLOAD_TOURNAMENT_INFO: manageDownloadingTournamentInfo(data); break;
        case Packet::ID_FINISH_TOURNAMENT: manageTournamentFinishing(data); break;
        case Packet::ID_ADD_NEW_ROUND: manageAddingNewRound(data); break;
        case Packet::ID_DOWNLOAD_TOURNAMENT_LEADERBOARD: manageDownloadingTournamentLeaderboard(data); break;
        case Packet::ID_DOWNLOAD_ROUND_LEADERBOARD: manageDownloadingRoundLeaderboard(data); break;
        case Packet::ID_PULL_MATCHES: managePullingMatches(data); break;
        case Packet::ID_CREATE_MATCH: manageCreatingNewMatch(data); break;
        case Packet::ID_DELETE_MATCH: manageDeletingMatch(data); break;
        case Packet::ID_UPDATE_MATCH_SCORE: manageUpdatingMatchScore(data); break;
        case Packet::ID_PULL_MATCHES_PREDICTIONS: managePullingMatchesPredictions(data); break;
        case Packet::ID_MAKE_PREDICTION: manageMakingPrediction(data); break;
        case Packet::ID_UPDATE_PREDICTION: manageUpdatingPrediction(data); break;

        default: break;
        }
    }

    void PacketProcessor::manageDownloadingStartingMessage()
    {
        QVariantList responseData;
        QFile file(STARTING_MESSAGE_PATH);

        if(!file.open(QIODevice::ReadOnly))
            responseData << Packet::ID_ERROR << QString("Couldn't load starting message.");
        else
        {
            QTextStream fileStream(&file);
            fileStream.setCodec("UTF-8");

            QString startingMessage = fileStream.readAll();

            file.close();
            responseData << Packet::ID_DOWNLOAD_STARTING_MESSAGE << startingMessage;
        }

        emit response(responseData);
    }

    void PacketProcessor::registerUser(const QVariantList & userData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;
        responseData << Packet::ID_REGISTER;

        if(query.isUserRegistered(userData[0].toString()))
            responseData << false << QString("This nickname is already occupied");
        else
        {
            if(query.registerUser(userData[0].toString(), userData[1].toString()))
                responseData << true << QString("Your account has been successfully created");
            else
                responseData << false << QString("A problem occured. Account could not be created");
        }

        emit response(responseData);
    }

    void PacketProcessor::loginUser(const QVariantList & userData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;
        responseData << Packet::ID_LOGIN;

        if(query.isUserRegistered(userData[0].toString()))
        {
            responseData << true;

            if(query.isPasswordCorrect(userData[0].toString(), userData[1].toString()))
                responseData << true << userData[0].toString();
            else
                responseData << false << QString("Invalid password");
        }
        else
            responseData << false << false << QString("Invalid nickname");

        emit response(responseData);
    }

    void PacketProcessor::manageDownloadingUserInfo(const QVariantList & userData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.getUserInfo(userData[0].toString()))
        {
            QString avatarPath = query.value("avatar_path").toString();
            QImage avatar;
            QByteArray avatarData;
            QBuffer avatarBuffer(&avatarData);
            QFileInfo avatarInfo(avatarPath);

            if(!avatarInfo.exists())
                responseData << Packet::ID_ERROR << QString("Couldn't load user data");
            else
            {
                avatar.load(avatarPath);
                avatar.save(&avatarBuffer, avatarInfo.suffix().toLocal8Bit().constData());

                responseData << Packet::ID_DOWNLOAD_USER_PROFILE_INFO << query.value("description")
                             << avatarData;
            }
        }
        else
            responseData << Packet::ID_ERROR << QString("Couldn't load user data");

        emit response(responseData);
    }

    void PacketProcessor::managePullingUserTournaments(const QVariantList & userData, bool opened)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(userData[0].toString()))
        {
            if(opened)
                responseData << Packet::ID_PULL_ONGOING_TOURNAMENTS;
            else
                responseData << Packet::ID_PULL_FINISHED_TOURNAMENTS;

            query.findUserTournaments(query.value("id").toUInt(), opened);

            while(query.next())
            {
                QVariantList tournamentData;
                tournamentData << query.value("name") << query.value("host_name");
                responseData << QVariant::fromValue(tournamentData);
            }
        }
        else
            responseData << Packet::ID_ERROR << QString("User does not exist");

        emit response(responseData);
    }

    void PacketProcessor::manageUpdatingUserProfileDescription(const QVariantList & requestData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(requestData[0].toString()))
        {
            unsigned int userId = query.value("id").toUInt();

            if(query.updateUserProfileDescription(userId, requestData[1].toString()))
            {
                responseData << Packet::ID_UPDATE_USER_PROFILE_DESCRIPTION
                             << QString("Description successfully updated.");
            }
            else
            {
                responseData << Packet::ID_UPDATE_USER_PROFILE_DESCRIPTION_ERROR
                             << QString("Description couldn't be updated. Try again later.");
            }
        }
        else
            responseData << Packet::ID_ERROR << QString("User does not exist");

        emit response(responseData);
    }

    void PacketProcessor::manageUpdatingUserProfileAvatar(const QVariantList & requestData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(requestData[0].toString()))
        {
            unsigned int userId = query.value("id").toUInt();

            query.findUserProfileAvatarPath(userId);
            QString oldAvatarPath = query.value("avatar_path").toString();
            QString newAvatarPath = DEFAULT_AVATAR_PATH.left(DEFAULT_AVATAR_PATH.lastIndexOf('/')) + "/" +
                                    requestData[0].toString() + "." + requestData[2].toString();

            QImage avatar;
            avatar.loadFromData(requestData[1].toByteArray());

            if(!avatar.isNull())
            {
                if(oldAvatarPath != DEFAULT_AVATAR_PATH)
                {
                    QFile oldAvatar(oldAvatarPath);
                    oldAvatar.remove();
                }

                if(avatar.save(newAvatarPath))
                {
                    if(oldAvatarPath != newAvatarPath)
                        query.updateUserProfileAvatarPath(userId, newAvatarPath);

                    responseData << Packet::ID_UPDATE_USER_PROFILE_AVATAR
                                 << QString("Avatar successfully updated.");
                }
                else
                {
                    responseData << Packet::ID_UPDATE_USER_PROFILE_AVATAR_ERROR
                                 << QString("Avatar couldn't be updated. Try again later.");
                }
            }
            else
            {
                responseData << Packet::ID_UPDATE_USER_PROFILE_AVATAR_ERROR
                             << QString("Avatar couldn't be updated. Try again later.");
            }
        }
        else
            responseData << Packet::ID_ERROR << QString("User does not exist");

        emit response(responseData);
    }

    void PacketProcessor::manageTournamentCreationRequest(QVariantList & tournamentData)
    {
        Tournament tournament(tournamentData[0].value<QVariantList>());
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(tournament.getHostName()))
        {
            unsigned int hostId = query.value("id").toUInt();

            if(!query.tournamentExists(tournament.getName(), hostId))
            {
                if(tournament.getEntriesEndTime() < QDateTime::currentDateTime())
                {
                    responseData << Packet::ID_CREATE_TOURNAMENT << false
                                 << QString("Entries end time must be greater than the current time");
                }
                else if(query.createTournament(tournament, hostId, tournamentData[1].toString()))
                {
                    responseData << Packet::ID_CREATE_TOURNAMENT << true
                                 << QString("Tournament created successfully");
                }
                else
                {
                    responseData << Packet::ID_CREATE_TOURNAMENT << false
                                 << QString("Tournament couldn't be created. Try again later.");
                }
            }
            else
            {
                responseData << Packet::ID_CREATE_TOURNAMENT << false <<
                                QString("You can't create two tournaments with the same name!");
            }
        }
        else
            responseData << Packet::ID_ERROR << QString("User does not exist");

        emit response(responseData);
    }

    void PacketProcessor::managePullingTournaments(const QVariantList & requestData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(requestData[0].toString()))
        {
            responseData << Packet::ID_PULL_TOURNAMENTS;
            QDateTime startFromTime;

            if(requestData.size() == 4)
                startFromTime = requestData[3].toDateTime();
            else
                startFromTime = QDateTime::currentDateTime();

            query.findTournaments(query.value("id").toUInt(), startFromTime, requestData[1].toInt(),
                                  requestData[2].toString());

            while(query.next())
            {
                QVariantList tournamentData;
                tournamentData << query.value("name") << query.value("host_name")
                               << query.value("password_required") << query.value("entries_end_time")
                               << query.value("predictors") << query.value("predictors_limit");
                responseData << QVariant::fromValue(tournamentData);
            }
        }
        else
            responseData << Packet::ID_ERROR << QString("User does not exist");

        emit response(responseData);
    }

    void PacketProcessor::manageJoiningTournament(const QVariantList & requestData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(requestData[0].toString()))
        {
            unsigned int userId = query.value("id").toUInt();

            if(query.findUserId(requestData[2].toString()) &&
               query.findTournamentId(requestData[1].toString(), query.value("id").toUInt()))
            {
                unsigned int tournamentId = query.value("id").toUInt();
                QString validationReply = validateTournamentJoining(tournamentId, userId);

                if(validationReply.size() > 0)
                    responseData << Packet::ID_JOIN_TOURNAMENT << false << validationReply;

                else if(query.tournamentRequiresPassword(tournamentId))
                    responseData << Packet::ID_JOIN_TOURNAMENT << false <<
                                    QString("This tournament requires password");

                else if(query.addUserToTournament(tournamentId, userId))
                    responseData << Packet::ID_JOIN_TOURNAMENT << true << QString("You joined the tournament");

                else
                    responseData << Packet::ID_ERROR << false << QString("A problem occured. Try again later.");
            }
            else
                responseData << Packet::ID_JOIN_TOURNAMENT << false << QString("This tournament does not exist");
        }
        else
            responseData << Packet::ID_ERROR << QString("User does not exist");

        emit response(responseData);
    }

    void PacketProcessor::manageJoiningTournamentWithPassword(const QVariantList & requestData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(requestData[0].toString()))
        {
            unsigned int userId = query.value("id").toUInt();

            if(query.findUserId(requestData[2].toString()) &&
               query.findTournamentId(requestData[1].toString(), query.value("id").toUInt()))
            {
                unsigned int tournamentId = query.value("id").toUInt();
                QString validationReply = validateTournamentJoining(tournamentId, userId);

                if(validationReply.size() > 0)
                    responseData << Packet::ID_JOIN_TOURNAMENT << false << validationReply;

                else if(!query.tournamentPasswordIsCorrect(tournamentId, requestData[3].toString()))
                    responseData << Packet::ID_JOIN_TOURNAMENT << false << QString("Incorrect password");

                else if(query.addUserToTournament(tournamentId, userId))
                    responseData << Packet::ID_JOIN_TOURNAMENT << true << QString("You joined the tournament");

                else
                    responseData << Packet::ID_ERROR << false << QString("A problem occured. Try again later.");
            }
            else
                responseData << Packet::ID_JOIN_TOURNAMENT << false << QString("This tournament does not exist");
        }
        else
            responseData << Packet::ID_ERROR << QString("User does not exist");

        emit response(responseData);
    }

    QString PacketProcessor::validateTournamentJoining(unsigned int tournamentId, unsigned int userId)
    {
        Query query(dbConnection->getConnection());

        if(!query.tournamentIsOpened(tournamentId))
            return QString("This tournament is closed");

        if(query.tournamentEntriesExpired(tournamentId))
            return QString("Entries for this tournament have expired");

        if(query.userPatricipatesInTournament(tournamentId, userId))
            return QString("You are already participating in this tournament");

        if(query.tournamentIsFull(tournamentId))
            return QString("This tournament is full");

        return QString("");
    }

    void PacketProcessor::manageDownloadingTournamentInfo(const QVariantList & tournamentData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(tournamentData[1].toString()) &&
           query.findTournamentId(tournamentData[0].toString(), query.value("id").toUInt()) )
        {
            unsigned int tournamentId = query.value("id").toUInt();
            query.findTournamentInfo(tournamentId);
            query.next();

            QVariantList tournamentInfo;

            tournamentInfo << query.value("password_required").toBool() << query.value("entries_end_time")
                           << query.value("predictors").toUInt() << query.value("predictors_limit").toUInt();

            responseData << Packet::ID_DOWNLOAD_TOURNAMENT_INFO << QVariant::fromValue(tournamentInfo)
                         << query.value("opened").toBool();

            query.findTournamentRounds(tournamentId);
            QVariantList roundsData;

            while(query.next())
                roundsData << query.value("name");

            responseData << QVariant::fromValue(roundsData);
        }
        else
            responseData << Packet::ID_ERROR << QString("This tournament does not exist");

        emit response(responseData);
    }

    void PacketProcessor::manageTournamentFinishing(const QVariantList & tournamentData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(tournamentData[1].toString()) &&
           query.findTournamentId(tournamentData[0].toString(), query.value("id").toUInt()) )
        {
            responseData << Packet::ID_FINISH_TOURNAMENT;
            unsigned int tournamentId = query.value("id").toUInt();

            if(query.tournamentIsOpened(tournamentId))
            {
                if(!query.tournamentEntriesExpired(tournamentId))
                    responseData << false << QString("You cannot close this tournament because it did not start yet.");

                else if(!query.allMatchesFinished(tournamentId))
                    responseData << false << QString("You cannot close this tournament because there are still "
                                                     "unfinished matches.");

                else if(query.finishTournament(tournamentId))
                    responseData << true << QString("Tournament finished");
                else
                    responseData << false << QString("Finishing this tournament is not possible now. Try again later.");
            }
            else
                responseData << Packet::ID_FINISH_TOURNAMENT << false << QString("This tournament is already closed.");
        }
        else
            responseData << Packet::ID_ERROR << QString("This tournament does not exist");

        emit response(responseData);
    }

    void PacketProcessor::manageAddingNewRound(const QVariantList & tournamentData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(tournamentData[1].toString()) &&
           query.findTournamentId(tournamentData[0].toString(), query.value("id").toUInt()) )
        {
            responseData << Packet::ID_ADD_NEW_ROUND;
            unsigned int tournamentId = query.value("id").toUInt();

            if(query.tournamentIsOpened(tournamentId))
            {
                if(query.duplicateNameOfRound(tournamentData[2].toString(), tournamentId))
                    responseData << false << QString("A round with the same name already exists.");

                else if(query.addNewRound(tournamentData[2].toString(), tournamentId))
                    responseData << true << tournamentData[2].toString();
                else
                    responseData << false << QString("Adding new round is not possible right now. Try again later.");
            }
            else
                responseData << false << QString("This tournament is closed.");
        }
        else
            responseData << Packet::ID_ERROR << QString("This tournament does not exist.");

        emit response(responseData);
    }

    void PacketProcessor::manageDownloadingTournamentLeaderboard(const QVariantList & tournamentData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(tournamentData[1].toString()) &&
           query.findTournamentId(tournamentData[0].toString(), query.value("id").toUInt()) )
        {
            unsigned int tournamentId = query.value("id").toUInt();
            query.findTournamentLeaderboard(tournamentId);

            if(query.next())
                sendParticipantsInChunks(query, Packet::ID_DOWNLOAD_TOURNAMENT_LEADERBOARD);
            else
            {
                responseData << Packet::ID_ERROR << QString("This tournament has no participants.");
                emit response(responseData);
            }
        }
        else
        {
            responseData << Packet::ID_ERROR << QString("This tournament does not exist.");
            emit response(responseData);
        }
    }

    void PacketProcessor::manageDownloadingRoundLeaderboard(const QVariantList & roundData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(roundData[1].toString()) &&
           query.findTournamentId(roundData[0].toString(), query.value("id").toUInt()) )
        {
            unsigned int tournamentId = query.value("id").toUInt();
            query.findTournamentLeaderboard(tournamentId);

            if(query.findRoundId(roundData[2].toString(), tournamentId))
            {
                unsigned int roundId = query.value("id").toUInt();
                query.findRoundLeaderboard(tournamentId, roundId);

                if(query.next())
                    sendParticipantsInChunks(query, Packet::ID_DOWNLOAD_ROUND_LEADERBOARD);
                else
                {
                    responseData << Packet::ID_ERROR << QString("This tournament has no participants.");
                    emit response(responseData);
                }
            }
            else
            {
                responseData << Packet::ID_ERROR << QString("This round does not exist.");
                emit response(responseData);
            }
        }
        else
        {
            responseData << Packet::ID_ERROR << QString("This tournament does not exist.");
            emit response(responseData);
        }
    }

    void PacketProcessor::sendParticipantsInChunks(QSqlQuery & query, const int packetId)
    {
        QVariantList responseData;
        int chunkSize = 50;
        int i = 0;

        do
        {
            if(i == chunkSize)
            {
                emit response(responseData);
                responseData.clear();
                i = 0;
            }

            if(i == 0)
                responseData << packetId;

            QVariantList leaderboardData;
            leaderboardData << query.value("nickname")
                            << query.value("exact_score")
                            << query.value("predicted_result")
                            << query.value("points");
            responseData << QVariant::fromValue(leaderboardData);

            i++;
        } while(query.next());

        emit response(responseData);
    }

    void PacketProcessor::managePullingMatches(const QVariantList & requestData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(requestData[1].toString()) &&
           query.findTournamentId(requestData[0].toString(), query.value("id").toUInt()) &&
           query.findRoundId(requestData[2].toString(), query.value("id").toUInt()) )
        {
            unsigned int roundId = query.value("id").toUInt();
            query.findMatches(roundId);
            bool matchesFound = true;

            if(!query.next())
            {
                responseData << Packet::ID_ZERO_MATCHES_TO_PULL;
                emit response(responseData);
                matchesFound = false;
            }
            else
                sendMatchesInChunks(query);

            if(matchesFound)
            {
                responseData.clear();
                responseData << Packet::ID_ALL_MATCHES_PULLED;
                emit response(responseData);
            }
        }
        else
        {
            responseData << Packet::ID_ERROR << QString("This round does not exist.");
            emit response(responseData);
        }
    }

    void PacketProcessor::sendMatchesInChunks(QSqlQuery & query)
    {
        QVariantList responseData;
        int chunkSize = 40;
        int i = 0;

        do
        {
            if(i == chunkSize)
            {
                emit response(responseData);
                responseData.clear();
                i = 0;
            }

            if(i == 0)
                responseData << Packet::ID_PULL_MATCHES;

            QVariantList matchData;
            matchData << query.value("competitor_1")
                      << query.value("competitor_2")
                      << query.value("competitor_1_score")
                      << query.value("competitor_2_score")
                      << query.value("predictions_end_time").toDateTime().toString("dd.MM.yyyy hh:mm");
            responseData << QVariant::fromValue(matchData);

            i++;
        } while(query.next());

        emit response(responseData);
    }

    void PacketProcessor::manageCreatingNewMatch(const QVariantList & matchData)
    {
        Match match(matchData[0].value<QVariantList>());
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(match.getTournamentHostName()) &&
           query.findTournamentId(match.getTournamentName(), query.value("id").toUInt()))
        {
            responseData << Packet::ID_CREATE_MATCH;
            unsigned int tournamentId = query.value("id").toUInt();

            if(!query.tournamentIsOpened(tournamentId))
                responseData << false << QString("This tournament is closed.");

            else if(!query.matchStartsAfterEntriesEndTime(tournamentId, match.getPredictionsEndTime()))
                responseData << false << QString("The match can't start before the entries end.");

            else if(!query.findRoundId(match.getRoundName(), tournamentId))
                responseData << false << QString("This round does not exist.");
            else
            {
                unsigned int roundId = query.value("id").toUInt();

                if(query.duplicateMatch(match.getFirstCompetitor(), match.getSecondCompetitor(), roundId))
                    responseData << false << QString("The same match already exists.");

                else if(match.getPredictionsEndTime() < QDateTime::currentDateTime())
                    responseData << false << QString("Predictions end time must be greater than the current time.");

                else if(query.createMatch(roundId, match.getFirstCompetitor(), match.getSecondCompetitor(),
                                          match.getPredictionsEndTime()))
                    responseData << true << QString("The match was created successfully.");
                else
                    responseData << false << QString("The match couldn't be created. Try again later.");
            }
        }
        else
            responseData << Packet::ID_ERROR << QString("This tournament does not exist.");

        emit response(responseData);
    }

    void PacketProcessor::manageDeletingMatch(const QVariantList & matchData)
    {
        Match match(matchData[0].value<QVariantList>());
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(match.getTournamentHostName()) &&
           query.findTournamentId(match.getTournamentName(), query.value("id").toUInt()))
        {
            unsigned int tournamentId = query.value("id").toUInt();

            if(!query.tournamentIsOpened(tournamentId))
                responseData << Packet::ID_MATCH_DELETING_ERROR << QString("This tournament is closed.");

            else if(!query.findRoundId(match.getRoundName(), tournamentId))
                responseData << Packet::ID_MATCH_DELETING_ERROR << QString("This round does not exist.");
            else
            {
                unsigned int roundId = query.value("id").toUInt();

                if(query.deleteMatch(roundId, match.getFirstCompetitor(), match.getSecondCompetitor()))
                    responseData << Packet::ID_MATCH_DELETED << match.getFirstCompetitor() << match.getSecondCompetitor();
                else
                    responseData << Packet::ID_MATCH_DELETING_ERROR
                                 << QString("The match couldn't be deleted. Try again later.");
            }
        }
        else
            responseData << Packet::ID_ERROR << QString("This tournament does not exist.");

        emit response(responseData);
    }

    void PacketProcessor::manageUpdatingMatchScore(const QVariantList & matchData)
    {
        Match match(matchData[0].value<QVariantList>());
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(query.findUserId(match.getTournamentHostName()) &&
           query.findTournamentId(match.getTournamentName(), query.value("id").toUInt()))
        {
            unsigned int tournamentId = query.value("id").toUInt();

            if(!query.tournamentIsOpened(tournamentId))
                responseData << Packet::ID_MATCH_SCORE_UPDATE_ERROR << QString("This tournament is closed.");

            else if(!query.findRoundId(match.getRoundName(), tournamentId))
                responseData << Packet::ID_MATCH_SCORE_UPDATE_ERROR << QString("This round does not exist.");
            else
            {
                unsigned int roundId = query.value("id").toUInt();

                if(query.findMatchId(match.getFirstCompetitor(), match.getSecondCompetitor(), roundId))
                {
                    unsigned int matchId = query.value("id").toUInt();

                    if(query.updateMatchScore(matchId, match.getFirstCompetitorScore(), match.getSecondCompetitorScore()))
                    {
                        QVariantList updatedMatchData;
                        updatedMatchData << match.getFirstCompetitor() << match.getSecondCompetitor()
                                         << match.getFirstCompetitorScore()
                                         << match.getSecondCompetitorScore();

                        responseData << Packet::ID_MATCH_SCORE_UPDATED << QVariant::fromValue(updatedMatchData);
                    }
                    else
                        responseData << Packet::ID_MATCH_SCORE_UPDATE_ERROR
                                     << QString("The score couldn't be udpated. Try again later.");
                }
                else
                    responseData << Packet::ID_MATCH_SCORE_UPDATE_ERROR << QString("This match does not exist.");
            }
        }
        else
            responseData << Packet::ID_ERROR << QString("This tournament does not exist.");

        emit response(responseData);
    }

    void PacketProcessor::managePullingMatchesPredictions(const QVariantList & requestData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(!query.findUserId(requestData[0].toString()))
        {
            responseData << Packet::ID_ERROR << QString("User does not exist.");
            emit response(responseData);
            return;
        }

        unsigned int requesterId = query.value("id").toUInt();

        if(query.findUserId(requestData[2].toString()) &&
           query.findTournamentId(requestData[1].toString(), query.value("id").toUInt()))
        {
            unsigned int tournamentId = query.value("id").toUInt();

            if(query.findRoundId(requestData[3].toString(), tournamentId))
            {
                unsigned int roundId = query.value("id").toUInt();
                query.findMatchesPredictions(tournamentId, roundId, requesterId);

                if(query.next())
                    sendMatchesPredictionsInChunks(query);

                responseData << Packet::ID_ALL_MATCHES_PREDICTIONS_PULLED;
                emit response(responseData);
            }
            else
                responseData << Packet::ID_ERROR << QString("This round does not exist.");
        }
        else
        {
            responseData << Packet::ID_ERROR << QString("This tournament does not exist.");
            emit response(responseData);
        }
    }

    void PacketProcessor::sendMatchesPredictionsInChunks(QSqlQuery & query)
    {
        QVariantList responseData;
        int chunkSize = 40;
        int i = 0;

        do
        {
            if(i == chunkSize)
            {
                emit response(responseData);
                responseData.clear();
                i = 0;
            }

            if(i == 0)
                responseData << Packet::ID_PULL_MATCHES_PREDICTIONS;

            QVariantList predictionsData;
            predictionsData << query.value("nickname")
                            << query.value("competitor_1_score_prediction")
                            << query.value("competitor_2_score_prediction")
                            << query.value("competitor_1")
                            << query.value("competitor_2");
            responseData << QVariant::fromValue(predictionsData);

            i++;
        } while(query.next());

        emit response(responseData);
    }

    void PacketProcessor::manageMakingPrediction(const QVariantList & predictionData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(!query.findUserId(predictionData[0].toString()))
        {
            responseData << Packet::ID_ERROR << QString("User does not exist.");
            emit response(responseData);
            return;
        }

        unsigned int predictorId = query.value("id").toUInt();

        if(query.findUserId(predictionData[2].toString()) &&
           query.findTournamentId(predictionData[1].toString(), query.value("id").toUInt()))
        {
            unsigned int tournamentId = query.value("id").toUInt();

            if(!query.tournamentIsOpened(tournamentId))
                responseData << Packet::ID_MAKE_PREDICTION_ERROR << QString("This tournament is closed.");

            else if(!query.findTournamentParticipantId(predictorId, tournamentId))
                responseData << Packet::ID_MAKE_PREDICTION_ERROR << QString("You are not taking part in this tournament.");
            else
            {
                unsigned int participantId = query.value("id").toUInt();

                if(query.findRoundId(predictionData[3].toString(), tournamentId))
                {
                    unsigned int roundId = query.value("id").toUInt();

                    if(query.findMatchId(predictionData[4].toString(), predictionData[5].toString(), roundId))
                    {
                        unsigned int matchId = query.value("id").toUInt();

                        if(!query.matchAcceptsPredictions(matchId))
                            responseData << Packet::ID_MAKE_PREDICTION_ERROR
                                         << QString("The time to predict the result of this match has come to an end.");

                        else if(query.matchPredictionAlreadyExists(matchId, participantId))
                            responseData << Packet::ID_MAKE_PREDICTION_ERROR
                                         << QString("You have already predicted the result of this match.");

                        else if(query.createMatchPrediction(matchId, participantId, predictionData[6].toUInt(),
                                                            predictionData[7].toUInt()))
                        {
                            responseData << Packet::ID_MAKE_PREDICTION << predictionData[0] << predictionData[4]
                                         << predictionData[5] << predictionData[6].toUInt() << predictionData[7].toUInt();
                        }
                        else
                            responseData << Packet::ID_MAKE_PREDICTION_ERROR <<
                                            QString("The prediction could not have been made.");
                    }
                    else
                        responseData << Packet::ID_MAKE_PREDICTION_ERROR << QString("This match does not exist.");
                }
                else
                    responseData << Packet::ID_MAKE_PREDICTION_ERROR << QString("This round does not exist.");
            }
        }
        else
            responseData << Packet::ID_MAKE_PREDICTION_ERROR << QString("This tournament does not exist.");

        emit response(responseData);
    }

    void PacketProcessor::manageUpdatingPrediction(const QVariantList & predictionData)
    {
        Query query(dbConnection->getConnection());
        QVariantList responseData;

        if(!query.findUserId(predictionData[0].toString()))
        {
            responseData << Packet::ID_ERROR << QString("User does not exist.");
            emit response(responseData);
            return;
        }

        unsigned int predictorId = query.value("id").toUInt();

        if(query.findUserId(predictionData[2].toString()) &&
           query.findTournamentId(predictionData[1].toString(), query.value("id").toUInt()))
        {
            unsigned int tournamentId = query.value("id").toUInt();

            if(!query.tournamentIsOpened(tournamentId))
                responseData << Packet::ID_UPDATE_PREDICTION_ERROR << QString("This tournament is closed.");

            else if(!query.findTournamentParticipantId(predictorId, tournamentId))
                responseData << Packet::ID_UPDATE_PREDICTION_ERROR << QString("You are not taking part in this tournament.");
            else
            {
                unsigned int participantId = query.value("id").toUInt();

                if(query.findRoundId(predictionData[3].toString(), tournamentId))
                {
                    unsigned int roundId = query.value("id").toUInt();

                    if(query.findMatchId(predictionData[4].toString(), predictionData[5].toString(), roundId))
                    {
                        unsigned int matchId = query.value("id").toUInt();

                        if(!query.matchAcceptsPredictions(matchId))
                            responseData << Packet::ID_UPDATE_PREDICTION_ERROR
                                         << QString("The time to predict the result of this match has come to an end.");

                        else if(!query.matchPredictionAlreadyExists(matchId, participantId))
                            responseData << Packet::ID_UPDATE_PREDICTION_ERROR
                                         << QString("You have not made a result prediction for this match yet.");

                        else if(query.updateMatchPrediction(matchId, participantId, predictionData[6].toUInt(),
                                                            predictionData[7].toUInt()))
                        {
                            responseData << Packet::ID_UPDATE_PREDICTION << predictionData[0] << predictionData[4]
                                         << predictionData[5] << predictionData[6].toUInt() << predictionData[7].toUInt();
                        }
                        else
                            responseData << Packet::ID_UPDATE_PREDICTION_ERROR <<
                                            QString("The prediction could not have been updated.");
                    }
                    else
                        responseData << Packet::ID_UPDATE_PREDICTION_ERROR << QString("This match does not exist.");
                }
                else
                    responseData << Packet::ID_UPDATE_PREDICTION_ERROR << QString("This round does not exist.");
            }
        }
        else
            responseData << Packet::ID_UPDATE_PREDICTION_ERROR << QString("This tournament does not exist.");

        emit response(responseData);
    }
}
