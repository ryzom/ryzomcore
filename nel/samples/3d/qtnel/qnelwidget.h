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

#ifndef NL_QTNEL_H
#define NL_QTNEL_H

#include <QGLWidget>
#include <QX11Info>

#include <nel/3d/scene.h>
#include <nel/3d/font_manager.h>
#include <nel/3d/computed_string.h>
#include <nel/3d/text_context.h>
#include <nel/3d/driver_user.h>

/** A QWidget specialised for embedding an Nel window. */
class QNelWidget : public QGLWidget
{
	Q_OBJECT

///--QWidget Methods------------------------------------------------------------
public:

	QNelWidget(QWidget *parent = 0) : QGLWidget( parent ),_SceneRoot(NULL)
	{
		// Initialize NeL
		init();
	}

	// Do nothing.
	QNelWidget(const QNelWidget &copy) : QGLWidget(copy.parentWidget()) { }

	virtual ~QNelWidget ()
	{
		// Destroy NeL.
		// ...

		destroy();
	}

	virtual void swapBuffers();
protected:
	virtual void initializeGL();
	virtual void resizeGL( int, int );
	virtual void paintGL();


	/// Initialize NeL
	void init();

	NL3D::CScene *_SceneRoot;
	NL3D::CFontManager fontManager;
	NL3D::CTextContext tc;
};

#endif // NL_QTNEL_H
