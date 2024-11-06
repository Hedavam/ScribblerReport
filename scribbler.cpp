#include "scribbler.h"

#include <QtWidgets>

MouseEvent::MouseEvent(int _action, QPointF _pos, quint64 _time, QGraphicsLineItem* _line, QGraphicsEllipseItem* _dot)
    :action(_action),pos(_pos),time(_time), line(_line), dot(_dot) { }

QDataStream &operator<<(QDataStream &out, const MouseEvent &evt) {
    return out << evt.action << evt.pos << evt.time;
}

QDataStream &operator>>(QDataStream &in, MouseEvent &evt) {
    return in >> evt.action >> evt.pos >> evt.time;
}

//========== Scribbler
Scribbler::Scribbler()
    :lineWidth(4.0), isLine(true) {
    setScene(&scene);
    setSceneRect(QRectF(0.0, 0.0, 800.0, 600.0));
    setMaximumSize(800, 600);
    setRenderHint(QPainter::Antialiasing, true);
    setBackgroundBrush(Qt::white);

    scene.addRect(sceneRect());

    /* start grabbing scribbles right away */
    scribble = new QGraphicsItemGroup();
}


void Scribbler::mousePressEvent(QMouseEvent *evt) {
    QGraphicsView::mousePressEvent(evt); //call base class's method so default behavior still occurs


    /* view coords -> scene coords */
    QPointF p = mapToScene(evt->pos());
    lastPoint = p;

    QRectF initDot = QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth));

    QGraphicsEllipseItem *ellipseItem = scene.addEllipse(initDot, Qt::NoPen, Qt::black);

    /* add init dot to dot list */
    if (isCapturing) {
        dots.append(initDot);
    }

    /* add scribble to group, want all scribbles! cuz we wanna be able to make non-captured ones opaque too */
    scribble->addToGroup(ellipseItem);
    scene.addItem(scribble); //pretty sure this causes duplicate additions to the graphicscene, but save coding lines - ask//

    events << MouseEvent(MouseEvent::Press, p, evt->timestamp(), nullptr, ellipseItem);
}

void Scribbler::mouseMoveEvent(QMouseEvent *evt) {
    QGraphicsView::mouseMoveEvent(evt);

    QPointF p = mapToScene(evt->pos());

    if(isLine) {
        QLineF line = QLineF(lastPoint, p);

        QGraphicsLineItem *lineItem = scene.addLine(line, QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));

        if(isCapturing) {
            lines.append(line);
        }

        scribble->addToGroup(lineItem);
        scene.addItem(scribble);

        /* start new line where old one ended */
        lastPoint = p;
        events << MouseEvent(MouseEvent::Move, p, evt->timestamp(), lineItem, nullptr);

    }

    else {
        QRectF dot = QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth));

        QGraphicsEllipseItem *ellipseItem = scene.addEllipse(dot, Qt::NoPen, Qt::black);

        if (isCapturing) {
            dots.append(dot);
        }
        scribble->addToGroup(ellipseItem);
        scene.addItem(scribble);

        lastPoint = p;
        events << MouseEvent(MouseEvent::Move, p, evt->timestamp(), nullptr, ellipseItem);
    }
}

void Scribbler::mouseReleaseEvent(QMouseEvent *evt) {
    QGraphicsView::mouseReleaseEvent(evt);

    QPointF p = mapToScene(evt->pos());
    events << MouseEvent(MouseEvent::Release, p, evt->timestamp());
}


void Scribbler::startCaptureSlot() {
    /* clear, so we start capturing on empty slate */
    isCapturing = true;
    events.clear();

    /* add scribble to scribbles, start fresh to look for new ones */
    scribbles.append(scribble);
    scribble = new QGraphicsItemGroup();

}

/* when done capturing (based on menu), emit signal, clear; also, add item to scene, and add scribble to itemgroup */
void Scribbler::endCaptureSlot() {

    if(!isCapturing) {
        QMessageBox::information(this, "Cancelled", QString("No Capture has been Started"));
        return;
    }

    allEvents.append(QPair<int, QList<MouseEvent>>(captureCount, events));

    scribbles.append(scribble);
    scribble = new QGraphicsItemGroup();

    emit doneCapturingSignal(events);
    ++captureCount;
    events.clear();

    isCapturing = false;
}

void Scribbler::opacityControl(int activeTab){
    qDebug() << "im the culprit";
    /* for all item groups, set opacity to 0.25 */
    for (QGraphicsItem *scribble : scribbles) {
        scribble->setOpacity(0.25);
    }

    /* then, set the opacity for item group associated with activeTab to 1 */
    scribbles[activeTab * 2 + 1]->setOpacity(1.0);
}

void Scribbler::highlightSections() {
    clearHighlights();

    QTableWidget *activeTable = qobject_cast<QTableWidget*>(sender()); //online help: https://stackoverflow.com/questions/4046839/how-to-get-sender-widget-with-a-signal-slot-mechanism
    QList<QTableWidgetItem*> selectedItems = activeTable->selectedItems();

    /* color lines/dots corresponding to selected events; keep track of graphicItems that we highlight */
    foreach (QTableWidgetItem *item, selectedItems) {
        int row = item->row();

        QGraphicsLineItem *lineItem = allEvents[activeTab].second[row].line;
        QGraphicsEllipseItem *dotItem = allEvents[activeTab].second[row].dot;

        if (lineItem) {
            lineItem->setPen(QPen(Qt::green, lineWidth/2, Qt::SolidLine, Qt::FlatCap));
            highlightedLines.append(lineItem);
        }

        if (dotItem) {
            dotItem->setBrush(QBrush(Qt::green));
            highlightedDots.append(dotItem);
        }
    }
}

void Scribbler::clearHighlights() {
    /* color highlighted lines/dots black - esssentially clearing the highlight; clear highlighted graphicItems */
    for (QGraphicsLineItem *lineItem : highlightedLines) {
        lineItem->setPen(QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
    }
    for (QGraphicsEllipseItem *dotItem : highlightedDots) {
        dotItem->setBrush(QBrush(Qt::black));
    }

    highlightedLines.clear();
    highlightedDots.clear();
}

/* using our de-serialzed lines,dots -> draw them again to get the user back to where they were after saving file */
void Scribbler::drawAgain(QList<QLineF> _lines, QList<QRectF> _dots) {
    int linesSize = _lines.size();
    int dotsSize = _dots.size();

    for (int line = 0; line < linesSize; line++) {
        scene.addLine(_lines[line], QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
    }
    for (int dot = 0; dot < dotsSize; dot++) {
        scene.addEllipse(_dots[dot], Qt::NoPen, Qt::black);
    }
}
