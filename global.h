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
    QString gsPassword;
    QString session;
    QUuid uuid;
};

#endif // GLOBAL_H
