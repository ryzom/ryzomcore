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

#ifndef SETUP_FOG_DIALOG_H
#define SETUP_FOG_DIALOG_H

#include <nel/misc/types_nl.h>
#include "ui_setup_fog_form.h"

// STL includes

// NeL includes
#include <nel/misc/rgba.h>

// Project includes

namespace NLQT
{

/**
@class CSetupFog
@brief Dialog to edit the fog in a 3D scene.
*/
class CSetupFog: public QDockWidget
{
	Q_OBJECT

public:
	CSetupFog(QWidget *parent = 0);
	~CSetupFog();

private Q_SLOTS:
	void apply();
	void setColor();

private:
	NLMISC::CRGBA colorFog;

	Ui::CSetupFog ui;

}; /* class CSetupFog */

} /* namespace NLQT */

#endif // SETUP_FOG_DIALOG_H
