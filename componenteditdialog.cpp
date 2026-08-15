#include "componenteditdialog.h"

#include <QLineEdit>
#include <QLabel>
#include <QFont>
#include <QDoubleValidator>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFrame>

ComponentEditDialog::ComponentEditDialog(const QString &componentLabel,
                                         const QVector<EditField> &fields,
                                         QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Edit Component Properties");
    setModal(true);
    setMinimumWidth(300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFormLayout *topForm = new QFormLayout();
    m_labelEdit = new QLineEdit(componentLabel, this);
    topForm->addRow("Label:", m_labelEdit);
    mainLayout->addLayout(topForm);

    if (!fields.isEmpty()) {
        QFrame *sep = new QFrame(this);
        sep->setFrameShape(QFrame::HLine);
        mainLayout->addWidget(sep);

        QFormLayout *valuesForm = new QFormLayout();
        for (const EditField &f : fields) {
            QLineEdit *edit = new QLineEdit(this);
            QDoubleValidator *v = new QDoubleValidator(-1.0e12, 1.0e12, 12, edit);
            v->setNotation(QDoubleValidator::ScientificNotation);
            edit->setValidator(v);
            edit->setText(QString::number(f.value, 'g', 10));

            QString suffix = f.unit.isEmpty() ? QString() : QString(" (%1)").arg(f.unit);
            valuesForm->addRow(f.label + suffix, edit);

            m_fieldEdits.push_back(edit);
            m_fieldValues.push_back(f.value);
            m_fieldLabels.push_back(f.label);
        }
        mainLayout->addLayout(valuesForm);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ComponentEditDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ComponentEditDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void ComponentEditDialog::accept()
{
    if (m_labelEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Label", "Component label cannot be empty.");
        return;
    }

    for (int i = 0; i < m_fieldEdits.size(); ++i) {
        bool ok = false;
        double v = m_fieldEdits[i]->text().toDouble(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Invalid Value",
                                 QString("Please enter a valid number for %1.").arg(m_fieldLabels[i]));
            return;
        }
        m_fieldValues[i] = v;
    }

    QDialog::accept();
}

QString ComponentEditDialog::label() const { return m_labelEdit->text().trimmed(); }

double ComponentEditDialog::fieldValue(int index) const
{
    if (index < 0 || index >= m_fieldValues.size()) return 0.0;
    return m_fieldValues[index];
}