#ifndef SCRIBBLER_H
#define SCRIBBLER_H

#include <QGraphicsView>
#include <QtWidgets/qgraphicsitem.h>

class MouseEvent {
public:
    enum {
        Press,
        Move,
        Release
    };

    int action;
    QPointF pos;
    quint64 time;

    MouseEvent() {}; //empty constructor; could also use MouseEvent(const MouseEvent&)
    MouseEvent(int _action, QPointF _pos, quint64 _time);

    friend QDataStream &operator<<(QDataStream &out, const MouseEvent &evt);
    friend QDataStream &operator>>(QDataStream &in, MouseEvent &evt);
};

class Scribbler : public QGraphicsView
{
    Q_OBJECT

public:
    Scribbler();

private:
    QGraphicsScene scene;
    double lineWidth;
    QPointF lastPoint;

    QList<MouseEvent> events;

    QList<QLineF> lines;
    QList<QRectF> dots;

    /* add scribble (tab) to item group for opacity setting */
    QList<QGraphicsItemGroup*> scribbles;
    QGraphicsItemGroup *scribble;
    QList<int> captureIndices;

    /* boolean for line segments/dots only to be set by main window's menu */
    bool isLine;
    bool isCapturing;


public:
    void setLine() {
        isLine = true;
    }
    void setDots() {
        isLine = false;
    }

    void resetScribbler() {
        events.clear();
        scene.clear();
        lines.clear();
        dots.clear();
    }

    void opacityControl(int activeTab);

    QList<QLineF> getLines() {
        return lines;
    }
    QList<QRectF> getDots() {
        return dots;
    }

    QList<int> getCaptureIndices() {
        return captureIndices;
    };


    void drawAgain(QList<QLineF> _lines, QList<QRectF> _dots, QList<int> _captureIndices);

protected:
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;


//this throws it off, no matching constructor for initialization *
signals:
    void doneCapturingSignal(QList<MouseEvent> _events);

public slots:
    void startCaptureSlot();
    /* when done capturing (based on menu), emit signal and clear */
    void endCaptureSlot();
};

#endif // SCRIBBLER_H
