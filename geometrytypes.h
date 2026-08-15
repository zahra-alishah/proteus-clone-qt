#ifndef GEOMETRYTYPES_H
#define GEOMETRYTYPES_H

struct Position {
    int x = 0, y = 0;
    Position() = default;
    Position(int x, int y) : x(x), y(y) {}
    bool operator==(const Position &other) const {
        return x == other.x && y == other.y;
    }
    Position operator+(const Position &other) const {
        return Position(x + other.x, y + other.y);
    }
    Position operator-(const Position &other) const {
        return Position(x - other.x, y - other.y);
    }
};

enum class Orientation { DEG_0, DEG_90, DEG_180, DEG_270 };

inline Position rotatePoint(const Position &local, Orientation o)
{
    switch (o) {
    case Orientation::DEG_0:   return Position(local.x, local.y);
    case Orientation::DEG_90:  return Position(-local.y, local.x);
    case Orientation::DEG_180: return Position(-local.x, -local.y);
    case Orientation::DEG_270: return Position(local.y, -local.x);
    }
    return local;
}

inline Position mirrorPoint(const Position &local, bool flipX, bool flipY)
{
    return Position(flipX ? -local.x : local.x, flipY ? -local.y : local.y);
}

#endif // GEOMETRYTYPES_H