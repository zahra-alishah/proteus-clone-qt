#ifndef SHEMATICCLASS_H
#define SHEMATICCLASS_H
#include <QGraphicsView>

class shematicClass : public QGraphicsView
{
    Q_OBJECT
public:
    explicit shematicClass(QWidget *parent = nullptr);
};

#endif // SHEMATICCLASS_H
