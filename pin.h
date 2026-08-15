#ifndef PIN_H
#define PIN_H

#include <QString>
#include "geometrytypes.h"

enum class PinType {
    Input,
    Output,
    Bidirectional,
    Power,
    Ground
};

class Wire;

class Pin
{
public:
    Pin() = default;
    Pin(const QString &name, PinType type, const Position &localPosition)
        : name(name), type(type), localPosition(localPosition) {}

    QString name;
    PinType type = PinType::Input;
    Position localPosition;

    bool isHighlighted = false;
    int sensitivityRadius = 6;

    Wire *connectedWire = nullptr;

    bool isConnected() const { return connectedWire != nullptr; }

    bool isNear(const Position &worldMousePos,
                const Position &componentOrigin,
                Orientation componentOrientation) const;
};

#endif // PIN_H
