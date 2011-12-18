#ifndef UTIL_H
#define UTIL_H

#include <QString>

class Util
{
private:
    static QString getRandom();

public:
    Util();

    static QString getTokenFor(QString method);
};

#endif // UTIL_H
