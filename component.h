#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <QPainter>
#include <QString>
#include <QPainterPath>
#include <QPolygon>
#include <QColor>
#include <QRect>
#include <QDebug>
#include "geometrytypes.h"   // Position, Orientation, rotatePoint, mirrorPoint
#include "pin.h"             // Pin, PinType

using namespace std;


class Component {
protected:
    double voltage = 0.0, current = 0.0;
    Position pos{0, 0};
    Orientation angle = Orientation::DEG_0;

    bool flipX = false;
    bool flipY = false;
    bool selected = false;

    QString label;

    std::vector<Pin> pins;

public:
    Component() = default;
    virtual ~Component() = default;

    void setposition(int x, int y) { pos = Position(x, y); }
    void setPosition(const Position &pt) { pos = pt; }
    Position getPosition() const { return pos; }

    void setOrientation(Orientation o) { angle = o; }
    Orientation getOrientation() const { return angle; }

    void rotateNext() {
        switch (angle) {
        case Orientation::DEG_0:   angle = Orientation::DEG_90;  break;
        case Orientation::DEG_90:  angle = Orientation::DEG_180; break;
        case Orientation::DEG_180: angle = Orientation::DEG_270; break;
        case Orientation::DEG_270: angle = Orientation::DEG_0;   break;
        }
    }


    void mirrorHorizontal() { flipX = !flipX; }
    void mirrorVertical()   { flipY = !flipY; }
    bool isMirroredHorizontal() const { return flipX; }
    bool isMirroredVertical()   const { return flipY; }
    void setSelected(bool s) { selected = s; }
    bool isSelected() const { return selected; }

    virtual QString getName() const { return label; }
    virtual void setName(const QString &newLabel) { label = newLabel; }

    const std::vector<Pin>& getPins() const { return pins; }
    int pinCount() const { return static_cast<int>(pins.size()); }

    Position getPinScenePosition(int index) const {
        Position mirrored = mirrorPoint(pins[index].localPosition, flipX, flipY);
        Position rotated = rotatePoint(mirrored, angle);
        return pos + rotated;
    }

    int findPinNear(const Position &worldMousePos) const {
        for (int i = 0; i < pinCount(); ++i) {
            Position scenePos = getPinScenePosition(i);
            double dx = static_cast<double>(worldMousePos.x - scenePos.x);
            double dy = static_cast<double>(worldMousePos.y - scenePos.y);
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist <= static_cast<double>(pins[i].sensitivityRadius)) {
                return i;
            }
        }
        return -1;
    }

    void setPinHighlighted(int index, bool highlighted) {
        if (index >= 0 && index < pinCount()) pins[index].isHighlighted = highlighted;
    }
    void clearPinHighlights() {
        for (auto &p : pins) p.isHighlighted = false;
    }

    virtual QRect localBoundingRect() const {
        if (pins.empty()) return QRect(-20, -20, 40, 40);

        int minX = pins.front().localPosition.x, maxX = minX;
        int minY = pins.front().localPosition.y, maxY = minY;
        for (const Pin &p : pins) {
            minX = std::min(minX, p.localPosition.x);
            maxX = std::max(maxX, p.localPosition.x);
            minY = std::min(minY, p.localPosition.y);
            maxY = std::max(maxY, p.localPosition.y);
        }

        const int margin = 20;
        minX -= margin; maxX += margin;
        minY -= margin; maxY += margin;

        if (maxX - minX < 40) { int c = (minX + maxX) / 2; minX = c - 20; maxX = c + 20; }
        if (maxY - minY < 40) { int c = (minY + maxY) / 2; minY = c - 20; maxY = c + 20; }

        return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
    }

    QRect boundingRect() const {
        QRect lr = localBoundingRect();
        QPoint corners[4] = { lr.topLeft(), lr.topRight(), lr.bottomLeft(), lr.bottomRight() };

        int minX = 0, maxX = 0, minY = 0, maxY = 0;
        for (int i = 0; i < 4; ++i) {
            Position local(corners[i].x(), corners[i].y());
            Position mirrored = mirrorPoint(local, flipX, flipY);
            Position rotated = rotatePoint(mirrored, angle);
            Position world = pos + rotated;
            if (i == 0) { minX = maxX = world.x; minY = maxY = world.y; }
            else {
                minX = std::min(minX, world.x); maxX = std::max(maxX, world.x);
                minY = std::min(minY, world.y); maxY = std::max(maxY, world.y);
            }
        }
        return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
    }

    virtual void draw(QPainter *painter) = 0;

    void drawPins(QPainter *painter) const {
        for (int i = 0; i < pinCount(); ++i) {
            const Pin &p = pins[i];
            Position scenePos = getPinScenePosition(i);
            painter->save();
            QPen pen(p.isHighlighted ? Qt::red : Qt::darkGray, 1);
            painter->setPen(pen);
            painter->setBrush(p.isHighlighted ? Qt::red : Qt::lightGray);
            int r = 3;
            painter->drawEllipse(QPoint(scenePos.x, scenePos.y), r, r);
            painter->restore();
        }
    }

protected:

    void applyTransform(QPainter *painter) const {
        painter->translate(pos.x, pos.y);
        switch (angle) {
        case Orientation::DEG_90:  painter->rotate(90);  break;
        case Orientation::DEG_180: painter->rotate(180); break;
        case Orientation::DEG_270: painter->rotate(270); break;
        default: break;
        }
        painter->scale(flipX ? -1.0 : 1.0, flipY ? -1.0 : 1.0);
    }

    void addPin(const QString &name, PinType type, int localX, int localY) {
        pins.emplace_back(name, type, Position(localX, localY));
    }
};

class Primary_source : public Component {
public:
    double getVoltage() const { return voltage; }
    void setVoltage(double v) { voltage = v; }
};

class GND : public Primary_source {
private:
    static inline int id_counter = 0;
public:
    GND() {
        id_counter++;
        label = "GND" + QString::number(id_counter);
        addPin("G", PinType::Ground, 0, -20);
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->drawLine(0, -20, 0, 0);
        painter->drawLine(-15, 0, 15, 0);
        painter->drawLine(-10, 4, 10, 4);
        painter->drawLine(-5, 8, 5, 8);

        painter->restore();
        drawPins(painter);
    }
};

class DC_vol_source : public Primary_source {
private:
    static inline int id_counter = 0;
public:
    explicit DC_vol_source(double v) {
        voltage = v;
        id_counter++;
        label = "V" + QString::number(id_counter);
        addPin("+", PinType::Power,  0, -35);
        addPin("-", PinType::Ground, 0,  35);
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);

        painter->drawLine(0, -35, 0, -20);
        painter->drawLine(0, 20, 0, 35);
        painter->drawEllipse(-20, -20, 40, 40);
        painter->drawLine(-6, -10, 6, -10);
        painter->drawLine(0, -16, 0, -4);
        painter->drawLine(-6, 10, 6, 10);

        painter->drawText(25, -5, label);
        painter->drawText(25, 12, QString::number(voltage) + "V");

        painter->restore();
        drawPins(painter);
    }
};

class Battery : public Primary_source {
private:
    static inline int id_counter = 0;
protected:
    double inter_res = 0.1;
public:
    Battery(double v, double r) {
        voltage = v;
        inter_res = r;
        id_counter++;
        label = "B" + QString::number(id_counter);
        addPin("+", PinType::Power,  0, -35);
        addPin("-", PinType::Ground, 0,  35);
    }

    double getInternalResistance() const { return inter_res; }
    void setInternalResistance(double r) { inter_res = r; }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);

        painter->drawLine(0, -35, 0, -20);
        painter->drawLine(-12, -20, 12, -20);
        painter->drawLine(-6, -14, 6, -14);
        painter->drawLine(0, -10, 0, -8);
        painter->drawLine(0, -4,  0, -2);
        painter->drawLine(0,  2,  0,  4);
        painter->drawLine(0,  8,  0, 10);
        painter->drawLine(-12, 14, 12, 14);
        painter->drawLine(-6, 20, 6, 20);
        painter->drawLine(0, 20, 0, 35);
        painter->drawText(20, -5, label);
        painter->drawText(20, 12, QString::number(voltage) + "V");

        painter->restore();
        drawPins(painter);
    }
};


class Clock_gen : public Primary_source {
private:
    static inline int id_counter = 0;
protected:
    double T = 0.0;
    double elapsedTime = 0.0;
    bool highLevel = false;

public:
    Clock_gen(double v, double period) : T(period) {
        voltage = v;
        id_counter++;
        label = "CLK" + QString::number(id_counter);
        addPin("OUT", PinType::Output, 20, 0);
    }

    void setPeriod(double period) { T = period; }
    double getPeriod() const { return T; }

    double getFrequency() const { return (T > 0.0) ? (1.0 / T) : 0.0; }
    void setFrequency(double f) { if (f > 0.0) T = 1.0 / f; }

    void advance(double dt) {
        elapsedTime += dt;
        if (T > 0 && elapsedTime >= T / 2.0) {
            elapsedTime -= T / 2.0;
            highLevel = !highLevel;
            voltage = highLevel ? voltage : 0.0;
        }
    }

    bool isHigh() const { return highLevel; }
    double getOutputVoltage() const { return highLevel ? voltage : 0.0; }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        painter->drawRect(-20, -15, 40, 30);

        QPolygon wave;
        wave << QPoint(-14, 6) << QPoint(-14, -6) << QPoint(-4, -6)
             << QPoint(-4, 6)  << QPoint(6, 6)     << QPoint(6, -6)
             << QPoint(14, -6);
        painter->drawPolyline(wave);

        painter->drawLine(20, 0, 30, 0);
        painter->drawText(-15, -20, label);

        painter->restore();
        drawPins(painter);
    }
};

// ============================================================================
// 6.2 (Passive Parts)
// ============================================================================
class Passive_part : public Component {
protected:
    double prevoltage = 0.0, precurrent = 0.0;
public:
    virtual void updatestate(double dt) = 0;
};

class Resistor : public Passive_part {
private:
    static inline int id_counter = 0;
public:
    double resistance;
    Resistor(double r) : resistance(r) {
        id_counter++;
        label = "R" + QString::number(id_counter);
        addPin("1", PinType::Bidirectional, -30, 0);
        addPin("2", PinType::Bidirectional,  30, 0);
    }

    void updatestate(double dt) override {
        Q_UNUSED(dt);
        current = voltage / resistance;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);

        painter->drawLine(-30, 0, -15, 0);
        painter->drawRect(-15, -8, 30, 16);
        painter->drawLine(15, 0, 30, 0);

        painter->drawText(-15, -12, label);
        painter->drawText(20, 12, QString::number(resistance)+" \u2126");
        painter->restore();
        drawPins(painter);
    }
};

class Capacitor : public Passive_part {
private:
    static inline int id_counter = 0;
public:
    double capacitance;

    Capacitor(double c) : capacitance(c) {
        id_counter++;
        label = "C" + QString::number(id_counter);
        addPin("1", PinType::Bidirectional, -25, 0);
        addPin("2", PinType::Bidirectional,  25, 0);
    }

    void updatestate(double dt) override {
        if (dt > 0) {
            current = capacitance * (voltage - prevoltage) / dt;
        }
        prevoltage = voltage;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(QColor(128, 0, 0), 2);
        painter->setPen(pen);


        painter->drawLine(-25, 0, -4, 0);
        painter->drawLine(4, 0, 25, 0);

        painter->drawLine(-4, -12, -4, 12);
        painter->drawLine(4, -12, 4, 12);
        painter->setPen(Qt::black);

        painter->drawText(-10, -18, label);

        painter->drawText(-15, 26, QString::number(capacitance) + "F");

        painter->restore();

        drawPins(painter);
    }
};

class Inductor : public Passive_part {
private:
    static inline int id_counter = 0;
public:
    double inductance;
    Inductor(double i) : inductance(i) {
        id_counter++;
        label = "L" + QString::number(id_counter);
        addPin("1", PinType::Bidirectional, -20, 0);
        addPin("2", PinType::Bidirectional,  20, 0);
    }

    void updatestate(double dt) override {
        if (dt > 0) {
            voltage = inductance * (current - precurrent) / dt;
        }
        precurrent = current;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->drawLine(-20, 0, -12, 0);
        painter->drawArc(-12, -6, 8, 12, 0, 180 * 16);
        painter->drawArc(-4, -6, 8, 12, 0, 180 * 16);
        painter->drawArc(4, -6, 8, 12, 0, 180 * 16);
        painter->drawLine(12, 0, 20, 0);

        painter->drawText(-10, -12, label);
        painter->restore();
        drawPins(painter);
    }
};

// ============================================================================
// 6.3(Interactive & Simple Output Parts)
// ============================================================================
class Intractive_part : public Component {};

class Switch : public Intractive_part {
private:
    static inline int id_counter = 0;
    bool closed = false;

public:
    Switch() {
        id_counter++;
        label = "SW" + QString::number(id_counter);
        addPin("1", PinType::Bidirectional, -20, 0);
        addPin("2", PinType::Bidirectional,  20, 0);
    }

    void toggle() { closed = !closed; }
    bool isClosed() const { return closed; }
    double equivalentResistance() const { return closed ? 0.0 : 1e12; }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(Qt::red);

        painter->drawLine(-20, 0, -8, 0);
        painter->drawLine(8, 0, 20, 0);

        painter->drawEllipse(QPoint(-8, 0), 2, 2);
        painter->drawEllipse(QPoint(8, 0), 2, 2);

        if (closed) {
            painter->drawLine(-8, 0, 8, 0);
        } else {
            painter->drawLine(-8, 0, 7, -10);
        }

        painter->drawText(-10, -16, label);

        painter->restore();
        drawPins(painter);
    }
};

class Push_button : public Intractive_part {
private:
    static inline int id_counter = 0;
    bool pressed = false;

public:
    Push_button() {
        id_counter++;
        label = "BTN" + QString::number(id_counter);
        addPin("1", PinType::Bidirectional, -30, 0);
        addPin("2", PinType::Bidirectional,  30, 0);
    }

    void press()   { pressed = true; }
    void release() { pressed = false; }
    bool isPressed() const { return pressed; }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen mainPen(QColor(128, 0, 0), 2);
        painter->setPen(mainPen);

        painter->drawLine(-30, 0, -12, 0);
        painter->drawLine(12, 0, 30, 0);

        painter->setBrush(QColor(210, 210, 190));
        painter->drawEllipse(QPoint(-12, 0), 5, 5);
        painter->drawEllipse(QPoint(12, 0), 5, 5);

        int yOffset = pressed ? 5 : 0;

        painter->drawLine(-18, -12 + yOffset, 18, -12 + yOffset);

        QRect capRect(-8, -18 + yOffset, 16, 6);
        painter->setBrush(QColor(210, 210, 190));
        painter->drawRect(capRect);

        painter->setPen(Qt::black);
        painter->drawText(-15, -22, label);

        painter->restore();
        drawPins(painter);
    }
};

class LED : public Intractive_part {
private:
    static inline int id_counter = 0;
    QColor color = Qt::blue;

public:
    double Vth;
    explicit LED(double v = 0.7) : Vth(v) {
        id_counter++;
        label = "D" + QString::number(id_counter);
        addPin("A", PinType::Input,  -10, 0);
        addPin("K", PinType::Output,  10, 0);
    }

    void setColor(const QColor &c) { color = c; }
    QColor getColor() const { return color; }

    bool isOn() const {
        return (voltage >= Vth) && (current > 0);
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(isOn() ? QBrush(color) : Qt::NoBrush);

        painter->drawLine(-10, 0, -5, 0);
        painter->drawLine(10, 0, 5, 0);

        QPolygon triangle;
        triangle << QPoint(-5, -6) << QPoint(5, 0) << QPoint(-5, 6);
        painter->drawPolygon(triangle);
        painter->drawLine(5, -6, 5, 6);

        painter->drawLine(3, -7, 9, -13);
        painter->drawLine(5, -13, 9, -13);
        painter->drawLine(9, -13, 9, -9);

        painter->drawLine(7, -3, 13, -9);
        painter->drawLine(9, -9, 13, -9);
        painter->drawLine(13, -9, 13, -5);

        painter->drawText(-10, 14, label);

        painter->restore();
        drawPins(painter);
    }
};

class seven_seg : public Intractive_part {
private:
    static inline int id_counter = 0;
    bool segState[7] = {false, false, false, false, false, false, false};
    bool dpState = false;
    bool hasDecimalPoint;

public:
    explicit seven_seg(bool decimalPoint = true) : hasDecimalPoint(decimalPoint) {
        id_counter++;
        label = "SEG" + QString::number(id_counter);

        addPin("a", PinType::Input, -15, 55);
        addPin("b", PinType::Input,  -5, 55);
        addPin("c", PinType::Input,   5, 55);
        addPin("d", PinType::Input,  15, 55);
        addPin("e", PinType::Input, -25, 10);
        addPin("f", PinType::Input, -25, -10);
        addPin("g", PinType::Input,  25, 0);
        if (hasDecimalPoint) {
            addPin("dp", PinType::Input, 25, 20);
        }
    }

    void setSegment(int index, bool on) {
        if (index >= 0 && index < 7) segState[index] = on;
    }
    bool getSegment(int index) const {
        return (index >= 0 && index < 7) ? segState[index] : false;
    }
    void setDecimalPoint(bool on) { dpState = on; }
    bool hasDP() const { return hasDecimalPoint; }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QRectF bodyRect(-25, -45, 50, 90);
        painter->setBrush(QColor(30, 0, 0));
        painter->setPen(QPen(QColor(220, 0, 0), 2.5));
        painter->drawRect(bodyRect);

        painter->setPen(QPen(QColor(220, 0, 0), 2));
        painter->drawLine(-15, 45, -15, 55);
        painter->drawLine(-5,  45, -5,  55);
        painter->drawLine(5,   45, 5,   55);
        painter->drawLine(15,  45, 15,  55);

        QColor offColor(70, 0, 0);
        QPen offPen(QColor(110, 0, 0), 1);

        QColor onColor(255, 25, 25);
        QPen onPen(QColor(255, 120, 120), 1);

        auto drawSegmentPoly = [&](bool state, const QPolygonF &poly) {
            painter->setBrush(state ? onColor : offColor);
            painter->setPen(state ? onPen : offPen);
            painter->drawPolygon(poly);
        };

        QPolygonF segA;
        segA << QPointF(-16, -38) << QPointF(16, -38)
             << QPointF(11, -31)  << QPointF(-11, -31);

        QPolygonF segB;
        segB << QPointF(18, -36) << QPointF(18, -2)
             << QPointF(12, -5)  << QPointF(12, -29);

        QPolygonF segC;
        segC << QPointF(18, 2)  << QPointF(18, 36)
             << QPointF(12, 29) << QPointF(12, 5);

        QPolygonF segD;
        segD << QPointF(-16, 38) << QPointF(16, 38)
             << QPointF(11, 31)  << QPointF(-11, 31);

        QPolygonF segE;
        segE << QPointF(-18, 2)  << QPointF(-18, 36)
             << QPointF(-12, 29) << QPointF(-12, 5);

        QPolygonF segF;
        segF << QPointF(-18, -36) << QPointF(-18, -2)
             << QPointF(-12, -5)  << QPointF(-12, -29);

        QPolygonF segG;
        segG << QPointF(-10, -4) << QPointF(10, -4)
             << QPointF(15, 0)   << QPointF(10, 4)
             << QPointF(-10, 4)  << QPointF(-15, 0);

        drawSegmentPoly(segState[0], segA);
        drawSegmentPoly(segState[1], segB);
        drawSegmentPoly(segState[2], segC);
        drawSegmentPoly(segState[3], segD);
        drawSegmentPoly(segState[4], segE);
        drawSegmentPoly(segState[5], segF);
        drawSegmentPoly(segState[6], segG);

        if (hasDecimalPoint) {
            painter->setBrush(dpState ? onColor : offColor);
            painter->setPen(dpState ? onPen : offPen);
            painter->drawEllipse(QPointF(20, 35), 3.5, 3.5);
        }

        painter->setPen(Qt::black);
        painter->drawText(-15, -48, label);

        painter->restore();
        drawPins(painter);
    }
};

// ============================================================================
// 6.4 Logical gate
// ============================================================================

enum class LogicState { Low, High, Undefined };

class Logical {
public:
    static constexpr double HIGH_VOLTAGE = 5.0;
    static constexpr double LOW_VOLTAGE = 0.0;
    static constexpr double HIGH_THRESHOLD = 3.5;
    static constexpr double LOW_THRESHOLD = 1.5;

    static LogicState voltageToState(double voltage) {
        if (voltage >= HIGH_THRESHOLD) return LogicState::High;
        if (voltage <= LOW_THRESHOLD) return LogicState::Low;
        return LogicState::Undefined;
    }

    static double stateToVoltage(LogicState s) {
        return (s == LogicState::High) ? HIGH_VOLTAGE : LOW_VOLTAGE;
    }

    static void reportFloatingInput() {
        qWarning("Floating input detected.");
    }
};

class LogicGate : public Component {
protected:
    double propagationDelay = 0.0;
    int numInputs;
    std::vector<LogicState> inputStates;
    LogicState outputState = LogicState::Undefined;

public:
    LogicGate(int numIn, double delay = 0.0)
        : propagationDelay(delay), numInputs(numIn) {
        inputStates.assign(numInputs, LogicState::Undefined);
        setupPins();
    }

    virtual ~LogicGate() = default;

    void setDelay(double d) { propagationDelay = d; }
    double getDelay() const { return propagationDelay; }

    void setInputState(int index, LogicState state) {
        if (index >= 0 && index < numInputs) {
            inputStates[index] = state;
        }
    }

    void setNumInputs(int n) {
        numInputs = n;
        inputStates.assign(numInputs, LogicState::Undefined);
        setupPins();
    }
    int getNumInputs() const { return numInputs; }

    LogicState getOutputState() const { return outputState; }

    virtual LogicState computeOutput() const = 0;

    void update(double currentTime) {
        Q_UNUSED(currentTime);
        bool floating = false;
        for (LogicState s : inputStates) {
            if (s == LogicState::Undefined) { floating = true; break; }
        }
        if (floating) {
            Logical::reportFloatingInput();
        }

        outputState = computeOutput();
        if (outputState != LogicState::Undefined) {
            voltage = Logical::stateToVoltage(outputState);
        }
    }

protected:
    virtual void setupPins() {
        pins.clear();
        int startY = -((numInputs - 1) * 10);
        for (int i = 0; i < numInputs; ++i) {
            QString pinName = QString("IN%1").arg(i + 1);
            addPin(pinName, PinType::Input, -30, startY + i * 20);
        }
        addPin("OUT", PinType::Output, 30, 0);
    }
};

// ----------------------------------------------------------------------------
// 6.4.1 AND Gate
// ----------------------------------------------------------------------------
class ANDGate : public LogicGate {
private:
    static inline int id_counter = 0;

public:
    ANDGate(int numIn = 2, double delay = 0.0) : LogicGate(numIn, delay) {
        id_counter++;
        label = "AND" + QString::number(id_counter);
    }

    LogicState computeOutput() const override {
        for (auto state : inputStates) {
            if (state == LogicState::Low) return LogicState::Low;
            if (state == LogicState::Undefined) return LogicState::Undefined;
        }
        return LogicState::High;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        int startY = -((numInputs - 1) * 10);
        for (int i = 0; i < numInputs; ++i) {
            painter->drawLine(-30, startY + i * 20, -15, startY + i * 20);
        }
        painter->drawLine(15, 0, 30, 0);


        int h = std::max(20, (numInputs - 1) * 10 + 10);

        QPainterPath path;
        path.moveTo(-15, -h);
        path.lineTo(0, -h);
        path.arcTo(-20, -h, 40, 2 * h, 90, -180);
        path.lineTo(-15, h);
        path.closeSubpath();

        painter->drawPath(path);
        painter->drawText(-10, -h - 5, label);

        painter->restore();
        drawPins(painter);
    }
};

// ----------------------------------------------------------------------------
// 6.4.2 OR Gate
// ----------------------------------------------------------------------------
class ORGate : public LogicGate {
private:
    static inline int id_counter = 0;

public:
    ORGate(int numIn = 2, double delay = 0.0) : LogicGate(numIn, delay) {
        id_counter++;
        label = "OR" + QString::number(id_counter);
    }

    LogicState computeOutput() const override {
        bool hasUndefined = false;
        for (auto state : inputStates) {
            if (state == LogicState::High) return LogicState::High;
            if (state == LogicState::Undefined) hasUndefined = true;
        }
        return hasUndefined ? LogicState::Undefined : LogicState::Low;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        int startY = -((numInputs - 1) * 10);
        for (int i = 0; i < numInputs; ++i) {
            painter->drawLine(-30, startY + i * 20, -10, startY + i * 20);
        }
        painter->drawLine(15, 0, 30, 0);

        int h = std::max(20, (numInputs - 1) * 10 + 10);

        QPainterPath path;
        path.moveTo(-20, -h);
        path.quadTo(-10, 0, -20, h);
        path.quadTo(0, h, 15, 0);
        path.quadTo(0, -h, -20, -h);

        painter->drawPath(path);
        painter->drawText(-10, -h - 5, label);

        painter->restore();
        drawPins(painter);
    }
};

// ----------------------------------------------------------------------------
// 6.4.3 NOT Gate (Inverter)
// ----------------------------------------------------------------------------
class NOTGate : public LogicGate {
private:
    static inline int id_counter = 0;

public:

    NOTGate(double delay = 0.0) : LogicGate(1, delay) {
        id_counter++;
        label = "NOT" + QString::number(id_counter);
    }

    LogicState computeOutput() const override {
        if (inputStates[0] == LogicState::High) return LogicState::Low;
        if (inputStates[0] == LogicState::Low) return LogicState::High;
        return LogicState::Undefined;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        painter->drawLine(-30, 0, -15, 0);
        painter->drawLine(13, 0, 30, 0);

        QPolygon triangle;
        triangle << QPoint(-15, -15) << QPoint(5, 0) << QPoint(-15, 15);
        painter->drawPolygon(triangle);

        painter->drawEllipse(5, -4, 8, 8);

        painter->drawText(-10, -20, label);

        painter->restore();
        drawPins(painter);
    }
};

// ----------------------------------------------------------------------------
// 6.4.4 NAND Gate
// ----------------------------------------------------------------------------
class NANDGate : public LogicGate {
private:
    static inline int id_counter = 0;

public:
    NANDGate(int numIn = 2, double delay = 0.0) : LogicGate(numIn, delay) {
        id_counter++;
        label = "NAND" + QString::number(id_counter);
    }

    LogicState computeOutput() const override {
        for (auto state : inputStates) {
            if (state == LogicState::Low) return LogicState::High;
            if (state == LogicState::Undefined) return LogicState::Undefined;
        }
        return LogicState::Low;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        int startY = -((numInputs - 1) * 10);
        for (int i = 0; i < numInputs; ++i) {
            painter->drawLine(-30, startY + i * 20, -15, startY + i * 20);
        }
        painter->drawLine(23, 0, 30, 0);

        int h = std::max(20, (numInputs - 1) * 10 + 10);

        QPainterPath path;
        path.moveTo(-15, -h);
        path.lineTo(0, -h);
        path.arcTo(-20, -h, 35, 2 * h, 90, -180);
        path.lineTo(-15, h);
        path.closeSubpath();

        painter->drawPath(path);
        painter->drawEllipse(15, -4, 8, 8);

        painter->drawText(-10, -h - 5, label);

        painter->restore();
        drawPins(painter);
    }
};

// ----------------------------------------------------------------------------
// 6.4.5 NOR Gate
// ----------------------------------------------------------------------------
class NORGate : public LogicGate {
private:
    static inline int id_counter = 0;

public:
    NORGate(int numIn = 2, double delay = 0.0) : LogicGate(numIn, delay) {
        id_counter++;
        label = "NOR" + QString::number(id_counter);
    }

    LogicState computeOutput() const override {
        bool hasUndefined = false;
        for (auto state : inputStates) {
            if (state == LogicState::High) return LogicState::Low;
            if (state == LogicState::Undefined) hasUndefined = true;
        }
        return hasUndefined ? LogicState::Undefined : LogicState::High;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        int startY = -((numInputs - 1) * 10);
        for (int i = 0; i < numInputs; ++i) {
            painter->drawLine(-30, startY + i * 20, -10, startY + i * 20);
        }
        painter->drawLine(23, 0, 30, 0);

        int h = std::max(20, (numInputs - 1) * 10 + 10);

        QPainterPath path;
        path.moveTo(-20, -h);
        path.quadTo(-10, 0, -20, h);
        path.quadTo(0, h, 15, 0);
        path.quadTo(0, -h, -20, -h);

        painter->drawPath(path);
        painter->drawEllipse(15, -4, 8, 8);

        painter->drawText(-10, -h - 5, label);

        painter->restore();
        drawPins(painter);
    }
};

// ----------------------------------------------------------------------------
// 6.4.6 XOR Gate
// ----------------------------------------------------------------------------
class XORGate : public LogicGate {
private:
    static inline int id_counter = 0;

public:
    XORGate(int numIn = 2, double delay = 0.0) : LogicGate(numIn, delay) {
        id_counter++;
        label = "XOR" + QString::number(id_counter);
    }

    LogicState computeOutput() const override {
        int highCount = 0;
        for (auto state : inputStates) {
            if (state == LogicState::Undefined) return LogicState::Undefined;
            if (state == LogicState::High) highCount++;
        }
        return (highCount % 2 != 0) ? LogicState::High : LogicState::Low;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        int startY = -((numInputs - 1) * 10);
        for (int i = 0; i < numInputs; ++i) {
            painter->drawLine(-30, startY + i * 20, -15, startY + i * 20);
        }
        painter->drawLine(15, 0, 30, 0);

        int h = std::max(20, (numInputs - 1) * 10 + 10);

        QPainterPath backArc;
        backArc.moveTo(-24, -h);
        backArc.quadTo(-14, 0, -24, h);
        painter->drawPath(backArc);

        QPainterPath path;
        path.moveTo(-18, -h);
        path.quadTo(-8, 0, -18, h);
        path.quadTo(0, h, 15, 0);
        path.quadTo(0, -h, -18, -h);

        painter->drawPath(path);
        painter->drawText(-10, -h - 5, label);

        painter->restore();
        drawPins(painter);
    }
};

// ----------------------------------------------------------------------------
// 6.4.7 XNOR Gate
// ----------------------------------------------------------------------------
class XNORGate : public LogicGate {
private:
    static inline int id_counter = 0;

public:
    XNORGate(int numIn = 2, double delay = 0.0) : LogicGate(numIn, delay) {
        id_counter++;
        label = "XNOR" + QString::number(id_counter);
    }

    LogicState computeOutput() const override {
        int highCount = 0;
        for (auto state : inputStates) {
            if (state == LogicState::Undefined) return LogicState::Undefined;
            if (state == LogicState::High) highCount++;
        }
        return (highCount % 2 != 0) ? LogicState::Low : LogicState::High;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        int startY = -((numInputs - 1) * 10);
        for (int i = 0; i < numInputs; ++i) {
            painter->drawLine(-30, startY + i * 20, -15, startY + i * 20);
        }
        painter->drawLine(23, 0, 30, 0);

        int h = std::max(20, (numInputs - 1) * 10 + 10);

        QPainterPath backArc;
        backArc.moveTo(-24, -h);
        backArc.quadTo(-14, 0, -24, h);
        painter->drawPath(backArc);

        QPainterPath path;
        path.moveTo(-18, -h);
        path.quadTo(-8, 0, -18, h);
        path.quadTo(0, h, 15, 0);
        path.quadTo(0, -h, -18, -h);

        painter->drawPath(path);
        painter->drawEllipse(15, -4, 8, 8);

        painter->drawText(-10, -h - 5, label);

        painter->restore();
        drawPins(painter);
    }
};

// ----------------------------------------------------------------------------
// 6.4.8 D Flip-Flop
// ----------------------------------------------------------------------------
class DFlipFlop : public Component {
private:
    static inline int id_counter = 0;

    double propagationDelay = 0.0;

    LogicState D_state   = LogicState::Undefined;
    LogicState CLK_state = LogicState::Undefined;
    LogicState CLK_prev  = LogicState::Undefined;
    LogicState Q_state   = LogicState::Undefined;

public:
    explicit DFlipFlop(double delay = 0.0) : propagationDelay(delay) {
        id_counter++;
        label = "U" + QString::number(id_counter);

        addPin("D",   PinType::Input,  -30, -15);
        addPin("CLK", PinType::Input,  -30,  15);
        addPin("Q",   PinType::Output,  30, -15);
        addPin("Qn",  PinType::Output,  30,  15);
    }

    void setDelay(double d) { propagationDelay = d; }
    double getDelay() const { return propagationDelay; }

    void setD(LogicState d) { D_state = d; }

    void setClock(LogicState clk) {
        CLK_prev = CLK_state;
        CLK_state = clk;
    }

    void evaluate() {
        if (D_state == LogicState::Undefined || CLK_state == LogicState::Undefined) {
            Logical::reportFloatingInput();
            Q_state = LogicState::Undefined;
            return;
        }

        bool risingEdge = (CLK_prev == LogicState::Low && CLK_state == LogicState::High);
        if (risingEdge) {
            Q_state = D_state;
        }

        voltage = Logical::stateToVoltage(Q_state);
    }

    LogicState getQ() const { return Q_state; }
    LogicState getQn() const {
        if (Q_state == LogicState::Undefined) return LogicState::Undefined;
        return (Q_state == LogicState::High) ? LogicState::Low : LogicState::High;
    }

    void draw(QPainter *painter) override {
        painter->save();
        applyTransform(painter);

        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        painter->drawRect(-25, -25, 50, 50);

        painter->drawLine(-30, -15, -25, -15);
        painter->drawLine(-30,  15, -25,  15);
        painter->drawLine(25, -15, 30, -15);
        painter->drawLine(25,  15, 30,  15);

        QPolygon clkTri;
        clkTri << QPoint(-25, 10) << QPoint(-17, 15) << QPoint(-25, 20);
        painter->drawPolygon(clkTri);

        painter->drawText(-20, -16, "D");
        painter->drawText(-14, 26, "CLK");
        painter->drawText(15, -16, "Q");
        painter->drawText(12, 22, QString::fromUtf8("Q\u0305"));

        painter->drawText(-12, -30, label);
        painter->drawText(-14, 5, "DTFF");

        painter->restore();
        drawPins(painter);
    }
};

#endif // COMPONENT_H