#ifndef SCHEMATICPAGE_H
#define SCHEMATICPAGE_H

#include <QWidget>

namespace Ui {
class schematicPage;
}

class schematicPage : public QWidget
{
    Q_OBJECT

public:
    explicit schematicPage(QWidget *parent = nullptr);
    ~schematicPage();

private:
    Ui::schematicPage *ui;
};

#endif // SCHEMATICPAGE_H
