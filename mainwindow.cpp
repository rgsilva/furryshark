#include <QList>
#include <QDebug>
#include "songinfo.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "grooveshark.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create the table view model.
    this->tableModel = new QStandardItemModel();

    // Create all columns.
    QStringList list;
    list.append("Song ID");
    list.append("Artist");
    list.append("Title");
    list.append("Album");
    list.append("Track No.");
    this->tableModel->setHorizontalHeaderLabels(list);

    // No lines for now.
    this->tableModel->setRowCount(0);

    // Apply the model.
    ui->searchTable->setModel(this->tableModel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::searchButtonClicked() {
    int row = 0;

    // Setup user interface.
    ui->statusBar->showMessage("Searching...");
    ui->searchText->setEnabled(false);
    ui->searchButton->setEnabled(false);

    // Test 1: authentication.
    Grooveshark *gs = new Grooveshark(this);

    gs->getSessionId();
    gs->authenticate();

    QList<SongInfo*> *songs = gs->search(ui->searchText->text());

    // Set the number of rows.
    this->tableModel->setRowCount(songs->count());

    // Add each song to the table.
    foreach (SongInfo *songInfo, songs->toStdList()) {
        this->tableModel->setItem(row, 0, new QStandardItem(songInfo->id));
        this->tableModel->setItem(row, 1, new QStandardItem(songInfo->artist));
        this->tableModel->setItem(row, 2, new QStandardItem(songInfo->title));
        this->tableModel->setItem(row, 3, new QStandardItem(songInfo->album));
        this->tableModel->setItem(row++, 4, new QStandardItem(songInfo->track));
    }

    qDebug() << songs;
    qDebug() << songs->count();

}
