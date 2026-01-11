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

#ifndef TILE_ROTATIONDLG_H
#define TILE_ROTATIONDLG_H

#include "ui_tile_rotation_qt.h"

class CTile_rotation_dlg : public QDialog
{
	Q_OBJECT

public:
	static int getRotation(QWidget *parent, bool *ok = 0,Qt::WindowFlags f = 0);

	enum TileRotation
	{ 
		_0Rotation = 0,
		_90Rotation = 3,
		_180Rotation = 2,
		_270Rotation = 1
	};

	int getCheckedRotation() const {	return rotationButtonGroup->checkedId();	}

private:
	CTile_rotation_dlg(QWidget *parent = 0, Qt::WindowFlags f = 0);
	Ui::TileRotationDialog ui;
	QButtonGroup* rotationButtonGroup;
};

#endif
