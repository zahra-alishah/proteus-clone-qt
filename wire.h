#ifndef WIRE_H
#define WIRE_H
#include <QVector>
#include <QPainter>
#include "geometrytypes.h"
class Component;


class Wire
{
public:
    Wire() = default;


    Wire(Component *startComp, int startPin, Component *endComp, int endPin,
         const QVector<Position> &waypoints = QVector<Position>(),
         const QVector<Component*> *allComponents = nullptr);

    Wire(Component *startComp, int startPin, const Position &danglingEnd,
         const QVector<Position> &waypoints = QVector<Position>(),
         const QVector<Component*> *allComponents = nullptr);

    Component* startComponent() const { return m_startComponent; }
    Component* endComponent() const { return m_endComponent; }
    int startPinIndex() const { return m_startPinIndex; }
    int endPinIndex() const { return m_endPinIndex; }
    bool isDangling() const { return m_isDangling; }

    Position startScenePos() const;
    Position endScenePos() const;

    void recalculateRoute();
    const QVector<Position>& path() const { return m_path; }

    void draw(QPainter *painter) const;

    void setSelected(bool s) { m_selected = s; }
    bool isSelected() const { return m_selected; }

    bool isNear(const Position &pt, int tolerance = 5) const;

    bool isConnectedTo(const Component *comp) const {
        return comp && ((comp == m_startComponent) || (comp == m_endComponent));
    }
    const QVector<Position>& waypoints() const { return m_waypoints; }

private:
    Component *m_startComponent = nullptr;
    int m_startPinIndex = -1;

    Component *m_endComponent = nullptr;
    int m_endPinIndex = -1;

    bool m_isDangling = false;
    Position m_danglingPos;

    QVector<Position> m_waypoints;
    QVector<Position> m_path;
    bool m_selected = false;

    const QVector<Component*> *m_allComponents = nullptr;

    static double pointSegmentDistance(const Position &p, const Position &a, const Position &b);
    static void appendOrthogonal(QVector<Position> &path, const Position &from, const Position &to);
};

#endif // WIRE_H