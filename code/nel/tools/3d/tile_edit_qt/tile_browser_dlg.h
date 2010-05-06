#ifndef TILE_BROWSERDLG_H
#define TILE_BROWSERDLG_H

#include <QtGui/QtGui>
#include <QtGui/QDialog>
#include <QtGui/QButtonGroup>
#include "ui_tile_browser_qt.h"
#include "tiles_model.h"
#include "browser_model.h"


class CTile_browser_dlg : public QDialog
{
	Q_OBJECT

public:	

	enum TileLabel
	{ 
		Index = 0,
		FileName = 1
	};

	enum TileZoom
	{ 
		Small = 32,
		Normal =64,
		Big = 128
	};

	CTile_browser_dlg(QWidget *parent = 0, Qt::WindowFlags f = 0);
	void initDialog(const int&);   
	

private slots:
	void on_tileTypeButtonGroup_clicked(int);
	void on_tileTextureButtonGroup_clicked(int);
	void on_tileLabelButtonGroup_clicked(int);
	void on_tileZoomButtonGroup_clicked(int);

	void on_refreshPushButton_clicked();
	void on_batchLoadPushButton_clicked();
	void on_exportBorderPushButton_clicked();
	void on_importBorderPushButton_clicked();

	void on_tiles_model_selectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );

	void on_actionAddTile_triggered(bool);
	void on_actionDeleteTile_triggered(bool);
	void on_actionReplaceImage_triggered(bool);
	void on_actionDeleteImage_triggered(bool);

protected:
    void closeEvent(QCloseEvent *event);
	void accept();
	void reject();

private:
	int tileSetIndex;
	tiles_model *tileViewModel;
	TileList browserModel;

	QButtonGroup* tileTypeButtonGroup;
	QButtonGroup* tileTextureButtonGroup;
	QButtonGroup* tileLabelButtonGroup;
	QButtonGroup* tileZoomButtonGroup;

	void RefreshView();
	void EnableBrowserInteractions();
	void LoadInThread(void);

	Ui::TileBrowserDialog ui;
};


#endif
