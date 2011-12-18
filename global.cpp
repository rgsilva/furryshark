#include <QtCore/qmath.h>
#include <QDebug>
#include "global.h"

QUuid uuid = NULL;
QString token = "";

Global* Global::instance = NULL;

Global::Global() {
    this->commToken = QString("");
    this->gsPassword = "bewareOfTheGingerApocalypse";
    this->session = QString("");
    this->uuid = QUuid::createUuid();

    qDebug() << "-- Global startup --";
    qDebug() << "gsPassword:" << this->gsPassword;
    qDebug() << "uuid:" << this->uuid.toString();
}

Global::~Global() {

}

Global* Global::getInstance() {
    if (Global::instance == NULL) {
        Global::instance = new Global();
    }

    return Global::instance;
}

