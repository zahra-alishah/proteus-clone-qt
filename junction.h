#ifndef JUNCTION_H
#define JUNCTION_H

#include <QPainter>
#include "geometrytypes.h"


class Junction
{
public:
    Junction() = default;
    explicit Junction(const Position &pos) : m_pos(pos) {}

    Position position() const { return m_pos; }
    void setPosition(const Position &p) { m_pos = p; }

    void draw(QPainter *painter) const
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::black);
        painter->drawEllipse(QPoint(m_pos.x, m_pos.y), m_radius, m_radius);
        painter->restore();
    }

    bool isNear(const Position &pt, int tolerance = 5) const
    {
        int dx = pt.x - m_pos.x;
        int dy = pt.y - m_pos.y;
        return (dx * dx + dy * dy) <= (tolerance * tolerance);
    }

private:
    Position m_pos;
    int m_radius = 3;
};

#endif // JUNCTION_H