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
    QGraphicsLineItem* line;
    QGraphicsEllipseItem* dot;

    MouseEvent() {}; //empty constructor; could also use MouseEvent(const MouseEvent&)
    MouseEvent(int _action, QPointF _pos, quint64 _time, QGraphicsLineItem* _line = nullptr, QGraphicsEllipseItem* _dot = nullptr); //default vals. in case they're not provided

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

    /* grows/disappears based on capturing functionality */
    QList<MouseEvent> events;

    /* to re-draw later */
    QList<QLineF> lines;
    QList<QRectF> dots;

    /* keep track of all scribbles for opacity setting */
    QList<QGraphicsItemGroup*> scribbles;
    QGraphicsItemGroup *scribble;

    /* for highligting, we want access to all our captured events */
    QList<QPair<int, QList<MouseEvent>>> allEvents;
    /* keep track of num. captures (proxy) for tabCounter to construct allEvents, and then the activeTab */
    int captureCount;
    int activeTab;

    /* boolean for line segments/dots only to be set by main window's menu */
    bool isLine;
    bool isCapturing;

public:
    /* some setters/getters/helpers - maybe they should not be implemented in header, but seems chill - ask */
    void setLine() {
        isLine = true;
    }
    void setDots() {
        isLine = false;
    }

    void resetScribbler() {
        allEvents.clear();
        highlightedLines.clear();
        highlightedDots.clear();
        events.clear();
        scene.clear();
        lines.clear();
        dots.clear();

    }

    void setActiveTab(int tabIndex) {
        activeTab = tabIndex;
    }


    void clearHighlights();
    QList<QGraphicsLineItem*> highlightedLines;
    QList<QGraphicsEllipseItem*> highlightedDots;

    QList<QLineF> getLines() {
        return lines;
    }
    QList<QRectF> getDots() {
        return dots;
    }


    void drawAgain(QList<QLineF> _lines, QList<QRectF> _dots);

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

    void opacityControl(int activeTab);
    void highlightSections();
};

#endif // SCRIBBLER_H
