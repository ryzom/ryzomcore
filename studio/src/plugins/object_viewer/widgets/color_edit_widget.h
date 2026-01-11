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

#ifndef COLOR_EDIT_WIDGET_H
#define COLOR_EDIT_WIDGET_H

#include "ui_color_edit_form.h"

// Qt includes

// Project includes
#include "ps_wrapper.h"

namespace NLQT
{

/**
@class CColorEditWidget
@brief The widget provides a 4 horizontal slider or color dialog, to set the color.
@details Using this widget you can set the color(RGBA) using the four sliders or through the color selection dialog.
Widget at the same time displays the color in the shaded rectangle at the side of the sliders.
Use this widget, have two ways: Qt Q_SIGNAL/SLOT or wrapper.

1. Using the Qt Q_SIGNALS/SLOT can be set current color by class methods setColor() and
changes in current color(QSliders or QColorDialog) emits the signal colorChanged().

2. Using wrapper, create wpapper struct, example:
@code
struct CAmbientColorWrapper : public IPSWrapperRGBA
{
	NL3D::UScene *S;
	void set(const NLMISC::CRGBA &col) { S->setSunAmbient(col); }
	NLMISC::CRGBA get() const  { return S->getSunAmbient(); }
} _AmbientColorWrapper;
@endcode
to set the current values, need call class methods updateUi();
*/
class CColorEditWidget: public QWidget
{
	Q_OBJECT

public:
	/// Constructor, sets the default color (255, 255, 255, 255)
	CColorEditWidget(QWidget *parent = 0);
	~CColorEditWidget();

	/// Sets the current color.
	/// @param color - NeL NLMISC::CRGBA color
	/// @param emit - will emit colorChanged() if the new value is different from the old one and param emit = true
	void setColor(const NLMISC::CRGBA &color, bool emit = true);

	/// Sets the current color.
	/// @param color - Qt QColor color
	/// @param emit - will emit colorChanged() if the new value is different from the old one and param emit = true
	void setColor(const QColor &color, bool emit = true);

	/// Set a wrapper to get/set the datas.
	void setWrapper(IPSWrapperRGBA *wrapper);

	/// Update the content of the widget using the wrapper.
	void updateUi();

Q_SIGNALS:
	void colorChanged(NLMISC::CRGBA color);

private Q_SLOTS:
	void setRed(int r);
	void setGreen(int g);
	void setBlue(int b);
	void setAlpha(int a);
	void browseColor();

private:

	bool eventFilter(QObject *object, QEvent *event);
	// wrapper to the datas
	IPSWrapperRGBA *_Wrapper;

	QColor _color;
	bool _emit;

	Ui::CColorEditWidget _ui;

}; /* class CColorEditWidget */

} /* namespace NLQT */

#endif // COLOR_EDIT_WIDGET_H
