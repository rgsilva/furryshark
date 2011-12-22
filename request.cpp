#include <QDebug>
#include "request.h"
#include "global.h"
#include "util.h"

QVariantMap Request::country() {
    QVariantMap country;

    country.insert("CC1", "1073741824");
    country.insert("CC2", "0");
    country.insert("CC3", "9");
    country.insert("CC4", "0");
    country.insert("IPR", "9119");
    country.insert("ID", "31");

    return country;
}

QVariantMap Request::callMethod(QString methodName, QVariant parameters, Client client) {
    // Build the header object.
    QVariantMap header;

    header.insert("uuid", Global::getInstance()->uuid.toString());
    header.insert("token", Util::getTokenFor(methodName, client));
    if (client == htmlshark) {
        header.insert("client", "htmlshark");
        header.insert("clientRevision", Global::getInstance()->htmlsharkVersion);
    } else if (client == jsqueue) {
        header.insert("client", "jsqueue");
        header.insert("clientRevision", Global::getInstance()->jsqueueVersion);
    } else {
        qDebug() << "callMethod() received an unknown client!";
    }
    header.insert("privacy", 0);
    header.insert("session", Global::getInstance()->session);
    header.insert("country", Request::country());

    // Build the request.
    QVariantMap request;

    request.insert("method", methodName);
    request.insert("parameters", parameters);
    request.insert("header", header);

    return request;
}
