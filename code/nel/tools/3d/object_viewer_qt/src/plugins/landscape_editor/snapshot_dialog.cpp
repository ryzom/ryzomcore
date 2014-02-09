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

// Project includes
#include "snapshot_dialog.h"
#include "landscape_editor_constants.h"

#include "../core/icore.h"
#include "../core/core_constants.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>

namespace LandscapeEditor
{

SnapshotDialog::SnapshotDialog(QWidget *parent)
	: QDialog(parent)
{
	m_ui.setupUi(this);
	setFixedHeight(sizeHint().height());
}

SnapshotDialog::~SnapshotDialog()
{
}

bool SnapshotDialog::isCustomSize() const
{
	return m_ui.customSizeRadioButton->isChecked();
}

bool SnapshotDialog::isKeepRatio() const
{
	return m_ui.keepRatioCheckBox->isChecked();
}

int SnapshotDialog::resolutionZone() const
{
	return m_ui.resSpinBox->value();
}

int SnapshotDialog::widthSnapshot() const
{
	return m_ui.widthSpinBox->value();
}

int SnapshotDialog::heightSnapshot() const
{
	return m_ui.heightSpinBox->value();
}

} /* namespace LandscapeEditor */
