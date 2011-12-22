#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void doubleClickSong();
    void searchButtonClicked();

private:
    Ui::MainWindow *ui;

    QStandardItemModel *tableModel;
};

#endif // MAINWINDOW_H
