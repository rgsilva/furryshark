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

    static Grooveshark *instance;

    QNetworkAccessManager *manager;
    QNetworkReply *authenticateReply;
    QNetworkReply *getSessionReply;
    QNetworkReply *searchReply;

    QList<SongInfo> searchResults;

public:
    static Grooveshark* getInstance();

    QList<SongInfo> search(QString queryStr);

public slots:
    void authenticateFinished();
    void getSessionFinished();
    void searchFinished();
};

#endif // GROOVESHARK_H
