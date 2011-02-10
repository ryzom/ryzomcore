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

#ifndef BASIC_EDIT_WIDGET_H
#define BASIC_EDIT_WIDGET_H

#include <nel/misc/types_nl.h>
#include "ui_basic_edit_form.h"

// STL includes

// NeL includes
#include "nel/3d/ps_plane_basis.h"
#include "ps_wrapper.h"

// Project includes

namespace NLQT
{

class CBasicEditWidget: public QWidget
{
	Q_OBJECT

public:
	CBasicEditWidget(QWidget *parent = 0);
	~CBasicEditWidget();

	void setWrapper(IPSWrapper<NL3D::CPlaneBasis> *wrapper);
	void updateUi();

private Q_SLOTS:
	void updateGraphics();

private:
	bool eventFilter(QObject *object, QEvent *event);

	// wrapper to the datas
	IPSWrapper<NL3D::CPlaneBasis> *_Wrapper ;

	Ui::CBasicEditWidget _ui;

}; /* class CBasicEditWidget */

} /* namespace NLQT */

#endif // BASIC_EDIT_WIDGET_H
