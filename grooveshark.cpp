#include "grooveshark.h"
#include "request.h"
#include <iostream>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include "global.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QEventLoop>
#include <QFile>

Grooveshark* Grooveshark::instance = NULL;

Grooveshark::Grooveshark(QObject *parent) : QObject(parent)
{
    this->manager = new QNetworkAccessManager(parent);
}

Grooveshark::~Grooveshark() {
    delete this->manager;
}

// ----------------------------------------

Grooveshark* Grooveshark::getInstance()
{
    if (Grooveshark::instance == NULL) {
        Grooveshark::instance = new Grooveshark();
    }

    return Grooveshark::instance;
}

QString Grooveshark::getStateString(GSState state)
{
    switch (state) {
        case gsCreatingSession:
            return "Creating session (cookie)...";

        case gsGettingCommToken:
            return "Getting communication token...";

        case gsReady:
            return "Ready.";

        case gsSearching:
            return "Searching...";

        case gsSearchComplete:
            return "Search complete.";

        case gsDownloading:
            return "Downloading song...";

        case gsDownloadComplete:
            return "Download complete.";

        default:
            return "!!! No text for the current state !!!";
    }
}

// ----------------------------------------

bool Grooveshark::checkStatus() {
    Global *global = Global::getInstance();

    // Check PHPSESSID.
    if (global->session.isEmpty()) {
        this->createSession();

        // If it's still empty, try again. But this time, let's just force an
        // authentication (getCommunicationToken). This should fix some issues
        // in Germany, since just opening GS main page don't create a PHP session
        // there because of the block.
        if (global->session.isEmpty()) {
            this->getCommunicationToken();
            this->createSession();

            // Let's reset the communication token: we shouldn't have a valid one
            // here since we just tested a small hack.
            global->commToken = "";
        }

        // If it's empty again, I give up! This program has crashed lol
        if (global->session.isEmpty()) {
            return false;
        }
    }

    // Check communication token.
    if (global->commToken.isEmpty()) {
        this->getCommunicationToken();

        // If there's no communication token, then we crashed.
        if (global->commToken.isEmpty()) {
            return false;
        }
    }

    // All set!
    emit stateChanged(gsReady, NULL);
    return true;
}

void Grooveshark::createSession() {
    QEventLoop eventLoop;

    // Do a simple GET for grooveshark.com.
    QNetworkRequest request;
    request.setUrl(QUrl("http://grooveshark.com/"));

    // Send the request.
    this->getSessionReply = this->manager->get(request);
    connect(this->getSessionReply, SIGNAL(finished()), this, SLOT(createSessionFinished()));

    emit stateChanged(gsCreatingSession, NULL);

    // Wait the request to finish first.
    connect(this->getSessionReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

void Grooveshark::createSessionFinished() {
    QList<QNetworkCookie> cookieList = this->manager->cookieJar()->cookiesForUrl(QUrl("http://grooveshark.com/"));

    foreach(QNetworkCookie cookie, cookieList) {
        // Find the PHPSESSID cookie.
        if (cookie.name() == "PHPSESSID") {
            Global::getInstance()->session = QString(cookie.value());
            qDebug() << "[createSessionFinished]" << "PHPSESSID cookie found:" << Global::getInstance()->session;

            // Leave this function.
            return;
        }
    }

    // Something went wrong.
    qDebug() << "[createSessionFinished]" << "No PHPSESSID cookie found.";
}

void Grooveshark::getCommunicationToken() {
    QEventLoop eventLoop;

    // Create the paramaters object with our secret key (MD5 of session id).
    QVariantMap parameters;
    parameters.insert("secretKey", QCryptographicHash::hash(Global::getInstance()->session.toAscii(), QCryptographicHash::Md5).toHex());

    // Create the JSON object for getCommunicationToken.
    QVariantMap postData = Request::callMethod("getCommunicationToken", parameters, Request::htmlshark);

    // Serialize the JSON object.
    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(postData);

    // Prepare the request.
    QNetworkRequest request;
    request.setUrl(QUrl("https://grooveshark.com/more.php?getCommunicationToken"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Set the reply signals.
    this->authenticateReply = this->manager->post(request, json);
    connect(this->authenticateReply, SIGNAL(finished()), this, SLOT(getCommunicationTokenFinished()));

    emit stateChanged(gsGettingCommToken, NULL);

    // Wait the request to finish first.
    connect(this->authenticateReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

void Grooveshark::getCommunicationTokenFinished() {
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
            if (Global::getInstance()->commToken == "") {
                qDebug() << "[getCommunicationTokenFinished]" << "empty communication token.";
                qDebug() << "[getCommunicationTokenFinished]" << "responseBytes as string:" << QString(responseBytes);
            } else {
                qDebug() << "[getCommunicationTokenFinished]" << "Got communication token:" << Global::getInstance()->commToken;
            }
        } else {
            qDebug() << "[getCommunicationTokenFinished]" << "failed: !parseOk";
            qDebug() << "[getCommunicationTokenFinished]" << "responseBytes as string:" << QString(responseBytes);
        }
    } else {
        qDebug() << "[getCommunicationTokenFinished]" << "failed: reply has errors.";
        qDebug() << "[getCommunicationTokenFinished]" << "errorString():" << authenticateReply->errorString();
    }
}

void Grooveshark::querySong(QString queryStr) {
    // Make sure we're ok to search first.
    if (!this->checkStatus()) {
        return;
    }

    // Create the paramaters object with our search parameters.
    QVariantMap parameters;
    parameters.insert("guts", 0);
    parameters.insert("ppOverride", "");
    parameters.insert("query", queryStr);
    parameters.insert("type", "Songs");

    // Create the JSON object for getResultsFromSearch.
    QVariantMap postData = Request::callMethod("getResultsFromSearch", parameters, Request::htmlshark);

    // Serialize the JSON object.
    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(postData);

    // Prepare the request.
    QNetworkRequest request;
    request.setUrl(QUrl("https://grooveshark.com/more.php?getResultsFromSearch"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Set the reply signals.
    this->searchReply = this->manager->post(request, json);
    connect(this->searchReply, SIGNAL(finished()), this, SLOT(querySongFinished()));

    emit this->stateChanged(gsSearching, NULL);
}

void Grooveshark::querySongFinished() {
    bool parseOk = true;

    QList<SongInfo*>* searchResults = new QList<SongInfo*>();

    // Check for errors.
    if (searchReply->error() == QNetworkReply::NoError) {
        // Read data.
        QByteArray responseBytes = this->searchReply->readAll();

        // Parse the data to a map.
        QVariantMap responseData = QJson::Parser().parse(responseBytes, &parseOk).toMap();

        // If everything went fine, let's save the result. Otherwise, errors.
        if (parseOk) {
            foreach (QVariant song, responseData["result"].toMap()["result"].toList()) {
                SongInfo* songInfo = new SongInfo();

                songInfo->id = song.toMap()["SongID"].toString();
                songInfo->artist = song.toMap()["ArtistName"].toString();
                songInfo->title = song.toMap()["SongName"].toString();
                songInfo->album = song.toMap()["AlbumName"].toString();
                songInfo->track = song.toMap()["TrackNum"].toString();

                searchResults->append(songInfo);
            }
        } else {
            qDebug() << "[querySongFinished]" << "failed: !parseOk";
            qDebug() << "[querySongFinished]" << "responseBytes as string:" << QString(responseBytes);
        }
    } else {
        qDebug() << "[querySongFinished]" << "failed: reply has errors.";
        qDebug() << "[querySongFinished]" << "errorString():" << this->searchReply->errorString();
    }

    emit searchFinished(searchResults);
    emit stateChanged(gsSearchComplete, (void*)searchResults);
}

void Grooveshark::downloadSong(SongInfo* song, QString filename) {
    // Let's try to get the stream key here.
    if (this->getStreamKey(song->id)) {
        // We're now downloading the song.
        emit stateChanged(gsDownloading, NULL);

        // Request the song stream.
        if (this->getStreamData(this->streamIp, this->streamKey)) {
            // We have our data. Let's write it down to the file.
            QFile file(filename);

            if (file.open(QFile::WriteOnly)) {
                // Read and write stream data.
                file.write(this->streamData);
                file.flush();
                file.close();

                // And we now have finished the song download.
                emit stateChanged(gsDownloadComplete, (void*)song);
            } else {
                qDebug() << "[downloadSong]" << "Couldn't open file" << filename << "for write.";
                return;
            }
        } else {
            qDebug() << "[downloadSong]" << "Failed to get stream data for songID" << song->id;
            return;
        }
    } else {
        qDebug() << "[downloadSong]" << "Failed to get stream key for songID" << song->id;
        return;
    }
}

bool Grooveshark::getStreamKey(QString songID) {
    QEventLoop eventLoop;

    // Save the song ID for the future.
    this->songId = songID;

    // Create the paramaters object with our request.
    QVariantMap parameters;
    parameters.insert("songID", songID);
    parameters.insert("mobile", false);
    parameters.insert("prefetch", false);
    parameters.insert("country", Request::country());

    // Create the JSON object for getStreamKeyFromSongIDEx.
    QVariantMap postData = Request::callMethod("getStreamKeyFromSongIDEx", parameters, Request::jsqueue);

    // Serialize the JSON object.
    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(postData);

    // Prepare the request.
    QNetworkRequest request;
    request.setUrl(QUrl("http://grooveshark.com/more.php?getStreamKeyFromSongIDEx"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Set the reply signals.
    this->getStreamKeyReply = this->manager->post(request, json);
    connect(this->getStreamKeyReply, SIGNAL(finished()), this, SLOT(getStreamKeyFinished()));

    // Wait the request to finish first.
    connect(this->getStreamKeyReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    return true;
}

void Grooveshark::getStreamKeyFinished() {
    bool parseOk;

    // Check for errors.
    if (this->getStreamKeyReply->error() == QNetworkReply::NoError) {
        // Read data.
        QByteArray responseBytes = this->getStreamKeyReply->readAll();

        // Parse the data to a map.
        QVariantMap responseData = QJson::Parser().parse(responseBytes, &parseOk).toMap();

        // If everything went fine, let's save the result. Otherwise, errors.
        if (parseOk) {
            // Get song data.
            QVariantMap songData = responseData["result"].toMap();

            // And now, finally, get the stream information.
            this->streamIp = songData["ip"].toString();
            this->streamKey = songData["streamKey"].toString();

            qDebug() << "[getStreamKeyFinished]" << "Stream from server" << this->streamIp << "using key" << this->streamKey;
        } else {
            qDebug() << "[getStreamKeyFinished]" << "failed: !parseOk";
            qDebug() << "[getStreamKeyFinished]" << "responseBytes as string:" << QString(responseBytes);
        }
    } else {
        qDebug() << "[getStreamKeyFinished]" << "failed: reply has errors.";
        qDebug() << "[getStreamKeyFinished]" << "errorString():" << this->getStreamKeyReply->errorString();
    }
}

bool Grooveshark::getStreamData(QString ip, QString key) {
    QEventLoop eventLoop;

    // Prepare the POST information.
    QByteArray postData = QString("streamKey=" + key).toAscii();

    // Prepare the request.
    QNetworkRequest request;
    request.setUrl(QUrl("http://" + ip + "/stream.php"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Referer", "http://grooveshark.com/JSQueue.swf?20111213.02");
    request.setRawHeader("Origin", "http://grooveshark.com");
    request.setRawHeader("Accept", "*/*");

    // Set the reply signals.
    this->getStreamDataReply = this->manager->post(request, postData);
    connect(this->getStreamDataReply, SIGNAL(finished()), this, SLOT(getStreamDataFinished()));

    // Wait the request to finish first.
    connect(this->getStreamDataReply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    return true;
}

void Grooveshark::getStreamDataFinished() {
    // Check for errors.
    if (this->getStreamDataReply->error() == QNetworkReply::NoError) {
        // Everything is fine, let's just read our data.
        this->streamData = this->getStreamDataReply->readAll();
    } else {
        // Errors found.
        qDebug() << "[getStreamDataFinished]" << "failed: reply has errors.";
        qDebug() << "[getStreamDataFinished]" << this->getStreamDataReply->errorString();
    }
}

// ----------------------------------------
// - Public methods


