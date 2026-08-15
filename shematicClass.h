#ifndef SHEMATICCLASS_H
#define SHEMATICCLASS_H
#include <QGraphicsView>

class shematicClass : public QGraphicsView
{
    Q_OBJECT
public:
    explicit shematicClass(QWidget *parent = nullptr);
    void setPlacementActive(bool active) { m_placementActive = active; }
    bool isPlacementActive() const { return m_placementActive; }

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

private:
    bool m_placementActive = false;
};
#endif // SHEMATICCLASS_H