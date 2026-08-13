#include "pickdevicesdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QPainter>
#include <QPolygon>
#include <QRegularExpression>

namespace {

// Very small heuristic used only to decide which symbol to sketch in the
// preview box. It does NOT depend on Component.h on purpose, so this file
// has no coupling with the simulation-side component classes.
enum class SymbolKind { Resistor, Capacitor, Inductor, Battery, Ground, Diode, Led, Ic, Switch, Generic };

SymbolKind classify(const QString &name, const QString &category)
{
    const QString n = name.toUpper();
    const QString c = category.toUpper();

    if (n.contains("RES") || c.contains("RESIST"))   return SymbolKind::Resistor;
    if (n.contains("CAP")  || c.contains("CAPACIT")) return SymbolKind::Capacitor;
    if (n.contains("IND")  || c.contains("INDUCT") || n.contains("CHOKE")) return SymbolKind::Inductor;
    if (n.contains("BATT") || n.contains("CELL") || n.contains("DC-SOURCE")) return SymbolKind::Battery;
    if (n == "GND" || n.contains("GROUND"))          return SymbolKind::Ground;
    if (n.contains("LED"))                           return SymbolKind::Led;
    if (n.contains("DIO") || n.startsWith("1N"))      return SymbolKind::Diode;
    if (n.contains("SW")  || c.contains("SWITCH"))   return SymbolKind::Switch;
    if (c.contains("IC") || c.contains("74") || c.contains("40") ||
        c.contains("MICRO") || c.contains("ANALOG"))
        return SymbolKind::Ic;
    return SymbolKind::Generic;
}

} // namespace

// ---------------------------------------------------------------------
// A tiny widget that sketches a schematic-like preview of the selected
// device. This is intentionally simple - it is a hint, not a real render.
// ---------------------------------------------------------------------
class DevicePreviewWidget : public QWidget
{
public:
    explicit DevicePreviewWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumSize(200, 200);
        setStyleSheet("background-color:#EAE8DE; border:1px solid #A0A0A0;");
    }

    void setDevice(const QString &name, const QString &category)
    {
        m_name = name;
        m_kind = name.isEmpty() ? SymbolKind::Generic : classify(name, category);
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#EAE8DE"));

        if (m_name.isEmpty())
            return;

        p.translate(width() / 2, height() / 2);
        p.setPen(QPen(Qt::black, 2));

        switch (m_kind) {
        case SymbolKind::Resistor:
            p.drawLine(-40, 0, -18, 0);
            p.drawRect(-18, -10, 36, 20);
            p.drawLine(18, 0, 40, 0);
            break;
        case SymbolKind::Capacitor:
            p.drawLine(-30, 0, -6, 0);
            p.drawLine(-6, -18, -6, 18);
            p.drawLine(6, -18, 6, 18);
            p.drawLine(6, 0, 30, 0);
            break;
        case SymbolKind::Inductor:
            p.drawLine(-40, 0, -24, 0);
            for (int i = 0; i < 4; ++i)
                p.drawArc(-24 + i * 12, -10, 12, 20, 0, 180 * 16);
            p.drawLine(24, 0, 40, 0);
            break;
        case SymbolKind::Battery:
            p.drawLine(0, -40, 0, -18);
            p.drawLine(-14, -18, 14, -18);
            p.drawLine(-7, -8, 7, -8);
            p.drawLine(-14, 8, 14, 8);
            p.drawLine(-7, 18, 7, 18);
            p.drawLine(0, 18, 0, 40);
            break;
        case SymbolKind::Ground:
            p.drawLine(0, -25, 0, 0);
            p.drawLine(-18, 0, 18, 0);
            p.drawLine(-11, 6, 11, 6);
            p.drawLine(-4, 12, 4, 12);
            break;
        case SymbolKind::Diode:
        case SymbolKind::Led: {
            p.drawLine(-30, 0, -10, 0);
            QPolygon tri;
            tri << QPoint(-10, -14) << QPoint(-10, 14) << QPoint(10, 0);
            p.setBrush(m_kind == SymbolKind::Led ? QColor("#ff6b6b") : Qt::black);
            p.drawPolygon(tri);
            p.drawLine(10, -14, 10, 14);
            p.drawLine(10, 0, 30, 0);
            break;
        }
        case SymbolKind::Switch:
            p.drawEllipse(QPoint(-25, 0), 3, 3);
            p.drawEllipse(QPoint(25, 0), 3, 3);
            p.drawLine(-22, 0, 18, -14);
            break;
        case SymbolKind::Ic:
            p.setBrush(QColor("#dcdcdc"));
            p.drawRect(-35, -25, 70, 50);
            for (int i = 0; i < 4; ++i) {
                p.drawLine(-35, -18 + i * 12, -45, -18 + i * 12);
                p.drawLine(35, -18 + i * 12, 45, -18 + i * 12);
            }
            break;
        default:
            p.setBrush(QColor("#dcdcdc"));
            p.drawRect(-30, -20, 60, 40);
            break;
        }
    }

private:
    QString m_name;
    SymbolKind m_kind = SymbolKind::Generic;
};

// ---------------------------------------------------------------------
// PickDevicesDialog
// ---------------------------------------------------------------------
PickDevicesDialog::PickDevicesDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Pick Devices");
    resize(1000, 620);

    buildDatabase();

    // ---- Left column: filters ------------------------------------------------
    auto *keywordLabel = new QLabel("Keywords:", this);
    keywordEdit = new QLineEdit(this);

    matchWholeWordsBox = new QCheckBox(this);
    onlyWithModelsBox  = new QCheckBox(this);
    onlyManagedBox     = new QCheckBox(this);

    auto makeCheckRow = [this](const QString &text, QCheckBox *box) {
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(text, this);
        row->addStretch();
        row->addWidget(lbl);
        row->addWidget(box);
        return row;
    };

    auto *categoryLabel = new QLabel("Category:", this);
    categoryList = new QListWidget(this);

    auto *subCategoryLabel = new QLabel("Sub-category:", this);
    subCategoryList = new QListWidget(this);

    auto *manufacturerLabel = new QLabel("Manufacturer:", this);
    manufacturerList = new QListWidget(this);

    auto *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(keywordLabel);
    leftLayout->addWidget(keywordEdit);
    leftLayout->addLayout(makeCheckRow("Match whole words?", matchWholeWordsBox));
    leftLayout->addLayout(makeCheckRow("Show only parts with models?", onlyWithModelsBox));
    leftLayout->addLayout(makeCheckRow("Show Only Managed parts", onlyManagedBox));
    leftLayout->addSpacing(8);
    leftLayout->addWidget(categoryLabel);
    leftLayout->addWidget(categoryList, 3);
    leftLayout->addWidget(subCategoryLabel);
    leftLayout->addWidget(subCategoryList, 2);
    leftLayout->addWidget(manufacturerLabel);
    leftLayout->addWidget(manufacturerList, 2);

    // ---- Middle column: results ------------------------------------------------
    resultsHeaderLabel = new QLabel("Results (No Filter):", this);

    auto *placeholder = new QLabel(
        "No search criteria.\n"
        "Please enter one or more keywords and/or\n"
        "select a Category, Sub-category or Manufacturer.", this);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("color:#606060;");

    resultsTable = new QTableWidget(0, 1, this);
    resultsTable->setHorizontalHeaderLabels({"Device"});
    resultsTable->horizontalHeader()->setStretchLastSection(true);
    resultsTable->verticalHeader()->setVisible(false);
    resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    resultsStack = new QStackedWidget(this);
    resultsStack->addWidget(placeholder);   // index 0: "no criteria" message
    resultsStack->addWidget(resultsTable);  // index 1: actual results

    auto *middleLayout = new QVBoxLayout();
    middleLayout->addWidget(resultsHeaderLabel);
    middleLayout->addWidget(resultsStack, 1);

    // ---- Right column: preview (PCB preview intentionally omitted) -------------
    auto *previewLabel = new QLabel("Preview", this);
    previewWidget = new DevicePreviewWidget(this);
    previewNameLabel = new QLabel(this);
    previewNameLabel->setAlignment(Qt::AlignCenter);
    previewNameLabel->setStyleSheet("font-weight:bold;");

    auto *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(previewLabel);
    rightLayout->addWidget(previewWidget, 1);
    rightLayout->addWidget(previewNameLabel);
    rightLayout->addStretch();

    // ---- Top area (3 columns) ---------------------------------------------------
    auto *columnsLayout = new QHBoxLayout();
    columnsLayout->addLayout(leftLayout, 2);
    columnsLayout->addLayout(middleLayout, 3);
    columnsLayout->addLayout(rightLayout, 2);

    // ---- Bottom buttons -----------------------------------------------------------
    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);
    okButton->setEnabled(false);

    auto *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(columnsLayout, 1);
    mainLayout->addLayout(buttonsLayout);

    // ---- Fill category / manufacturer lists once ---------------------------------
    populateCategoryList();
    populateManufacturerList();
    subCategoryList->addItem("(All Sub-categories)");
    subCategoryList->setCurrentRow(0);

    // ---- Signals --------------------------------------------------------------------
    connect(keywordEdit, &QLineEdit::textChanged, this, &PickDevicesDialog::onKeywordChanged);
    connect(matchWholeWordsBox, &QCheckBox::toggled, this, [this](bool){ runSearch(); });
    connect(onlyWithModelsBox,  &QCheckBox::toggled, this, [this](bool){ runSearch(); });
    connect(onlyManagedBox,     &QCheckBox::toggled, this, [this](bool){ runSearch(); });

    connect(categoryList, &QListWidget::currentItemChanged, this, &PickDevicesDialog::onCategoryChanged);
    connect(subCategoryList, &QListWidget::currentItemChanged, this, &PickDevicesDialog::onSubCategoryChanged);
    connect(manufacturerList, &QListWidget::currentItemChanged, this, &PickDevicesDialog::onManufacturerChanged);

    connect(resultsTable, &QTableWidget::itemSelectionChanged, this, &PickDevicesDialog::onResultRowChanged);
    connect(resultsTable, &QTableWidget::cellDoubleClicked, this, &PickDevicesDialog::onResultActivated);

    connect(okButton, &QPushButton::clicked, this, &PickDevicesDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Start on "(All Categories)" -> no active filter -> placeholder shown,
    // exactly like the real Proteus dialog when it first opens.
    categoryList->setCurrentRow(0);
}

void PickDevicesDialog::addDevice(const QString &name, const QString &category,
                                   const QString &subCategory, const QString &manufacturer,
                                   const QString &description)
{
    database.append({name, category, subCategory, manufacturer, description});
}

void PickDevicesDialog::buildDatabase()
{
    // A small, representative subset of the Proteus device library.
    // Where it makes sense, names line up with the concrete classes that
    // already exist in Component.h (Resistor, Capacitor, Inductor, GND,
    // Battery, DC_vol_source) so this can be wired to real instantiation
    // later without renaming anything here.

    addDevice("10WATT0R1",        "Resistors", "10 Watt Wirewound", "Vishay",  "10W 0.1R wirewound resistor");
    addDevice("10WATT4K7",        "Resistors", "10 Watt Wirewound", "Vishay",  "10W 4.7k wirewound resistor");
    addDevice("02013A0R5CAT2A",   "Resistors", "0.6W Metal Film",   "Yageo",   "0.6W metal film resistor");
    addDevice("RES1K",            "Resistors", "2 Watt Metal Film", "Bourns",  "1k metal film resistor");
    addDevice("RES10K",           "Resistors", "2 Watt Metal Film", "Bourns",  "10k metal film resistor");
    addDevice("RES100",           "Resistors", "0.6W Metal Film",   "Yageo",   "100R metal film resistor");
    addDevice("POT-HG",           "Resistors", "Potentiometers",    "Bourns",  "General purpose potentiometer");

    addDevice("CAP",              "Capacitors", "Ceramic",      "Generic",   "Generic ceramic capacitor");
    addDevice("CAP-ELEC",         "Capacitors", "Electrolytic", "Panasonic", "Electrolytic capacitor");
    addDevice("CAP-TANT",         "Capacitors", "Tantalum",     "AVX",       "Tantalum capacitor");
    addDevice("CAP-POLY",         "Capacitors", "Polyester",    "Vishay",    "Polyester film capacitor");

    addDevice("INDUCTOR",         "Inductors", "Radial", "Bourns", "General purpose inductor");
    addDevice("INDUCTOR-1UH",     "Inductors", "Radial", "Bourns", "1uH radial inductor");
    addDevice("CHOKE",            "Inductors", "Axial",  "Vishay", "RF choke inductor");

    addDevice("BATTERY",          "Simulator Primitives", "Sources", "(Unspecified)", "Ideal battery / DC voltage source");
    addDevice("CELL",             "Simulator Primitives", "Sources", "(Unspecified)", "Single cell battery");
    addDevice("DC-SOURCE",        "Simulator Primitives", "Sources", "(Unspecified)", "Ideal DC voltage source");
    addDevice("GROUND",           "Miscellaneous", "Terminals", "(Unspecified)", "Ground / reference terminal");

    addDevice("LED-RED",          "Optoelectronics", "LEDs", "Generic", "Red LED");
    addDevice("LED-GREEN",        "Optoelectronics", "LEDs", "Generic", "Green LED");
    addDevice("LED-YELLOW",       "Optoelectronics", "LEDs", "Generic", "Yellow LED");
    addDevice("DIODE",            "Diodes", "Signal",    "ON Semiconductor", "Small-signal diode");
    addDevice("1N4148",           "Diodes", "Signal",    "ON Semiconductor", "Fast switching diode");
    addDevice("1N4001",           "Diodes", "Rectifier", "ON Semiconductor", "1A rectifier diode");

    addDevice("SW-SPST",          "Switches & Relays", "Toggle",      "Generic", "Single pole single throw switch");
    addDevice("SW-PB",            "Switches & Relays", "Push Button", "Generic", "Momentary push button");
    addDevice("RELAY",            "Switches & Relays", "Relays",      "Omron",   "General purpose relay");

    addDevice("7SEG-COM-CATHODE", "Optoelectronics", "7-Segment Displays", "Generic", "Common cathode 7-segment display");

    addDevice("7400",             "TTL 74 series",    "Gates", "Texas Instruments", "Quad 2-input NAND gate");
    addDevice("7404",             "TTL 74 series",    "Gates", "Texas Instruments", "Hex inverter");
    addDevice("4001",             "CMOS 4000 series", "Gates", "Texas Instruments", "Quad 2-input NOR gate");
    addDevice("4011",             "CMOS 4000 series", "Gates", "Texas Instruments", "Quad 2-input NAND gate");

    addDevice("ATMEGA328P",       "Microprocessor ICs", "AVR",    "Microchip", "8-bit AVR microcontroller");
    addDevice("ARDUINO UNO",      "Microprocessor ICs", "Boards", "Arduino",   "Arduino Uno development board");
    addDevice("PIC16F877A",       "Microprocessor ICs", "PIC",    "Microchip", "8-bit PIC microcontroller");

    addDevice("LM358",            "Analog ICs", "Op-Amps", "Texas Instruments", "Dual op-amp");
    addDevice("LM741",            "Analog ICs", "Op-Amps", "Texas Instruments", "Single op-amp");
    addDevice("NE555",            "Analog ICs", "Timers",  "Texas Instruments", "Precision timer IC");

    addDevice("2N2222",           "Transistors", "NPN Bipolar", "ON Semiconductor", "General purpose NPN transistor");
    addDevice("2N3906",           "Transistors", "PNP Bipolar", "ON Semiconductor", "General purpose PNP transistor");
    addDevice("BC547",            "Transistors", "NPN Bipolar", "Fairchild",        "General purpose NPN transistor");

    addDevice("CONN-SIL2",        "Connectors", "Headers", "Generic", "2 way pin header");
    addDevice("CONN-SIL4",        "Connectors", "Headers", "Generic", "4 way pin header");
    addDevice("USB-A",            "Connectors", "USB",     "Molex",   "USB Type-A connector");

    addDevice("CRYSTAL",          "Miscellaneous", "Crystals",   "Generic",     "Quartz crystal resonator");
    addDevice("FUSE",             "Miscellaneous", "Protection", "Littelfuse", "Fuse element");
    addDevice("MOTOR",            "Miscellaneous", "Motors",     "Generic",     "DC motor");
}

void PickDevicesDialog::populateCategoryList()
{
    QStringList categories;
    for (const auto &d : database)
        if (!categories.contains(d.category))
            categories << d.category;
    categories.sort(Qt::CaseInsensitive);

    categoryList->clear();
    categoryList->addItem("(All Categories)");
    categoryList->addItem("(Unspecified)");
    categoryList->addItems(categories);
}

void PickDevicesDialog::populateSubCategoryList()
{
    subCategoryList->blockSignals(true);
    subCategoryList->clear();
    subCategoryList->addItem("(All Sub-categories)");

    QString cat = categoryList->currentItem() ? categoryList->currentItem()->text() : QString();
    QStringList subs;
    for (const auto &d : database) {
        if (cat.isEmpty() || cat == "(All Categories)" || d.category == cat) {
            if (!d.subCategory.isEmpty() && !subs.contains(d.subCategory))
                subs << d.subCategory;
        }
    }
    subs.sort(Qt::CaseInsensitive);
    subCategoryList->addItems(subs);
    subCategoryList->setCurrentRow(0);
    subCategoryList->blockSignals(false);
}

void PickDevicesDialog::populateManufacturerList()
{
    QStringList manufacturers;
    for (const auto &d : database)
        if (!manufacturers.contains(d.manufacturer))
            manufacturers << d.manufacturer;
    manufacturers.sort(Qt::CaseInsensitive);

    manufacturerList->clear();
    manufacturerList->addItem("(All Manufacturers)");
    manufacturerList->addItem("(Unspecified)");
    manufacturerList->addItems(manufacturers);
    manufacturerList->setCurrentRow(0);
}

void PickDevicesDialog::onKeywordChanged(const QString &)
{
    runSearch();
}

void PickDevicesDialog::onCategoryChanged(QListWidgetItem *, QListWidgetItem *)
{
    populateSubCategoryList();
    runSearch();
}

void PickDevicesDialog::onSubCategoryChanged(QListWidgetItem *, QListWidgetItem *)
{
    runSearch();
}

void PickDevicesDialog::onManufacturerChanged(QListWidgetItem *, QListWidgetItem *)
{
    runSearch();
}

void PickDevicesDialog::runSearch()
{
    const QString keyword = keywordEdit->text().trimmed();

    const QString category     = categoryList->currentItem()     ? categoryList->currentItem()->text()     : QString();
    const QString subCategory  = subCategoryList->currentItem()  ? subCategoryList->currentItem()->text()  : QString();
    const QString manufacturer = manufacturerList->currentItem() ? manufacturerList->currentItem()->text() : QString();

    const bool categoryActive     = !category.isEmpty()     && category     != "(All Categories)";
    const bool subCategoryActive  = !subCategory.isEmpty()  && subCategory  != "(All Sub-categories)";
    const bool manufacturerActive = !manufacturer.isEmpty() && manufacturer != "(All Manufacturers)";

    const bool hasCriteria = !keyword.isEmpty() || categoryActive || subCategoryActive || manufacturerActive;

    if (!hasCriteria) {
        resultsStack->setCurrentIndex(0);
        resultsHeaderLabel->setText("Results (No Filter):");
        okButton->setEnabled(false);
        updatePreview(QString());
        return;
    }

    QRegularExpression wordRegex;
    if (matchWholeWordsBox->isChecked() && !keyword.isEmpty()) {
        wordRegex = QRegularExpression("\\b" + QRegularExpression::escape(keyword) + "\\b",
                                        QRegularExpression::CaseInsensitiveOption);
    }

    QVector<DeviceInfo> matches;
    for (const auto &d : database) {
        if (categoryActive     && d.category     != category)     continue;
        if (subCategoryActive  && d.subCategory  != subCategory)  continue;
        if (manufacturerActive && d.manufacturer != manufacturer) continue;

        if (!keyword.isEmpty()) {
            if (matchWholeWordsBox->isChecked()) {
                if (!wordRegex.match(d.name).hasMatch() && !wordRegex.match(d.description).hasMatch())
                    continue;
            } else {
                if (!d.name.contains(keyword, Qt::CaseInsensitive) &&
                    !d.description.contains(keyword, Qt::CaseInsensitive))
                    continue;
            }
        }
        matches << d;
    }

    resultsHeaderLabel->setText(QString("Results (%1 %2):")
                                     .arg(matches.size())
                                     .arg(matches.size() == 1 ? "item" : "items"));

    resultsTable->setRowCount(matches.size());
    for (int i = 0; i < matches.size(); ++i) {
        auto *item = new QTableWidgetItem(matches[i].name);
        item->setData(Qt::UserRole, matches[i].category);
        resultsTable->setItem(i, 0, item);
    }

    resultsStack->setCurrentIndex(1);
    okButton->setEnabled(false);
    updatePreview(QString());

    if (!matches.isEmpty())
        resultsTable->selectRow(0);
}

void PickDevicesDialog::onResultRowChanged()
{
    const auto items = resultsTable->selectedItems();
    if (items.isEmpty()) {
        okButton->setEnabled(false);
        updatePreview(QString());
        return;
    }
    const QString name = items.first()->text();
    const QString category = items.first()->data(Qt::UserRole).toString();
    okButton->setEnabled(true);
    updatePreview(name);
    previewWidget->setDevice(name, category);
}

void PickDevicesDialog::onResultActivated(int row, int)
{
    if (row < 0) return;
    auto *item = resultsTable->item(row, 0);
    if (!item) return;
    m_selectedDevice = item->text();
    emit deviceAccepted(m_selectedDevice);
    accept();
}

void PickDevicesDialog::updatePreview(const QString &deviceName)
{
    previewNameLabel->setText(deviceName);
    if (deviceName.isEmpty())
        previewWidget->setDevice(QString(), QString());
}

void PickDevicesDialog::onOkClicked()
{
    const auto items = resultsTable->selectedItems();
    if (items.isEmpty()) return;
    m_selectedDevice = items.first()->text();
    emit deviceAccepted(m_selectedDevice);
    accept();
}
