#ifndef SHEMATICCLASS_H
#define SHEMATICCLASS_H
#include <QGraphicsView>

class shematicClass : public QGraphicsView
{
    Q_OBJECT
public:
    explicit shematicClass(QWidget *parent = nullptr);

signals:
    void mouseCoordinatesChanged(QString coords);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // SHEMATICCLASS_H
