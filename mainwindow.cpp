#include "mainwindow.h"

#include <QtWidgets>

#include "scribbler.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), tabCounter(1){

    /* UI Stuff */

    /* Main Layout for scribbler and tab */
    mainWidget = new QWidget();
    mainLayout = new QHBoxLayout();

    scribbler = new Scribbler();
    mainTable = new QTabWidget();
    mainTable->setHidden(true);

    mainLayout->addWidget(scribbler,1);
    mainLayout->addWidget(mainTable);

    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);

    /* Menus */

    /* File Menu */

    QMenu *fileMenu = new QMenu("&File");

    /* Define Actions */
    QAction *openFileAct = new QAction("&Open File");
    QAction *saveFileAct = new QAction("&Save File");
    QAction *resetScribbleAct = new QAction("&Reset Scribble");

    /* Add keyboard shortcuts */
    openFileAct->setShortcut(Qt::CTRL | Qt::Key_O);
    saveFileAct->setShortcut(Qt::CTRL | Qt::Key_S);
    resetScribbleAct->setShortcut(Qt::CTRL | Qt::Key_R);

    /* Connect actions to appropriate slots */
    connect(openFileAct, &QAction::triggered, this, &MainWindow::openFileSlot);
    connect(saveFileAct, &QAction::triggered, this, &MainWindow::saveFileSlot);
    connect(resetScribbleAct, &QAction::triggered, this, &MainWindow::resetScribbleSlot);

    /* Add file menu to the menu bar */

    fileMenu->addAction(openFileAct);
    fileMenu->addAction(saveFileAct);
    fileMenu->addAction(resetScribbleAct);

    menuBar()->addMenu(fileMenu);


    /* Capture Menu */

    QMenu *captureMenu = new QMenu("&Capture");

    /* Define Actions */
    QAction *startCaptureAct = new QAction("&Start Capture");
    QAction *endCaptureAct = new QAction("&End Capture");

    /* Add keyboard shortcuts */
    startCaptureAct->setShortcut(Qt::CTRL | Qt::Key_3);
    endCaptureAct->setShortcut(Qt::CTRL | Qt::Key_Escape);

    /* Connect actions to appropriate slots */
    connect(startCaptureAct, &QAction::triggered, scribbler, &Scribbler::startCaptureSlot);
    connect(endCaptureAct, &QAction::triggered, scribbler, &Scribbler::endCaptureSlot);

    connect(scribbler, &Scribbler::doneCapturingSignal, this, &MainWindow::showData);

    /* Add file menu to the menu bar */

    captureMenu->addAction(startCaptureAct);
    captureMenu->addAction(endCaptureAct);

    menuBar()->addMenu(captureMenu);


    /* View Menu */

    QMenu *viewMenu = new QMenu("&View");

    /* Define Actions */
    QAction *lineSegmentAct = new QAction("&Show Lines Only");
    QAction *dotsAct = new QAction("&Show Dots Only");

    /* Add keyboard shortcuts */
    lineSegmentAct->setShortcut(Qt::CTRL | Qt::Key_L);
    dotsAct->setShortcut(Qt::CTRL | Qt::Key_D);

    /* Connect actions to appropriate slots */
    connect(lineSegmentAct, &QAction::triggered, this, &MainWindow::lineSegmentSlot);
    connect(dotsAct, &QAction::triggered, this, &MainWindow::dotsSlot);


    /* Add file menu to the menu bar */

    viewMenu->addAction(lineSegmentAct);
    viewMenu->addAction(dotsAct);

    menuBar()->addMenu(viewMenu);
}

MainWindow::~MainWindow() {}

void MainWindow::lineSegmentSlot() {
    scribbler->setLine();
}

void MainWindow::dotsSlot() {
    scribbler->setDots();
}

//TODO: Need the events to be sent thru from the signal
void MainWindow::showData() {
    mainTable->setHidden(false);
    QTableWidget *mouseEvtTable = new QTableWidget();

    mouseEvtTable->setHorizontalHeaderLabels(QStringList() << "Action" << "Position" << "Time");

    //TODO: loop thru events to populate QTableWidget and add to tab
    // int mouseEventSize = events.size();
    // for (int i = 0; i < mouseEventSize; ++i) {

    // }

    mainTable->addTab(mouseEvtTable, QString("capture %1").arg(tabCounter));
    ++tabCounter;
}

//TODO: add error messages for permissions, etc.
void MainWindow::openFileSlot() {
    QString fName = QFileDialog::getOpenFileName(this, "Select file",  lastDir, "Image files (*.png *.jpeg *.jpg *.bmp)");
    if (fName.isEmpty()) return;

    QImage image(fName);
    if (image.isNull()) return;

    lastDir = QFileInfo(fName).absolutePath();
}

void MainWindow::saveFileSlot() {
    QString fName = QFileDialog::getSaveFileName(this, "Select file",  lastDir, "Image files (*.png *.jpeg *.jpg *.bmp)");
    if (fName.isEmpty()) return;

    QImage image(fName);
    if (image.isNull()) return;

    lastDir = QFileInfo(fName).absolutePath();

    QFile outFile(fName);
    QDataStream out(&outFile);


    //TODO: loop thru mainTable's tabs:

    // out << mainTable-> tabText() << QTableWidgetItems << numBytes << encodingSize;
}

void MainWindow::resetScribbleSlot() {
    scribbler->resetScribbler();
    mainTable->clear();
    mainTable->setHidden(true);
}



