#include "mainwindow.h"

#include <QtWidgets>

#include "scribbler.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), tabCounter(0), reOpenedFile(false){

    /* UI Stuff + tabwidget connection */

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
    endCaptureAct->setShortcut(Qt::CTRL | Qt::Key_4);

    /* Connect actions to appropriate slots */
    connect(startCaptureAct, &QAction::triggered, scribbler, &Scribbler::startCaptureSlot);
    connect(endCaptureAct, &QAction::triggered, scribbler, &Scribbler::endCaptureSlot);

    /* to show data after receiving signal from scribbler that it's done capturing */
    connect(scribbler, &Scribbler::doneCapturingSignal, this, &MainWindow::showData);

    /* opacity control */ //!!TODO: could be problem area!!!
    // if(!reOpenedFile) {
    //     qDebug() << "should not be in here";
    //     connect(mainTable, &QTabWidget::currentChanged, scribbler, &Scribbler::opacityControl);
    // }

    /* pre-step for higlighting */
    connect(mainTable, &QTabWidget::currentChanged, this, &MainWindow::findActiveTab);


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

    /* save last directory */
    QSettings settings("FJS Systems", "Scribbler");
    lastDir = settings.value("lastDir", "").toString();
}

MainWindow::~MainWindow() {
    QSettings settings("FJS Systems", "Scribbler");
    settings.setValue("lastDir", lastDir);
}

/* set line/dot mode by invoking scribbler methods*/
void MainWindow::lineSegmentSlot() {
    scribbler->setLine();
}

void MainWindow::dotsSlot() {
    scribbler->setDots();
}

/* Events are sent thru from the signal */
void MainWindow::showData(QList<MouseEvent> _events) {
    /* increment tab counter, show main table w/ tabs */
    ++tabCounter;
    mainTable->setHidden(false);

    /* initialize temp table with appropriate headers */
    mouseEvtTable = new QTableWidget();
    mouseEvtTable->setHorizontalHeaderLabels(QStringList() << "Action" << "Position" << "Time");

    /* create temp table, add it as tab */
    createEventTable(_events);
    mainTable->addTab(mouseEvtTable, QString("capture %1").arg(tabCounter));
}

void MainWindow::saveFileSlot() {

    QString fName = QFileDialog::getSaveFileName(this, "Select file",  lastDir, "Image files (*.png *.jpeg *.jpg *.bmp)");
    if (fName.isEmpty()) return;

    lastDir = QFileInfo(fName).absolutePath();

    QFile outFile(fName);

    /* if file creation/opening fails, show error, return */
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(this, "Cancelled", QString("Could not write to File \"%1\"").arg(fName));
        return;
    }

    /* use Data Stream to send serialized outputs into file */
    QDataStream out(&outFile);
    lines = scribbler->getLines();
    dots =scribbler->getDots();

    out << tabCounter << captureList << scribbler->getLines() << scribbler->getDots();

}

void MainWindow::openFileSlot() {
    reOpenedFile = true; //important! - it's so that we disable opacityControl when we reOpen file since I coudn't serialize the QGraphicsItemGroup*
    disconnect(mainTable, &QTabWidget::currentChanged, scribbler, &Scribbler::opacityControl); //temp fix

    QList<QList<MouseEvent>> captureList;
    QString fName = QFileDialog::getOpenFileName(this, "Open a saved drawing file",  lastDir, "Drawing files (*.png *.jpeg *.jpg *.bmp)");
    if (fName.isEmpty()) return;

    lastDir = QFileInfo(fName).absolutePath();

    QFile inFile(fName);

    if (!inFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Cancelled", QString("Could not open File \"%1\"").arg(fName));
        return;
    }

    /* Data stream operations again, but in reverse w/ in */
    QDataStream in(&inFile);
    in >> tabCounter >> captureList >> lines >> dots;

    /* Reconstruct captured drawings & tabs */

    /* drawings first */
    scribbler->drawAgain(lines, dots);

    /* tabs then */
    tabCounter = 0;
    int numCaptures = captureList.size();
    for (int capture = 0; capture < numCaptures; capture++) {
        showData(captureList[capture]);
    }
}

void MainWindow::resetScribbleSlot() {
    scribbler->resetScribbler();
    captureList.clear();
    mainTable->clear();
    mainTable->setHidden(true);
}

void MainWindow::createEventTable(QList<MouseEvent> _events) {
    /* define table parameters */
    events = _events;
    captureList.append(events);

    int nRows = events.size(), nCols = 5;

    mouseEvtTable -> setRowCount(nRows);
    mouseEvtTable -> setColumnCount(nCols);
    mouseEvtTable->setHorizontalHeaderLabels(QStringList() << "Action" << "Position" << "Time" <<  "Distance" << "Speed");


    /* fill in table */
    for(int nRow = 0; nRow < nRows; nRow++) {
        /* fill in 1st col w/ actions (press, release, move) */
        QTableWidgetItem *actionItem = new QTableWidgetItem(QString::number(events[nRow].action));
        mouseEvtTable->setItem(nRow, 0, actionItem);

        /* fill in 2nd col w/ positions */
        QString posStr = QString::number(events[nRow].pos.x()) + "," + QString::number(events[nRow].pos.y());
        QTableWidgetItem *posItem = new QTableWidgetItem(posStr);
        mouseEvtTable->setItem(nRow, 1, posItem);

        /* fill in 3rd col w/ time */
        QTableWidgetItem *timeItem = new QTableWidgetItem(QString::number(events[nRow].time));
        mouseEvtTable->setItem(nRow, 2, timeItem);

        /* can't calculate speed/dist until we get to 2nd point */
        if (nRow > 0) {
            /* fill in 4th col w/ distance between consecutivve points? */
            MouseEvent prevPoint = events[nRow-1];
            MouseEvent curPoint = events[nRow];

            QLineF line(prevPoint.pos, curPoint.pos);
            double dist = line.length();

            QTableWidgetItem *distItem = new QTableWidgetItem(QString::number(dist));
            mouseEvtTable->setItem(nRow, 3, distItem);

            /* fill in 5th col w/ speed between consecutive points? */
            double time = curPoint.time - prevPoint.time;
            double speed = dist/time;
            QTableWidgetItem *speedItem = new QTableWidgetItem(QString::number(speed));
            mouseEvtTable->setItem(nRow, 4, speedItem);
        }
    }

    connect(mouseEvtTable, &QTableWidget::itemSelectionChanged, scribbler, &Scribbler::highlightSections);
}

void MainWindow::findActiveTab(int index) {
    scribbler->setActiveTab(index);
}
