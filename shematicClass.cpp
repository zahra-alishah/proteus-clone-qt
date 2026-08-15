#include "shematicClass.h"
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QKeyEvent>

shematicClass::shematicClass(QWidget *parent) : QGraphicsView(parent){

    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);

    scene -> setBackgroundBrush(QBrush(QColor("#EAE8DE")));

    QPen borderPen(QColor("#5A4FCF"), 1);
    borderPen.setStyle(Qt::DashLine);
    scene -> addRect(0, 0, 1000, 1000, QPen(QColor("#5A4FCF"), 2));

    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);
    setFocusPolicy(Qt::StrongFocus);
}

void shematicClass::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());

    QString coords = QString("x: %1 y: %2")
                         .arg(QString::number(scenePos.x(), 'f', 1))
                         .arg(QString::number(scenePos.y(), 'f', 1));
    emit mouseCoordinatesChanged(coords);

    emit canvasDragMoved(scenePos, event->buttons() & Qt::LeftButton);

    QGraphicsView::mouseMoveEvent(event);
}

void shematicClass::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());
        emit canvasClicked(scenePos, event->modifiers());
        if (m_placementActive) {
            event->accept();
            return;
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void shematicClass::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());
        emit canvasReleased(scenePos);
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void shematicClass::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit escapePressed();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        emit deleteKeyPressed();
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void shematicClass::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    emit componentDoubleClicked(scenePos);
    QGraphicsView::mouseDoubleClickEvent(event);
}