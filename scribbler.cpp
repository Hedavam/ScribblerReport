#include "scribbler.h"

#include <QtWidgets>

MouseEvent::MouseEvent(int _action, QPointF _pos, quint64 _time)
    :action(_action),pos(_pos),time(_time) { }

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

    //important - start grabbing scribbles right away//
    scribble = new QGraphicsItemGroup();
}


void Scribbler::mousePressEvent(QMouseEvent *evt) {
    QGraphicsView::mousePressEvent(evt);

    QPointF p = mapToScene(evt->pos());
    lastPoint = p;

    QRectF initDot = QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth));



    QGraphicsEllipseItem *ellipseItem = scene.addEllipse(initDot, Qt::NoPen, Qt::black);
    /* add item to group for respective scribble and init dot to dot list */
    if (isCapturing) {
        dots.append(initDot);
    }
    scribble->addToGroup(ellipseItem);
    scene.addItem(scribble);

    events << MouseEvent(MouseEvent::Press, p, evt->timestamp());
}

void Scribbler::mouseMoveEvent(QMouseEvent *evt) {
    QGraphicsView::mouseMoveEvent(evt);

    QPointF p = mapToScene(evt->pos());

    if(isLine) {
        QLineF line = QLineF(lastPoint, p);

        QGraphicsLineItem *lineItem = scene.addLine(line, QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
        /* add item to group for respective scribble and to line list */
        if(isCapturing) {
            lines.append(line);
        }
        scribble->addToGroup(lineItem);
        scene.addItem(scribble); //needed!

    }

    else {
        QRectF dot = QRectF(p - QPointF(0.5*lineWidth, 0.5*lineWidth), QSizeF(lineWidth, lineWidth));


        QGraphicsEllipseItem *ellipseItem = scene.addEllipse(dot, Qt::NoPen, Qt::black);
        /* add item to group for respective capture and to dots list*/
        if (isCapturing) {
            dots.append(dot);
        }
        scribble->addToGroup(ellipseItem);
        scene.addItem(scribble); //needed!
    }

    lastPoint = p;

    events << MouseEvent(MouseEvent::Move, p, evt->timestamp());
}

void Scribbler::mouseReleaseEvent(QMouseEvent *evt) {
    QGraphicsView::mouseReleaseEvent(evt);
    QPointF p = mapToScene(evt->pos());

    events << MouseEvent(MouseEvent::Release, p, evt->timestamp());
}

/* clear, so we start capturing on empty slate */
void Scribbler::startCaptureSlot() {
    isCapturing = true;
    events.clear();


    scribbles.append(scribble);
    scribble = new QGraphicsItemGroup();

}

/* when done capturing (based on menu), emit signal, clear; also, add item to scene, and add scribble to itemgroup */
void Scribbler::endCaptureSlot() {

    scene.addItem(scribble);

    scribbles.append(scribble);
    captureIndices.append(scribble->childItems().size());


    //start keeeping track again
    scribble = new QGraphicsItemGroup();

    if (isCapturing) {
        emit doneCapturingSignal(events);
        events.clear();
    }

    isCapturing = false;
}

void Scribbler::opacityControl(int activeTab){
    // qDebug() << "not in here plzaas";
    // qDebug() << activeTab << scribbles << "t and s";
    // /* for all item groups, set opacity to 0.25 */
    // for (QGraphicsItem *scribble : scribbles) {
    //     scribble->setOpacity(0.25);
    // }

    // /* then, set the opacity for item group associated with activeTab to 1.0 */
    // scribbles[activeTab * 2 + 1]->setOpacity(1.0);
}

// void Scribbler::scribbleHighlight(capture)

void Scribbler::drawAgain(QList<QLineF> _lines, QList<QRectF> _dots, QList<int> _captureIndices) {
    // qDebug() << "in here";
    // qDebug() << "drawAgain with:" << _lines << _dots << _captureIndices;

    // int numCaptures = _captureIndices.size();


    // qDebug() << numCaptures << linesSize << dotsSize << "sizes";

    // for (int i = 0; i < numCaptures; ++i) {
    //     int capturefinalIndex = _captureIndices[i];

    //     /* temp item group for given capture */
    //     QGraphicsItemGroup* group = scene.createItemGroup(QList<QGraphicsItem*>());

    //     /* for given capture, loop thru dots, lines and add to group */
    //     for (int captureIndex = 0;  captureIndex < capturefinalIndex; ++captureIndex) {
    //         qDebug() << captureIndex << "idx";
    //         qDebug() << lines[captureIndex] << "item";
    //         QGraphicsLineItem* lineItem = new QGraphicsLineItem(lines[captureIndex]);
    //         group->addToGroup(lineItem);
    //     }


    //     scribbles.append(group);
    // }

    // qDebug() << "past reconstruct";


    int linesSize = _lines.size();
    int dotsSize = _dots.size();

    for (int line = 0; line < linesSize; line++) {
        qDebug() << _lines[line] << line;
        scene.addLine(_lines[line], QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
    }
    for (int dot = 0; dot < dotsSize; dot++) {
        qDebug() << _dots[dot] << dot;
        scene.addEllipse(_dots[dot], Qt::NoPen, Qt::black);
    }

    qDebug() << "so we make it thru?";
}
