#ifndef GLOBAL_H
#define GLOBAL_H

#include <QUuid>
#include <QString>

class Global {
private:
    Global();
    ~Global();

    static Global *instance;

public:
    static Global* getInstance();

    QString random();

    QString commToken;
    QString htmlsharkPassword;
    QString jsqueuePassword;
    QString session;
    QUuid uuid;

    QString htmlsharkVersion;
    QString jsqueueVersion;
};

#endif // GLOBAL_H
