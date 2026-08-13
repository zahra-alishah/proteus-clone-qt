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
class PickDevicesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PickDevicesDialog(QWidget *parent = nullptr);

    QString selectedDeviceName() const { return m_selectedDevice; }

signals:
    // Emitted once, right before the dialog closes with Accepted,
    // carrying the name of the device the user picked.
    void deviceAccepted(const QString &deviceName);

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
