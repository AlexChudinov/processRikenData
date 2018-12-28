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
class MSPlot;
class QMdiSubWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();

private slots:
    void on_actionTileSubWindows_triggered();

    void on_actionReaccumulate_mass_spectra_triggered();

    void on_actionCredits_triggered();

    void on_actionAbout_triggered();

private:
    Q_SLOT void msg(const QString& msg);
    Q_SLOT void on_actionOpenDataFile_triggered();

    Ui::MainWindow *ui;

    QVector<QMdiSubWindow*> mSubWindows;
    void clearSubwindows();

    void openRikenDataFile(const QString& fileName);

    void createTicAndMsGraphs();
};

#endif // MAINWINDOW_H
