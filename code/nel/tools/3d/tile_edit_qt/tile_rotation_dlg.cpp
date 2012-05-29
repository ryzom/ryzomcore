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

#include "tile_rotation_dlg.h"

CTile_rotation_dlg::CTile_rotation_dlg(QWidget *parent, Qt::WindowFlags flags)
     : QDialog(parent, flags)

{
	ui.setupUi(this);

	rotationButtonGroup = new QButtonGroup;
    
	rotationButtonGroup->addButton(ui._0PushButton, CTile_rotation_dlg::_0Rotation);
	rotationButtonGroup->addButton(ui._90PushButton, CTile_rotation_dlg::_90Rotation);
	rotationButtonGroup->addButton(ui._180PushButton, CTile_rotation_dlg::_180Rotation);
	rotationButtonGroup->addButton(ui._270PushButton, CTile_rotation_dlg::_270Rotation);
}


int CTile_rotation_dlg::getRotation(QWidget *parent, bool *ok, Qt::WindowFlags f)
{
    CTile_rotation_dlg dlg(parent, f);
    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return dlg.getCheckedRotation();
}