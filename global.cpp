#include <QtCore/qmath.h>
#include <QDebug>
#include "global.h"

QUuid uuid = NULL;
QString token = "";

Global* Global::instance = NULL;

Global::Global() {
    this->commToken = QString("");

    this->session = QString("");
    this->uuid = QUuid::createUuid();

    this->htmlsharkPassword = "nuggetsOfBaller";
    this->htmlsharkVersion = "20130520";

    this->jsqueuePassword = "chickenFingers";
    this->jsqueueVersion = "20130520";

    qDebug() << "-- Global startup --";
    qDebug() << "uuid:" << this->uuid.toString();
    qDebug() << "htmlshark password:" << this->htmlsharkPassword;
    qDebug() << "htmlshark version:" << this->htmlsharkVersion;
    qDebug() << "jsqueue password:" << this->jsqueuePassword;
    qDebug() << "jsqueue version:" << this->jsqueueVersion;
}

Global::~Global() {

}

Global* Global::getInstance() {
    if (Global::instance == NULL) {
        Global::instance = new Global();
    }

    return Global::instance;
}

