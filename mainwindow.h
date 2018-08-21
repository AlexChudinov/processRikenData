#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>

namespace Ui {
class MainWindow;
}

class RawRikenData;
class RikenDataHeaderForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionSimpleMassSpecAcc_triggered();

private:
    Q_SLOT void on_actionShowDataHeader_triggered();
    Q_SLOT void on_actionShowDataProperties_triggered();
    Q_SLOT void on_actionOpenRikenDataFile_triggered();

private:
    Ui::MainWindow *ui;

    QString m_StrRikenFileName;
    RawRikenData* m_pData;

    void readRikenDataFile();
    void msg(const QString& msg);

    QPointer<RikenDataHeaderForm> m_pHeaderForm;
    QPointer<RikenDataHeaderForm> m_pPropForm;
};

#endif // MAINWINDOW_H
