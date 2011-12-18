#include "grooveshark.h"
#include "request.h"
#include <iostream>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include "global.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QEventLoop>

Grooveshark::Grooveshark(QObject *parent) : QObject(parent)
{
    this->manager = new QNetworkAccessManager(this);
    this->searchResults = NULL;
}

Grooveshark::~Grooveshark() {
    delete this->manager;
}

// ----------------------------------------
// - Public methods

void Grooveshark::authenticate() {
    QEventLoop eventLoop;

    // Create the paramaters object with our secret key (MD5 of session id).
    QVariantMap parameters;
    parameters.insert("secretKey", QCryptographicHash::hash(Global::getInstance()->session.toAscii(), QCryptographicHash::Md5).toHex());

    // Create the JSON object for getCommunicationToken.
    QVariantMap postData = Request::callMethod("getCommunicationToken", parameters);

    // Serialize the JSON object.
    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(postData);

    // Prepare the request.
    QNetworkRequest request;
    request.setUrl(QUrl("http://grooveshark.com/more.php?getCommunicationToken"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Set the reply signals.
    this->authenticateReply = this->manager->post(request, json);
    connect(this->authenticateReply, SIGNAL(finished()), this, SLOT(authenticateFinished()));

    // Wait the request to finish first.
    connect(this->authenticateReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

void Grooveshark::getSessionId() {
    QEventLoop eventLoop;

    // Do a simple GET for grooveshark.com.
    QNetworkRequest request;
    request.setUrl(QUrl("http://grooveshark.com/"));

    // Send the request.
    this->getSessionReply = this->manager->get(request);
    connect(this->getSessionReply, SIGNAL(finished()), this, SLOT(getSessionFinished()));

    // Wait the request to finish first.
    connect(this->getSessionReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

QList<SongInfo*>* Grooveshark::search(QString queryStr) {
    QEventLoop eventLoop;

    // Create the paramaters object with our search parameters.
    QVariantMap parameters;
    parameters.insert("guts", 0);
    parameters.insert("ppOverride", "");
    parameters.insert("query", queryStr);
    parameters.insert("type", "Songs");

    // Create the JSON object for getCommunicationToken.
    QVariantMap postData = Request::callMethod("getResultsFromSearch", parameters);

    // Serialize the JSON object.
    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(postData);

    // Prepare the request.
    QNetworkRequest request;
    request.setUrl(QUrl("https://grooveshark.com/more.php?getResultsFromSearch"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Set the reply signals.
    this->searchReply = this->manager->post(request, json);
    connect(this->searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));

    // Wait the request to finish first.
    connect(this->searchReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    return this->searchResults;
}

// ----------------------------------------
// - Slots

void Grooveshark::authenticateFinished() {
    bool parseOk = true;

    // Check for errors.
    if (authenticateReply->error() == QNetworkReply::NoError) {
        // Read data.
        QByteArray responseBytes = authenticateReply->readAll();

        // Parse the data to a map.
        QVariantMap responseData = QJson::Parser().parse(responseBytes, &parseOk).toMap();

        // If everything went fine, save our token. Otherwise, errors.
        if (parseOk) {
            Global::getInstance()->commToken = responseData["result"].toString();
            qDebug() << "Got communication token:" << Global::getInstance()->commToken;
        } else {
            qDebug() << "authenticationFinished() failed: !parseOk";
            qDebug() << "responseBytes as string: " << QString(responseBytes);
        }
    } else {
        qDebug() << "authenticationFinished() failed: reply has errors.";
        qDebug() << "errorString():" << authenticateReply->errorString();
    }
}

void Grooveshark::getSessionFinished() {
    QList<QNetworkCookie> cookieList = this->manager->cookieJar()->cookiesForUrl(QUrl("http://grooveshark.com/"));

    foreach(QNetworkCookie cookie, cookieList) {
        // Find the PHPSESSID cookie.
        if (cookie.name() == "PHPSESSID") {
            Global::getInstance()->session = QString(cookie.value());
            qDebug() << "PHPSESSID cookie found:" << Global::getInstance()->session;

            // Leave this function.
            return;
        }
    }

    // Something went wrong...
    qDebug() << "No PHPSESSID cookie found.";
}

void Grooveshark::searchFinished() {
    bool parseOk = true;

    // Delete the old list.
    if (this->searchResults != NULL) {
        foreach (SongInfo *songInfo, this->searchResults->toStdList()) {
            delete songInfo;
        }

        delete this->searchResults;
    }

    this->searchResults = new QList<SongInfo*>();

    // Check for errors.
    if (searchReply->error() == QNetworkReply::NoError) {
        // Read data.
        QByteArray responseBytes = this->searchReply->readAll();

        // Parse the data to a map.
        QVariantMap responseData = QJson::Parser().parse(responseBytes, &parseOk).toMap();

        // If everything went fine, let's save the result. Otherwise, errors.
        if (parseOk) {
            foreach (QVariant song, responseData["result"].toMap()["result"].toList()) {
                SongInfo *songInfo = new SongInfo();

                songInfo->id = song.toMap()["SongID"].toString();
                songInfo->artist = song.toMap()["ArtistName"].toString();
                songInfo->title = song.toMap()["SongName"].toString();
                songInfo->album = song.toMap()["AlbumName"].toString();
                songInfo->track = song.toMap()["TrackNum"].toString();

                this->searchResults->append(songInfo);
            }
        } else {
            qDebug() << "searchFinished() failed: !parseOk";
            qDebug() << "responseBytes as string: " << QString(responseBytes);
        }
    } else {
        qDebug() << "searchFinished() failed: reply has errors.";
        qDebug() << "errorString():" << authenticateReply->errorString();
    }
}
