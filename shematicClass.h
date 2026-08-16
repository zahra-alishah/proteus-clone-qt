#ifndef SHEMATICCLASS_H
#define SHEMATICCLASS_H
#include <QGraphicsView>

enum GridMode {
    GridOff = 0,
    GridDots = 1,
    GridLines = 2
};

class shematicClass : public QGraphicsView
{
    Q_OBJECT
public:
    explicit shematicClass(QWidget *parent = nullptr);
    void setPlacementActive(bool active) { m_placementActive = active; }
    bool isPlacementActive() const { return m_placementActive; }
    void setGridMode(int mode);
    void zoomOut();
    void zoomIn();
    void centerOnCursor();
    void centerOnPage();

signals:
    void mouseCoordinatesChanged(QString coords);
    void canvasClicked(QPointF scenePos, Qt::KeyboardModifiers modifiers); // mousePress
    void canvasDragMoved(QPointF scenePos, bool leftButtonDown);           // mouseMove
    void canvasReleased(QPointF scenePos);                                 // mouseRelease
    void escapePressed();
    void deleteKeyPressed();
    void componentDoubleClicked(QPointF pos);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void keyMovement(QKeyEvent *event);

private:
    bool m_placementActive = false;
    int currentGridMode = GridLines;
};
#endif // SHEMATICCLASS_H