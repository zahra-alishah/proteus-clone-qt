#include "shematicClass.h"
#include <QGraphicsScene>

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
