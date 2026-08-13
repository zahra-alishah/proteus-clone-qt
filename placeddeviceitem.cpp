#include "placeddeviceitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace {

// Same small heuristic used in PickDevicesDialog's preview - kept as a
// local, independent copy on purpose so this file has no dependency on
// the dialog.
enum class SymbolKind { Resistor, Capacitor, Inductor, Battery, Ground, Generic };

SymbolKind classify(const QString &name, const QString &category)
{
    const QString n = name.toUpper();
    const QString c = category.toUpper();

    if (n.contains("RESISTOR")  || c.contains("RESISTOR"))  return SymbolKind::Resistor;
    if (n.contains("CAPACITOR") || c.contains("CAPACITOR")) return SymbolKind::Capacitor;
    if (n.contains("INDUCTOR")  || c.contains("INDUCTOR"))  return SymbolKind::Inductor;
    if (n.contains("BATTERY")   || n.contains("SOURCE"))    return SymbolKind::Battery;
    if (n == "GND" || n.contains("GROUND"))                 return SymbolKind::Ground;
    return SymbolKind::Generic;
}

} // namespace

PlacedDeviceItem::PlacedDeviceItem(const QString &deviceName, const QString &category)
    : m_name(deviceName), m_category(category)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setZValue(10);
}

QRectF PlacedDeviceItem::boundingRect() const
{
    return QRectF(-45, -35, 90, 70);
}

void PlacedDeviceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::black, 2));

    const SymbolKind kind = classify(m_name, m_category);

    switch (kind) {
    case SymbolKind::Resistor:
        painter->drawLine(-40, 0, -18, 0);
        painter->drawRect(-18, -10, 36, 20);
        painter->drawLine(18, 0, 40, 0);
        break;
    case SymbolKind::Capacitor:
        painter->drawLine(-30, 0, -6, 0);
        painter->drawLine(-6, -18, -6, 18);
        painter->drawLine(6, -18, 6, 18);
        painter->drawLine(6, 0, 30, 0);
        break;
    case SymbolKind::Inductor:
        painter->drawLine(-40, 0, -24, 0);
        for (int i = 0; i < 4; ++i)
            painter->drawArc(-24 + i * 12, -10, 12, 20, 0, 180 * 16);
        painter->drawLine(24, 0, 40, 0);
        break;
    case SymbolKind::Battery:
        painter->drawLine(0, -30, 0, -12);
        painter->drawLine(-14, -12, 14, -12);
        painter->drawLine(-7, -4, 7, -4);
        painter->drawLine(-14, 4, 14, 4);
        painter->drawLine(-7, 12, 7, 12);
        painter->drawLine(0, 12, 0, 30);
        break;
    case SymbolKind::Ground:
        painter->drawLine(0, -20, 0, 0);
        painter->drawLine(-18, 0, 18, 0);
        painter->drawLine(-11, 6, 11, 6);
        painter->drawLine(-4, 12, 4, 12);
        break;
    default:
        painter->setBrush(QColor("#dcdcdc"));
        painter->drawRect(-30, -20, 60, 40);
        break;
    }

    painter->drawText(QRectF(-45, 18, 90, 16), Qt::AlignCenter, m_name);

    if (isSelected()) {
        painter->setPen(QPen(QColor("#4a90e2"), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}
