#include "wizard.h"
#include <QFileDialog>
#include <QIcon>
#include <QStandardPaths>
#include "ui_wizard.h"

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
    ui->pushButton_back_2->setVisible(false);

    connect(ui->pushButton_next, &QPushButton::clicked, this, [=]() {
        QString fullPath = ui->lineEdit_path->text() + "/" + ui->lineEdit_name->text() + ".pdsprj";
        ui->label_address->setText("Saving As: " + fullPath);

        ui->stackedWidget->setCurrentIndex(1);
        ui->pushButton_next->setVisible(false);
        ui->pushButton_back->setVisible(false);
        ui->pushButton_back_2->setVisible(true);
        ui->pushButton_next_2->setVisible(true);
        ui->pushButton_next_2->setText("Finish");
    });

    connect(ui->pushButton_back_2, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(0);

        ui->pushButton_next->setVisible(true);
        ui->pushButton_next->setText("Next");
        ui->pushButton_back_2->setVisible(false);
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

    connect(ui->pushButton_next_2, &QPushButton::clicked, this, &QDialog::accept);
}

wizard::~wizard()
{
    delete ui;
}
