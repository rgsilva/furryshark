#ifndef REQUEST_H
#define REQUEST_H

#include <QObject>
#include <QVariant>

class Request
{
public:
    static QVariantMap callMethod(QString methodName, QVariant parameters);
};

#endif // REQUEST_H
