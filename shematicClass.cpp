#include "shematicClass.h"
#include <QGraphicsScene>
#include <QMouseEvent>


shematicClass::shematicClass(QWidget *parent) : QGraphicsView(parent){

    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);

    scene -> setBackgroundBrush(QBrush(QColor("#EAE8DE")));

    QPen borderPen(QColor("#5A4FCF"), 1);
    borderPen.setStyle(Qt::DashLine);
    scene -> addRect(0, 0, 1000, 1000, QPen(QColor("#5A4FCF"), 2));

    setDragMode(QGraphicsView::RubberBandDrag);
    setRenderHint(QPainter::Antialiasing);


}

void shematicClass::mouseMoveEvent(QMouseEvent *event)
{
    // 1. Get the coordinates where the mouse is hovering on the actual scene
    QPointF scenePos = mapToScene(event->pos());

    // 2. Format the text to match your screenshot (1 decimal place)
    QString coords = QString("x: %1 y: %2")
                         .arg(QString::number(scenePos.x(), 'f', 1))
                         .arg(QString::number(scenePos.y(), 'f', 1));

    // 3. Send it to the parent widget (schematicPage)
    emit mouseCoordinatesChanged(coords);

    // 4. Pass the event to the base class so the grid can still scroll/zoom
    QGraphicsView::mouseMoveEvent(event);
}