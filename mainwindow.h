#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "grooveshark.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Grooveshark *grooveshark;
    QList<SongInfo*> *searchResults;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void searchTable_doubleClicked();
    void searchButton_clicked();

    void downloadStarted();
    void downloadFinished(SongInfo *song);
    void searchStarted();
    void searchFinished(QList<SongInfo*> *songs);

private:
    Ui::MainWindow *ui;

    QStandardItemModel *tableModel;
};

#endif // MAINWINDOW_H
