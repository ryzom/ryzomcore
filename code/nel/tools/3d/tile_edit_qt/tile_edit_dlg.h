#ifndef TILE_EDITDLG_H
#define TILE_EDITDLG_H

#include <QtGui/QtGui>
#include <QtGui/QMainWindow>
#include "ui_tile_edit_qt.h"

class QCheckBox;
class QPushButton;
class QLineEdit;
class QLabel;
class QListWidget;

class CTile_edit_dlg : public QMainWindow
{
	Q_OBJECT

protected:
     void closeEvent(QCloseEvent *event);

public:
	CTile_edit_dlg(QWidget *parent = 0);

private slots:
	void on_addLandPushButton_clicked();
	void on_editLandPushButton_clicked();
	void on_deleteLandPushButton_clicked();

	void on_addTileSetPushButton_clicked();
	void on_editTileSetPushButton_clicked();
	void on_deleteTileSetPushButton_clicked();
	void on_chooseVegetPushButton_clicked();
	void on_resetVegetPushButton_clicked();
	void on_surfaceDataLineEdit_textChanged();
	void on_orientedCheckBox_stateChanged ( int state );
	void on_tileSetListWidget_itemSelectionChanged();
	void on_landListWidget_itemSelectionChanged();

	void on_quitPushButton_clicked();
	void on_loadPushButton_clicked();
	void on_savePushButton_clicked();
	void on_saveAsPushButton_clicked();
	void on_exportPushButton_clicked();
	void on_absolutePathPushButton_clicked();

	void on_upPushButton_clicked();
	void on_downPushButton_clicked();

private:
	QFileInfo mainFile;

	void initDialog();
	Ui::MainWindow ui;
};


#endif
