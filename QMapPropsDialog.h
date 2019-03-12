#ifndef QMAPPROPSDIALOG_H
#define QMAPPROPSDIALOG_H

#include <QDialog>
#include <QVariantMap>

namespace Ui {
class QMapPropsDialog;
}

class QMapPropsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QMapPropsDialog(QWidget *parent = Q_NULLPTR);
    ~QMapPropsDialog();


    const QVariantMap& props() const;
    void setProps(const QVariantMap &props);

private:
    Q_SLOT void readProps();

    static int numberOfDecimals(double d);

    QVariantMap m_props;
    QObjectList m_widgets;

    Ui::QMapPropsDialog *ui;
};

#endif // QMAPPROPSDIALOG_H
