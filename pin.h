#ifndef PIN_H
#define PIN_H

#include <QString>
#include "geometrytypes.h"

// نوع پايه: برای مشخص کردن رفتار الکتريکی هر پايه (بخش ۵ و ۶)
enum class PinType {
    Input,
    Output,
    Bidirectional,
    Power,
    Ground
};

class Wire; // forward declare - در بخش سيم کشی (۵) تعريف می شود

// Pin نماينده يک پايه فيزيکی روی بدنه يک قطعه است.
// localPosition هميشه نسبت به مبدأ محلی خود قطعه ذخيره می شود (يعنی همون
// مختصاتی که داخل تابع draw() برای رسم پايه ها استفاده کردی، مثلا (-30,0)
// برای پايه چپ مقاومت). موقعيت واقعی روی صفحه را Component محاسبه می کند
// چون او هم Position و هم Orientation قطعه را در اختيار دارد.
class Pin
{
public:
    Pin() = default;
    Pin(const QString &name, PinType type, const Position &localPosition)
        : name(name), type(type), localPosition(localPosition) {}

    QString name;
    PinType type = PinType::Input;
    Position localPosition;

    bool isHighlighted = false;    // برای هايلايت هنگام نزديک شدن موس (بخش ۵.۱)
    int sensitivityRadius = 6;     // شعاع حساسيت برخورد موس با پايه (پيکسل)

    Wire *connectedWire = nullptr; // فعلا استفاده نمی شود؛ برای بخش سيم کشی رزرو شده

    bool isConnected() const { return connectedWire != nullptr; }

    // چک می کند آیا يک نقطه روی صفحه (worldMousePos) به اندازه کافی به اين پايه
    // نزديک است يا نه. componentOrigin و componentOrientation را Component
    // موقع صدا زدن اين تابع پاس می دهد. برای بخش ۵ (تشخيص شروع سيم کشی) استفاده می شود.
    bool isNear(const Position &worldMousePos,
                const Position &componentOrigin,
                Orientation componentOrientation) const;
};

#endif // PIN_H
