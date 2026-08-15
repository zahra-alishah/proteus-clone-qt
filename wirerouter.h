#ifndef WIREROUTER_H
#define WIREROUTER_H

#include <QVector>
#include <QRect>
#include "geometrytypes.h"


namespace WireRouter {

QVector<Position> findPath(const Position &start, const Position &end,
                           const QVector<QRect> &obstacles, int gridStep = 10);

}

#endif // WIREROUTER_H