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

#ifndef SUN_COLOR_DIALOG_H
#define SUN_COLOR_DIALOG_H

#include "ui_sun_color_form.h"

// STL includes

// NeL includes

// Project includes

namespace NLQT
{

/**
@class CSunColorDialog
@brief Dialog to choose the sun color in a 3D scene
*/
class CSunColorDialog: public QDockWidget
{
	Q_OBJECT

public:
	CSunColorDialog(QWidget *parent = 0);
	~CSunColorDialog();

private Q_SLOTS:
	void setAmbientSunColor(NLMISC::CRGBA color);
	void setDiffuseSunColor(NLMISC::CRGBA color);
	void setSpecularSunColor(NLMISC::CRGBA color);

private:

	Ui::CSunColorDialog _ui;
}; /* class CSunColorDialog */

} /* namespace NLQT */

#endif // SUN_COLOR_DIALOG_H
