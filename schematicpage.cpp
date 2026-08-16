#include "schematicpage.h"
#include <QButtonGroup>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMenu>
#include <QMouseEvent>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include "shematicclass.h"
#include "ui_schematicpage.h"
#include "pickdevicesdialog.h"
#include "component.h"
#include <QPainter>
#include "componenteditdialog.h"
#include <QtMath>
#include "wire.h"
#include "junction.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QIcon>
#include <QGraphicsScene>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>

namespace {

bool intersectOrthogonal(const Position &a1, const Position &a2,
                         const Position &b1, const Position &b2,
                         Position &out)
{
    bool aHoriz = (a1.y == a2.y);
    bool bHoriz = (b1.y == b2.y);
    if (aHoriz == bHoriz) return false;

    const Position &h1 = aHoriz ? a1 : b1;
    const Position &h2 = aHoriz ? a2 : b2;
    const Position &v1 = aHoriz ? b1 : a1;
    const Position &v2 = aHoriz ? b2 : a2;

    int hy = h1.y;
    int vx = v1.x;
    int hxMin = std::min(h1.x, h2.x), hxMax = std::max(h1.x, h2.x);
    int vyMin = std::min(v1.y, v2.y), vyMax = std::max(v1.y, v2.y);

    if (vx >= hxMin && vx <= hxMax && hy >= vyMin && hy <= vyMax) {
        out = Position(vx, hy);
        return true;
    }
    return false;
}

class ComponentOverlay : public QWidget
{
public:
    explicit ComponentOverlay(QVector<Component*> *components,
                              QVector<Wire*> *wires,
                              QVector<Junction*> *junctions,
                              bool *previewActive,
                              QVector<Position> *previewPath,
                              shematicClass *view,
                              bool *probeActive,
                              QString *probeText,
                              Position *probePos,
                              QWidget *parent = nullptr)
        : QWidget(parent), m_components(components), m_wires(wires), m_junctions(junctions),
        m_previewActive(previewActive), m_previewPath(previewPath), m_view(view),
        m_probeActive(probeActive), m_probeText(probeText), m_probePos(probePos)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAutoFillBackground(false);
    }

    void setRubberBandRect(const QRect &r) { m_rubberBandRect = r; update(); }
    void clearRubberBandRect() { m_rubberBandRect = QRect(); update(); }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        if (m_view) {
            painter.setTransform(m_view->viewportTransform());
        }

        if (m_wires) {
            for (Wire *w : *m_wires) {
                if (!w) continue;
                w->recalculateRoute();
                w->draw(&painter);
            }
        }

        if (m_components) {
            for (Component *comp : *m_components) {
                if (!comp) continue;
                comp->draw(&painter);

                if (comp->isSelected()) {
                    painter.save();
                    QPen pen(QColor("#2f6fdb"), 2, Qt::DashLine);
                    painter.setPen(pen);
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRect(comp->boundingRect());
                    painter.restore();
                }
            }
        }


        if (m_junctions) {
            for (Junction *j : *m_junctions) {
                if (j) j->draw(&painter);
            }
        }


        if (m_previewActive && *m_previewActive && m_previewPath && m_previewPath->size() >= 2) {
            painter.save();
            QPen pen(QColor("#2f6fdb"), 2, Qt::DashLine);
            painter.setPen(pen);
            const QVector<Position> &path = *m_previewPath;
            for (int i = 0; i < path.size() - 1; ++i) {
                painter.drawLine(QPoint(path[i].x, path[i].y),
                                 QPoint(path[i + 1].x, path[i + 1].y));
            }
            painter.restore();
        }

        if (!m_rubberBandRect.isNull()) {
            painter.save();
            QPen pen(QColor("#4a90e2"), 1, Qt::DashLine);
            painter.setPen(pen);
            painter.setBrush(QColor(74, 144, 226, 40));
            painter.drawRect(m_rubberBandRect.normalized());
            painter.restore();
        }

        if (m_probeActive && *m_probeActive && m_view && m_probeText && m_probePos) {
            painter.save();
            painter.resetTransform();
            QPoint screenPt = m_view->mapFromScene(QPointF(m_probePos->x, m_probePos->y));
            QRect labelRect(screenPt.x() + 8, screenPt.y() - 22, 74, 20);
            painter.setBrush(QColor(255, 255, 180));
            painter.setPen(QPen(Qt::black, 1));
            painter.drawRect(labelRect);
            painter.drawText(labelRect, Qt::AlignCenter, *m_probeText);
            painter.restore();
        }
    }

private:
    QVector<Component*> *m_components;
    QVector<Wire*> *m_wires;
    QVector<Junction*> *m_junctions;
    bool *m_previewActive;
    QVector<Position> *m_previewPath;
    shematicClass *m_view = nullptr;
    QRect m_rubberBandRect;
    bool *m_probeActive;
    QString *m_probeText;
    Position *m_probePos;
};

QString chooseProjectDialog(QWidget *parent, const QString &dir)
{
    QDir d(dir);
    QStringList files = d.entryList(QStringList() << "*.txt", QDir::Files, QDir::Time);

    QDialog dlg(parent);
    dlg.setWindowTitle("Open Project");
    dlg.resize(320, 400);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QListWidget *list = new QListWidget(&dlg);
    for (const QString &fileName : files) {
        list->addItem(QFileInfo(fileName).completeBaseName());
    }
    layout->addWidget(list);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    QObject::connect(list, &QListWidget::itemDoubleClicked, &dlg, &QDialog::accept);

    if (dlg.exec() != QDialog::Accepted) return QString();
    QListWidgetItem *item = list->currentItem();
    if (!item) return QString();

    return dir + "/" + item->text() + ".txt";
}

}

schematicPage::schematicPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::schematicPage)
{
    ui->setupUi(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QMenuBar *myMenuBar = new QMenuBar(this);
    mainLayout->addWidget(myMenuBar);

    QMenu *fileMenu = myMenuBar->addMenu("&File");
    QMenu *editMenu = myMenuBar->addMenu("&Edit");
    QMenu *viewMenu = myMenuBar->addMenu("&View");
    QMenu *toolMenu = myMenuBar->addMenu("&Tool");
    QMenu *designMenu = myMenuBar->addMenu("&Design");
    QMenu *graphMenu = myMenuBar->addMenu("&Graph");
    QMenu *debugMenu = myMenuBar->addMenu("&Debug");
    QMenu *libraryMenu = myMenuBar->addMenu("&Library");
    QMenu *templateMenu = myMenuBar->addMenu("&Template");
    QMenu *systemMenu = myMenuBar->addMenu("&System");
    QMenu *helpMenu = myMenuBar->addMenu("&Help");

    QAction *actNew = fileMenu->addAction("&New Project");
    QAction *actOpen = fileMenu->addAction("&Open Project");
    QAction *actSave = fileMenu->addAction("&Save Project");

    fileMenu->addAction("Open Sa&mple Project");
    fileMenu->addAction("Import &Legacy Project");
    fileMenu->addAction("Import &ECAD Files");
    QAction *actSaveAs = fileMenu->addAction("Save Project &As");
    QAction *actClose = editMenu->addAction("Close Project");
    fileMenu->addSeparator();
    fileMenu->addAction("&Import Image");
    fileMenu->addAction("Import Project &Clip");
    fileMenu->addSeparator();

    QMenu *exportMenu = fileMenu->addMenu("Export &Graphics");
    Q_UNUSED(exportMenu);

    QAction *actExportClip = fileMenu->addAction("Export Project Clip");
    actExportClip->setEnabled(false);
    fileMenu->addSeparator();

    fileMenu->addAction("&Print Design");
    fileMenu->addAction("Print Set&up");
    fileMenu->addAction("Printer &Information");
    fileMenu->addAction("Mark &Output Area");
    fileMenu->addSeparator();
    fileMenu->addAction("E&xplore Project Folder");
    fileMenu->addAction("Edit Project &Description");
    fileMenu->addSeparator();
    QAction *actExit = fileMenu->addAction("E&xit Application");
    fileMenu->addSeparator();
    fileMenu->addAction("1 C:\\Mac\\Home\\Documents\\New Project");
    QAction *actScreenshot = fileMenu->addAction("Save &Screenshot");
    actScreenshot->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

    actNew->setShortcut(QKeySequence(Qt::ALT | Qt::Key_N));
    actOpen->setShortcut(QKeySequence(Qt::ALT | Qt::Key_O));
    actSave->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    actExit->setShortcut(QKeySequence(Qt::ALT | Qt::Key_F4));

    actUndo = editMenu->addAction("Undo Changes");
    actUndo->setEnabled(false);

    actRedo = editMenu->addAction("Redo Changes");
    actRedo->setEnabled(false);

    editMenu->addSeparator();
    editMenu->addAction("Find/Edit Component E");
    editMenu->addAction("Select All Objects");
    editMenu->addAction("Clear Selection");

    QAction *actCut = editMenu->addAction("Cut To Clipboard");
    QAction *actCopy = editMenu->addAction("Copy To Clipboard");
    QAction *actPaste = editMenu->addAction("Paste From Clipboard");
    editMenu->addSeparator();

    QAction *actAlign = editMenu->addAction("Align Objects");
    QAction *actSendBack = editMenu->addAction("Send To Back");
    QAction *actBringFront = editMenu->addAction("Bring To Front");
    editMenu->addSeparator();

    editMenu->addAction("Tidy Design");

    actUndo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    actRedo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    actAlign->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
    actSendBack->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    actBringFront->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));

    QAction *actRedraw = viewMenu->addAction("Redraw Display");
    QAction *actGrid = viewMenu->addAction("Toggle Grid");
    QAction *actOrigin = viewMenu->addAction("Toggle False Origin");
    QAction *actCursor = viewMenu->addAction("Toggle X-Cursor");
    viewMenu->addSeparator();
    QAction *actSnap10 = viewMenu->addAction("Snap 10th");
    QAction *actSnap50 = viewMenu->addAction("Snap 50th");
    QAction *actSnap01 = viewMenu->addAction("Snap 0.1in");
    QAction *actSnap05 = viewMenu->addAction("Snap 0.5in");
    viewMenu->addSeparator();
    QAction *actCenter = viewMenu->addAction("Center At Cursor");
    QAction *actZoomIn = viewMenu->addAction("Zoom In");
    QAction *actZoomOut = viewMenu->addAction("Zoom Out");
    QAction *actZoomAll = viewMenu->addAction("Zoom To View Entire Sheet");
    QAction *actZoomArea = viewMenu->addAction("Zoom To Area");

    viewMenu->addSeparator();
    viewMenu->addAction("Toolbar Configuration");

    actRedraw->setShortcut(QKeySequence(Qt::Key_R));
    actGrid->setShortcut(QKeySequence(Qt::Key_G));
    actOrigin->setShortcut(QKeySequence(Qt::Key_O));
    actCursor->setShortcut(QKeySequence(Qt::Key_X));
    actSnap10->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F1));
    actSnap50->setShortcut(QKeySequence(Qt::Key_F2));
    actSnap01->setShortcut(QKeySequence(Qt::Key_F3));
    actSnap05->setShortcut(QKeySequence(Qt::Key_F4));
    actCenter->setShortcut(QKeySequence(Qt::Key_F5));
    actZoomIn->setShortcut(QKeySequence(Qt::Key_F6));
    actZoomOut->setShortcut(QKeySequence(Qt::Key_F7));
    actZoomAll->setShortcut(QKeySequence(Qt::Key_F8));

    QAction *actWire = toolMenu->addAction("Wire Autorouter");
    QAction *actSearch = toolMenu->addAction("Search & Tag");
    QAction *actProp = toolMenu->addAction("Property Assignment Tool");
    toolMenu->addSeparator();

    toolMenu->addAction("Global Annotator");
    toolMenu->addAction("ASCII Data Import Tool");
    toolMenu->addAction("Electrical Rules Check");
    toolMenu->addAction("Netlist Compiler");
    toolMenu->addAction("Model Compiler");

    actWire->setShortcut(QKeySequence(Qt::Key_W));
    actSearch->setShortcut(QKeySequence(Qt::Key_T));
    actProp->setShortcut(QKeySequence(Qt::Key_A));

    designMenu->addAction("Edit Design Properties");
    designMenu->addAction("Edit Sheet Properties");
    designMenu->addAction("Edit Design Notes");
    designMenu->addSeparator();
    designMenu->addAction("Configure Power Rails...");
    designMenu->addSeparator();

    QAction *actNewSheet = designMenu->addAction("New Sheet");
    QAction *actRemove = designMenu->addAction("Remove/Delete Sheet");
    QAction *actPrevSheet = designMenu->addAction("Goto Previous Sheet");
    QAction *actNextSheet = designMenu->addAction("Goto Next Sheet");
    QAction *actExitParent = designMenu->addAction("Exit to Parent Sheet");

    designMenu->addAction("Goto Sheet");

    designMenu->addSeparator();
    QAction *actRootSheet = designMenu->addAction("Root sheet 1");

    actPrevSheet->setShortcut(QKeySequence(Qt::Key_PageUp));
    actNextSheet->setShortcut(QKeySequence(Qt::Key_PageDown));
    actExitParent->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));

    actExitParent->setEnabled(false);
    actRootSheet->setCheckable(true);
    actRootSheet->setChecked(true);

    QAction *actEdit = graphMenu->addAction("Edit Graph...");
    QAction *actTraces = graphMenu->addAction("Add Traces...");
    QAction *actSim = graphMenu->addAction("Simulate Graph");
    QAction *actLog = graphMenu->addAction("View Simulation Log");

    graphMenu->addAction("Export Graph Data...");
    graphMenu->addAction("Clear Graph Data...");
    graphMenu->addSeparator();

    QAction *actAudio = graphMenu->addAction("Play Audio");
    graphMenu->addSeparator();

    graphMenu->addAction("Verify Graphs");
    graphMenu->addAction("Verify Files");

    actEdit->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    actTraces->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    actSim->setShortcut(QKeySequence(Qt::Key_Space));
    actLog->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));
    actAudio->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Space));

    QAction *actStart = debugMenu->addAction("Start VSM Debugging");
    QAction *actPause = debugMenu->addAction("Pause VSM Debugging");
    QAction *actStop = debugMenu->addAction("Stop VSM Debugging");
    debugMenu->addSeparator();
    QAction *actRun = debugMenu->addAction("Run Simulation");
    QAction *actRunNB = debugMenu->addAction("Run Simulation (no breakpoints)");

    debugMenu->addAction("Run Simulation (timed breakpoint)");
    debugMenu->addSeparator();

    QAction *actStepOver = debugMenu->addAction("Step Over Source Line");
    QAction *actStepInto = debugMenu->addAction("Step Into Source Line");
    QAction *actStepOut = debugMenu->addAction("Step Out from Source Line");
    QAction *actRunTo = debugMenu->addAction("Run To Source Line");
    QAction *actAnim = debugMenu->addAction("Animated Single Step");
    debugMenu->addSeparator();

    debugMenu->addAction("Reset Debug Popup Windows");
    debugMenu->addAction("Reset Persistent Model Data");
    debugMenu->addAction("Configure Diagnostics");
    debugMenu->addAction("Enable Remote Debug Monitor");
    debugMenu->addSeparator();
    debugMenu->addAction("Horz. Tile Popup Windows");
    debugMenu->addAction("Vertical Tile Popup Windows");

    actStart->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F12));
    actPause->setShortcut(QKeySequence(Qt::Key_Pause));
    actStop->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Pause));
    actRun->setShortcut(QKeySequence(Qt::Key_F12));
    actRunNB->setShortcut(QKeySequence(Qt::ALT | Qt::Key_F12));
    actStepOver->setShortcut(QKeySequence(Qt::Key_F10));
    actStepInto->setShortcut(QKeySequence(Qt::Key_F11));
    actStepOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F11));
    actRunTo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F10));
    actAnim->setShortcut(QKeySequence(Qt::ALT | Qt::Key_F11));

    QAction *actPick = libraryMenu->addAction("Pick Parts");

    libraryMenu->addAction("Import Parts");
    libraryMenu->addSeparator();

    QAction *actMake = libraryMenu->addAction("Make Device");

    libraryMenu->addAction("Make Symbol");

    QAction *actPackaging = libraryMenu->addAction("Packaging Tool");
    QAction *actDecompose = libraryMenu->addAction("Decompose");

    libraryMenu->addSeparator();
    libraryMenu->addAction("Compile To Library");
    libraryMenu->addAction("Place Library");
    libraryMenu->addAction("Verify Packagings");
    libraryMenu->addAction("Manage Changes");
    libraryMenu->addSeparator();
    libraryMenu->addAction("Library Manager");

    actPick->setShortcut(QKeySequence(Qt::Key_P));
    connect(actPick, &QAction::triggered, this, &schematicPage::onPickDevices);
    connect(actSave, &QAction::triggered, this, &schematicPage::saveProject);
    connect(actSaveAs, &QAction::triggered, this, &schematicPage::saveProjectAs);
    connect(actOpen, &QAction::triggered, this, &schematicPage::openProject);
    connect(actScreenshot, &QAction::triggered, this, &schematicPage::takeScreenshot);

    connect(actNew, &QAction::triggered, this, [this]() {
        clearCircuit();
        currentProjectName.clear();
        currentProjectPath.clear();
        saved = false;
    });

    templateMenu->addAction("Goto Master Sheet");
    templateMenu->addAction("Set Design Colours");
    templateMenu->addAction("Set Graph & Trace Colours");
    templateMenu->addAction("Set Graphic Styles");
    templateMenu->addAction("Set Text Styles");
    templateMenu->addAction("Set 2D Graphics Defaults");
    templateMenu->addAction("Set Junction Dot Style");
    templateMenu->addSeparator();
    templateMenu->addAction("Apply Styles From Template");
    templateMenu->addAction("Save Design as Template");

    systemMenu->addAction("System Settings");
    systemMenu->addAction("Text Viewer");
    systemMenu->addAction("Set Display Options");
    systemMenu->addAction("Set Keyboard Mapping");
    systemMenu->addAction("Set Property Definitions");
    systemMenu->addAction("Set Sheet Sizes");
    systemMenu->addAction("Set Text Editor");
    systemMenu->addSeparator();
    systemMenu->addAction("Set Animation Options");
    systemMenu->addAction("Set Simulation Options");
    systemMenu->addSeparator();
    systemMenu->addAction("Restore Default Settings");

    QAction *actOverview = helpMenu->addAction("&Overview");

    helpMenu->addAction("About &Proteus 8");
    helpMenu->addAction("About &Qt");
    helpMenu->addSeparator();

    QAction *actHelp = helpMenu->addAction("&Schematic Capture Help");

    helpMenu->addAction("Schematic Capture &Tutorial");
    helpMenu->addAction("&Simulation Help");
    helpMenu->addAction("VSM Model/&SDK Help");

    actOverview->setShortcut(QKeySequence(Qt::Key_F1));
    actHelp->setShortcut(QKeySequence(Qt::Key_F1));

    QToolBar *myToolBar = new QToolBar(this);
    myToolBar->setIconSize(QSize(20, 20));
    myToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    QAction *actHome = editMenu->addAction("Home Page");
    QAction *actSchematic = editMenu->addAction("Schematic Capture");
    QAction *actPCB = editMenu->addAction("PCB Layout");
    QAction *act3D = editMenu->addAction("3D Visualizer");
    QAction *actGerber = editMenu->addAction("Gerber Viewer");
    QAction *actDesign = editMenu->addAction("Design Expolerer");
    QAction *actBill = editMenu->addAction("Bill of Materials");
    QAction *actCode = editMenu->addAction("SOURCE Code");
    QAction *actNotes = editMenu->addAction("Project Notes");
    QAction *actBlockCopy = editMenu->addAction("Block Copy");
    QAction *actBlockMove = editMenu->addAction("Block Move");
    QAction *actBlockDelete = editMenu->addAction("Block Delete");
    QAction *actBlockRotate = editMenu->addAction("Project Rotate");
    QAction *actRulesCheck = editMenu->addAction("Electrical Rules Check");

    myToolBar->addAction(actNew);
    myToolBar->addAction(actOpen);
    myToolBar->addAction(actSave);
    myToolBar->addAction(actClose);
    myToolBar->addSeparator();
    myToolBar->addAction(actHome);
    myToolBar->addAction(actSchematic);
    myToolBar->addAction(actPCB);
    myToolBar->addAction(act3D);
    myToolBar->addAction(actGerber);
    myToolBar->addAction(actDesign);
    myToolBar->addAction(actBill);
    myToolBar->addAction(actCode);
    myToolBar->addAction(actNotes);
    myToolBar->addSeparator();
    myToolBar->addAction(actOverview);

    actNew->setIcon(QIcon(":/icons/new.png"));
    actOpen->setIcon(QIcon(":/icons/open-folder.png"));
    actSave->setIcon(QIcon(":/icons/personal-data.png"));
    actClose->setIcon(QIcon(":/icons/room.png"));
    actHome->setIcon(QIcon(":/icons/home.png"));
    actSchematic->setIcon(QIcon(":/icons/structure.png"));
    actPCB->setIcon(QIcon(":/icons/schematic.png"));
    act3D->setIcon(QIcon(":/icons/3D.png"));
    actGerber->setIcon(QIcon(":/icons/gerber.png"));
    actDesign->setIcon(QIcon(":/icons/explorer.png"));
    actBill->setIcon(QIcon(":/icons/Bill.png"));
    actCode->setIcon(QIcon(":/icons/code.png"));
    actNotes->setIcon(QIcon(":/icons/notes.png"));
    actOverview->setIcon(QIcon(":/icons/overview.png"));

    myToolBar->addSeparator();
    QComboBox *comboBaseDesign = new QComboBox(this);
    comboBaseDesign->addItem("Base Design");
    myToolBar->addWidget(comboBaseDesign);

    myToolBar->addSeparator();
    QComboBox *comboRoot = new QComboBox(this);
    comboRoot->addItem("ROOT");
    myToolBar->addWidget(comboRoot);
    myToolBar->addSeparator();

    myToolBar->addAction(actRedraw);
    myToolBar->addAction(actGrid);
    myToolBar->addAction(actOrigin);
    myToolBar->addSeparator();
    myToolBar->addAction(actCenter);
    myToolBar->addAction(actZoomIn);
    myToolBar->addAction(actZoomOut);
    myToolBar->addAction(actZoomAll);
    myToolBar->addAction(actZoomArea);
    myToolBar->addSeparator();
    myToolBar->addAction(actUndo);
    myToolBar->addAction(actRedo);
    myToolBar->addSeparator();
    myToolBar->addAction(actCut);
    myToolBar->addAction(actCopy);
    myToolBar->addAction(actPaste);
    myToolBar->addSeparator();
    myToolBar->addAction(actBlockCopy);
    myToolBar->addAction(actBlockMove);
    myToolBar->addAction(actBlockRotate);
    myToolBar->addAction(actBlockDelete);
    myToolBar->addSeparator();
    myToolBar->addAction(actPick);
    myToolBar->addAction(actMake);
    myToolBar->addAction(actPackaging);
    myToolBar->addAction(actDecompose);
    myToolBar->addSeparator();
    myToolBar->addAction(actWire);
    myToolBar->addSeparator();
    myToolBar->addAction(actSearch);
    myToolBar->addAction(actProp);
    myToolBar->addSeparator();
    myToolBar->addAction(actNewSheet);
    myToolBar->addAction(actRemove);
    myToolBar->addAction(actExitParent);
    myToolBar->addSeparator();
    myToolBar->addAction(actRulesCheck);

    actRedraw->setIcon(QIcon(":/icons/redraw.png"));
    actGrid->setIcon(QIcon(":/icons/toggle.png"));
    actOrigin->setIcon(QIcon(":/icons/toggleOff.png"));
    actCenter->setIcon(QIcon(":/icons/center.png"));
    actZoomIn->setIcon(QIcon(":/icons/zoomIn.png"));
    actZoomOut->setIcon(QIcon(":/icons/zoomOut.png"));
    actZoomAll->setIcon(QIcon(":/icons/zoomToEntire.png"));
    actZoomArea->setIcon(QIcon(":/icons/zoomToArea.png"));
    actUndo->setIcon(QIcon(":/icons/undo.png"));
    actRedo->setIcon(QIcon(":/icons/redo.png"));
    actCut->setIcon(QIcon(":/icons/cut.png"));
    actCopy->setIcon(QIcon(":/icons/copy.png"));
    actPaste->setIcon(QIcon(":/icons/paste.png"));
    actBlockCopy->setIcon(QIcon(":/icons/personal-data.png"));
    actBlockMove->setIcon(QIcon(":/icons/blockMove.png"));
    actBlockRotate->setIcon(QIcon(":/icons/open-folder.png"));
    actBlockDelete->setIcon(QIcon(":/icons/blockDelete.png"));
    actPick->setIcon(QIcon(":/icons/pick.png"));
    actMake->setIcon(QIcon(":/icons/make.png"));
    actPackaging->setIcon(QIcon(":/icons/package.png"));
    actDecompose->setIcon(QIcon(":/icons/decompose.png"));
    actWire->setIcon(QIcon(":/icons/wire.png"));
    actSearch->setIcon(QIcon(":/icons/search.png"));
    actProp->setIcon(QIcon(":/icons/assign.png"));
    actNewSheet->setIcon(QIcon(":/icons/newSheet.png"));
    actRemove->setIcon(QIcon(":/icons/deleteSheet.png"));
    actExitParent->setIcon(QIcon(":/icons/parents.png"));
    actRulesCheck->setIcon(QIcon(":/icons/check.png"));

    connect(actGrid, &QAction::triggered, this, [=]{
        static int gridState = GridLines;
        gridState = (gridState + 1) % 3;
        schematicCanvas->setGridMode(gridState);
    });

    schematicCanvas = new shematicClass();
    connect(actZoomIn, &QAction::triggered, this, [this]() {
        schematicCanvas->zoomIn();
        if (componentOverlay) componentOverlay->update();
    });
    connect(actZoomOut, &QAction::triggered, this, [this]() {
        schematicCanvas->zoomOut();
        if (componentOverlay) componentOverlay->update();
    });
    connect(actCenter, &QAction::triggered, this, [this]() {
        schematicCanvas->centerOnPage();
        if (componentOverlay) componentOverlay->update();
    });

    connect(schematicCanvas->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        if (componentOverlay) componentOverlay->update();
    });
    connect(schematicCanvas->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        if (componentOverlay) componentOverlay->update();
    });

    connect(schematicCanvas, &shematicClass::escapePressed, this, [=](){
        qDebug() << "Escape pressed! Clearing selection...";
    });

    connect(schematicCanvas, &shematicClass::deleteKeyPressed, this, [=](){
        qDebug() << "Delete pressed! Removing selected item...";
    });

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(0);

    QWidget *sidebar = new QWidget();
    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(2, 2, 2, 2);
    sideLayout->setSpacing(0);

    QToolButton *btnSelection = new QToolButton;
    QToolButton *btnComponent = new QToolButton;
    QToolButton *btnJunctionDot = new QToolButton;
    QToolButton *btnWireLabel = new QToolButton;
    QToolButton *btnTextScript = new QToolButton;
    QToolButton *btnBuses = new QToolButton;
    QToolButton *btnSubCircuit = new QToolButton;
    QToolButton *btnTerminals = new QToolButton;
    QToolButton *btnDevicePins = new QToolButton;
    QToolButton *btnGraph = new QToolButton;
    QToolButton *btnActivePopUp = new QToolButton;
    QToolButton *btnGenerator = new QToolButton;
    QToolButton *btnProbMode = new QToolButton;
    QToolButton *btnVirtualInstruments = new QToolButton;
    QToolButton *btn2DGraphicsLine = new QToolButton;
    QToolButton *btn2DGraphicsBox = new QToolButton;
    QToolButton *btn2DGraphicsCircle = new QToolButton;
    QToolButton *btn2DGraphicsArc = new QToolButton;
    QToolButton *btn2DGraphicsClosedPath = new QToolButton;
    QToolButton *btn2DGraphicsText = new QToolButton;
    QToolButton *btn2DGraphicsSymbols = new QToolButton;
    QToolButton *btn2DGraphicsMarkers = new QToolButton;
    QToolButton *btnRotateClockwise = new QToolButton;
    QToolButton *btnRotateAntiClockwise = new QToolButton;
    QToolButton *btnXMirror = new QToolButton;
    QToolButton *btnYMirror = new QToolButton;

    QSpinBox *rotationSpin = new QSpinBox(this);

    btnSelection->setChecked(true);
    btnComponent->setChecked(false);
    btnJunctionDot->setChecked(false);
    btnWireLabel->setChecked(false);
    btnTextScript->setChecked(false);
    btnBuses->setChecked(false);
    btnSubCircuit->setChecked(false);
    btnTerminals->setChecked(false);
    btnDevicePins->setChecked(false);
    btnGraph->setChecked(false);
    btnActivePopUp->setChecked(false);
    btnGenerator->setChecked(false);
    btnProbMode->setChecked(false);
    btnVirtualInstruments->setChecked(false);
    btn2DGraphicsLine->setChecked(false);
    btn2DGraphicsBox->setChecked(false);
    btn2DGraphicsCircle->setChecked(false);
    btn2DGraphicsArc->setChecked(false);
    btn2DGraphicsClosedPath->setChecked(false);
    btn2DGraphicsText->setChecked(false);
    btn2DGraphicsSymbols->setChecked(false);
    btn2DGraphicsMarkers->setChecked(false);
    btnRotateClockwise->setChecked(false);
    btnRotateAntiClockwise->setChecked(false);
    btnXMirror->setChecked(false);
    btnYMirror->setChecked(false);

    btnSelection->setCheckable(true);
    btnComponent->setCheckable(true);
    btnJunctionDot->setCheckable(true);
    btnWireLabel->setCheckable(true);
    btnTextScript->setCheckable(true);
    btnBuses->setCheckable(true);
    btnSubCircuit->setCheckable(true);
    btnTerminals->setCheckable(true);
    btnDevicePins->setCheckable(true);
    btnGraph->setCheckable(true);
    btnActivePopUp->setCheckable(true);
    btnGenerator->setCheckable(true);
    btnProbMode->setCheckable(true);
    btnVirtualInstruments->setCheckable(true);
    btn2DGraphicsLine->setCheckable(true);
    btn2DGraphicsBox->setCheckable(true);
    btn2DGraphicsCircle->setCheckable(true);
    btn2DGraphicsArc->setCheckable(true);
    btn2DGraphicsClosedPath->setCheckable(true);
    btn2DGraphicsText->setCheckable(true);
    btn2DGraphicsSymbols->setCheckable(true);
    btn2DGraphicsMarkers->setCheckable(true);
    btnRotateClockwise->setCheckable(true);
    btnRotateAntiClockwise->setCheckable(true);
    btnXMirror->setCheckable(true);
    btnYMirror->setCheckable(true);

    QButtonGroup *toolGroup = new QButtonGroup(this);
    toolGroup->addButton(btnSelection);
    toolGroup->addButton(btnComponent);
    toolGroup->addButton(btnJunctionDot);
    toolGroup->addButton(btnWireLabel);
    toolGroup->addButton(btnTextScript);
    toolGroup->addButton(btnBuses);
    toolGroup->addButton(btnSubCircuit);
    toolGroup->addButton(btnTerminals);
    toolGroup->addButton(btnDevicePins);
    toolGroup->addButton(btnGraph);
    toolGroup->addButton(btnActivePopUp);
    toolGroup->addButton(btnGenerator);
    toolGroup->addButton(btnProbMode);
    toolGroup->addButton(btnVirtualInstruments);
    toolGroup->addButton(btn2DGraphicsLine);
    toolGroup->addButton(btn2DGraphicsBox);
    toolGroup->addButton(btn2DGraphicsCircle);
    toolGroup->addButton(btn2DGraphicsArc);
    toolGroup->addButton(btn2DGraphicsClosedPath);
    toolGroup->addButton(btn2DGraphicsText);
    toolGroup->addButton(btn2DGraphicsSymbols);
    toolGroup->addButton(btn2DGraphicsMarkers);
    toolGroup->addButton(btnRotateClockwise);
    toolGroup->addButton(btnRotateAntiClockwise);
    toolGroup->addButton(btnXMirror);
    toolGroup->addButton(btnYMirror);

    QStackedWidget *modeStack = new QStackedWidget();
    sideLayout->addWidget(modeStack);
    modeStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    sideLayout->addWidget(btnSelection);
    sideLayout->addWidget(btnComponent);
    sideLayout->addWidget(btnJunctionDot);
    sideLayout->addWidget(btnWireLabel);
    sideLayout->addWidget(btnTextScript);
    sideLayout->addWidget(btnBuses);
    sideLayout->addWidget(btnSubCircuit);
    sideLayout->addWidget(btnTerminals);
    sideLayout->addWidget(btnDevicePins);
    sideLayout->addWidget(btnGraph);
    sideLayout->addWidget(btnActivePopUp);
    sideLayout->addWidget(btnGenerator);
    sideLayout->addWidget(btnProbMode);
    sideLayout->addWidget(btnVirtualInstruments);
    sideLayout->addWidget(btn2DGraphicsLine);
    sideLayout->addWidget(btn2DGraphicsBox);
    sideLayout->addWidget(btn2DGraphicsCircle);
    sideLayout->addWidget(btn2DGraphicsArc);
    sideLayout->addWidget(btn2DGraphicsClosedPath);
    sideLayout->addWidget(btn2DGraphicsText);
    sideLayout->addWidget(btn2DGraphicsSymbols);
    sideLayout->addWidget(btn2DGraphicsMarkers);
    sideLayout->addWidget(btnRotateClockwise);
    sideLayout->addWidget(btnRotateAntiClockwise);

    rotationSpin->setRange(0, 359);
    rotationSpin->setSuffix("°");
    rotationSpin->setValue(0);

    btnSelection->setToolTip("Selection Mode");
    btnComponent->setToolTip("Component Mode");
    btnJunctionDot->setToolTip("Junction Dot Mode");
    btnWireLabel->setToolTip("Wire Label Mode");
    btnTextScript->setToolTip("Text Script Mode");
    btnBuses->setToolTip("Buses Mode");
    btnSubCircuit->setToolTip("Subcurcuit Mode");
    btnTerminals->setToolTip("Terminal Mode");
    btnDevicePins->setToolTip("Device Pins Mode");
    btnGraph->setToolTip("Graph Mode");
    btnActivePopUp->setToolTip("Active Popup Mode");
    btnGenerator->setToolTip("Generator Mode");
    btnProbMode->setToolTip("Probe Mode");
    btnVirtualInstruments->setToolTip("Virtual Instruments Mode");
    btn2DGraphicsLine->setToolTip("2D Graphics Line Mode");
    btn2DGraphicsBox->setToolTip("2D Graphics Box Mode");
    btn2DGraphicsCircle->setToolTip("2D Graphics Circle Mode");
    btn2DGraphicsArc->setToolTip("2D Graphics Arc Mode");
    btn2DGraphicsClosedPath->setToolTip("2D Graphics Closed Path Mode");
    btn2DGraphicsText->setToolTip("2D Graphics Text Mode");
    btn2DGraphicsSymbols->setToolTip("2D Graphics Symbols Mode");
    btn2DGraphicsMarkers->setToolTip("2D Graphics Markers Mode");
    btnRotateClockwise->setToolTip("Rotate Clockwise");
    btnRotateAntiClockwise->setToolTip("Rotate Anti-Clockwise");
    btnXMirror->setToolTip("Mirror Horizontal");
    btnYMirror->setToolTip("Mirror Vertical");

    btnSelection->setIcon(QIcon(":/icons/mouse.png"));
    btnComponent->setIcon(QIcon(":/icons/symbol.png"));
    btnJunctionDot->setIcon(QIcon(":/icons/junctiondot.png"));
    btnWireLabel->setIcon(QIcon(":/icons/wirelabel.png"));
    btnTextScript->setIcon(QIcon(":/icons/textscript.png"));
    btnBuses->setIcon(QIcon(":/icons/buses.png"));
    btnSubCircuit->setIcon(QIcon(":/icons/subcircuit.png"));
    btnTerminals->setIcon(QIcon(":/icons/terminals.png"));
    btnDevicePins->setIcon(QIcon(":/icons/devicepins.png"));
    btnGraph->setIcon(QIcon(":/icons/graph.png"));
    btnActivePopUp->setIcon(QIcon(":/icons/activepopup.png"));
    btnGenerator->setIcon(QIcon(":/icons/generator.png"));
    btnProbMode->setIcon(QIcon(":/icons/probemode.png"));
    btnVirtualInstruments->setIcon(QIcon(":/icons/virtualinstruments.png"));
    btn2DGraphicsLine->setIcon(QIcon(":/icons/2dgraphicsline.png"));
    btn2DGraphicsBox->setIcon(QIcon(":/icons/2dgraphicsbox.png"));
    btn2DGraphicsCircle->setIcon(QIcon(":/icons/2dgraphicscircle.png"));
    btn2DGraphicsArc->setIcon(QIcon(":/icons/2dgraphicsarc.png"));
    btn2DGraphicsClosedPath->setIcon(QIcon(":/icons/2dgraphicsclosedpath.png"));
    btn2DGraphicsText->setIcon(QIcon(":/icons/2dgraphicstext.png"));
    btn2DGraphicsSymbols->setIcon(QIcon(":/icons/2dgraphicssymbols.png"));
    btn2DGraphicsMarkers->setIcon(QIcon(":/icons/2dgraphicsmarkers.png"));
    btnRotateClockwise->setIcon(QIcon(":/icons/rotateclockwise.svg"));
    btnRotateAntiClockwise->setIcon(QIcon(":/icons/rotateanticlockwise.svg"));
    btnXMirror->setIcon(QIcon(":/icons/xmirror.svg"));
    btnYMirror->setIcon(QIcon(":/icons/ymirror.svg"));

    // INDEX 0
    QWidget *widgetDevices = new QWidget();
    QVBoxLayout *layoutDevices = new QVBoxLayout(widgetDevices);
    layoutDevices->setContentsMargins(0, 0, 0, 0);
    QLabel *headerDev = new QLabel("P L    DEVICES");
    headerDev->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutDevices->addWidget(headerDev);
    devicesListWidget = new QListWidget();
    devicesListWidget->setStyleSheet(
        "QListWidget { background-color: white; color: black; border: none; }"
        "QListWidget::item { color: black; padding: 2px; }"
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }"
        );
    layoutDevices->addWidget(devicesListWidget);
    connect(devicesListWidget, &QListWidget::itemClicked, this, &schematicPage::onDeviceListItemClicked);

    modeStack->addWidget(widgetDevices);

    // INDEX 1
    QWidget *widgetPorts = new QWidget();
    QVBoxLayout *layoutPorts = new QVBoxLayout(widgetPorts);
    layoutPorts->setContentsMargins(0, 0, 0, 0);
    layoutPorts->setSpacing(0);
    QLabel *headerPort = new QLabel("P    PORTS");
    headerPort->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutPorts->addWidget(headerPort);
    QListWidget *listPorts = new QListWidget();
    listPorts->addItems({"DEFAULT", "INPUT", "OUTPUT", "BIDIR", "POWER", "GROUND", "BUS"});
    listPorts->setSelectionMode(QAbstractItemView::SingleSelection);
    listPorts->setCurrentRow(0);

    listPorts->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listPorts->setStyleSheet(
        "QListWidget { background-color: white; color: black; border: none; }"
        "QListWidget::item { color: black; }"
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }");
    layoutPorts->addWidget(listPorts);

    modeStack->addWidget(widgetPorts);

    // INDEX 2
    QWidget *widgetTerminals = new QWidget();
    QVBoxLayout *layoutTerminals = new QVBoxLayout(widgetTerminals);
    layoutTerminals->setContentsMargins(0, 0, 0, 0);
    layoutTerminals->setSpacing(0);
    QLabel *headerTerminals = new QLabel("P    Terminals");
    headerTerminals->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutTerminals->addWidget(headerTerminals);
    QListWidget *listTerminals = new QListWidget();
    listTerminals->addItems({"DEFAULT", "INPUT", "OUTPUT", "BIDIR", "POWER", "GROUND", "CHASSIS", "RETURN", "DYNAMIC", "TESTPOINT", "NC"});
    listTerminals->setSelectionMode(QAbstractItemView::SingleSelection);
    listTerminals->setCurrentRow(0);
    listTerminals->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listTerminals->setStyleSheet(
        "QListWidget { background-color: white; color: black; border: none; }"
        "QListWidget::item { color: black; }"
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }");
    layoutTerminals->addWidget(listTerminals);

    modeStack->addWidget(widgetTerminals);

    // INDEX 3
    QWidget *widgetPins = new QWidget();
    QVBoxLayout *layoutPins = new QVBoxLayout(widgetPins);
    layoutPins->setContentsMargins(0, 0, 0, 0);
    layoutPins->setSpacing(0);
    QLabel *headerPins = new QLabel("P    Pins");
    headerPins->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutPins->addWidget(headerPins);
    QListWidget *listPins = new QListWidget();
    listPins->addItems({"DEFAULT", "INVERT", "POSCLK", "NEGCLK", "SHORT", "BUS", "INSNEGOP"});
    listPins->setSelectionMode(QAbstractItemView::SingleSelection);
    listPins->setCurrentRow(0);
    listPins->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listPins->setStyleSheet(
        "QListWidget { background-color: white; color: black; border: none; }"
        "QListWidget::item { color: black; }"
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }");
    layoutPins->addWidget(listPins);

    modeStack->addWidget(widgetPins);

    // INDEX 4
    QWidget *widgetGraphs = new QWidget();
    QVBoxLayout *layoutGraphs = new QVBoxLayout(widgetGraphs);
    layoutGraphs->setContentsMargins(0, 0, 0, 0);
    layoutGraphs->setSpacing(0);
    QLabel *headerGraphs = new QLabel("Graphs");
    headerGraphs->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutGraphs->addWidget(headerGraphs);
    QListWidget *listGraphs = new QListWidget();
    listGraphs->addItems({"ANALOGUE", "DIGITAL", "MIXED", "FREQUENCY", "TRASFER", "NOISE", "DISTORTION", "BUSFOURIER", "INTERACTIVE", "CONFORMANCE", "DC SWEEP", "AC SWEEP"});
    listGraphs->setSelectionMode(QAbstractItemView::SingleSelection);
    listGraphs->setCurrentRow(0);
    listGraphs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listGraphs->setStyleSheet(
        "QListWidget { background-color: white; color: black; border: none; }"
        "QListWidget::item { color: black; }"
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }");
    layoutGraphs->addWidget(listGraphs);

    modeStack->addWidget(widgetGraphs);

    // INDEX 5
    QWidget *widgetGenerators = new QWidget();
    QVBoxLayout *layoutGenerators = new QVBoxLayout(widgetGenerators);
    layoutGenerators->setContentsMargins(0, 0, 0, 0);
    layoutGenerators->setSpacing(0);
    QLabel *headerGenerators = new QLabel("Generators");
    headerGenerators->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutGenerators->addWidget(headerGenerators);
    QListWidget *listGenerators = new QListWidget();
    listGenerators->addItems({"C", "SIN", "PULSE", "EXP", "SFFM", "PWLIN", "FILE", "AUDIO", "RANDOM", "DSTATE", "DEDGE", "DPULSE", "DCLOCK", "DPATTERN", "SCRIPTABLE"});
    listGenerators->setSelectionMode(QAbstractItemView::SingleSelection);
    listGenerators->setCurrentRow(0);
    listGenerators->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listGenerators->setStyleSheet(
        "QListWidget { background-color: white; color: black; border: none; }"
        "QListWidget::item { color: black; }"
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }");
    layoutGenerators->addWidget(listGenerators);

    modeStack->addWidget(widgetGenerators);

    // INDEX 6
    QWidget *widgetProbes = new QWidget();
    QVBoxLayout *layoutProbes = new QVBoxLayout(widgetProbes);
    layoutProbes->setContentsMargins(0, 0, 0, 0);
    layoutProbes->setSpacing(0);
    QLabel *headerProbes = new QLabel("Probes");
    headerProbes->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutProbes->addWidget(headerProbes);
    QListWidget *listProbes = new QListWidget();
    listProbes->addItems({"VOLTAGE", "CURRENT", "TAPE"});
    listProbes->setSelectionMode(QAbstractItemView::SingleSelection);
    listProbes->setCurrentRow(0);
    connect(listProbes, &QListWidget::currentRowChanged, this, [this](int row) {
        selectedProbeType = (row == 1) ? "CURRENT" : "VOLTAGE";
    });
    listProbes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listProbes->setStyleSheet(
        "QListWidget { background-color: white; color: black; border: none; }"
        "QListWidget::item { color: black; }"
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }");
    layoutProbes->addWidget(listProbes);

    modeStack->addWidget(widgetProbes);

    // INDEX 7
    QWidget *widgetInstruments = new QWidget();
    QVBoxLayout *layoutInstruments = new QVBoxLayout(widgetInstruments);
    layoutInstruments->setContentsMargins(0, 0, 0, 0);
    layoutInstruments->setSpacing(0);
    QLabel *headerInstruments = new QLabel("Instruments");
    headerInstruments->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutInstruments->addWidget(headerInstruments);
    QListWidget *listInstruments = new QListWidget();
    listInstruments->addItems({"OSCILLISCOPE", "LOGIC ANALYSER", "COUNTER TIMER", "VIRTUAL TERMINAL", "SPI DEBUGGER", "I2C DEBUGGER", "SIGNAL GENERATOR", "PATTERN GENERATOR", "DC VOLTMETER", "DC AMMETER", "AC VOLTMETER", "AC AMMETER", "WATTMETER"});
    listInstruments->setSelectionMode(QAbstractItemView::SingleSelection);
    listInstruments->setCurrentRow(0);
    listInstruments->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listInstruments->setStyleSheet(
        "QListWidget { background-color: white; color: black; border: none; }"
        "QListWidget::item { color: black; }"
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }");
    layoutInstruments->addWidget(listInstruments);

    modeStack->addWidget(widgetInstruments);

    // INDEX 8
    QWidget *widgetGraphics = new QWidget();
    QVBoxLayout *layoutGraphics = new QVBoxLayout(widgetGraphics);
    layoutGraphics->setContentsMargins(0, 0, 0, 0);
    layoutGraphics->setSpacing(0);
    QLabel *headerGraphics = new QLabel("C E  Graphics");
    headerGraphics->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutGraphics->addWidget(headerGraphics);
    QListWidget *listGraphics = new QListWidget();
    listGraphics->addItems({"COMPONENT", "PIN", "PORT", "MARKER", "ACTUATOR", "INDICATOR", "VPROBE", "IPROBE", "TAPE", "GENERATOR", "TERMINAL", "SUBCIRCUIT", "2D GRAPHIC", "WIRE DOT", "WIRE", "BUS WIRE", "BORDER", "TEMPLATE"});
    listGraphics->setSelectionMode(QAbstractItemView::SingleSelection);
    listGraphics->setCurrentRow(0);
    listGraphics->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listGraphics->setStyleSheet(
        "QListWidget { background-color: white; color: black; border: none; }"
        "QListWidget::item { color: black; }"
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }");
    layoutGraphics->addWidget(listGraphics);

    modeStack->addWidget(widgetGraphics);

    // INDEX 9
    QWidget *widgetMarkers = new QWidget();
    QVBoxLayout *layoutMarkers = new QVBoxLayout(widgetMarkers);
    layoutMarkers->setContentsMargins(0, 0, 0, 0);
    layoutMarkers->setSpacing(0);
    QLabel *headerMarkers = new QLabel("Markers");
    headerMarkers->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutMarkers->addWidget(headerMarkers);
    QListWidget *listMarkers = new QListWidget();
    listMarkers->addItems({"ORIGIN", "NODE", "BUSNODE", "LABEL", "DEVICEREF", "DEVICEVAL", "PINNAME", "PINNUM", "INCREMENT", "DECREMENT", "TOGGLE", "GRID"});
    listMarkers->setSelectionMode(QAbstractItemView::SingleSelection);
    listMarkers->setCurrentRow(0);
    listMarkers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listMarkers->setStyleSheet(
        "QListWidget { background-color: white; color: black; border: none; }"
        "QListWidget::item { color: black; }"
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }");
    layoutMarkers->addWidget(listMarkers);

    modeStack->addWidget(widgetMarkers);

    connect(btnSelection, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(0);
    });

    connect(btnComponent, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(0);
    });

    connect(btnJunctionDot, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(0);
    });

    connect(btn2DGraphicsSymbols, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(0);
    });

    connect(btnSubCircuit, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(1);
    });

    connect(btnTerminals, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(2);
    });

    connect(btnDevicePins, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(3);
    });

    connect(btnGraph, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(4);
    });

    connect(btnGenerator, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(5);
    });

    connect(btnProbMode, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) {
            modeStack->setCurrentIndex(6);
            interactionMode = InteractionMode::Probing;
        } else if (interactionMode == InteractionMode::Probing) {
            interactionMode = InteractionMode::Idle;
            probeDisplayActive = false;
            if (componentOverlay) componentOverlay->update();
        }
    });

    connect(btnVirtualInstruments, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(7);
    });

    connect(btn2DGraphicsBox, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(8);
    });

    connect(btn2DGraphicsCircle, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(8);
    });

    connect(btn2DGraphicsArc, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(8);
    });

    connect(btn2DGraphicsMarkers, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(9);
    });

    connect(btn2DGraphicsClosedPath, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(8);
    });

    connect(btn2DGraphicsText, &QToolButton::toggled, this, [=](bool checked) {
        if (checked) modeStack->setCurrentIndex(8);
    });

    connect(btnRotateClockwise, &QToolButton::clicked, this, [this]() {
        if (selectedComponents.isEmpty()) return;
        for (Component *c : selectedComponents) c->rotateNext();
        if (componentOverlay) componentOverlay->update();
        pushUndoState();
    });
    connect(btnRotateAntiClockwise, &QToolButton::clicked, this, [this]() {
        if (selectedComponents.isEmpty()) return;
        for (Component *c : selectedComponents) {
            c->rotateNext(); c->rotateNext(); c->rotateNext();
        }
        if (componentOverlay) componentOverlay->update();
        pushUndoState();
    });
    connect(btnXMirror, &QToolButton::clicked, this, [this]() {
        if (selectedComponents.isEmpty()) return;
        for (Component *c : selectedComponents) c->mirrorHorizontal();
        if (componentOverlay) componentOverlay->update();
        pushUndoState();
    });
    connect(btnYMirror, &QToolButton::clicked, this, [this]() {
        if (selectedComponents.isEmpty()) return;
        for (Component *c : selectedComponents) c->mirrorVertical();
        if (componentOverlay) componentOverlay->update();
        pushUndoState();
    });

    connect(actUndo, &QAction::triggered, this, &schematicPage::undoAction);
    connect(actRedo, &QAction::triggered, this, &schematicPage::redoAction);

    sideLayout->addWidget(rotationSpin);
    sideLayout->addWidget(btnXMirror);
    sideLayout->addWidget(btnYMirror);

    // --- Build the canvas and bottom bar ---
    schematicCanvas->setMouseTracking(true);
    schematicCanvas->setContextMenuPolicy(Qt::CustomContextMenu);

    componentOverlay = new ComponentOverlay(&componentsOnboard, &wiresOnboard, &junctionsOnboard, &wiringPreviewActive, &wiringPreviewPath, schematicCanvas, &probeDisplayActive, &probeDisplayText, &probeDisplayPos, schematicCanvas->viewport());    componentOverlay->setGeometry(schematicCanvas->viewport()->rect());
    componentOverlay->raise();
    componentOverlay->show();

    schematicCanvas->viewport()->installEventFilter(this);

    connect(schematicCanvas, &shematicClass::canvasClicked,
            this, &schematicPage::onCanvasClicked);

    connect(schematicCanvas, &shematicClass::canvasDragMoved,
            this, &schematicPage::onCanvasDragMoved);

    connect(schematicCanvas, &shematicClass::componentDoubleClicked,
            this, &schematicPage::onComponentDoubleClicked);

    connect(schematicCanvas, &shematicClass::canvasReleased,
            this, &schematicPage::onCanvasReleased);

    connect(schematicCanvas, &shematicClass::deleteKeyPressed,
            this, &schematicPage::onDeleteSelected);

    connect(schematicCanvas, &QWidget::customContextMenuRequested,
            this, &schematicPage::onCanvasContextMenu);

    connect(schematicCanvas, &shematicClass::escapePressed, this, [this]() {
        selectedComponentType.clear();
        schematicCanvas->setPlacementActive(false);
        unsetCursor();
        if (interactionMode == InteractionMode::Wiring) {
            cancelWiring();
        } else {
            interactionMode = InteractionMode::Idle;
        }
        if (componentOverlay) {
            static_cast<ComponentOverlay*>(componentOverlay)->clearRubberBandRect();
        }
    });
    connect(actNew, &QAction::triggered, this, [this]() {
        clearCircuit();
        currentProjectName.clear();
        currentProjectPath.clear();
        saved = false;
        resetUndoHistory();
    });

    QWidget *bottomBar = new QWidget();
    bottomBar->setFixedHeight(30);
    bottomBar->setStyleSheet("background-color: #f0f0f0; border-top: 1px solid #d0d0d0;");
    bottomBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(5, 2, 5, 2);
    bottomLayout->setSpacing(15);

    statusLabel = new QLabel("No Messages");
    statusLabel->setStyleSheet("color: black;");
    timeLabel = new QLabel("t = 0.000 s");
    timeLabel->setStyleSheet("color: black; font-family: monospace; font-size: 12px;");
    bottomLayout->addWidget(statusLabel);
    bottomLayout->addWidget(timeLabel);

    btnRun = new QPushButton();
    btnRun->setToolTip("Run");
    btnRun->setStyleSheet("color: black;");

    btnPause = new QPushButton();
    btnPause->setToolTip("Pause");
    btnPause->setStyleSheet("color: black;");

    btnStop = new QPushButton();
    btnStop->setToolTip("Stop");
    btnStop->setStyleSheet("color: black;");

    btnRestart = new QPushButton();
    btnRestart->setToolTip("Restart");
    btnRestart->setStyleSheet("color: black;");


    bottomLayout->insertWidget(0, btnRun);
    bottomLayout->insertWidget(1, btnPause);
    bottomLayout->insertWidget(2, btnStop);
    bottomLayout->insertWidget(3, btnRestart);

    connect(btnRun, &QPushButton::clicked, this, &schematicPage::onRun);
    connect(btnPause, &QPushButton::clicked, this, &schematicPage::onPause);
    connect(btnStop, &QPushButton::clicked, this, &schematicPage::onStop);
    connect(btnRestart, &QPushButton::clicked, this, &schematicPage::onRestart);

    btnRun->setIcon(QIcon(":/icons/play.png"));
    btnPause->setIcon(QIcon(":/icons/pause.png"));
    btnStop->setIcon(QIcon(":/icons/stop.png"));
    btnRestart->setIcon(QIcon(":/icons/resume.png"));


    simTimer = new QTimer(this);
    simTimer->setInterval(50);
    connect(simTimer, &QTimer::timeout, this, &schematicPage::advanceSimulation);

    isRunning = false;
    isPaused = false;
    simTime = 0.0;

    updateButtonStates();

    QLabel *sheetLabel = new QLabel("ROOT - Root sheet 1");
    sheetLabel->setStyleSheet("color: black;");
    QLabel *coordsLabel = new QLabel("x: 0.0 y: 0.0");
    coordsLabel->setFixedWidth(100);
    coordsLabel->setStyleSheet("font-family: monospace; font-size: 12px; color: #000000;");

    bottomLayout->addWidget(sheetLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(coordsLabel);

    connect(schematicCanvas, &shematicClass::mouseCoordinatesChanged, this, [=](QString coords) {
        coordsLabel->setText(coords);
    });

    splitter->addWidget(sidebar);
    splitter->addWidget(schematicCanvas);

    splitter->setStyleSheet("QSplitter { border: none; }");

    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(myMenuBar, 0);
    mainLayout->addWidget(myToolBar, 0);
    mainLayout->addWidget(splitter, 1);
    mainLayout->addWidget(bottomBar, 0);
    resetUndoHistory();
}

schematicPage::~schematicPage()
{
    delete ui;
}

void schematicPage::onPickDevices() {
    PickDevicesDialog dlg(this);
    connect(&dlg, &PickDevicesDialog::deviceAccepted, this,
            [this](const QString &name, const QString &/*category*/){
                if (devicesListWidget && devicesListWidget->findItems(name, Qt::MatchExactly).isEmpty()) {
                    devicesListWidget->addItem(name);
                }
            });
    dlg.exec();
}

void schematicPage::onDeviceListItemClicked(QListWidgetItem *item) {
    if (!item) return;
    selectedComponentType = item->text();
    setCursor(Qt::CrossCursor);
    clearSelection();
    if (componentOverlay) componentOverlay->update();

    if (schematicCanvas) {
        schematicCanvas->setPlacementActive(true);
    }
}

Component* schematicPage::findComponentAt(const QPointF &scenePos) const
{
    QPoint p = scenePos.toPoint();

    for (auto it = componentsOnboard.rbegin(); it != componentsOnboard.rend(); ++it) {
        Component *c = *it;
        if (c && c->boundingRect().contains(p)) return c;
    }
    return nullptr;
}

void schematicPage::clearSelection()
{
    for (Component *c : selectedComponents) {
        if (c) c->setSelected(false);
    }
    selectedComponents.clear();
}

void schematicPage::selectComponent(Component *c)
{
    if (!c || selectedComponents.contains(c)) return;
    c->setSelected(true);
    selectedComponents.push_back(c);
}

void schematicPage::deselectComponent(Component *c)
{
    if (!c) return;
    c->setSelected(false);
    selectedComponents.removeOne(c);
}

bool schematicPage::findPinAt(const QPointF &scenePos, Component *&outComp, int &outPinIdx) const
{
    Position p(qRound(scenePos.x()), qRound(scenePos.y()));
    for (Component *c : componentsOnboard) {
        if (!c) continue;
        int idx = c->findPinNear(p);
        if (idx >= 0) { outComp = c; outPinIdx = idx; return true; }
    }
    outComp = nullptr;
    outPinIdx = -1;
    return false;
}

QVector<Position> schematicPage::buildWiringPreviewPath(const Position &mousePos) const
{
    QVector<Position> full;
    if (!wireStartComponent) return full;

    Position start = wireStartComponent->getPinScenePosition(wireStartPinIndex);
    full.push_back(start);
    Position current = start;

    for (const Position &wp : wireWaypoints) {
        if (current.x != wp.x && current.y != wp.y) {
            full.push_back(Position(wp.x, current.y));
        }
        full.push_back(wp);
        current = wp;
    }

    if (current.x != mousePos.x && current.y != mousePos.y) {
        full.push_back(Position(mousePos.x, current.y));
    }
    full.push_back(mousePos);

    return full;
}

void schematicPage::startWiring(Component *comp, int pinIdx)
{
    wireStartComponent = comp;
    wireStartPinIndex = pinIdx;
    wireWaypoints.clear();
    interactionMode = InteractionMode::Wiring;
    wiringPreviewActive = true;

    Position startPos = comp->getPinScenePosition(pinIdx);
    wiringPreviewPath = buildWiringPreviewPath(startPos);

    if (componentOverlay) componentOverlay->update();
}

void schematicPage::addWireWaypoint(const Position &pt)
{
    if (!wireStartComponent) return;

    Position lastAnchor = wireWaypoints.isEmpty()
                              ? wireStartComponent->getPinScenePosition(wireStartPinIndex)
                              : wireWaypoints.last();

    if (pt == lastAnchor) return;

    wireWaypoints.push_back(pt);
    wiringPreviewPath = buildWiringPreviewPath(pt);

    if (componentOverlay) componentOverlay->update();
}


void schematicPage::finishWiringAtPin(Component *targetComp, int targetPinIdx)
{
    bool isSamePin = (targetComp == wireStartComponent && targetPinIdx == wireStartPinIndex);
    bool wireCreated = false;
    if (!isSamePin && wireStartComponent) {
        Wire *w = new Wire(wireStartComponent, wireStartPinIndex, targetComp, targetPinIdx,
                           wireWaypoints, &componentsOnboard);
        wiresOnboard.push_back(w);
        wireCreated = true;
    }
    cancelWiring();
    if (wireCreated) pushUndoState();
}

void schematicPage::finishWiringDangling(const Position &pt)
{
    bool wireCreated = false;
    if (wireStartComponent) {
        if (!wireWaypoints.isEmpty() && wireWaypoints.last() == pt) {
            wireWaypoints.removeLast();
        }
        Wire *w = new Wire(wireStartComponent, wireStartPinIndex, pt,
                           wireWaypoints, &componentsOnboard);
        wiresOnboard.push_back(w);
        wireCreated = true;
    }
    cancelWiring();
    if (wireCreated) pushUndoState();
}

void schematicPage::cancelWiring()
{
    wireStartComponent = nullptr;
    wireStartPinIndex = -1;
    wireWaypoints.clear();
    wiringPreviewActive = false;
    wiringPreviewPath.clear();
    interactionMode = InteractionMode::Idle;
    if (componentOverlay) componentOverlay->update();
}

void schematicPage::updatePinHover(const QPointF &scenePos)
{
    Position mousePos(qRound(scenePos.x()), qRound(scenePos.y()));
    Component *nearComp = nullptr;
    int nearPinIdx = -1;

    for (Component *c : componentsOnboard) {
        if (!c) continue;
        int idx = c->findPinNear(mousePos);
        if (idx >= 0) { nearComp = c; nearPinIdx = idx; break; }
    }

    if (hoveredPinComponent && (hoveredPinComponent != nearComp || hoveredPinIndex != nearPinIdx)) {
        hoveredPinComponent->setPinHighlighted(hoveredPinIndex, false);
    }

    hoveredPinComponent = nearComp;
    hoveredPinIndex = nearPinIdx;

    if (hoveredPinComponent) {
        hoveredPinComponent->setPinHighlighted(hoveredPinIndex, true);
        if (componentOverlay) componentOverlay->update();
    } else if (componentOverlay) {
        componentOverlay->update();
    }
}

void schematicPage::updateProbeAt(const QPointF &scenePos)
{
    Position p(qRound(scenePos.x()), qRound(scenePos.y()));

    Component *comp = nullptr;
    int pinIdx = -1;
    if (findPinAt(scenePos, comp, pinIdx)) {
        if (selectedProbeType == "CURRENT") {
            probeDisplayText = QString("%1 A").arg(comp->getComponentCurrent(), 0, 'f', 3);
        } else {
            probeDisplayText = QString("%1 V").arg(comp->getComponentVoltage(), 0, 'f', 2);
        }
        probeDisplayPos = comp->getPinScenePosition(pinIdx);
        probeDisplayActive = true;
        if (componentOverlay) componentOverlay->update();
        return;
    }

    Wire *w = findWireNear(scenePos);
    if (w) {
        probeDisplayText = (selectedProbeType == "CURRENT") ? "N/A" : w->probeVoltageLabel();
        probeDisplayPos = p;
        probeDisplayActive = true;
        if (componentOverlay) componentOverlay->update();
        return;
    }

    if (probeDisplayActive) {
        probeDisplayActive = false;
        if (componentOverlay) componentOverlay->update();
    }
}

void schematicPage::recalcAllWires()
{
    for (Wire *w : wiresOnboard) {
        if (w) w->recalculateRoute();
    }
}

void schematicPage::pruneJunctions()
{
    recalcAllWires();
    for (int i = junctionsOnboard.size() - 1; i >= 0; --i) {
        Junction *j = junctionsOnboard[i];
        bool stillValid = false;
        for (Wire *w : wiresOnboard) {
            if (w->isNear(j->position(), 3)) { stillValid = true; break; }
        }
        if (!stillValid) {
            delete j;
            junctionsOnboard.remove(i);
        }
    }
}

Wire* schematicPage::findWireNear(const QPointF &scenePos) const
{
    Position p(qRound(scenePos.x()), qRound(scenePos.y()));
    for (auto it = wiresOnboard.rbegin(); it != wiresOnboard.rend(); ++it) {
        if (*it && (*it)->isNear(p)) return *it;
    }
    return nullptr;
}

bool schematicPage::findWireIntersectionNear(const QPointF &scenePos, Position &outPoint) const
{
    Position click(qRound(scenePos.x()), qRound(scenePos.y()));
    for (int i = 0; i < wiresOnboard.size(); ++i) {
        const auto &pathA = wiresOnboard[i]->path();
        for (int si = 0; si + 1 < pathA.size(); ++si) {
            for (int j = i + 1; j < wiresOnboard.size(); ++j) {
                const auto &pathB = wiresOnboard[j]->path();
                for (int sj = 0; sj + 1 < pathB.size(); ++sj) {
                    Position inter;
                    if (intersectOrthogonal(pathA[si], pathA[si + 1], pathB[sj], pathB[sj + 1], inter)) {
                        int dx = inter.x - click.x, dy = inter.y - click.y;
                        if (dx * dx + dy * dy <= 36) {
                            outPoint = inter;
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}


void schematicPage::onCanvasClicked(QPointF pos, Qt::KeyboardModifiers modifiers)
{
    if (interactionMode == InteractionMode::Probing) {
        return;
    }

    if (!selectedComponentType.isEmpty()) {
        clearSelection();

        Component *newComp = nullptr;
        QString type = selectedComponentType.toUpper();

        if (type.contains("GND") || type.contains("GROUND"))
            newComp = new GND();
        else if (type.contains("BATTERY"))
            newComp = new Battery(12.0, 0.1);
        else if (type.contains("CLOCK"))
            newComp = new Clock_gen(5.0, 1.0);
        else if (type.contains("DC"))
            newComp = new DC_vol_source(5.0);
        else if (type.contains("RESISTOR"))
            newComp = new Resistor(1000);
        else if (type.contains("CAPACITOR"))
            newComp = new Capacitor(0.000001);
        else if (type.contains("INDUCTOR"))
            newComp = new Inductor(0.001);
        else if (type.contains("PUSH") || type.contains("BUTTON"))
            newComp = new Push_button();
        else if (type.contains("SWITCH"))
            newComp = new Switch();
        else if (type.contains("LED"))
            newComp = new LED(0.7);
        else if (type.contains("SEG") || type.contains("SEVEN"))
            newComp = new seven_seg();
        else if (type.contains("XNOR"))
            newComp = new XNORGate();
        else if (type.contains("XOR"))
            newComp = new XORGate();
        else if (type.contains("NAND"))
            newComp = new NANDGate();
        else if (type.contains("NOR"))
            newComp = new NORGate();
        else if (type.contains("AND"))
            newComp = new ANDGate();
        else if (type.contains("OR"))
            newComp = new ORGate();
        else if (type.contains("NOT"))
            newComp = new NOTGate();
        else if (type.contains("FLIP") || type.contains("DFF") || type.contains("DTFF") || type.contains("D-FF"))
            newComp = new DFlipFlop();

        if (newComp) {
            int sx = qRound(pos.x() / gridSize) * gridSize;
            int sy = qRound(pos.y() / gridSize) * gridSize;
            newComp->setPosition(Position(sx, sy));
            componentsOnboard.push_back(newComp);
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }

        selectedComponentType.clear();
        unsetCursor();
        if (schematicCanvas) {
            schematicCanvas->setPlacementActive(false);
        }
        return;
    }

    if (interactionMode == InteractionMode::Wiring && wireStartComponent) {
        Component *pinComp = nullptr;
        int pinIdx = -1;
        if (findPinAt(pos, pinComp, pinIdx)) {

            finishWiringAtPin(pinComp, pinIdx);
        } else {

            Position clickPt(qRound(pos.x()), qRound(pos.y()));
            addWireWaypoint(clickPt);
        }
        return;
    }


    {
        Component *pinComp = nullptr;
        int pinIdx = -1;
        if (findPinAt(pos, pinComp, pinIdx)) {
            if (selectedWire) { selectedWire->setSelected(false); selectedWire = nullptr; }
            clearSelection();
            startWiring(pinComp, pinIdx);
            return;
        }
    }


    {
        recalcAllWires();
        Position junctionPt;
        if (findWireIntersectionNear(pos, junctionPt)) {
            bool exists = false;
            for (Junction *j : junctionsOnboard) {
                if (j->isNear(junctionPt)) { exists = true; break; }
            }
            if (!exists) {
                junctionsOnboard.push_back(new Junction(junctionPt));
                pushUndoState();
            }
            if (componentOverlay) componentOverlay->update();
            return;
        }
    }

    Component *hit = findComponentAt(pos);

    if (hit) {
        if (selectedWire) { selectedWire->setSelected(false); selectedWire = nullptr; }

        if (auto *sw = dynamic_cast<Switch *>(hit)) {
            sw->toggle();
        } else if (auto *pb = dynamic_cast<Push_button *>(hit)) {
            pb->press();
            pressedPushButton = pb;
        }

        bool ctrl = modifiers.testFlag(Qt::ControlModifier);
        if (ctrl) {
            if (selectedComponents.contains(hit)) deselectComponent(hit);
            else selectComponent(hit);
        } else if (!selectedComponents.contains(hit)) {
            clearSelection();
            selectComponent(hit);
        }

        interactionMode = InteractionMode::MovingComponents;
        dragAnchorScenePos = pos;
        moveStartPositions.clear();
        for (Component *c : selectedComponents) {
            moveStartPositions.push_back(c->getPosition());
        }
    } else {

        Wire *wireHit = findWireNear(pos);
        if (wireHit) {
            if (selectedWire) selectedWire->setSelected(false);
            selectedWire = wireHit;
            selectedWire->setSelected(true);
            clearSelection();
            interactionMode = InteractionMode::Idle;
            if (componentOverlay) componentOverlay->update();
            return;
        }

        if (selectedWire) { selectedWire->setSelected(false); selectedWire = nullptr; }

        if (!modifiers.testFlag(Qt::ControlModifier)) {
            clearSelection();
        }
        interactionMode = InteractionMode::RubberBandSelecting;
        rubberBandStartPoint = pos.toPoint();
    }

    if (componentOverlay) componentOverlay->update();
}

void schematicPage::onCanvasDragMoved(QPointF scenePos, bool leftButtonDown)
{

    updatePinHover(scenePos);

    if (interactionMode == InteractionMode::Probing) {
        updateProbeAt(scenePos);
    }

    if (interactionMode == InteractionMode::Wiring && wireStartComponent) {
        Position mouse(qRound(scenePos.x()), qRound(scenePos.y()));
        wiringPreviewPath = buildWiringPreviewPath(mouse);
        if (componentOverlay) componentOverlay->update();
        return;
    }

    if (!leftButtonDown) return;

    if (interactionMode == InteractionMode::RubberBandSelecting) {
        if (componentOverlay) {
            QRect r(rubberBandStartPoint, scenePos.toPoint());
            static_cast<ComponentOverlay*>(componentOverlay)->setRubberBandRect(r.normalized());
        }
        return;
    }

    if (interactionMode == InteractionMode::MovingComponents) {
        int dx = static_cast<int>(scenePos.x() - dragAnchorScenePos.x());
        int dy = static_cast<int>(scenePos.y() - dragAnchorScenePos.y());

        for (int i = 0; i < selectedComponents.size() && i < moveStartPositions.size(); ++i) {
            Position start = moveStartPositions[i];
            int nx = start.x + dx;
            int ny = start.y + dy;

            nx = qRound(static_cast<double>(nx) / gridSize) * gridSize;
            ny = qRound(static_cast<double>(ny) / gridSize) * gridSize;

            selectedComponents[i]->setPosition(Position(nx, ny));
        }
        if (componentOverlay) componentOverlay->update();
    }
}

void schematicPage::onCanvasReleased(QPointF scenePos)
{
    if (pressedPushButton) {
        pressedPushButton->release();
        pressedPushButton = nullptr;
        if (componentOverlay) componentOverlay->update();
    }

    if (interactionMode == InteractionMode::RubberBandSelecting) {
        QRect r = QRect(rubberBandStartPoint, scenePos.toPoint()).normalized();

        if (r.width() > 2 || r.height() > 2) {
            for (Component *c : componentsOnboard) {
                if (c && c->boundingRect().intersects(r)) {
                    selectComponent(c);
                }
            }
        }
        if (componentOverlay) {
            static_cast<ComponentOverlay*>(componentOverlay)->clearRubberBandRect();
        }
        interactionMode = InteractionMode::Idle;
    } else if (interactionMode == InteractionMode::MovingComponents) {
        interactionMode = InteractionMode::Idle;

        bool actuallyMoved = false;
        for (int i = 0; i < selectedComponents.size() && i < moveStartPositions.size(); ++i) {
            if (selectedComponents[i]->getPosition().x != moveStartPositions[i].x ||
                selectedComponents[i]->getPosition().y != moveStartPositions[i].y) {
                actuallyMoved = true;
                break;
            }
        }
        if (actuallyMoved) pushUndoState();
    }

    if (componentOverlay) componentOverlay->update();
}

bool schematicPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == schematicCanvas->viewport() && event->type() == QEvent::Resize) {
        if (componentOverlay)
            componentOverlay->setGeometry(schematicCanvas->viewport()->rect());
    }
    return QWidget::eventFilter(watched, event);
}

void schematicPage::onComponentDoubleClicked(QPointF scenePos)
{

    if (interactionMode == InteractionMode::Wiring && wireStartComponent) {
        Position dangle(qRound(scenePos.x()), qRound(scenePos.y()));
        finishWiringDangling(dangle);
        return;
    }

    Component *hit = findComponentAt(scenePos);
    if (hit) openEditDialogFor(hit);
}

void schematicPage::onDeleteSelected()
{
    bool didDeleteWire = false;
    if (selectedWire) {
        wiresOnboard.removeOne(selectedWire);
        delete selectedWire;
        selectedWire = nullptr;
        pruneJunctions();
        if (componentOverlay) componentOverlay->update();
        didDeleteWire = true;
    }

    bool didDeleteComponents = !selectedComponents.isEmpty();
    if (didDeleteComponents) {
        for (Component *c : selectedComponents) {
            for (int i = wiresOnboard.size() - 1; i >= 0; --i) {
                if (wiresOnboard[i]->isConnectedTo(c)) {
                    delete wiresOnboard[i];
                    wiresOnboard.remove(i);
                }
            }
            int idx = componentsOnboard.indexOf(c);
            if (idx >= 0) componentsOnboard.remove(idx);
            if (c == pressedPushButton) pressedPushButton = nullptr;
            if (c == hoveredPinComponent) { hoveredPinComponent = nullptr; hoveredPinIndex = -1; }
            delete c;
        }
        selectedComponents.clear();
        pruneJunctions();
    }

    if (componentOverlay) componentOverlay->update();

    if (didDeleteWire || didDeleteComponents) {
        pushUndoState();
    }
}


void schematicPage::onCanvasContextMenu(const QPoint &viewPos)
{
    QPointF scenePos = schematicCanvas->mapToScene(viewPos);
    recalcAllWires();
    Component *hit = findComponentAt(scenePos);

    if (hit) {
        if (selectedWire) { selectedWire->setSelected(false); selectedWire = nullptr; }
        if (!selectedComponents.contains(hit)) {
            clearSelection();
            selectComponent(hit);
        }
        if (componentOverlay) componentOverlay->update();
    } else {
        Wire *wireHit = findWireNear(scenePos);
        if (wireHit) {
            if (selectedWire) selectedWire->setSelected(false);
            selectedWire = wireHit;
            selectedWire->setSelected(true);
            clearSelection();
            if (componentOverlay) componentOverlay->update();
        }
    }

    if (selectedComponents.isEmpty() && !selectedWire) return;

    QMenu menu(this);
    QAction *actDelete = menu.addAction("Delete");
    QAction *actRotateCW = selectedComponents.isEmpty() ? nullptr : menu.addAction(QString::fromUtf8("Rotate 90\u00b0"));
    QAction *actMirrorH = selectedComponents.isEmpty() ? nullptr : menu.addAction("Mirror Horizontal");
    QAction *actMirrorV = selectedComponents.isEmpty() ? nullptr : menu.addAction("Mirror Vertical");

    QAction *chosen = menu.exec(schematicCanvas->mapToGlobal(viewPos));
    if (!chosen) return;

    if (chosen == actDelete) {
        onDeleteSelected();
    } else if (actRotateCW && chosen == actRotateCW) {
        for (Component *c : selectedComponents) c->rotateNext();
        if (componentOverlay) componentOverlay->update();
        pushUndoState();
    } else if (actMirrorH && chosen == actMirrorH) {
        for (Component *c : selectedComponents) c->mirrorHorizontal();
        if (componentOverlay) componentOverlay->update();
        pushUndoState();
    } else if (actMirrorV && chosen == actMirrorV) {
        for (Component *c : selectedComponents) c->mirrorVertical();
        if (componentOverlay) componentOverlay->update();
        pushUndoState();
    }
}

void schematicPage::openEditDialogFor(Component *comp)
{
    if (!comp) return;

    QVector<EditField> fields;

    if (auto *r = dynamic_cast<Resistor *>(comp)) {
        fields.push_back({"Resistance", r->resistance, QString::fromUtf8("\u2126")});
        ComponentEditDialog dlg(comp->getName(), fields, this);
        if (dlg.exec() == QDialog::Accepted) {
            comp->setName(dlg.label());
            r->resistance = dlg.fieldValue(0);
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }
        return;
    }

    if (auto *c = dynamic_cast<Capacitor *>(comp)) {
        fields.push_back({"Capacitance", c->capacitance, "F"});
        ComponentEditDialog dlg(comp->getName(), fields, this);
        if (dlg.exec() == QDialog::Accepted) {
            comp->setName(dlg.label());
            c->capacitance = dlg.fieldValue(0);
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }
        return;
    }

    if (auto *l = dynamic_cast<Inductor *>(comp)) {
        fields.push_back({"Inductance", l->inductance, "H"});
        ComponentEditDialog dlg(comp->getName(), fields, this);
        if (dlg.exec() == QDialog::Accepted) {
            comp->setName(dlg.label());
            l->inductance = dlg.fieldValue(0);
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }
        return;
    }

    if (auto *b = dynamic_cast<Battery *>(comp)) {
        fields.push_back({"Voltage", b->getVoltage(), "V"});
        fields.push_back({"Internal Resistance", b->getInternalResistance(), QString::fromUtf8("\u2126")});
        ComponentEditDialog dlg(comp->getName(), fields, this);
        if (dlg.exec() == QDialog::Accepted) {
            comp->setName(dlg.label());
            b->setVoltage(dlg.fieldValue(0));
            b->setInternalResistance(dlg.fieldValue(1));
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }
        return;
    }

    if (auto *dc = dynamic_cast<DC_vol_source *>(comp)) {
        fields.push_back({"Voltage", dc->getVoltage(), "V"});
        ComponentEditDialog dlg(comp->getName(), fields, this);
        if (dlg.exec() == QDialog::Accepted) {
            comp->setName(dlg.label());
            dc->setVoltage(dlg.fieldValue(0));
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }
        return;
    }


    if (auto *clk = dynamic_cast<Clock_gen *>(comp)) {
        fields.push_back({"Frequency", clk->getFrequency(), "Hz"});
        ComponentEditDialog dlg(comp->getName(), fields, this);
        if (dlg.exec() == QDialog::Accepted) {
            comp->setName(dlg.label());
            clk->setFrequency(dlg.fieldValue(0));
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }
        return;
    }

    if (auto *led = dynamic_cast<LED *>(comp)) {
        fields.push_back({"Threshold Voltage", led->Vth, "V"});
        ComponentEditDialog dlg(comp->getName(), fields, this);
        if (dlg.exec() == QDialog::Accepted) {
            comp->setName(dlg.label());
            led->Vth = dlg.fieldValue(0);
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }
        return;
    }


    if (auto *notGate = dynamic_cast<NOTGate *>(comp)) {
        fields.push_back({"Propagation Delay", notGate->getDelay(), "s"});
        ComponentEditDialog dlg(comp->getName(), fields, this);
        if (dlg.exec() == QDialog::Accepted) {
            comp->setName(dlg.label());
            notGate->setDelay(dlg.fieldValue(0));
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }
        return;
    }


    if (auto *dff = dynamic_cast<DFlipFlop *>(comp)) {
        fields.push_back({"Propagation Delay", dff->getDelay(), "s"});
        ComponentEditDialog dlg(comp->getName(), fields, this);
        if (dlg.exec() == QDialog::Accepted) {
            comp->setName(dlg.label());
            dff->setDelay(dlg.fieldValue(0));
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }
        return;
    }


    if (auto *gate = dynamic_cast<LogicGate *>(comp)) {
        fields.push_back({"Input Count", static_cast<double>(gate->getNumInputs()), ""});
        fields.push_back({"Propagation Delay", gate->getDelay(), "s"});
        ComponentEditDialog dlg(comp->getName(), fields, this);
        if (dlg.exec() == QDialog::Accepted) {
            comp->setName(dlg.label());
            int n = qMax(1, static_cast<int>(qRound(dlg.fieldValue(0))));
            gate->setNumInputs(n);
            gate->setDelay(dlg.fieldValue(1));
            if (componentOverlay) componentOverlay->update();
            pushUndoState();
        }
        return;
    }

    ComponentEditDialog dlg(comp->getName(), fields, this);
    if (dlg.exec() == QDialog::Accepted) {
        comp->setName(dlg.label());
        if (componentOverlay) componentOverlay->update();
        pushUndoState();
    }
}


QString schematicPage::projectsDirectory() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Proteus Mini Projects";
    QDir().mkpath(dir);
    return dir;
}

void schematicPage::clearCircuit()
{
    if (selectedWire) selectedWire = nullptr;
    clearSelection();
    cancelWiring();
    pressedPushButton = nullptr;
    hoveredPinComponent = nullptr;
    hoveredPinIndex = -1;

    for (Wire *w : wiresOnboard) delete w;
    wiresOnboard.clear();

    for (Junction *j : junctionsOnboard) delete j;
    junctionsOnboard.clear();

    for (Component *c : componentsOnboard) delete c;
    componentsOnboard.clear();

    if (componentOverlay) componentOverlay->update();
}

// Undo / Redo + serialize


void schematicPage::writeProjectData(QTextStream &out)
{
    out << "PROTEUS_MINI_PROJECT|1\n";
    out << "COMPONENT_COUNT|" << componentsOnboard.size() << "\n";

    for (int i = 0; i < componentsOnboard.size(); ++i) {
        serializeComponent(componentsOnboard[i], out, i);
    }

    out << "WIRE_COUNT|" << wiresOnboard.size() << "\n";
    for (Wire *w : wiresOnboard) {
        int startIdx = componentsOnboard.indexOf(w->startComponent());
        int endIdx = w->isDangling() ? -1 : componentsOnboard.indexOf(w->endComponent());
        Position dangle = w->isDangling() ? w->endScenePos() : Position(0, 0);

        QStringList fields;
        fields << "WIRE"
               << QString::number(startIdx)
               << QString::number(w->startPinIndex())
               << (w->isDangling() ? "1" : "0")
               << QString::number(endIdx)
               << QString::number(w->endPinIndex())
               << QString::number(dangle.x)
               << QString::number(dangle.y)
               << QString::number(w->waypoints().size());
        for (const Position &wp : w->waypoints()) {
            fields << QString::number(wp.x) << QString::number(wp.y);
        }
        out << fields.join('|') << "\n";
    }

    out << "JUNCTION_COUNT|" << junctionsOnboard.size() << "\n";
    for (Junction *j : junctionsOnboard) {
        out << "JUNCTION|" << j->position().x << "|" << j->position().y << "\n";
    }
}

bool schematicPage::readProjectData(QTextStream &in)
{
    QString header = in.readLine();
    if (!header.startsWith("PROTEUS_MINI_PROJECT")) return false;

    QString compCountLine = in.readLine();
    int compCount = compCountLine.split('|').value(1).toInt();

    QVector<Component*> loadedComponents;
    loadedComponents.reserve(compCount);

    for (int i = 0; i < compCount; ++i) {
        QString line = in.readLine();
        QStringList f = line.split('|');
        if (f.size() < 10 || f[0] != "COMPONENT") { loadedComponents.push_back(nullptr); continue; }

        QString code = f[2];
        QString label = f[3];
        int x = f[4].toInt();
        int y = f[5].toInt();
        int orientationInt = f[6].toInt();
        bool flipX = (f[7] == "1");
        bool flipY = (f[8] == "1");
        int extraCount = f[9].toInt();

        QStringList extra;
        for (int k = 0; k < extraCount; ++k) extra << f.value(10 + k);

        Component *comp = createComponentFromCode(code, extra);
        if (comp) {
            comp->setName(label);
            comp->setPosition(Position(x, y));
            comp->setOrientation(static_cast<Orientation>(orientationInt));
            if (flipX) comp->mirrorHorizontal();
            if (flipY) comp->mirrorVertical();
        }
        loadedComponents.push_back(comp);
    }

    QString wireCountLine = in.readLine();
    int wireCount = wireCountLine.split('|').value(1).toInt();

    QVector<Wire*> loadedWires;
    loadedWires.reserve(wireCount);

    for (int i = 0; i < wireCount; ++i) {
        QString line = in.readLine();
        QStringList f = line.split('|');
        if (f.isEmpty() || f[0] != "WIRE") continue;

        int startIdx = f.value(1).toInt();
        int startPin = f.value(2).toInt();
        bool isDangling = (f.value(3) == "1");
        int endIdx = f.value(4).toInt();
        int endPin = f.value(5).toInt();
        int dx = f.value(6).toInt();
        int dy = f.value(7).toInt();
        int wpCount = f.value(8).toInt();

        QVector<Position> waypoints;
        for (int k = 0; k < wpCount; ++k) {
            int wx = f.value(9 + 2 * k).toInt();
            int wy = f.value(10 + 2 * k).toInt();
            waypoints.push_back(Position(wx, wy));
        }

        Component *startComp = (startIdx >= 0 && startIdx < loadedComponents.size())
                                   ? loadedComponents[startIdx] : nullptr;
        if (!startComp) continue;

        Wire *w = nullptr;
        if (isDangling) {
            w = new Wire(startComp, startPin, Position(dx, dy), waypoints, &componentsOnboard);
        } else {
            Component *endComp = (endIdx >= 0 && endIdx < loadedComponents.size())
            ? loadedComponents[endIdx] : nullptr;
            if (!endComp) continue;
            w = new Wire(startComp, startPin, endComp, endPin, waypoints, &componentsOnboard);
        }
        loadedWires.push_back(w);
    }

    QString junctionCountLine = in.readLine();
    int junctionCount = junctionCountLine.split('|').value(1).toInt();

    QVector<Junction*> loadedJunctions;
    for (int i = 0; i < junctionCount; ++i) {
        QString line = in.readLine();
        QStringList f = line.split('|');
        if (f.size() < 3 || f[0] != "JUNCTION") continue;
        loadedJunctions.push_back(new Junction(Position(f[1].toInt(), f[2].toInt())));
    }

    for (Component *c : loadedComponents) {
        if (c) componentsOnboard.push_back(c);
    }
    for (Wire *w : loadedWires) wiresOnboard.push_back(w);
    for (Junction *j : loadedJunctions) junctionsOnboard.push_back(j);

    recalcAllWires();
    if (componentOverlay) componentOverlay->update();
    return true;
}

bool schematicPage::writeProjectToFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "writeProjectToFile failed:" << path << file.errorString();
        return false;
    }
    QTextStream out(&file);
    writeProjectData(out);
    file.close();
    return true;
}

bool schematicPage::loadProjectFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&file);
    bool ok = readProjectData(in);
    file.close();
    return ok;
}

QString schematicPage::serializeCurrentStateToString()
{
    QString data;
    QTextStream out(&data);
    writeProjectData(out);
    return data;
}

bool schematicPage::restoreStateFromString(const QString &data)
{
    clearCircuit();
    QString copy = data;
    QTextStream in(&copy, QIODevice::ReadOnly);
    return readProjectData(in);
}

void schematicPage::pushUndoState()
{
    QString snapshot = serializeCurrentStateToString();

    while (undoHistory.size() > historyIndex + 1) {
        undoHistory.removeLast();
    }

    undoHistory.push_back(snapshot);
    historyIndex = undoHistory.size() - 1;

    saved = false;
    updateUndoRedoActions();
}

void schematicPage::resetUndoHistory()
{
    undoHistory.clear();
    undoHistory.push_back(serializeCurrentStateToString());
    historyIndex = 0;
    updateUndoRedoActions();
}

void schematicPage::updateUndoRedoActions()
{
    if (actUndo) actUndo->setEnabled(historyIndex > 0);
    if (actRedo) actRedo->setEnabled(historyIndex >= 0 && historyIndex < undoHistory.size() - 1);
}

void schematicPage::undoAction()
{
    if (historyIndex <= 0) return;
    historyIndex--;
    restoreStateFromString(undoHistory[historyIndex]);
    updateUndoRedoActions();
}

void schematicPage::redoAction()
{
    if (historyIndex < 0 || historyIndex >= undoHistory.size() - 1) return;
    historyIndex++;
    restoreStateFromString(undoHistory[historyIndex]);
    updateUndoRedoActions();
}


void schematicPage::serializeComponent(Component *c, QTextStream &out, int index)
{
    QString code;
    QStringList extra;

    if (auto *r = dynamic_cast<Resistor*>(c)) {
        code = "RES"; extra << QString::number(r->resistance, 'g', 17);
    } else if (auto *cap = dynamic_cast<Capacitor*>(c)) {
        code = "CAP"; extra << QString::number(cap->capacitance, 'g', 17);
    } else if (auto *l = dynamic_cast<Inductor*>(c)) {
        code = "IND"; extra << QString::number(l->inductance, 'g', 17);
    } else if (auto *bat = dynamic_cast<Battery*>(c)) {
        code = "BAT";
        extra << QString::number(bat->getVoltage(), 'g', 17)
              << QString::number(bat->getInternalResistance(), 'g', 17);
    } else if (auto *dc = dynamic_cast<DC_vol_source*>(c)) {
        code = "DCV"; extra << QString::number(dc->getVoltage(), 'g', 17);
    } else if (auto *clk = dynamic_cast<Clock_gen*>(c)) {
        code = "CLK"; extra << QString::number(clk->getFrequency(), 'g', 17);
    } else if (dynamic_cast<GND*>(c)) {
        code = "GND";
    } else if (auto *sw = dynamic_cast<Switch*>(c)) {
        code = "SW"; extra << (sw->isClosed() ? "1" : "0");
    } else if (dynamic_cast<Push_button*>(c)) {
        code = "BTN";
    } else if (auto *led = dynamic_cast<LED*>(c)) {
        code = "LED"; extra << QString::number(led->Vth, 'g', 17);
    } else if (auto *seg = dynamic_cast<seven_seg*>(c)) {
        code = "SEG"; extra << (seg->hasDP() ? "1" : "0");
    } else if (auto *notg = dynamic_cast<NOTGate*>(c)) {
        code = "NOT"; extra << QString::number(notg->getDelay(), 'g', 17);
    } else if (auto *andg = dynamic_cast<ANDGate*>(c)) {
        code = "AND";
        extra << QString::number(andg->getNumInputs()) << QString::number(andg->getDelay(), 'g', 17);
    } else if (auto *org = dynamic_cast<ORGate*>(c)) {
        code = "OR";
        extra << QString::number(org->getNumInputs()) << QString::number(org->getDelay(), 'g', 17);
    } else if (auto *nandg = dynamic_cast<NANDGate*>(c)) {
        code = "NAND";
        extra << QString::number(nandg->getNumInputs()) << QString::number(nandg->getDelay(), 'g', 17);
    } else if (auto *norg = dynamic_cast<NORGate*>(c)) {
        code = "NOR";
        extra << QString::number(norg->getNumInputs()) << QString::number(norg->getDelay(), 'g', 17);
    } else if (auto *xorg = dynamic_cast<XORGate*>(c)) {
        code = "XOR";
        extra << QString::number(xorg->getNumInputs()) << QString::number(xorg->getDelay(), 'g', 17);
    } else if (auto *xnorg = dynamic_cast<XNORGate*>(c)) {
        code = "XNOR";
        extra << QString::number(xnorg->getNumInputs()) << QString::number(xnorg->getDelay(), 'g', 17);
    } else if (auto *dff = dynamic_cast<DFlipFlop*>(c)) {
        code = "DFF"; extra << QString::number(dff->getDelay(), 'g', 17);
    } else {
        code = "UNKNOWN";
    }

    QString label = c->getName();
    label.replace('|', ' ');

    QStringList fields;
    fields << "COMPONENT" << QString::number(index) << code << label
           << QString::number(c->getPosition().x) << QString::number(c->getPosition().y)
           << QString::number(static_cast<int>(c->getOrientation()))
           << (c->isMirroredHorizontal() ? "1" : "0")
           << (c->isMirroredVertical() ? "1" : "0")
           << QString::number(extra.size());
    fields += extra;

    out << fields.join('|') << "\n";
}

Component* schematicPage::createComponentFromCode(const QString &code, const QStringList &extra)
{
    if (code == "RES")  return new Resistor(extra.value(0).toDouble());
    if (code == "CAP")  return new Capacitor(extra.value(0).toDouble());
    if (code == "IND")  return new Inductor(extra.value(0).toDouble());
    if (code == "BAT")  return new Battery(extra.value(0).toDouble(), extra.value(1).toDouble());
    if (code == "DCV")  return new DC_vol_source(extra.value(0).toDouble());
    if (code == "CLK") {
        auto *c = new Clock_gen(5.0, 1.0);
        c->setFrequency(extra.value(0).toDouble());
        return c;
    }
    if (code == "GND")  return new GND();
    if (code == "SW") {
        auto *sw = new Switch();
        if (extra.value(0) == "1") sw->toggle();
        return sw;
    }
    if (code == "BTN")  return new Push_button();
    if (code == "LED")  return new LED(extra.value(0).toDouble());
    if (code == "SEG")  return new seven_seg(extra.value(0) == "1");
    if (code == "NOT")  return new NOTGate(extra.value(0).toDouble());
    if (code == "AND")  return new ANDGate(extra.value(0).toInt(), extra.value(1).toDouble());
    if (code == "OR")   return new ORGate(extra.value(0).toInt(), extra.value(1).toDouble());
    if (code == "NAND") return new NANDGate(extra.value(0).toInt(), extra.value(1).toDouble());
    if (code == "NOR")  return new NORGate(extra.value(0).toInt(), extra.value(1).toDouble());
    if (code == "XOR")  return new XORGate(extra.value(0).toInt(), extra.value(1).toDouble());
    if (code == "XNOR") return new XNORGate(extra.value(0).toInt(), extra.value(1).toDouble());
    if (code == "DFF")  return new DFlipFlop(extra.value(0).toDouble());

    return nullptr;
}

// ---- Open: ----
void schematicPage::openProject()
{
    QString startDir = currentProjectPath.isEmpty()
    ? projectsDirectory()
    : QFileInfo(currentProjectPath).absolutePath();

    QString path = QFileDialog::getOpenFileName(this, "Open Project", startDir, "Proteus Mini Project (*.txt)");
    if (path.isEmpty()) return;

    if (!loadProjectFile(path)) {
        QMessageBox::warning(this, "Open Failed", "Could not read the project file:\n" + path);
    }
}

bool schematicPage::loadProjectFile(const QString &path)
{
    clearCircuit();
    if (loadProjectFromFile(path)) {
        currentProjectPath = path;
        currentProjectName = QFileInfo(path).completeBaseName();
        saved = true;
        resetUndoHistory();
        return true;
    }
    return false;
}

void schematicPage::takeScreenshot()
{
    if (!schematicCanvas) return;


    QPixmap pixmap = schematicCanvas->grab();

    QString defaultPath = projectsDirectory() + "/" + (currentProjectName.isEmpty() ? "schematic" : currentProjectName) + ".png";

    QString path = QFileDialog::getSaveFileName(this, "Save Screenshot", defaultPath, "PNG Image (*.png);;JPEG Image (*.jpg)");
    if (path.isEmpty()) return;

    if (!pixmap.save(path)) {
        QMessageBox::warning(this, "Screenshot Failed", "Could not save the image.");
    }
}

// ---- Save: ----
void schematicPage::saveProject()
{
    if (currentProjectPath.isEmpty()) {
        saveProjectAs();
        return;
    }

    if (writeProjectToFile(currentProjectPath)) {
        saved = true;
    } else {
        QMessageBox::warning(this, "Save Failed",
                             "Could not save the project file:\n" + currentProjectPath);
    }
}

// ---- Save As:----
void schematicPage::saveProjectAs()
{
    QString defaultDir = currentProjectPath.isEmpty()
    ? projectsDirectory()
    : QFileInfo(currentProjectPath).absolutePath();
    QString defaultName = currentProjectName.isEmpty() ? "Untitled" : currentProjectName;
    QString defaultPath = defaultDir + "/" + defaultName + ".txt";

    QString path = QFileDialog::getSaveFileName(this, "Save Project As", defaultPath,
                                                "Proteus Mini Project (*.txt)");
    if (path.isEmpty()) return;

    if (!path.endsWith(".txt", Qt::CaseInsensitive))
        path += ".txt";

    if (writeProjectToFile(path)) {
        currentProjectPath = path;
        currentProjectName = QFileInfo(path).completeBaseName();
        saved = true;
    } else {
        QMessageBox::warning(this, "Save Failed",
                             "Could not save the project file:\n" + path);
    }
}

void schematicPage::updateButtonStates()
{
    btnRun->setEnabled(!isRunning || isPaused);
    btnPause->setEnabled(isRunning && !isPaused);
    btnStop->setEnabled(isRunning);
    btnRestart->setEnabled(true);
}

void schematicPage::onRun()
{
    if (isRunning && isPaused) {
        isPaused = false;
        simTimer->start();
        statusLabel->setText("Running...");
    } else {
        isRunning = true;
        isPaused = false;
        simTime = 0.0;
        simTimer->start();
        statusLabel->setText("Running...");
    }
    updateButtonStates();
}

void schematicPage::onPause()
{
    if (isRunning && !isPaused) {
        isPaused = true;
        simTimer->stop();
        statusLabel->setText("Paused");
        updateButtonStates();
    }
}

void schematicPage::onStop()
{
    isRunning = false;
    isPaused = false;
    simTimer->stop();
    simTime = 0.0;
    statusLabel->setText("Stopped");
    updateButtonStates();

    for (Wire* w : wiresOnboard) {
        w->resetColor();
    }
    if (schematicCanvas) schematicCanvas->update();
}

void schematicPage::onRestart()
{
    onStop();
    onRun();
}

void schematicPage::advanceSimulation()
{
    double step = 0.01;
    simTime += step;

    timeLabel->setText(QString("t = %1 s").arg(simTime, 0, 'f', 3));

    propagateLogicStates();
    updateWireColors();

    if (schematicCanvas) schematicCanvas->update();

    if (simTime >= 10.0) onStop();
}

Wire* schematicPage::findWireConnectedToPin(const Component* comp, int pinIdx) const
{
    for (Wire* w : wiresOnboard) {
        if (w->startComponent() == comp && w->startPinIndex() == pinIdx)
            return w;
        if (w->endComponent() == comp && w->endPinIndex() == pinIdx)
            return w;
    }
    return nullptr;
}

void schematicPage::propagateLogicStates()
{
    const int maxPasses = componentsOnboard.size() + 1; // enough for feed-forward chains to settle

    for (int pass = 0; pass < maxPasses; ++pass) {
        for (Component *comp : componentsOnboard) {

            if (auto *gate = dynamic_cast<LogicGate*>(comp)) {
                for (int i = 0; i < gate->getNumInputs(); ++i) {
                    LogicState s = LogicState::Undefined;
                    Wire *w = findWireConnectedToPin(gate, i);
                    if (w && !w->isDangling()) {
                        Component *other = (w->startComponent() == gate) ? w->endComponent() : w->startComponent();
                        int otherPin     = (w->startComponent() == gate) ? w->endPinIndex()   : w->startPinIndex();
                        if (other) {
                            int v = other->getPinValue(otherPin);
                            s = (v == 1) ? LogicState::High : (v == 0) ? LogicState::Low : LogicState::Undefined;
                        }
                    }
                    gate->setInputState(i, s);
                }
                gate->update(simTime);
            }
            else if (auto *dff = dynamic_cast<DFlipFlop*>(comp)) {
                auto readPin = [&](int pinIdx) -> LogicState {
                    Wire *w = findWireConnectedToPin(dff, pinIdx);
                    if (!w || w->isDangling()) return LogicState::Undefined;
                    Component *other = (w->startComponent() == dff) ? w->endComponent() : w->startComponent();
                    int otherPin     = (w->startComponent() == dff) ? w->endPinIndex()   : w->startPinIndex();
                    if (!other) return LogicState::Undefined;
                    int v = other->getPinValue(otherPin);
                    return (v == 1) ? LogicState::High : (v == 0) ? LogicState::Low : LogicState::Undefined;
                };
                dff->setD(readPin(0));
                dff->setClock(readPin(1));
                dff->evaluate();
            }
        }
    }
}

void schematicPage::updateWireColors()
{
    for (Wire* w : wiresOnboard) {
        w->clearLogicLevel();
        w->setSimRunning(true);
    }

    for (Component* comp : componentsOnboard) {
        QVector<int> outputs = comp->getOutputPinIndices();
        for (int pinIdx : outputs) {
            int value = comp->getPinValue(pinIdx);
            Wire* wire = findWireConnectedToPin(comp, pinIdx);
            if (wire) {
                wire->setLogicLevel(value);
            }
        }
    }
}