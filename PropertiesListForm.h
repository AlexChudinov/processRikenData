#ifndef RIKENDATAHEADERFORM_H
#define RIKENDATAHEADERFORM_H

#include <QWidget>

namespace Ui {
class RikenDataHeaderForm;
}

class RikenDataHeaderForm : public QWidget
{
    Q_OBJECT

public:
    explicit RikenDataHeaderForm(QWidget *parent = 0);
    ~RikenDataHeaderForm();

    void addListEntry(const QString& entry);

    void clearList();

private:
    Ui::RikenDataHeaderForm *ui;
};

#endif // RIKENDATAHEADERFORM_H
