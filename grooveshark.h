#ifndef GROOVESHARK_H
#define GROOVESHARK_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookieJar>
#include <QList>
#include "songinfo.h"

class Grooveshark : public QObject
{
    Q_OBJECT
private:
    Grooveshark(QObject *parent = NULL);
    ~Grooveshark();

    void checkStatus();
    void authenticate();
    void getSessionId();
    bool getStreamData(QString ip, QString key);
    bool getStreamKey(QString songID);

    static Grooveshark *instance;

    QNetworkAccessManager *manager;
    QNetworkReply *authenticateReply;
    QNetworkReply *getSessionReply;
    QNetworkReply *getStreamDataReply;
    QNetworkReply *getStreamKeyReply;
    QNetworkReply *searchReply;


    QByteArray streamData;
    QString streamIp;
    QString streamKey;
    QString songId;

public:
    static Grooveshark* getInstance();

    void querySong(QString queryStr);
    void downloadSong(SongInfo* song, QString filename);

public slots:
    void authenticateFinished();
    void getSessionFinished();
    void getStreamDataFinished();
    void getStreamKeyFinished();
    void querySongFinished();

signals:
    void searchStarted();
    void searchFinished(QList<SongInfo*>* songs);
    void downloadStarted();
    void downloadFinished(SongInfo* song);
};

#endif // GROOVESHARK_H
