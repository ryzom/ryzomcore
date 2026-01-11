#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QProcess>
#include <QFileDialog>

#include "ui_mainwindow.h"

namespace Ui
{
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
    explicit MainWindow(QWidget * parent = 0);
    ~MainWindow();

  public slots:
    void onButtonListBrowseClicked();
    void onButtonListClicked();
    void onProcessListComplete();

    void onButtonSearchBrowseClicked();
    void onButtonSearchClicked();
    void onProcessSearchComplete();

    void onButtonPackBrowseClicked();
    void onButtonPackClicked();
    void onProcessPackComplete();

    void onButtonUnpackBrowseClicked();
    void onButtonUnpackClicked();
    void onProcessUnpackComplete();

    void onButtonBnpMakeBrowseClicked();
    void onButtonDataPathBrowseClicked();

  private:
    Ui::MainWindow *  uiMainWindow_;
    QSettings *       settings_;
    QString           bnpMakeBinary_;
    QString           dataPath_;

    QProcess *        processList_;

    QProcess *        processSearch_;
    int               fileInfoListIndex_;
    QFileInfoList     fileInfoList_;

    QProcess *        processPack_;

    QProcess *        processUnpack_;
};

#endif // MAINWINDOW_H
