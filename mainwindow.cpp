#include "mainwindow.h"
#include <QDialog>
#include <QFileDialog>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include "schematicpage.h"
#include "ui_mainwindow.h"
#include "wizard.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Untitled - Proteus Professional - Home Page");
    setWindowIcon(QIcon(":/icons/proteus.svg"));

    myTabs = new QTabWidget(this);
    myTabs->addTab(new QWidget(), "Home Page");
    myTabs->setTabsClosable(true);

    setCentralWidget(myTabs);
    myTabs->installEventFilter(this);

    connect(myTabs, &QTabWidget::tabCloseRequested, myTabs, &QTabWidget::removeTab);

    ui->actionNew_Project->setShortcut(QKeySequence(Qt::ALT + Qt::Key_N));
    ui->actionOpen_Project->setShortcut(QKeySequence(Qt::ALT + Qt::Key_O));
    ui->actionSave_Project->setShortcut(QKeySequence(Qt::ALT + Qt::Key_S));
    ui->actionExit_Application->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));
    ui->actionCheck_for_Updates->setShortcut(QKeySequence(Qt::ALT + Qt::Key_U));
    ui->actionProteus->setShortcut(QKeySequence(Qt::Key_F1));

    QToolBar *myToolBar = new QToolBar(this);
    myToolBar->setIconSize(QSize(28, 28));
    myToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    myToolBar->addAction(ui->actionNew_Project);
    myToolBar->addAction(ui->actionOpen_Project);
    myToolBar->addAction(ui->actionSave_Project);
    myToolBar->addAction(ui->actionClose_Project);
    myToolBar->addAction(ui->actionHome_Page);
    myToolBar->addSeparator();
    myToolBar->addAction(ui->actionSchematic_Capture);
    myToolBar->addAction(ui->actionPCB_Layout);
    myToolBar->addAction(ui->actionSource_Code);
    myToolBar->addAction(ui->actionDesign_Explorer);
    myToolBar->addAction(ui->actionDevice_Library_Manager);
    myToolBar->addAction(ui->actionProject_Notes);
    myToolBar->addAction(ui->action3D_Visualizer);
    myToolBar->addAction(ui->actionGerber_Viewer);
    myToolBar->addAction(ui->actionBill_of_Materials);
    myToolBar->addSeparator();
    myToolBar->addAction(ui->actionOverview);
    myToolBar->addAction(ui->actionProPilot_AI_Assistant);

    ui->actionNew_Project->setIcon(QIcon(":/icons/new.png"));
    ui->actionOpen_Project->setIcon(QIcon(":/icons/open-folder.png"));
    ui->actionSave_Project->setIcon(QIcon(":/icons/personal-data.png"));
    ui->actionClose_Project->setIcon(QIcon(":/icons/room.png"));
    ui->actionHome_Page->setIcon(QIcon(":/icons/home.png"));
    ui->actionSchematic_Capture->setIcon(QIcon(":/icons/structure.png"));
    ui->actionPCB_Layout->setIcon(QIcon(":/icons/schematic.png"));
    ui->actionSource_Code->setIcon(QIcon(":/icons/code.png"));
    ui->actionDesign_Explorer->setIcon(QIcon(":/icons/explorer.png"));
    ui->actionDevice_Library_Manager->setIcon(QIcon(":/icons/symbol.png"));
    ui->actionProject_Notes->setIcon(QIcon(":/icons/notes.png"));
    ui->action3D_Visualizer->setIcon(QIcon(":/icons/3D.png"));
    ui->actionGerber_Viewer->setIcon(QIcon(":/icons/gerber.png"));
    ui->actionBill_of_Materials->setIcon(QIcon(":/icons/Bill.png"));
    ui->actionOverview->setIcon(QIcon(":/icons/overview.png"));
    ui->actionProPilot_AI_Assistant->setIcon(QIcon(":/icons/AI.png"));

    addToolBar(Qt::TopToolBarArea, myToolBar);

    connect(ui->actionNew_Project, &QAction::triggered, this, &MainWindow::onNewProject);
    connect(ui->actionOpen_Project, &QAction::triggered, this, &MainWindow::onOpenProject);

    connect(myTabs, &QTabWidget::currentChanged, this, [=](int index) {
        if (index == 0) {
            myToolBar->setVisible(true);
        } else {
            myToolBar->setVisible(false);
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow ::onNewProject()
{
    QDialog *initDialog = new QDialog(this);
    initDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    initDialog->setModal(true);
    initDialog->setFixedSize(350, 80);

    QVBoxLayout *initLayout = new QVBoxLayout(initDialog);
    QLabel *initLabel = new QLabel("Initialising the project wizard...", initDialog);
    initLabel->setAlignment(Qt::AlignCenter);
    initLayout->addWidget(initLabel);
    initDialog->setStyleSheet("QDialog { background-color: #f0f0f0; border: 2px solid #FFFFFF; } "
                              "QLabel { color: #000000; }");
    initDialog->show();
    QCoreApplication::processEvents();

    QTimer::singleShot(1500, this, [=] {
        initDialog->accept();
        initDialog->deleteLater();

        wizard myWizard(this);
        int result = myWizard.exec();
        qDebug() << "Wizard Result:" << result;
        if (result == QDialog::Accepted) {
            schematicPage *mySchematicPage = new schematicPage();
            connect(mySchematicPage, &schematicPage::homeRequested, this, [this]() {
                myTabs->setCurrentIndex(0);
            });

            myTabs->addTab(mySchematicPage, "Schematic Capture");
            myTabs->setCurrentIndex(myTabs->count() - 1);

            statusBar()->showMessage("ROOT - Root sheet 1");
        }
    });
}

void MainWindow::onOpenProject()
{
    QString projectsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
    + "/Proteus Mini Projects";
    QDir().mkpath(projectsDir);

    QString path = QFileDialog::getOpenFileName(this, "Open Project", projectsDir,
                                                "Proteus Mini Project (*.txt)");
    if (path.isEmpty()) return;

    schematicPage *mySchematicPage = new schematicPage();
    connect(mySchematicPage, &schematicPage::homeRequested, this, [this]() {
        myTabs->setCurrentIndex(0);
    });
    if (mySchematicPage->loadProjectFile(path)) {
        QString tabName = QFileInfo(path).completeBaseName();
        myTabs->addTab(mySchematicPage, tabName);
        myTabs->setCurrentIndex(myTabs->count() - 1);
        statusBar()->showMessage("Opened: " + tabName);
    } else {
        delete mySchematicPage;
        QMessageBox::warning(this, "Open Failed", "Could not read the project file:\n" + path);
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == myTabs && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent) {
            if (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right) {
                return true;\
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}