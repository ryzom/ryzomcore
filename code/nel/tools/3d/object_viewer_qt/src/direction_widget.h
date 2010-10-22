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

#ifndef DIRECTION_WIDGET_H
#define DIRECTION_WIDGET_H

#include "ui_direction_form.h"

// STL includes

// NeL includes
#include <nel/3d/ps_direction.h>

// Project includes
#include "ps_wrapper.h"

namespace NLQT {

/**
@class CDirectionWidget
@brief This widget helps to choose from several preset directions, or to choose a custom one.
@details This also allow to bind the direction to a global variable when it is supported
 */
class CDirectionWidget: public QWidget
{
     Q_OBJECT
	
public:
	CDirectionWidget(QWidget *parent = 0);
	~CDirectionWidget();
	
	void setWrapper(IPSWrapper<NLMISC::CVector> *wrapper);

	/// The CPSDirection object is used to see if a global variable can be bound to the direction.
	/// When set to NULL it has no effect (the default)
	void setDirectionWrapper(NL3D::CPSDirection *wrapper);
	void updateUi();
	
private Q_SLOTS:
	void setGlobalDirection();
	void incVecI();
	void incVecJ();
	void incVecK();
	void decVecI();
	void decVecJ();
	void decVecK();
	void setNewVecXZ(float x, float y);
	void setNewVecYZ(float x, float y);
	
private:
	void checkEnabledGlobalDirection();
  
	IPSWrapper<NLMISC::CVector> *_Wrapper ;
	NL3D::CPSDirection	    *_DirectionWrapper;

	Ui::CDirectionWidget _ui;
	
}; /* class CDirectionWidget */

} /* namespace NLQT */

#endif // DIRECTION_WIDGET_H
