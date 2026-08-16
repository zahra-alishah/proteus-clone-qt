#ifndef SCHEMATICPAGE_H
#define SCHEMATICPAGE_H
#include <QWidget>
#include <QListWidgetItem>
#include <QListWidget>
#include <QVector>
#include <QPointF>
#include <QPoint>
#include "component.h"
#include "componenteditdialog.h"
#include "wire.h"
#include "junction.h"
#include <QStringList>
namespace Ui {
class schematicPage;
}
class QListWidget;
class QListWidgetItem;
class QLabel;
class QAction;
class shematicClass;
class Component;
class QTextStream;

class schematicPage : public QWidget
{
    Q_OBJECT
public:
    explicit schematicPage(QWidget *parent = nullptr);
    ~schematicPage();
    bool loadProjectFile(const QString &path);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    Ui::schematicPage *ui;
    QListWidget *devicesListWidget;
    QString selectedComponentType;
    QVector<Component*> componentsOnboard;
    Push_button *pressedPushButton = nullptr;
    shematicClass *schematicCanvas = nullptr;
    QWidget *componentOverlay = nullptr;
    enum class InteractionMode { Idle, RubberBandSelecting, MovingComponents, Wiring };
    InteractionMode interactionMode = InteractionMode::Idle;
    QVector<Component*> selectedComponents;
    QPointF dragAnchorScenePos;
    QPoint rubberBandStartPoint;
    QVector<Position> moveStartPositions;
    int gridSize = 10;
    QVector<Wire*> wiresOnboard;
    QVector<Junction*> junctionsOnboard;
    Wire *selectedWire = nullptr;
    Component *wireStartComponent = nullptr;
    int wireStartPinIndex = -1;
    QVector<Position> wireWaypoints;
    bool wiringPreviewActive = false;
    QVector<Position> wiringPreviewPath;
    Component *hoveredPinComponent = nullptr;
    int hoveredPinIndex = -1;
    QString currentProjectName;
    QString currentProjectPath;
    bool saved = false;

    // ---- Undo / Redo ----
    QAction *actUndo = nullptr;
    QAction *actRedo = nullptr;
    QVector<QString> undoHistory;
    int historyIndex = -1;

    QString projectsDirectory() const;
    void clearCircuit();
    Component* createComponentFromCode(const QString &code, const QStringList &extra);
    void serializeComponent(Component *c, QTextStream &out, int index);

    void writeProjectData(QTextStream &out);
    bool readProjectData(QTextStream &in);
    bool writeProjectToFile(const QString &path);
    bool loadProjectFromFile(const QString &path);

    QString serializeCurrentStateToString();
    bool restoreStateFromString(const QString &data);
    void pushUndoState();
    void resetUndoHistory();
    void updateUndoRedoActions();

    void updatePinHover(const QPointF &scenePos);
    void recalcAllWires();
    void pruneJunctions();
    Wire* findWireNear(const QPointF &scenePos) const;
    bool findWireIntersectionNear(const QPointF &scenePos, Position &outPoint) const;
    bool findPinAt(const QPointF &scenePos, Component *&outComp, int &outPinIdx) const;
    void startWiring(Component *comp, int pinIdx);
    void addWireWaypoint(const Position &pt);
    void finishWiringAtPin(Component *targetComp, int targetPinIdx);
    void finishWiringDangling(const Position &pt);
    void cancelWiring();
    QVector<Position> buildWiringPreviewPath(const Position &mousePos) const;
    void openEditDialogFor(Component *comp);
    Component* findComponentAt(const QPointF &scenePos) const;
    void clearSelection();
    void selectComponent(Component *c);
    void deselectComponent(Component *c);

private slots:
    void onPickDevices();
    void onDeviceListItemClicked(QListWidgetItem *item);
    void onCanvasClicked(QPointF scenePos, Qt::KeyboardModifiers modifiers);
    void onCanvasDragMoved(QPointF scenePos, bool leftButtonDown);
    void onCanvasReleased(QPointF scenePos);
    void onComponentDoubleClicked(QPointF pos);
    void onDeleteSelected();
    void onCanvasContextMenu(const QPoint &viewPos);
    void saveProject();
    void saveProjectAs();
    void openProject();
    void takeScreenshot();
    void undoAction();
    void redoAction();

};
#endif // SCHEMATICPAGE_H