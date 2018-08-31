#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>

namespace Ui {
class MainWindow;
}

class PlotForm;
class RawRikenData;
class PropertiesListForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionCredits_triggered();

private:

    Q_SLOT void on_actionTimeShiftAcc_triggered();
    Q_SLOT void on_actionTileSubWindows_triggered();
    Q_SLOT void on_actionSimpleMassSpecAcc_triggered();
    Q_SLOT void on_actionShowDataHeader_triggered();
    Q_SLOT void on_actionShowDataProperties_triggered();
    Q_SLOT void on_actionOpenRikenDataFile_triggered();
    Q_SLOT void msg(const QString& msg);

    Ui::MainWindow *ui;

    QString m_strRikenFilePath;
    QString m_strRikenFileName;
    RawRikenData* m_pData;

    void readRikenDataFile();
    void disableRikenDataFileActions();
    void enableRikenDataFileActions();
    void plotSubwindow(PlotForm *plot);

    QPointer<PropertiesListForm> m_pHeaderForm;
    QPointer<PropertiesListForm> m_pPropForm;
};

#endif // MAINWINDOW_H
