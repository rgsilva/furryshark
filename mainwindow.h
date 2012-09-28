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

    void blockUi();
    void unblockUi();
    void clearResults();
    void showResults(QList<SongInfo*> *songs);
    void showDownload(SongInfo *song);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void searchTable_doubleClicked();
    void searchButton_clicked();
    void stateChanged(GSState state, void *extra);

private:
    Ui::MainWindow *ui;

    QStandardItemModel *tableModel;
};

#endif // MAINWINDOW_H
