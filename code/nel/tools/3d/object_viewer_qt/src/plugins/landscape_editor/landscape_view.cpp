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

// Project includes
#include "landscape_view.h"
#include "landscape_editor_constants.h"

#include "../core/icore.h"
#include "../core/core_constants.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QApplication>

namespace LandscapeEditor
{

LandscapeView::LandscapeView(QWidget *parent)
	: QGraphicsView(parent),
	  m_moveMouse(false)
{
	setDragMode(ScrollHandDrag);
	setTransformationAnchor(AnchorUnderMouse);
}

LandscapeView::~LandscapeView()
{
}

void LandscapeView::wheelEvent(QWheelEvent *event)
{
	double numDegrees = event->delta() / 8.0;
	double numSteps = numDegrees / 15.0;
	double factor = std::pow(1.125, numSteps);
	scale(factor, factor);
}

void LandscapeView::mousePressEvent(QMouseEvent *event)
{
	QGraphicsView::mousePressEvent(event);
	if (event->button() != Qt::MiddleButton)
		return;
	m_moveMouse = true;
	QApplication::setOverrideCursor(Qt::ClosedHandCursor);
}

void LandscapeView::mouseMoveEvent(QMouseEvent *event)
{
	if (m_moveMouse)
		translate(0.001, 0.001);
	QGraphicsView::mouseMoveEvent(event);
}

void LandscapeView::mouseReleaseEvent(QMouseEvent *event)
{
	QApplication::restoreOverrideCursor();
	m_moveMouse = false;
	QGraphicsView::mouseReleaseEvent(event);
}

} /* namespace LandscapeEditor */
