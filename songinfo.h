#ifndef SONGINFO_H
#define SONGINFO_H

#include <QString>

class SongInfo
{
public:
    SongInfo();

    QString id;
    QString artist;
    QString title;
    QString album;
    QString track;
};

#endif // SONGINFO_H
