// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef TILE_BROWSERDLG_H
#define TILE_BROWSERDLG_H

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
