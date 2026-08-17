#include "wizard.h"
#include <QFileDialog>
#include <QIcon>
#include <QStandardPaths>
#include "ui_wizard.h"
#include <QDesktopServices>
#include <QUrl>

wizard::wizard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::wizard)
{
    ui->setupUi(this);

    setWindowTitle("New Project Wizard | Summary");
    setWindowIcon(QIcon(":/icons/proteus.svg"));

    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    ui->lineEdit_path->setText(defaultPath);
    ui->lineEdit_name->setText("New Project.pdsp");
    ui->radioButton_newProject->setChecked(true);

    ui->stackedWidget->setCurrentIndex(0);
    ui->pushButton_back->setVisible(false);
    ui->pushButton_back_3->setVisible(false);
    ui->pushButton_back_4->setVisible(false);
    ui->pushButton_back_5->setVisible(false);
    ui->pushButton_back_2->setVisible(false);

    connect(ui->pushButton_next, &QPushButton::clicked, this, [=]() {
        QString fullPath = ui->lineEdit_path->text() + "/" + ui->lineEdit_name->text() + ".pdsprj";
        ui->label_address->setText("Saving As: " + fullPath);

        ui->stackedWidget->setCurrentWidget(ui->page_2);
        ui->pushButton_back->setVisible(false);
        ui->pushButton_back_3->setVisible(true);
        ui->pushButton_back_4->setVisible(false);
        ui->pushButton_back_5->setVisible(false);
        ui->pushButton_back_2->setVisible(false);

        ui->pushButton_next->setVisible(false);
        ui->pushButton_next_3->setVisible(true);
        ui->pushButton_next_4->setVisible(false);
        ui->pushButton_next_5->setVisible(false);
        ui->pushButton_next_2->setVisible(false);
    });

    connect(ui->pushButton_next_3, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->page_4);

        ui->pushButton_back->setVisible(false);
        ui->pushButton_back_3->setVisible(false);
        ui->pushButton_back_4->setVisible(true);
        ui->pushButton_back_5->setVisible(false);
        ui->pushButton_back_2->setVisible(false);

        ui->pushButton_next->setVisible(false);
        ui->pushButton_next_3->setVisible(false);
        ui->pushButton_next_4->setVisible(true);
        ui->pushButton_next_5->setVisible(false);
        ui->pushButton_next_2->setVisible(false);
    });

    connect(ui->pushButton_next_4, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->page_5);

        ui->pushButton_back->setVisible(false);
        ui->pushButton_back_3->setVisible(false);
        ui->pushButton_back_4->setVisible(false);
        ui->pushButton_back_5->setVisible(true);
        ui->pushButton_back_2->setVisible(false);

        ui->pushButton_next->setVisible(false);
        ui->pushButton_next_3->setVisible(false);
        ui->pushButton_next_4->setVisible(false);
        ui->pushButton_next_5->setVisible(true);
        ui->pushButton_next_2->setVisible(false);
    });

    connect(ui->pushButton_next_5, &QPushButton::clicked, this, [=]() {

        ui->stackedWidget->setCurrentWidget(ui->page_3);

        ui->pushButton_back->setVisible(false);
        ui->pushButton_back_3->setVisible(false);
        ui->pushButton_back_4->setVisible(false);
        ui->pushButton_back_5->setVisible(false);
        ui->pushButton_back_2->setVisible(true);

        ui->pushButton_next->setVisible(false);
        ui->pushButton_next_3->setVisible(false);
        ui->pushButton_next_4->setVisible(false);
        ui->pushButton_next_5->setVisible(false);
        ui->pushButton_next_2->setVisible(true);


    });

    connect(ui->pushButton_back_3, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->page);

        ui->pushButton_back->setVisible(false);
        ui->pushButton_back_3->setVisible(false);
        ui->pushButton_back_4->setVisible(false);
        ui->pushButton_back_5->setVisible(false);
        ui->pushButton_back_2->setVisible(false);

        ui->pushButton_next->setVisible(true);
        ui->pushButton_next_3->setVisible(false);
        ui->pushButton_next_4->setVisible(false);
        ui->pushButton_next_5->setVisible(false);
        ui->pushButton_next_2->setVisible(false);
    });

    connect(ui->pushButton_back_4, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->page_2);

        ui->pushButton_back->setVisible(false);
        ui->pushButton_back_3->setVisible(true);
        ui->pushButton_back_4->setVisible(false);
        ui->pushButton_back_5->setVisible(false);
        ui->pushButton_back_2->setVisible(false);

        ui->pushButton_next->setVisible(false);
        ui->pushButton_next_3->setVisible(true);
        ui->pushButton_next_4->setVisible(false);
        ui->pushButton_next_5->setVisible(false);
        ui->pushButton_next_2->setVisible(false);
    });

    connect(ui->pushButton_back_5, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->page_4);

        ui->pushButton_back->setVisible(false);
        ui->pushButton_back_3->setVisible(false);
        ui->pushButton_back_4->setVisible(true);
        ui->pushButton_back_5->setVisible(false);
        ui->pushButton_back_2->setVisible(false);

        ui->pushButton_next->setVisible(false);
        ui->pushButton_next_3->setVisible(false);
        ui->pushButton_next_4->setVisible(true);
        ui->pushButton_next_5->setVisible(false);
        ui->pushButton_next_2->setVisible(false);
    });

    connect(ui->pushButton_back_2, &QPushButton::clicked, this, [=]() {

        ui->stackedWidget->setCurrentWidget(ui->page_5);

        ui->pushButton_back->setVisible(false);
        ui->pushButton_back_3->setVisible(false);
        ui->pushButton_back_4->setVisible(false);
        ui->pushButton_back_5->setVisible(true);
        ui->pushButton_back_2->setVisible(false);

        ui->pushButton_next->setVisible(false);
        ui->pushButton_next_3->setVisible(false);
        ui->pushButton_next_4->setVisible(false);
        ui->pushButton_next_5->setVisible(true);
        ui->pushButton_next_2->setVisible(false);


    });


    connect(ui->pushButton_brwoes, &QPushButton::clicked, this, [=]() {
        QString dir = QFileDialog::getExistingDirectory(this,
                                                        "Select Project Directory",
                                                        ui->lineEdit_path->text());
        if (!dir.isEmpty()) {
            ui->lineEdit_path->setText(dir);
        }
    });

    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->pushButton_cancel_2, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->pushButton_cancel_3, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->pushButton_cancel_4, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->pushButton_cancel_5, &QPushButton::clicked, this, &QDialog::reject);

    connect(ui->pushButton_next_2, &QPushButton::clicked, this, &QDialog::accept);

    connect(ui->pushButton_help, &QPushButton::clicked, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://ble.ir/mahdi_ghalami_n84"));
    });
    connect(ui->pushButton_help_2, &QPushButton::clicked, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://ble.ir/mahdi_ghalami_n84"));
    });
    connect(ui->pushButton_help_3, &QPushButton::clicked, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://ble.ir/mahdi_ghalami_n84"));
    });
    connect(ui->pushButton_help_4, &QPushButton::clicked, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://ble.ir/mahdi_ghalami_n84"));
    });
    connect(ui->pushButton_help_5, &QPushButton::clicked, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://ble.ir/mahdi_ghalami_n84"));
    });
}

wizard::~wizard()
{
    delete ui;
}
