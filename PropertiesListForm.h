#ifndef RIKENDATAHEADERFORM_H
#define RIKENDATAHEADERFORM_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
class PropertiesListForm;
}

class PropertiesListForm : public QWidget
{
    Q_OBJECT

public:
    explicit PropertiesListForm(QWidget *parent = 0);
    ~PropertiesListForm();

    void addListEntry(const QString& entry);

    void clearList();

    Q_SIGNAL void itemChosen(const QString&);
private:
    Ui::PropertiesListForm *ui;

    Q_SLOT void emitItemStringFromIdx(QModelIndex idx);
};

#endif // RIKENDATAHEADERFORM_H
