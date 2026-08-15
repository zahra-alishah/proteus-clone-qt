#include "pin.h"
#include "geometrytypes.h"
#include <cmath>


bool Pin::isNear(const Position &worldMousePos,
                 const Position &componentOrigin,
                 Orientation componentOrientation) const
{
    Position rotated = rotatePoint(localPosition, componentOrientation);
    Position scenePos = componentOrigin + rotated;

    double dx = static_cast<double>(worldMousePos.x - scenePos.x);
    double dy = static_cast<double>(worldMousePos.y - scenePos.y);
    double distance = std::sqrt(dx * dx + dy * dy);

    return distance <= static_cast<double>(sensitivityRadius);
}