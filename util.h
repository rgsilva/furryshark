#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include "request.h"

class Util
{
private:
    static QString getRandom();

public:
    Util();

    static QString getTokenFor(QString method, Request::Client client);
};

#endif // UTIL_H
