// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef SYSINFOOPENGLWIDGET_H
#define SYSINFOOPENGLWIDGET_H

#include "ui_sys_info_opengl_widget.h"


/**
 @brief The OpenGL information page of the configuration tool
*/
class CSysInfoOpenGLWidget : public QWidget, public Ui::sys_info_opengl_widget
{
	Q_OBJECT
public:
	CSysInfoOpenGLWidget( QWidget *parent = NULL );
	~CSysInfoOpenGLWidget();

protected:
	void changeEvent( QEvent *event );

};

#endif // SYSINFOOPENGLWIDGET_H
