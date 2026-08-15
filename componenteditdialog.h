#ifndef COMPONENTEDITDIALOG_H
#define COMPONENTEDITDIALOG_H

#include <QDialog>
#include <QVector>
#include <QString>

class QLineEdit;

struct EditField {
    QString label;
    double value;
    QString unit;
};

class ComponentEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ComponentEditDialog(const QString &componentLabel,
                                 const QVector<EditField> &fields,
                                 QWidget *parent = nullptr);

    QString label() const;
    double fieldValue(int index) const;

protected:
    void accept() override;

private:
    QLineEdit *m_labelEdit = nullptr;
    QVector<QLineEdit*> m_fieldEdits;
    QVector<QString> m_fieldLabels;
    QVector<double> m_fieldValues;
};

#endif // COMPONENTEDITDIALOG_H