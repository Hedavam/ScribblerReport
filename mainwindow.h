#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scribbler.h"
#include <QMainWindow>
#include <QtWidgets>

class QHBoxLayout;
class QItemGroup;

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
    int activeTab;
    QTableWidget *mouseEvtTable;

    QList<MouseEvent> events;
    QList<QList<MouseEvent>> captureList;

    QGraphicsScene *tempScene;

    QList<QLineF> lines;
    QList<QRectF> dots;
    QList<int> captureIndices;

    bool isFirstOpened;
    bool isLine;

    /* for highligting, we want access to all our captured events */
    QList<QPair<int, QList<MouseEvent>>> allEvents;

    /* Menu Stuff */
    QString lastDir;

    /* Helper funcs */
    void createEventTable(QList<MouseEvent> _events);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void lineSegmentSlot();
    void dotsSlot();

    void showData(QList<MouseEvent> _events);

    void openFileSlot();
    void saveFileSlot();
    void resetScribbleSlot();

    void findActiveTab(int index);
};
#endif // MAINWINDOW_H
