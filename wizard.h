#ifndef WIZARD_H
#define WIZARD_H

#include <QDialog>

namespace Ui {
class wizard;
}

class wizard : public QDialog
{
    Q_OBJECT

public:
    explicit wizard(QWidget *parent = nullptr);
    ~wizard();

private:
    Ui::wizard *ui;
};

#endif // WIZARD_H
