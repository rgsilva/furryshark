#include <QList>
#include <QModelIndexList>
#include <QMessageBox>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Pointers initialization.
    this->searchResults = NULL;

    // Create and setup the model.
    this->tableModel = new QStandardItemModel();

    // Create all columns.
    QStringList list;
    list.append("Song ID");
    list.append("Artist");
    list.append("Title");
    list.append("Album");
    list.append("Track No.");
    this->tableModel->setHorizontalHeaderLabels(list);

    // Apply the model.
    ui->searchTable->setModel(this->tableModel);

    // Let's prepare the grooveshark stuff and connect to all signals.
    this->grooveshark = Grooveshark::getInstance();
    connect(this->grooveshark, SIGNAL(searchStarted()), this, SLOT(searchStarted()));
    connect(this->grooveshark, SIGNAL(searchFinished(QList<SongInfo*>*)), this, SLOT(searchFinished(QList<SongInfo*>*)));
    connect(this->grooveshark, SIGNAL(downloadStarted()), this, SLOT(downloadStarted()));
    connect(this->grooveshark, SIGNAL(downloadFinished(SongInfo*)), this, SLOT(downloadFinished(SongInfo*)));

    // Connect all UI signals.
    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(searchButton_clicked()));
    connect(ui->searchTable, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(searchTable_doubleClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ----------------------------------------------------------------------

void MainWindow::searchTable_doubleClicked() {
    int row = ui->searchTable->currentIndex().row();

    if (row < this->searchResults->count()) {
        SongInfo *song = this->searchResults->at(row);

        QString filename = "(" + song->id + ") " + song->artist + " - " + song->title + ".mp3";

        filename = QFileDialog::getSaveFileName(this, "Save as", filename, "Audio file (*.mp3);;All files (*.*)", 0);
        if (!filename.isEmpty()) {
            this->grooveshark->downloadSong(song, filename);
        }
    }
}

void MainWindow::searchButton_clicked() {
    // Call search.
    this->grooveshark->querySong(ui->searchText->text());
}

// ----------------------------------------------------------------------

void MainWindow::downloadStarted()
{
    ui->statusBar->showMessage("Downloading...");
}

void MainWindow::downloadFinished(SongInfo* song)
{
    QString songStr = QString("ID: %1\nArtist: %2\nAlbum: %3\nTitle: %4\nTrack: %5").arg(song->id, song->artist, song->album, song->title, song->track);

    QMessageBox *box = new QMessageBox(this);
    box->setText("Song downloaded!\n\n" + songStr);
    box->setStandardButtons(QMessageBox::Ok);
    box->setDefaultButton(QMessageBox::Ok);
    box->show();

    ui->statusBar->showMessage("");
}

void MainWindow::searchStarted()
{
    // Clear the table.
    this->tableModel->removeRows(0, this->tableModel->rowCount());

    // Setup user interface.
    ui->statusBar->showMessage("Searching...");
    ui->searchText->setEnabled(false);
    ui->searchButton->setEnabled(false);
}

void MainWindow::searchFinished(QList<SongInfo*>* songs)
{
    int row = 0;

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

    // Make sure we keep this list to us, but clear the old one.
    if (this->searchResults) {
        foreach (SongInfo *songInfo, this->searchResults->toStdList()) {
            delete songInfo;
        }

        delete this->searchResults;
    }
    this->searchResults = songs;

    // Setup user interface.
    ui->statusBar->showMessage("");
    ui->searchText->setEnabled(true);
    ui->searchButton->setEnabled(true);
}
