#include "shematicClass.h"
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QPainter>
#include <QMouseEvent>
#include <QCursor>
#include <QScrollBar>


shematicClass::shematicClass(QWidget *parent) : QGraphicsView(parent){

    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);

    scene -> setBackgroundBrush(QBrush(QColor("#EAE8DE")));

    QPen borderPen(QColor("#5A4FCF"), 1);
    borderPen.setStyle(Qt::DashLine);
    scene -> addRect(0, 0, 1000, 1000, QPen(QColor("#5A4FCF"), 2));

    setMouseTracking(true);

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

void shematicClass::setGridMode(int mode)
{
    currentGridMode = mode;
    update();
}

void shematicClass::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);

    if (currentGridMode == GridOff) {
        return;
    }

    painter->setPen(QPen(QColor("#8A8A8A"), 1));
    qreal left = int(rect.left()) - (int(rect.left()) % 50);
    qreal top = int(rect.top()) - (int(rect.top()) % 50);

    if (currentGridMode == GridDots) {
        QVector<QPointF> points;
        for (qreal x = left; x < rect.right(); x += 10) {
            for (qreal y = top; y < rect.bottom(); y += 10) {
                points.append(QPointF(x, y));
            }
        }
        painter->drawPoints(points.data(), points.size());
    }
    else if (currentGridMode == GridLines) {
        QPen linePen(QColor("#D3D3D3"), 1);
        painter->setPen(linePen);
        for (qreal x = left; x < rect.right(); x += 10) {
            painter->drawLine(QLineF(x, rect.top(), x, rect.bottom()));
        }
        for (qreal y = top; y < rect.bottom(); y += 10) {
            painter->drawLine(QLineF(rect.left(), y, rect.right(), y));
        }
    }
}


void shematicClass::zoomIn()
{
    scale(1.1, 1.1);
}

void shematicClass::zoomOut()
{
    scale(1.0 / 1.1, 1.0 / 1.1);
}

void shematicClass::centerOnPage()
{
    centerOn(500, 500);
}

void shematicClass::keyMovement(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        event->accept();
        return;
    }

    int step = 30;
    QPointF currentCenter = mapToScene(viewport()->rect().center());
    QPointF newCenter = currentCenter;

    bool arrowPressed = false;
    switch (event->key()) {
    case Qt::Key_Left:
        newCenter.setX(currentCenter.x() - step);
        arrowPressed = true;
        break;
    case Qt::Key_Right:
        newCenter.setX(currentCenter.x() + step);
        arrowPressed = true;
        break;
    case Qt::Key_Up:
        newCenter.setY(currentCenter.y() - step);
        arrowPressed = true;
        break;
    case Qt::Key_Down:
        newCenter.setY(currentCenter.y() + step);
        arrowPressed = true;
        break;
    }

    if (arrowPressed) {
        centerOn(newCenter);
        event->accept();
        return;
    }

    QGraphicsView::keyPressEvent(event);
}