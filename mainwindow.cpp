#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "grooveshark.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::searchButtonClicked() {
    // Setup user interface.
    ui->statusBar->showMessage("Searching...");
    //ui->searchText->setEnabled(false);
    ui->searchButton->setEnabled(false);

    // Test 1: authentication.
    Grooveshark *gs = new Grooveshark(this);

    gs->getSessionId();
    gs->authenticate();
    gs->search("Martin Solveig - Hello");
}
