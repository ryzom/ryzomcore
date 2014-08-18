// Ryzom Core Studio - Tile Editor plugin
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


#ifndef LAND_EDIT_DLG_H
#define LAND_EDIT_DLG_H


#include "ui_land_edit_dialog.h"
#include <QStringList>

class LandEditDialog : public QDialog, public Ui::LandEditDialog
{
	Q_OBJECT
public:
	LandEditDialog( QWidget *parent = NULL );
	~LandEditDialog();

	void getSelectedTileSets( QStringList &l ) const;
	void setSelectedTileSets( QStringList &l );

	void setTileSets( const QStringList &l );

private:
	void setupConnections();


private Q_SLOTS:
	void onOkClicked();
	void onCancelClicked();
	void onAddClicked();
	void onRemoveClicked();

};


#endif

