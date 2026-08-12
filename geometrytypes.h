#ifndef GEOMETRYTYPES_H
#define GEOMETRYTYPES_H

// این فایل را به پروژه اضافه کن (SOURCES/HEADERS در .pro)
// Position و Orientation از اینجا توسط component.h و pin.h هر دو include می شوند
// تا وابستگی حلقوی (circular include) پیش نیاید.

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
};

enum class Orientation { DEG_0, DEG_90, DEG_180, DEG_270 };

// چرخش یک نقطه محلی (نسبت به مبدأ قطعه) بر اساس جهت فعلی قطعه.
// چون Orientation فقط ۴ حالت گسسته دارد، از ماتریس چرخش کامل استفاده نمی کنیم
// و مستقیم فرمول ساده شده هر ۹۰ درجه را می نویسیم.
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

#endif // GEOMETRYTYPES_H
