// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef LANDSCAPE_VIEW_H
#define LANDSCAPE_VIEW_H

// Project includes

// Qt includes
#include <QtGui/QGraphicsView>
#include <QtGui/QWheelEvent>

namespace LandscapeEditor
{

class LandscapeView: public QGraphicsView
{
	Q_OBJECT

public:
	LandscapeView(QWidget *parent = 0);
	~LandscapeView();

protected:
    void wheelEvent(QWheelEvent *event); 

private Q_SLOTS:

private:

}; /* class LandscapeView */

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_VIEW_H
