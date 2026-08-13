#ifndef PLACEDDEVICEITEM_H
#define PLACEDDEVICEITEM_H

#include <QGraphicsItem>
#include <QString>

// A lightweight schematic symbol dropped onto the canvas by the
// "Pick Devices" workflow (see PickDevicesDialog / schematicPage::onCanvasClicked).
//
// NOTE: this is a *visual placeholder* only - it draws a symbol + a label
// and can be selected/moved/deleted like any QGraphicsItem, but it is not
// (yet) wired to an actual Resistor/Capacitor/... instance from
// Component.h. That hook can be added later, e.g. by giving this class a
// pointer to a real Component and forwarding paint() to Component::draw().
class PlacedDeviceItem : public QGraphicsItem
{
public:
    PlacedDeviceItem(const QString &deviceName, const QString &category);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString deviceName() const { return m_name; }
    QString category() const { return m_category; }

private:
    QString m_name;
    QString m_category;
};

#endif // PLACEDDEVICEITEM_H
