/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SPINNER_DIALOG_H
#define SPINNER_DIALOG_H

// Qt includes
#include <QtGui/QDialog>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

// Nel include
#include "nel/3d/ps_plane_basis_maker.h"

// Project includes
#include "direction_widget.h"
#include "edit_range_widget.h"
#include "ps_wrapper.h"

namespace NLQT
{

class CSpinnerDialog : public  QDialog
{
	Q_OBJECT

public:
	CSpinnerDialog(NL3D::CPSBasisSpinner *sf, CWorkspaceNode *ownerNode, QWidget *parent = 0);
	~CSpinnerDialog();

private Q_SLOTS:
	void setNumSamples(uint32 value);
	void setAxis(const NLMISC::CVector &axis);

protected:
	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	QLabel *_nbSamplesLabel;
	QVBoxLayout *_verticalLayout;
	CEditRangeUIntWidget *_nbSamplesWidget;
	CDirectionWidget *_dirWidget;

	CWorkspaceNode *_Node;
	NL3D::CPSBasisSpinner *_BasicSpinner;
};

} /* namespace NLQT */

#endif // SPINNER_DIALOG_H
