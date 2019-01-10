#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
    Ui::MainWindow *ui;

    void openRikenDataFile(const QString& fileName);

    void createTicAndMsGraphs();
};

#endif // MAINWINDOW_H
