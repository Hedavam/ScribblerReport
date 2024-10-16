#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scribbler.h"
#include <QMainWindow>
#include <QtWidgets>

class QHBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    /* UI Stuff */

    /* Main Layout */
    QWidget *mainWidget;
    QHBoxLayout *mainLayout;
    Scribbler *scribbler;
    QTabWidget *mainTable;
    int tabCounter;

    /* Menu Stuff */
    QString lastDir;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void lineSegmentSlot();
    void dotsSlot();

    void showData();

    void openFileSlot();
    void saveFileSlot();
    void resetScribbleSlot();
};
#endif // MAINWINDOW_H
