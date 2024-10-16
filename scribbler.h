#ifndef SCRIBBLER_H
#define SCRIBBLER_H

#include <QGraphicsView>

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

//    MouseEvent(const MOuseEvent&);

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

    /* boolean for line segments/dots only to be set by main window's menu */
    bool isLine;

public:
    void setLine() {
        isLine = true;
    };
    void setDots() {
        isLine = false;
    }
    void resetScribbler() {
        events.clear();
        scene.clear();
    }

protected:
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;


//this throws it off, no matching constructor for initialization *
signals:
    void doneCapturingSignal(QList<MouseEvent> _events);

public slots:
    void startCaptureSlot();
    void endCaptureSlot();
};

#endif // SCRIBBLER_H
