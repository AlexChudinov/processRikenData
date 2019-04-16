#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QPointer>

namespace Ui {
class MainWindow;
}

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

    void msg(const QString& msg);

    void on_actionOpenDataFile_triggered();

    void on_actionOpen_txt_from_folder_triggered();

    void on_actionTime_params_triggered();

    void on_actionopenManyBinFiles_triggered();

    void createDataPlot(QVector<double> x, QVector<double> y, QString capture, QString xLabel);
private:
    Ui::MainWindow *ui;

    void openRikenDataFile(const QString& fileName);

    void openRikenASCIIData(const QString& fileName);

    void openRikenDataFiles(const QStringList& fileNames);

    void createTicAndMsGraphs();

    void openSpamsFile(const QString& fileName);
private:
    QPointer<QProgressBar> mProgressBar;
};

#endif // MAINWINDOW_H
