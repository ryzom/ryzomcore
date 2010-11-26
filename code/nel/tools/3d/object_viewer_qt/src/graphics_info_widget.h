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

#ifndef GRAPHICS_INFO_WIDGET_H
#define GRAPHICS_INFO_WIDGET_H

// Qt includes
#include <QtGui/QWidget>

// STL includes

// NeL includes
#include <nel/misc/vector.h>

// Project includes

namespace NLQT
{

struct Mode
{
	enum List
	{
		Color = 0,
		Direction,
		PlaneBasic,
		RibbonShape
	};
};

class CGraphicsInfoWidget: public QWidget
{
	Q_OBJECT

public:
	CGraphicsInfoWidget(QWidget *parent = 0);
	~CGraphicsInfoWidget();

	void setMode(int mode);
	void setColor(const QColor &color);
	void setVector(float x, float y);
	void setText(const QString &text);
	void setRibbonShape(const std::vector<NLMISC::CVector> &verts, bool braceMode);

Q_SIGNALS:
	void applyNewVector(float x, float y);

protected:
	virtual void paintEvent(QPaintEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);

	int 	_mode;
	QColor 	_color;
	float _x;
	float _y;
	QString _text;
	std::vector<NLMISC::CVector> _verts;
	bool _braceMode;

}; /* class CGraphicsInfoWidget */

} /* namespace NLQT */

#endif // GRAPHICS_INFO_WIDGET_H
