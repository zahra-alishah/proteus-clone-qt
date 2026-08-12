#include "schematicpage.h"
#include "ui_schematicpage.h"
#include "shematicclass.h"
#include <QMenuBar>
#include <QVBoxLayout>
#include <QToolBar>
#include <QComboBox>
#include <QSplitter>
#include <QToolButton>
#include <QSpinBox>
#include <QLabel>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QListWidget>
#include <QMouseEvent>


schematicPage::schematicPage(QWidget *parent) : QWidget(parent), ui(new Ui::schematicPage)
{
    ui->setupUi(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QMenuBar *myMenuBar = new QMenuBar(this);
    mainLayout->addWidget(myMenuBar);

    QMenu *fileMenu = myMenuBar -> addMenu("&File");
    QMenu *editMenu = myMenuBar -> addMenu("&Edit");
    QMenu *viewMenu = myMenuBar -> addMenu("&View");
    QMenu *toolMenu = myMenuBar -> addMenu("&Tool");
    QMenu *designMenu = myMenuBar -> addMenu("&Design");
    QMenu *graphMenu = myMenuBar -> addMenu("&Graph");
    QMenu *debugMenu = myMenuBar -> addMenu("&Debug");
    QMenu *libraryMenu = myMenuBar -> addMenu("&Library");
    QMenu *templateMenu = myMenuBar -> addMenu("&Template");
    QMenu *systemMenu = myMenuBar -> addMenu("&System");
    QMenu *helpMenu = myMenuBar -> addMenu("&Help");

    QAction *actNew = fileMenu->addAction("&New Project");
    QAction *actOpen = fileMenu->addAction("&Open Project");
    QAction *actSave = fileMenu->addAction("&Save Project");

    fileMenu->addAction("Open Sa&mple Project");
    fileMenu->addAction("Import &Legacy Project");
    fileMenu->addAction("Import &ECAD Files");
    fileMenu->addAction("Save Project &As");
    QAction *actClose = editMenu->addAction("Close Project");
    fileMenu->addSeparator();
    fileMenu->addAction("&Import Image");
    fileMenu->addAction("Import Project &Clip");
    fileMenu->addSeparator();

    QMenu *exportMenu = fileMenu->addMenu("Export &Graphics");
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

    actNew->setShortcut(QKeySequence(Qt::ALT + Qt::Key_N));
    actOpen->setShortcut(QKeySequence(Qt::ALT + Qt::Key_O));
    actSave->setShortcut(QKeySequence(Qt::ALT + Qt::Key_S));
    actExit->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));

    QAction *actUndo = editMenu->addAction("Undo Changes");
    actUndo->setEnabled(false);

    QAction *actRedo = editMenu->addAction("Redo Changes");
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

    actUndo->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
    actRedo->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));
    actAlign->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
    actSendBack->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    actBringFront->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));

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
    actSnap10->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F1));
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
    actExitParent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_X));

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

    actEdit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    actTraces->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    actSim->setShortcut(QKeySequence(Qt::Key_Space));
    actLog->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
    actAudio->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space));

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

    actStart->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F12));
    actPause->setShortcut(QKeySequence(Qt::Key_Pause));
    actStop->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Pause));
    actRun->setShortcut(QKeySequence(Qt::Key_F12));
    actRunNB->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F12));
    actStepOver->setShortcut(QKeySequence(Qt::Key_F10));
    actStepInto->setShortcut(QKeySequence(Qt::Key_F11));
    actStepOut->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F11));
    actRunTo->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F10));
    actAnim->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F11));

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
    myToolBar -> setIconSize(QSize(28, 28));
    myToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    QAction *actHome= editMenu->addAction("Home Page");
    QAction *actSchematic= editMenu->addAction("Schematic Capture");
    QAction *actPCB= editMenu->addAction("PCB Layout");
    QAction *act3D= editMenu->addAction("3D Visualizer");
    QAction *actGerber= editMenu->addAction("Gerber Viewer");
    QAction *actDesign= editMenu->addAction("Design Expolerer");
    QAction *actBill= editMenu->addAction("Bill of Materials");
    QAction *actCode= editMenu->addAction("SOURCE Code");
    QAction *actNotes= editMenu->addAction("Project Notes");
    QAction *actBlockCopy= editMenu->addAction("Block Copy");
    QAction *actBlockMove= editMenu->addAction("Block Move");
    QAction *actBlockDelete= editMenu->addAction("Block Delete");
    QAction *actBlockRotate= editMenu->addAction("Project Rotate");
    QAction *actRulesCheck= editMenu->addAction("Electrical Rules Check");

    myToolBar -> addAction(actNew);
    myToolBar -> addAction(actOpen);
    myToolBar -> addAction(actSave);
    myToolBar -> addAction(actClose);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actHome);
    myToolBar -> addAction(actSchematic);
    myToolBar -> addAction(actPCB);
    myToolBar -> addAction(act3D);
    myToolBar -> addAction(actGerber);
    myToolBar -> addAction(actDesign);
    myToolBar -> addAction(actBill);
    myToolBar -> addAction(actCode);
    myToolBar -> addAction(actNotes);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actOverview);

    actNew -> setIcon(QIcon(":/icons/new.png"));
    actOpen -> setIcon(QIcon(":/icons/open-folder.png"));
    actSave -> setIcon(QIcon(":/icons/personal-data.png"));
    actClose -> setIcon(QIcon(":/icons/room.png"));
    actHome -> setIcon(QIcon(":/icons/home.png"));
    actSchematic -> setIcon(QIcon(":/icons/structure.png"));
    actPCB -> setIcon(QIcon(":/icons/schematic.png"));
    act3D -> setIcon(QIcon(":/icons/3D.png"));
    actGerber -> setIcon(QIcon(":/icons/gerber.png"));
    actDesign -> setIcon(QIcon(":/icons/explorer.png"));
    actBill -> setIcon(QIcon(":/icons/Bill.png"));
    actCode -> setIcon(QIcon(":/icons/code.png"));
    actNotes -> setIcon(QIcon(":/icons/notes.png"));
    actOverview -> setIcon(QIcon(":/icons/overview.png"));

    myToolBar -> addSeparator();
    QComboBox *comboBaseDesign = new QComboBox(this);
    comboBaseDesign->addItem("Base Design");
    myToolBar->addWidget(comboBaseDesign);

    myToolBar -> addSeparator();
    QComboBox *comboRoot = new QComboBox(this);
    comboRoot->addItem("ROOT");
    myToolBar->addWidget(comboRoot);
    myToolBar -> addSeparator();

    myToolBar -> addAction(actRedraw);
    myToolBar -> addAction(actGrid);
    myToolBar -> addAction(actOrigin);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actCenter);
    myToolBar -> addAction(actZoomIn);
    myToolBar -> addAction(actZoomOut);
    myToolBar -> addAction(actZoomAll);
    myToolBar -> addAction(actZoomArea);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actUndo);
    myToolBar -> addAction(actRedo);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actCut);
    myToolBar -> addAction(actCopy);
    myToolBar -> addAction(actPaste);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actBlockCopy);
    myToolBar -> addAction(actBlockMove);
    myToolBar -> addAction(actBlockRotate);
    myToolBar -> addAction(actBlockDelete);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actPick);
    myToolBar -> addAction(actMake);
    myToolBar -> addAction(actPackaging);
    myToolBar -> addAction(actDecompose);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actWire);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actSearch);
    myToolBar -> addAction(actProp);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actNewSheet);
    myToolBar -> addAction(actRemove);
    myToolBar -> addAction(actExitParent);
    myToolBar -> addSeparator();
    myToolBar -> addAction(actRulesCheck);

    actRedraw -> setIcon(QIcon(":/icons/new.png"));
    actGrid -> setIcon(QIcon(":/icons/open-folder.png"));
    actOrigin -> setIcon(QIcon(":/icons/personal-data.png"));
    actRedraw -> setIcon(QIcon(":/icons/new.png"));
    actGrid -> setIcon(QIcon(":/icons/open-folder.png"));
    actOrigin -> setIcon(QIcon(":/icons/personal-data.png"));
    actZoomAll -> setIcon(QIcon(":/icons/open-folder.png"));
    actZoomArea -> setIcon(QIcon(":/icons/personal-data.png"));
    actCut -> setIcon(QIcon(":/icons/personal-data.png"));
    actCopy -> setIcon(QIcon(":/icons/open-folder.png"));
    actPaste -> setIcon(QIcon(":/icons/personal-data.png"));
    actBlockCopy -> setIcon(QIcon(":/icons/personal-data.png"));
    actBlockMove -> setIcon(QIcon(":/icons/personal-data.png"));
    actBlockRotate -> setIcon(QIcon(":/icons/open-folder.png"));
    actBlockDelete -> setIcon(QIcon(":/icons/personal-data.png"));
    actPick -> setIcon(QIcon(":/icons/personal-data.png"));
    actMake -> setIcon(QIcon(":/icons/personal-data.png"));
    actPackaging -> setIcon(QIcon(":/icons/open-folder.png"));
    actDecompose -> setIcon(QIcon(":/icons/personal-data.png"));
    actWire -> setIcon(QIcon(":/icons/personal-data.png"));
    actSearch -> setIcon(QIcon(":/icons/open-folder.png"));
    actProp -> setIcon(QIcon(":/icons/personal-data.png"));
    actNewSheet -> setIcon(QIcon(":/icons/personal-data.png"));
    actRemove -> setIcon(QIcon(":/icons/open-folder.png"));
    actExitParent -> setIcon(QIcon(":/icons/personal-data.png"));
    actRulesCheck -> setIcon(QIcon(":/icons/personal-data.png"));

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(0);

    QWidget *sidebar = new QWidget();
    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(2, 2, 2, 2);
    sideLayout->setSpacing(2);

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

    btnSelection -> setChecked(true);
    btnComponent -> setChecked(false);
    btnJunctionDot -> setChecked(false);
    btnWireLabel -> setChecked(false);
    btnTextScript -> setChecked(false);
    btnBuses -> setChecked(false);
    btnSubCircuit -> setChecked(false);
    btnTerminals -> setChecked(false);
    btnDevicePins -> setChecked(false);
    btnGraph -> setChecked(false);
    btnActivePopUp -> setChecked(false);
    btnGenerator -> setChecked(false);
    btnProbMode -> setChecked(false);
    btnVirtualInstruments -> setChecked(false);
    btn2DGraphicsLine -> setChecked(false);
    btn2DGraphicsBox -> setChecked(false);
    btn2DGraphicsCircle -> setChecked(false);
    btn2DGraphicsArc -> setChecked(false);
    btn2DGraphicsClosedPath -> setChecked(false);
    btn2DGraphicsText -> setChecked(false);
    btn2DGraphicsSymbols -> setChecked(false);
    btn2DGraphicsMarkers -> setChecked(false);
    btnRotateClockwise -> setChecked(false);
    btnRotateAntiClockwise -> setChecked(false);
    btnXMirror -> setChecked(false);
    btnYMirror -> setChecked(false);

    btnSelection -> setCheckable(true);
    btnComponent -> setCheckable(true);
    btnJunctionDot -> setCheckable(true);
    btnWireLabel -> setCheckable(true);
    btnTextScript -> setCheckable(true);
    btnBuses -> setCheckable(true);
    btnSubCircuit -> setCheckable(true);
    btnTerminals -> setCheckable(true);
    btnDevicePins -> setCheckable(true);
    btnGraph -> setCheckable(true);
    btnActivePopUp -> setCheckable(true);
    btnGenerator -> setCheckable(true);
    btnProbMode -> setCheckable(true);
    btnVirtualInstruments -> setCheckable(true);
    btn2DGraphicsLine -> setCheckable(true);
    btn2DGraphicsBox -> setCheckable(true);
    btn2DGraphicsCircle -> setCheckable(true);
    btn2DGraphicsArc -> setCheckable(true);
    btn2DGraphicsClosedPath -> setCheckable(true);
    btn2DGraphicsText -> setCheckable(true);
    btn2DGraphicsSymbols -> setCheckable(true);
    btn2DGraphicsMarkers -> setCheckable(true);
    btnRotateClockwise -> setCheckable(true);
    btnRotateAntiClockwise -> setCheckable(true);
    btnXMirror -> setCheckable(true);
    btnYMirror -> setCheckable(true);

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


    sideLayout -> addWidget(btnSelection);
    sideLayout -> addWidget(btnComponent);
    sideLayout -> addWidget(btnJunctionDot);
    sideLayout -> addWidget(btnWireLabel);
    sideLayout -> addWidget(btnTextScript);
    sideLayout -> addWidget(btnBuses);
    sideLayout -> addWidget(btnSubCircuit);
    sideLayout -> addWidget(btnTerminals);
    sideLayout -> addWidget(btnDevicePins);
    sideLayout -> addWidget(btnGraph);
    sideLayout -> addWidget(btnActivePopUp);
    sideLayout -> addWidget(btnGenerator);
    sideLayout -> addWidget(btnProbMode);
    sideLayout -> addWidget(btnVirtualInstruments);
    sideLayout -> addWidget(btn2DGraphicsLine);
    sideLayout -> addWidget(btn2DGraphicsBox);
    sideLayout -> addWidget(btn2DGraphicsCircle);
    sideLayout -> addWidget(btn2DGraphicsArc);
    sideLayout -> addWidget(btn2DGraphicsClosedPath);
    sideLayout -> addWidget(btn2DGraphicsText);
    sideLayout -> addWidget(btn2DGraphicsSymbols);
    sideLayout -> addWidget(btn2DGraphicsMarkers);
    sideLayout -> addWidget(btnRotateClockwise);
    sideLayout -> addWidget(btnRotateAntiClockwise);

    rotationSpin -> setRange(0, 359);
    rotationSpin -> setSuffix("°");
    rotationSpin -> setValue(0);

    btnSelection -> setToolTip("Selection Mode");
    btnComponent -> setToolTip("Component Mode");
    btnJunctionDot -> setToolTip("Junction Dot Mode");
    btnWireLabel -> setToolTip("Wire Label Mode");
    btnTextScript -> setToolTip("Text Script Mode");
    btnBuses -> setToolTip("Buses Mode");
    btnSubCircuit -> setToolTip("Subcurcuit Mode");
    btnTerminals -> setToolTip("Terminal Mode");
    btnDevicePins -> setToolTip("Device Pins Mode");
    btnGraph -> setToolTip("Graph Mode");
    btnActivePopUp -> setToolTip("Active Popup Mode");
    btnGenerator -> setToolTip("Generator Mode");
    btnProbMode -> setToolTip("Probe Mode");
    btnVirtualInstruments -> setToolTip("Virtual Instruments Mode");
    btn2DGraphicsLine -> setToolTip("2D Graphics Line Mode");
    btn2DGraphicsBox -> setToolTip("2D Graphics Box Mode");
    btn2DGraphicsCircle -> setToolTip("2D Graphics Circle Mode");
    btn2DGraphicsArc -> setToolTip("2D Graphics Arc Mode");
    btn2DGraphicsClosedPath -> setToolTip("2D Graphics Closed Path Mode");
    btn2DGraphicsText -> setToolTip("2D Graphics Text Mode");
    btn2DGraphicsSymbols -> setToolTip("2D Graphics Symbols Mode");
    btn2DGraphicsMarkers -> setToolTip("2D Graphics Markers Mode");
    btnRotateClockwise -> setToolTip("Rotate Clockwise Mode");
    btnRotateAntiClockwise -> setToolTip("Rotate Anti-Clockwise Mode");
    btnXMirror -> setToolTip("X-Mirror Mode");
    btnYMirror -> setToolTip("Y-Mirror Mode");



    //INDEX 0
    QWidget *widgetDevices = new QWidget();
    QVBoxLayout *layoutDevices = new QVBoxLayout(widgetDevices);
    layoutDevices->setContentsMargins(0, 0, 0, 0);
    QLabel *headerDev = new QLabel("P L    DEVICES");
    headerDev->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutDevices->addWidget(headerDev);
    QWidget *whiteSpace = new QWidget();
    whiteSpace->setStyleSheet("background-color: white;");
    layoutDevices->addWidget(whiteSpace);

    modeStack->addWidget(widgetDevices);

    //INDEX1
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
        "QListWidget::item:selected { background-color: #4a90e2; color: white; }"
        );
    layoutPorts->addWidget(listPorts);

    modeStack->addWidget(widgetPorts);

    //INDEX2
    QWidget *widgetTerminals = new QWidget();
    QVBoxLayout *layoutTerminals = new QVBoxLayout(widgetTerminals);
    layoutTerminals->setContentsMargins(0, 0, 0, 0);
    layoutTerminals->setSpacing(0);
    QLabel *headerTerminals = new QLabel("P    Terminals");
    headerTerminals->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutTerminals->addWidget(headerTerminals);
    QComboBox *comboTerminals = new QComboBox();
    comboTerminals->addItems({"DEFAULT", "INPUT", "OUTPUT", "BIDIR", "POWER", "GROUND", "CHASSIS", "RETURN", "DYNAMIC", "TESTPOINT", "NC"});
    comboTerminals->setStyleSheet("QComboBox { background-color: white; border: none; padding: 4px; }");
    layoutTerminals->addWidget(comboTerminals);

    modeStack->addWidget(widgetTerminals);

    //INDEX3
    QWidget *widgetPins = new QWidget();
    QVBoxLayout *layoutPins = new QVBoxLayout(widgetPins);
    layoutPins->setContentsMargins(0, 0, 0, 0);
    layoutPins->setSpacing(0);
    QLabel *headerPins = new QLabel("P    Pins");
    headerPins->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutPins->addWidget(headerPins);
    QComboBox *comboPins = new QComboBox();
    comboPins->addItems({"DEFAULT", "INVERT", "POSCLK", "NEGCLK", "SHORT", "BUS", "INSNEGOP"});
    comboPins->setStyleSheet("QComboBox { background-color: white; border: none; padding: 4px; }");
    layoutPins->addWidget(comboPins);

    modeStack->addWidget(widgetPins);

    //INDEX4
    QWidget *widgetGraphs = new QWidget();
    QVBoxLayout *layoutGraphs = new QVBoxLayout(widgetGraphs);
    layoutGraphs->setContentsMargins(0, 0, 0, 0);
    layoutGraphs->setSpacing(0);
    QLabel *headerGraphs = new QLabel("Graphs");
    headerGraphs->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutGraphs->addWidget(headerGraphs);
    QComboBox *comboGraphs = new QComboBox();
    comboGraphs->addItems({"ANALOGUE", "DIGITAL", "MIXED", "FREQUENCY", "TRASFER", "NOISE", "DISTORTION", "BUSFOURIER", "INTERACTIVE", "CONFORMANCE", "DC SWEEP", "AC SWEEP"});
    comboGraphs->setStyleSheet("QComboBox { background-color: white; border: none; padding: 4px; }");
    layoutGraphs->addWidget(comboGraphs);

    modeStack->addWidget(widgetGraphs);

    //INDEX5
    QWidget *widgetGenerators = new QWidget();
    QVBoxLayout *layoutGenerators = new QVBoxLayout(widgetGenerators);
    layoutGenerators->setContentsMargins(0, 0, 0, 0);
    layoutGenerators->setSpacing(0);
    QLabel *headerGenerators = new QLabel("Generators");
    headerGenerators->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutGenerators->addWidget(headerGenerators);
    QComboBox *comboGenerators = new QComboBox();
    comboGenerators->addItems({"C", "SIN", "PULSE", "EXP", "SFFM", "PWLIN", "FILE", "AUDIO", "RANDOM", "DSTATE", "DEDGE", "DPULSE", "DCLOCK", "DPATTERN", "SCRIPTABLE"});
    comboGenerators->setStyleSheet("QComboBox { background-color: white; border: none; padding: 4px; }");
    layoutGenerators->addWidget(comboGenerators);

    modeStack->addWidget(widgetGenerators);

    //INDEX6
    QWidget *widgetProbes = new QWidget();
    QVBoxLayout *layoutProbes = new QVBoxLayout(widgetProbes);
    layoutProbes->setContentsMargins(0, 0, 0, 0);
    layoutProbes->setSpacing(0);
    QLabel *headerProbes = new QLabel("Probes");
    headerProbes->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutProbes->addWidget(headerProbes);
    QComboBox *comboProbes = new QComboBox();
    comboProbes->addItems({"VOLTAGE", "CURRENT", "TAPE"});
    comboProbes->setStyleSheet("QComboBox { background-color: white; border: none; padding: 4px; }");
    layoutProbes->addWidget(comboProbes);

    modeStack->addWidget(widgetProbes);

    //INDEX7
    QWidget *widgetInstruments = new QWidget();
    QVBoxLayout *layoutInstruments = new QVBoxLayout(widgetInstruments);
    layoutInstruments->setContentsMargins(0, 0, 0, 0);
    layoutInstruments->setSpacing(0);
    QLabel *headerInstruments = new QLabel("Instruments");
    headerInstruments->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutInstruments->addWidget(headerInstruments);
    QComboBox *comboInstruments = new QComboBox();
    comboInstruments->addItems({"OSCILLISCOPE", "LOGIC ANALYSER", "COUNTER TIMER", "VIRTUAL TERMINAL", "SPI DEBUGGER", "I2C DEBUGGER", "SIGNAL GENERATOR", "PATTERN GENERATOR", "DC VOLTMETER", "DC AMMETER", "AC VOLTMETER", "AC AMMETER", "WATTMETER"});
    comboInstruments->setStyleSheet("QComboBox { background-color: white; border: none; padding: 4px; }");
    layoutInstruments->addWidget(comboInstruments);

    modeStack->addWidget(widgetInstruments);

    //INDEX8
    QWidget *widgetGraphics = new QWidget();
    QVBoxLayout *layoutGraphics = new QVBoxLayout(widgetGraphics);
    layoutGraphics->setContentsMargins(0, 0, 0, 0);
    layoutGraphics->setSpacing(0);
    QLabel *headerGraphics = new QLabel("C E  Graphics");
    headerGraphics->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutGraphics->addWidget(headerGraphics);
    QComboBox *comboGraphics = new QComboBox();
    comboGraphics->addItems({"COMPONENT", "PIN", "PORT", "MARKER", "ACTUATOR", "INDICATOR", "VPROBE", "IPROBE", "TAPE", "GENERATOR", "TERMINAL", "SUBCIRCUIT", "2D GRAPHIC", "WIRE DOT", "WIRE", "BUS WIRE", "BORDER", "TEMPLATE"});
    comboGraphics->setStyleSheet("QComboBox { background-color: white; border: none; padding: 4px; }");
    layoutGraphics->addWidget(comboGraphics);

    modeStack->addWidget(widgetGraphics);

    //INDEX9
    QWidget *widgetMarkers = new QWidget();
    QVBoxLayout *layoutMarkers = new QVBoxLayout(widgetMarkers);
    layoutMarkers->setContentsMargins(0, 0, 0, 0);
    layoutMarkers->setSpacing(0);
    QLabel *headerMarkers = new QLabel("Markers");
    headerMarkers->setStyleSheet("background-color: #b7d5f5; font-weight: bold; padding: 2px;");
    layoutMarkers->addWidget(headerMarkers);
    QComboBox *comboMarkers = new QComboBox();
    comboMarkers->addItems({"ORIGIN", "NODE", "BUSNODE", "LABEL", "DEVICEREF", "DEVICEVAL", "PINNAME", "PINNUM", "INCREMENT", "DECREMENT", "TOGGLE", "GRID"});
    comboMarkers->setStyleSheet("QComboBox { background-color: white; border: none; padding: 4px; }");
    layoutMarkers->addWidget(comboMarkers);

    modeStack->addWidget(widgetMarkers);

    connect(btnSelection, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(0);
        }
    });

    connect(btnComponent, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(0);
        }
    });

    connect(btnJunctionDot, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(0);
        }
    });

    connect(btn2DGraphicsSymbols, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(0);
        }
    });

    connect(btnSubCircuit, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(1);
        }
    });

    connect(btnTerminals, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(2);
        }
    });

    connect(btnDevicePins, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(3);
        }
    });

    connect(btnGraph, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(4);
        }
    });

    connect(btnGenerator, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(5);
        }
    });

    connect(btnProbMode, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(6);
        }
    });

    connect(btnVirtualInstruments, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(7);
        }
    });

    connect(btn2DGraphicsBox, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(8);
        }
    });

    connect(btn2DGraphicsCircle, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(8);
        }
    });

    connect(btn2DGraphicsArc, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(8);
        }
    });

    connect(btn2DGraphicsMarkers, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(9);
        }
    });

    connect(btn2DGraphicsClosedPath, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(8);
        }
    });

    connect(btn2DGraphicsText, &QToolButton::toggled, this, [=](bool checked){
        if(checked){
            modeStack -> setCurrentIndex(8);
        }
    });

    sideLayout->addWidget(rotationSpin);
    sideLayout -> addWidget(btnXMirror);
    sideLayout -> addWidget(btnYMirror);

    // --- Build the canvas and bottom bar ---
    shematicClass *schematicCanvas = new shematicClass();
    QWidget *bottomBar = new QWidget();
    bottomBar->setFixedHeight(30);
    bottomBar->setStyleSheet("background-color: #f0f0f0; border-top: 1px solid #d0d0d0;");
    bottomBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); // Prevents stretching

    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(5, 2, 5, 2);
    bottomLayout->setSpacing(15);

    QLabel *statusMessage = new QLabel("No Messages");
    QLabel *sheetLabel = new QLabel("ROOT - Root sheet 1");
    QLabel *coordsLabel = new QLabel("x: 0.0 y: 0.0");
    coordsLabel->setFixedWidth(100);
    coordsLabel->setStyleSheet("font-family: monospace; font-size: 12px; color: #333;");

    bottomLayout->addWidget(statusMessage);
    bottomLayout->addWidget(sheetLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(coordsLabel);

    // Connect coordinates
    connect(schematicCanvas, &shematicClass::mouseCoordinatesChanged, this, [=](QString coords){
        coordsLabel->setText(coords);
    });

    // --- Add everything to the Main Layout (PROPERLY) ---
    splitter->addWidget(sidebar);
    splitter->addWidget(schematicCanvas);

    // Remove the layout border so splitter doesn't overlap the bottom bar
    splitter->setStyleSheet("QSplitter { border: none; }");

    // NOTE: use 0 margins! The stretch factors will handle the spacing.
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 0 = Fixed size, 1 = takes all extra space
    mainLayout->addWidget(myMenuBar, 0);
    mainLayout->addWidget(myToolBar, 0);
    mainLayout->addWidget(splitter, 1);
    mainLayout->addWidget(bottomBar, 0);




}

schematicPage::~schematicPage()
{
    delete ui;
}


