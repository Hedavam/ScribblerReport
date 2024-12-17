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

    QGraphicsEllipseItem *ellipseItem = new QGraphicsEllipseItem(initDot); //only add item to group, not to scene!
    ellipseItem->setPen(Qt::NoPen);
    ellipseItem->setBrush(Qt::black);

    /* add scribble to group, want all scribbles! cuz we wanna be able to make non-captured ones opaque too */
    scribble->addToGroup(ellipseItem);
    scene.addItem(scribble); //only needs to be added once

    /* add init dot to dot list */
    if (isCapturing) {
        dots.append(initDot);
    }

    events << MouseEvent(MouseEvent::Press, p, evt->timestamp(), nullptr, ellipseItem);
}

void Scribbler::mouseMoveEvent(QMouseEvent *evt) {
    QGraphicsView::mouseMoveEvent(evt);

    QPointF p = mapToScene(evt->pos());


    QLineF line = QLineF(lastPoint, p);

    QGraphicsLineItem *lineItem = new QGraphicsLineItem(line);
    lineItem->setPen(QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
    scribble->addToGroup(lineItem);

    if(!isLine) {
        lineItem->setVisible(false);
    }
    if(isCapturing) {
        lines.append(line);
    }

    /* start new line where old one ended */
    lastPoint = p;
    events << MouseEvent(MouseEvent::Move, p, evt->timestamp(), lineItem, nullptr);


    QRectF dot = QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth));

    QGraphicsEllipseItem *ellipseItem = new QGraphicsEllipseItem(dot); //only add item to group, not to scene!
    ellipseItem->setPen(Qt::NoPen);
    ellipseItem->setBrush(Qt::black);

    /* add scribble to group, want all scribbles! cuz we wanna be able to make non-captured ones opaque too */
    scribble->addToGroup(ellipseItem);

    /* add group with line,dot to scene! */
    //scene.addItem(scribble);

    if(isLine) {
        ellipseItem->setVisible(false);
    }
    if (isCapturing) {
        dots.append(dot);
    }

    lastPoint = p;
    events << MouseEvent(MouseEvent::Move, p, evt->timestamp(), nullptr, ellipseItem);
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

//TODO: this breaks a lotta things ngl!!!! fix it!!
void Scribbler::opacityControl(int activeTab){

    //crucial! - after resetting this function is invoked again, but we should exit early because there's no scribbles to parse!!
    if (scribbles.isEmpty()) {
        return;
    }

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


/* When menu is clicked we want to change all drawings to either lines/dots */

/* change all lines to dots - just change what's shown */
void Scribbler::showDots() {
    foreach(QGraphicsItem* item, scene.items()) {
        if (item->type() == QGraphicsLineItem::Type) {
            item->setVisible(false);
        } else {
            item->setVisible(true);
        }
    }
}

/* change all dots to lines - just change what's shown*/
void Scribbler::showLines() {
    foreach(QGraphicsItem* item, scene.items()) {
        if (item->type() == QGraphicsEllipseItem::Type) {
            item->setVisible(false);
        } else {
            item->setVisible(true);
        }
    }
}


/* using our de-serialzed lines,dots -> draw them again to get the user back to where they were after saving file */
void Scribbler::drawAgain(QList<QLineF> _lines, QList<QRectF> _dots, bool _isLine) {
    int linesSize = _lines.size();
    int dotsSize = _dots.size();

    for (int line = 0; line < linesSize; line++) {
        QGraphicsLineItem *lineItem  = scene.addLine(_lines[line], QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
        if(!_isLine) {
            lineItem->setVisible(false);
        }
    }
    for (int dot = 0; dot < dotsSize; dot++) {
        QGraphicsEllipseItem *ellipseItem = scene.addEllipse(_dots[dot], Qt::NoPen, Qt::black);
        if(_isLine) {
            ellipseItem->setVisible(false);
        }
    }
}
