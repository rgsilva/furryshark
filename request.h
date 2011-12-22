#ifndef REQUEST_H
#define REQUEST_H

#include <QObject>
#include <QVariant>

class Request
{
public:
    enum Client { htmlshark, jsqueue };

    static QVariantMap callMethod(QString methodName, QVariant parameters, Client client);
    static QVariantMap country();
};

#endif // REQUEST_H
