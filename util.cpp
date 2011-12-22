#include <QCryptographicHash>
#include <QDebug>
#include <QTime>
#include "util.h"
#include "global.h"

Util::Util()
{
}

QString Util::getRandom() {
    QString ret = "";
    int numChars = 0;

    // Seed.
    qsrand(QTime::currentTime().elapsed());

    while (numChars < 6) {
        int x = qrand() % 15;
        ret = ret + QString().setNum(x, 16);

        numChars++;
    }

    return ret;
}


QString Util::getTokenFor(QString method, Request::Client client) {
    Global *global = Global::getInstance();

    // Which password we should use?
    QString password;
    if (client == Request::htmlshark) {
        password = Global::getInstance()->htmlsharkPassword;
    } else if (client == Request::jsqueue){
        password = Global::getInstance()->jsqueuePassword;
    } else {
        qDebug() << "Unknown client in getTokenFor()";
    }

    // Get a new random number.
    QString random = Util::getRandom();

    // Format: method:communicationToken:password:lastRandomizer
    QString token = method + ":" + global->commToken + ":" + password + ":" + random;

    // SHA1 it.
    token = QCryptographicHash::hash(token.toAscii(), QCryptographicHash::Sha1).toHex();

    // token = random + SHA1-token.
    token = (random + token);

    qDebug() << "Token for" << method << "is" << token;

    return token;
}
