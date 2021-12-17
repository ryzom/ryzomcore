// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef NL_QNELWINDOW_H
#define NL_QNELWINDOW_H

///-----------------------------------------------------------------------------
/// C++ includes
#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <cmath>

///-----------------------------------------------------------------------------
/// Windowing Toolkit includes.
#include <QMainWindow>

///-----------------------------------------------------------------------------
/// The CameraTrack class which holds the objects and information to render
/// the terrain.
class QNelWindow : public QMainWindow {
	Q_OBJECT
public:
	//explicit QNelWindow (QWidget *parent=0, Qt::WFlags f=0) : QMainWindow (parent, f) { initWindow(); }
	explicit QNelWindow (QWidget *parent=0, Qt::WFlags f=0);
	virtual ~QNelWindow() { }



public slots:

protected:
	void initWindow();

	//CameraTrackWidget *myWidget;
	//CameraTrackWidget *myWidget2;
};

#endif // NL_QNELWINDOW_H
