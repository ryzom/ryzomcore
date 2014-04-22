// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef SNAPSHOT_DIALOG_H
#define SNAPSHOT_DIALOG_H

// Project includes
#include "ui_shapshot_dialog.h"

// Qt includes

namespace LandscapeEditor
{

class SnapshotDialog: public QDialog
{
	Q_OBJECT

public:
	explicit SnapshotDialog(QWidget *parent = 0);
	~SnapshotDialog();

	bool isCustomSize() const;
	bool isKeepRatio() const;
	int resolutionZone() const;
	int widthSnapshot() const;
	int heightSnapshot() const;

private:

	Ui::SnapshotDialog m_ui;
}; /* class SnapshotDialog */

} /* namespace LandscapeEditor */

#endif // SNAPSHOT_DIALOG_H
