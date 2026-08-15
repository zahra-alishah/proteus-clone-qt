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

// Small heuristic used only to decide which symbol to sketch in the preview
// box. Kept independent from Component.h on purpose (this is only a visual
// hint inside the dialog, not the real draw() used on the canvas).
enum class SymbolKind {
    Resistor, Capacitor, Inductor, Battery, DcSource, Clock, Ground,
    Switch, PushButton, Led, SevenSeg, Gate, FlipFlop, Generic
};

SymbolKind classify(const QString &name)
{
    const QString n = name.toUpper();

    if (n.contains("RESISTOR"))      return SymbolKind::Resistor;
    if (n.contains("CAPACITOR"))     return SymbolKind::Capacitor;
    if (n.contains("INDUCTOR"))      return SymbolKind::Inductor;
    if (n.contains("GROUND"))        return SymbolKind::Ground;
    if (n.contains("BATTERY"))       return SymbolKind::Battery;
    if (n.contains("CLOCK"))         return SymbolKind::Clock;
    if (n.contains("DC"))            return SymbolKind::DcSource;
    if (n.contains("PUSH"))          return SymbolKind::PushButton;
    if (n.contains("SWITCH"))        return SymbolKind::Switch;
    if (n.contains("LED"))           return SymbolKind::Led;
    if (n.contains("SEVEN") || n.contains("SEGMENT")) return SymbolKind::SevenSeg;
    if (n.contains("FLIP"))          return SymbolKind::FlipFlop;
    if (n.contains("AND") || n.contains("OR") || n.contains("NOT") ||
        n.contains("NAND") || n.contains("NOR") || n.contains("XOR") || n.contains("XNOR"))
        return SymbolKind::Gate;
    return SymbolKind::Generic;
}

} // namespace

// ---------------------------------------------------------------------
// A tiny widget that sketches a schematic-like preview of the selected
// device. Intentionally simple - it is a hint, not a real render.
// ---------------------------------------------------------------------
class DevicePreviewWidget : public QWidget
{
public:
    explicit DevicePreviewWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumSize(200, 200);
        setStyleSheet("background-color:#EAE8DE; border:1px solid #A0A0A0;");
    }

    void setDevice(const QString &name)
    {
        m_name = name;
        m_kind = name.isEmpty() ? SymbolKind::Generic : classify(name);
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
        case SymbolKind::DcSource:
            p.drawLine(0, -35, 0, -20);
            p.drawLine(0, 20, 0, 35);
            p.drawEllipse(-20, -20, 40, 40);
            p.drawLine(-6, -10, 6, -10);
            p.drawLine(0, -16, 0, -4);
            p.drawLine(-6, 10, 6, 10);
            break;
        case SymbolKind::Clock:
            p.setBrush(Qt::NoBrush);
            p.drawRect(-20, -15, 40, 30);
            {
                QPolygon wave;
                wave << QPoint(-14, 6) << QPoint(-14, -6) << QPoint(-4, -6)
                     << QPoint(-4, 6)  << QPoint(6, 6)     << QPoint(6, -6)
                     << QPoint(14, -6);
                p.drawPolyline(wave);
            }
            p.drawLine(20, 0, 30, 0);
            break;
        case SymbolKind::Ground:
            p.drawLine(0, -25, 0, 0);
            p.drawLine(-18, 0, 18, 0);
            p.drawLine(-11, 6, 11, 6);
            p.drawLine(-4, 12, 4, 12);
            break;
        case SymbolKind::Switch:
            p.drawEllipse(QPoint(-25, 0), 3, 3);
            p.drawEllipse(QPoint(25, 0), 3, 3);
            p.drawLine(-22, 0, 22, 0);
            p.drawLine(-22, 0, 18, -14);
            break;
        case SymbolKind::PushButton:
            p.drawEllipse(QPoint(-25, 0), 3, 3);
            p.drawEllipse(QPoint(25, 0), 3, 3);
            p.drawLine(-22, 0, 22, 0);
            p.drawLine(0, 0, 0, -12);
            p.drawLine(-8, -12, 8, -12);
            break;
        case SymbolKind::Led: {
            p.drawLine(-30, 0, -10, 0);
            QPolygon tri;
            tri << QPoint(-10, -14) << QPoint(-10, 14) << QPoint(10, 0);
            p.setBrush(QColor("#ff6b6b"));
            p.drawPolygon(tri);
            p.drawLine(10, -14, 10, 14);
            p.drawLine(10, 0, 30, 0);
            break;
        }
        case SymbolKind::SevenSeg: {
            p.setBrush(QColor(200, 40, 40));
            p.drawRect(-18, -36, 36, 6);   // a
            p.drawRect(16, -32, 6, 30);    // b
            p.drawRect(16, 2, 6, 30);      // c
            p.drawRect(-18, 32, 36, 6);    // d
            p.drawRect(-22, 2, 6, 30);     // e
            p.drawRect(-22, -32, 6, 30);   // f
            p.drawRect(-18, -3, 36, 6);    // g
            break;
        }
        case SymbolKind::Gate:
            p.setBrush(Qt::NoBrush);
            p.drawLine(-40, 0, -20, 0);
            p.drawLine(20, 0, 40, 0);
            p.drawRoundedRect(-20, -20, 40, 40, 12, 12);
            p.drawText(QRectF(-20, -20, 40, 40), Qt::AlignCenter, m_name.left(4));
            break;
        case SymbolKind::FlipFlop:
            p.setBrush(Qt::NoBrush);
            p.drawRect(-25, -25, 50, 50);
            p.drawText(-18, 5, "DFF");
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
    // ------------------------------------------------------------------
    // Only the components that actually exist in Component.h are listed
    // here, grouped exactly like the sections of that file:
    //   6.1 Primary Sources      -> GND, Battery, DC_vol_source, Clock_gen
    //   6.2 Passive Parts        -> Resistor, Capacitor, Inductor
    //   6.3 Interactive Parts    -> Switch, Push_button, LED, seven_seg
    //   6.4 Logic Gates & FF     -> AND/OR/NOT/NAND/NOR/XOR/XNOR, DFlipFlop
    //
    // Every name below contains the exact keyword that
    // schematicPage::onCanvasClicked() checks for, so selecting a device
    // here and clicking on the canvas instantiates the real matching class.
    // ------------------------------------------------------------------

    // ---- 6.1 Primary Sources ----
    addDevice("GROUND",             "Primary Sources", "Terminals", "(Unspecified)", "Ground / reference terminal");
    addDevice("BATTERY",            "Primary Sources", "Sources",   "(Unspecified)", "Battery with internal resistance");
    addDevice("DC VOLTAGE SOURCE",  "Primary Sources", "Sources",   "(Unspecified)", "Ideal DC voltage source");
    addDevice("CLOCK GENERATOR",    "Primary Sources", "Sources",   "(Unspecified)", "Square-wave clock / signal generator");

    // ---- 6.2 Passive Parts ----
    addDevice("RESISTOR",           "Passive Components", "Resistors",  "(Unspecified)", "Linear resistor");
    addDevice("CAPACITOR",          "Passive Components", "Capacitors", "(Unspecified)", "Linear capacitor");
    addDevice("INDUCTOR",           "Passive Components", "Inductors",  "(Unspecified)", "Linear inductor");

    // ---- 6.3 Interactive & Simple Output Parts ----
    addDevice("SWITCH",             "Interactive Devices", "Switches", "(Unspecified)", "SPST toggle switch");
    addDevice("PUSH BUTTON",        "Interactive Devices", "Switches", "(Unspecified)", "Momentary push button");
    addDevice("LED",                "Interactive Devices", "Displays", "(Unspecified)", "Light emitting diode");
    addDevice("SEVEN SEGMENT DISPLAY", "Interactive Devices", "Displays", "(Unspecified)", "7-segment display, common cathode");

    // ---- 6.4 Digital Logic Gates & Flip-Flop ----
    addDevice("AND GATE",           "Digital Logic", "Gates",      "(Unspecified)", "2-input AND gate");
    addDevice("OR GATE",            "Digital Logic", "Gates",      "(Unspecified)", "2-input OR gate");
    addDevice("NOT GATE",           "Digital Logic", "Gates",      "(Unspecified)", "Inverter (NOT gate)");
    addDevice("NAND GATE",          "Digital Logic", "Gates",      "(Unspecified)", "2-input NAND gate");
    addDevice("NOR GATE",           "Digital Logic", "Gates",      "(Unspecified)", "2-input NOR gate");
    addDevice("XOR GATE",           "Digital Logic", "Gates",      "(Unspecified)", "2-input XOR gate");
    addDevice("XNOR GATE",          "Digital Logic", "Gates",      "(Unspecified)", "2-input XNOR gate");
    addDevice("D FLIP-FLOP",        "Digital Logic", "Flip-Flops", "(Unspecified)", "D-type flip-flop, rising-edge triggered");
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
    okButton->setEnabled(true);
    updatePreview(name);
    previewWidget->setDevice(name);
}

void PickDevicesDialog::onResultActivated(int row, int)
{
    if (row < 0) return;
    auto *item = resultsTable->item(row, 0);
    if (!item) return;
    m_selectedDevice = item->text();
    const QString category = item->data(Qt::UserRole).toString();
    emit deviceAccepted(m_selectedDevice, category);
    accept();
}

void PickDevicesDialog::updatePreview(const QString &deviceName)
{
    previewNameLabel->setText(deviceName);
    if (deviceName.isEmpty())
        previewWidget->setDevice(QString());
}

void PickDevicesDialog::onOkClicked()
{
    const auto items = resultsTable->selectedItems();
    if (items.isEmpty()) return;
    m_selectedDevice = items.first()->text();
    const QString category = items.first()->data(Qt::UserRole).toString();
    emit deviceAccepted(m_selectedDevice, category);
    accept();
}