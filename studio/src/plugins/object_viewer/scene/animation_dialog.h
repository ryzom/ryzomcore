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

#ifndef ANIMATION_DIALOG_H
#define ANIMATION_DIALOG_H

#include <nel/misc/types_nl.h>
#include "ui_animation_form.h"

// Qt includes
#include <QtCore/QTimeLine>

// STL includes

// NeL includes
#include "nel/3d/animation_time.h"

// Project includes

namespace NLQT
{

/**
@class CAnimationDialog
@brief Animation model control dialog.
@details The dialogue doesn't affect on the model itself, but only calculates the current time of animations
that can be obtained through a class method getTime().
The user can influence on the duration of the animation, but he doesn't know the total time for all the animations in the playlist.
Therefore, the class provides a slot that requires a total duration of the animated object animations and set it.
*/
class CAnimationDialog: public QDockWidget
{
	Q_OBJECT

public:
	/// Constructor, sets the default length of time from 0 to 99
	CAnimationDialog(QWidget *parent = 0);
	~CAnimationDialog();

	/// Get the current time animations
	/// @return Returns the current time animations, which can then be use in class CObjectViewer
	NL3D::TAnimationTime getTime ();

public Q_SLOTS:
	/// Find the total time of the playlist and sets its
	void changeAnimLength();

	/// Updates animation status for the selected current object
	/// @param name - the name of the selected object
	void setCurrentShape(const QString &name);

private Q_SLOTS:
	void start();
	void play();
	void stop();
	void end();
	void changeFrame(int frame);
	void changeStartAnim(int start);
	void changeEndAnim(int end);
	void updateAnim(int frame);
	void setInPlace(bool state);
	void setIncPos(bool state);
	void finish();
	void setModePlayList();
	void setModeMixer();

private:

	static const int _frameRate = 50;
	QTimeLine *_timeLine;
	Ui::CAnimationDialog _ui;

	friend class CMainWindow;
}; /* CAnimationDialog */

} /* namespace NLQT */

#endif // ANIMATION_DIALOG_H
