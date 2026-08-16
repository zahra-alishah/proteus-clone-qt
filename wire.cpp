#include "wire.h"
#include "component.h"
#include "wirerouter.h"
#include <cmath>
#include <algorithm>

Wire::Wire(Component *startComp, int startPin, Component *endComp, int endPin,
           const QVector<Position> &waypoints, const QVector<Component*> *allComponents)
    : m_startComponent(startComp), m_startPinIndex(startPin),
    m_endComponent(endComp), m_endPinIndex(endPin),
    m_isDangling(false), m_waypoints(waypoints), m_allComponents(allComponents)
{
    recalculateRoute();
}

Wire::Wire(Component *startComp, int startPin, const Position &danglingEnd,
           const QVector<Position> &waypoints, const QVector<Component*> *allComponents)
    : m_startComponent(startComp), m_startPinIndex(startPin),
    m_endComponent(nullptr), m_endPinIndex(-1),
    m_isDangling(true), m_danglingPos(danglingEnd), m_waypoints(waypoints),
    m_allComponents(allComponents)
{
    recalculateRoute();
}

Position Wire::startScenePos() const
{
    if (!m_startComponent) return Position();
    return m_startComponent->getPinScenePosition(m_startPinIndex);
}

Position Wire::endScenePos() const
{
    if (m_isDangling) return m_danglingPos;
    if (!m_endComponent) return Position();
    return m_endComponent->getPinScenePosition(m_endPinIndex);
}

void Wire::appendOrthogonal(QVector<Position> &path, const Position &from, const Position &to)
{
    if (from == to) return;
    if (from.x != to.x && from.y != to.y) {
        path.push_back(Position(to.x, from.y));
    }
    path.push_back(to);
}

void Wire::recalculateRoute()
{
    m_path.clear();
    Position start = startScenePos();
    Position end = endScenePos();

    QVector<Position> effectiveWaypoints = m_waypoints;


    if (effectiveWaypoints.isEmpty() && m_allComponents) {
        QVector<QRect> obstacles;
        for (Component *c : *m_allComponents) {
            if (!c) continue;
            if (c == m_startComponent || c == m_endComponent) continue;
            obstacles.push_back(c->boundingRect());
        }
        effectiveWaypoints = WireRouter::findPath(start, end, obstacles);
    }

    m_path.push_back(start);
    Position current = start;
    for (const Position &wp : effectiveWaypoints) {
        appendOrthogonal(m_path, current, wp);
        current = wp;
    }
    appendOrthogonal(m_path, current, end);

    if (m_path.isEmpty()) m_path.push_back(start);
}

void Wire::draw(QPainter *painter) const
{
    if (m_path.size() < 2) return;

    painter->save();

    QColor penColor;
    if (m_selected) {
        penColor = QColor("#2f6fdb");
    } else if (m_simStopped) {
        penColor = m_defaultColor;
    } else {
        switch (m_logicLevel) {
        case 1:  penColor = Qt::red;    break;
        case 0:  penColor = Qt::blue;   break;
        default: penColor = Qt::gray;   break;
        }
    }

    QPen pen(penColor, 2);
    painter->setPen(pen);

    for (int i = 0; i < m_path.size() - 1; ++i) {
        painter->drawLine(QPoint(m_path[i].x, m_path[i].y),
                          QPoint(m_path[i + 1].x, m_path[i + 1].y));
    }

    if (m_isDangling) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(penColor);
        const Position &end = m_path.last();
        painter->drawEllipse(QPoint(end.x, end.y), 3, 3);
    }

    painter->restore();
}

double Wire::pointSegmentDistance(const Position &p, const Position &a, const Position &b)
{
    double ax = a.x, ay = a.y, bx = b.x, by = b.y, px = p.x, py = p.y;
    double dx = bx - ax, dy = by - ay;
    double lengthSq = dx * dx + dy * dy;
    double t = 0.0;
    if (lengthSq > 1e-9) {
        t = ((px - ax) * dx + (py - ay) * dy) / lengthSq;
        t = std::clamp(t, 0.0, 1.0);
    }
    double closestX = ax + t * dx;
    double closestY = ay + t * dy;
    double ddx = px - closestX, ddy = py - closestY;
    return std::sqrt(ddx * ddx + ddy * ddy);
}

bool Wire::isNear(const Position &pt, int tolerance) const
{
    for (int i = 0; i < m_path.size() - 1; ++i) {
        if (pointSegmentDistance(pt, m_path[i], m_path[i + 1]) <= tolerance)
            return true;
    }
    return false;
}

void Wire::setLogicLevel(int level)
{
    m_logicLevel = level;
    m_simStopped = false;
}

void Wire::resetColor()
{
    m_logicLevel = -1;
    m_simStopped = true;
}

void Wire::clearLogicLevel()
{
    m_logicLevel = -1;
}

QString Wire::probeVoltageLabel() const
{
    if (m_simStopped) return "N/A";
    switch (m_logicLevel) {
    case 1:  return "5.00 V";
    case 0:  return "0.00 V";
    default: return "Undefined";
    }
}