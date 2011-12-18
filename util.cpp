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


QString Util::getTokenFor(QString method) {
    Global *global = Global::getInstance();

    // Get a new random number.
    QString random = Util::getRandom();

    // Format: method:communicationToken:password:lastRandomizer
    QString token = method + ":" + global->commToken + ":" + global->gsPassword + ":" + random;

    // SHA1 it.
    token = QCryptographicHash::hash(token.toAscii(), QCryptographicHash::Sha1).toHex();

    // token = random + SHA1-token.
    token = (random + token);

    qDebug() << "Token for" << method << "is" << token;

    return token;
}
