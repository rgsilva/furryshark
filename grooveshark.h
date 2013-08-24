#ifndef GROOVESHARK_H
#define GROOVESHARK_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookieJar>
#include <QtNetwork/QNetworkCookie>
#include <QList>
#include "songinfo.h"

typedef enum {
    gsCreatingSession,
    gsGettingCommToken,
    gsReady,
    gsSearching,
    gsSearchComplete,
    gsDownloading,
    gsDownloadComplete
} GSState;

class Grooveshark : public QObject
{
    Q_OBJECT
private:
    Grooveshark(QObject *parent = NULL);
    ~Grooveshark();

    bool checkStatus();

    void createSession();
    void getCommunicationToken();

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
    static QString getStateString(GSState state);

    void querySong(QString queryStr);
    void downloadSong(SongInfo* song, QString filename);

public slots:
    void createSessionFinished();
    void getCommunicationTokenFinished();

    void getStreamDataFinished();
    void getStreamKeyFinished();
    void querySongFinished();

signals:
    void searchFinished(QList<SongInfo*>* songs);
    void downloadStarted();
    void downloadFinished(SongInfo* song);

    void stateChanged(GSState state, void* extra);
};

#endif // GROOVESHARK_H
