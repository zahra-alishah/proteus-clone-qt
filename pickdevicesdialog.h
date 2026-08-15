#ifndef PICKDEVICESDIALOG_H
#define PICKDEVICESDIALOG_H

#include <QDialog>
#include <QVector>
#include <QString>

class QLineEdit;
class QCheckBox;
class QListWidget;
class QListWidgetItem;
class QTableWidget;
class QLabel;
class QPushButton;
class QStackedWidget;

class DevicePreviewWidget; // defined in the .cpp, only used through a pointer here

struct DeviceInfo {
    QString name;
    QString category;
    QString subCategory;
    QString manufacturer;
    QString description;
};

// Mimics Proteus' "Pick Devices" dialog (opened with the 'P' shortcut / the
// "Pick Parts" action). It searches an in-memory device database and lets
// the user filter by keyword, category, sub-category and manufacturer.
//
// IMPORTANT: this database intentionally contains ONLY the components that
// actually exist as classes in Component.h (GND, Battery, DC_vol_source,
// Clock_gen, Resistor, Capacitor, Inductor, Switch, Push_button, LED,
// seven_seg, ANDGate, ORGate, NOTGate, NANDGate, NORGate, XORGate,
// XNORGate, DFlipFlop). Every device name here contains the exact keyword
// that schematicPage::onCanvasClicked() looks for, so picking a device from
// this dialog and clicking on the canvas will instantiate the matching real
// Component subclass. Do not add devices here unless a matching class is
// added to Component.h AND a matching keyword branch is added in
// onCanvasClicked().
class PickDevicesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PickDevicesDialog(QWidget *parent = nullptr);

    QString selectedDeviceName() const { return m_selectedDevice; }

signals:
    // Emitted once, right before the dialog closes with Accepted,
    // carrying the name of the device the user picked and its category.
    void deviceAccepted(const QString &deviceName, const QString &category);

private slots:
    void onKeywordChanged(const QString &text);
    void onCategoryChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onSubCategoryChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onManufacturerChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onResultRowChanged();
    void onResultActivated(int row, int column);
    void onOkClicked();

private:
    void buildDatabase();
    void addDevice(const QString &name, const QString &category, const QString &subCategory,
                   const QString &manufacturer, const QString &description = QString());

    void populateCategoryList();
    void populateSubCategoryList();
    void populateManufacturerList();
    void runSearch();
    void updatePreview(const QString &deviceName);

    // ---- widgets ----
    QLineEdit *keywordEdit = nullptr;
    QCheckBox *matchWholeWordsBox = nullptr;
    QCheckBox *onlyWithModelsBox = nullptr;
    QCheckBox *onlyManagedBox = nullptr;

    QListWidget *categoryList = nullptr;
    QListWidget *subCategoryList = nullptr;
    QListWidget *manufacturerList = nullptr;

    QLabel *resultsHeaderLabel = nullptr;
    QStackedWidget *resultsStack = nullptr;
    QTableWidget *resultsTable = nullptr;

    DevicePreviewWidget *previewWidget = nullptr;
    QLabel *previewNameLabel = nullptr;

    QPushButton *okButton = nullptr;
    QPushButton *cancelButton = nullptr;

    // ---- data ----
    QVector<DeviceInfo> database;
    QString m_selectedDevice;
};

#endif // PICKDEVICESDIALOG_H