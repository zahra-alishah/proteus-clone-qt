#include "pin.h"
#include <cmath>

bool Pin::isNear(const Position &worldMousePos,
                  const Position &componentOrigin,
                  Orientation componentOrientation) const
{
    // موقعیت واقعی این پایه روی صفحه = مبدأ قطعه + (پایه محلی چرخانده شده)
    Position rotated = rotatePoint(localPosition, componentOrientation);
    Position worldPinPos = componentOrigin + rotated;

    int dx = worldMousePos.x - worldPinPos.x;
    int dy = worldMousePos.y - worldPinPos.y;
    double distance = std::sqrt(static_cast<double>(dx * dx + dy * dy));

    return distance <= sensitivityRadius;
}
