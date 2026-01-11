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

#ifndef ANIMATION_SET_DIALOG_H
#define ANIMATION_SET_DIALOG_H

#include "ui_animation_set_form.h"

// STL includes

// NeL includes

// Project includes

namespace NLQT
{
/**
@class CAnimationSetDialog
@brief Dialog - Animations control, loading animation, weight for skeleton and playlist composition.
@details Dialog loads animations files and weight for skeleton animation. Files can also be unloaded.
For loading / unloading and animation control dialog uses the functionality of CEntity class.
But the main opportunity for dialogue is to generating animations playlists.
Also, the dialogue has a control element to select the current shape (emit a signal changeCurrentShape())
and switch of playlist/mixer (this functionality will soon be transferred to another specialized dialogue).
As each shape has its own list of loaded animations and playlist,when you switch your current shape,
the dialogue should be notified through the slot setCurrentShape ().
*/
class CAnimationSetDialog: public QDockWidget
{
	Q_OBJECT

public:
	CAnimationSetDialog(QWidget *parent = 0);
	~CAnimationSetDialog();

	/// Update the objects list (this function should be moved to another dialogue)
	void updateListObject();

	/// Update the list of loaded animation files
	void updateListAnim();

Q_SIGNALS:
	/// Signal emitted when changing the current animation object.
	void changeCurrentShape(const QString &name);

public Q_SLOTS:

	/// Updates and displays the list of loaded animations and playlist for the selected current object
	/// @param name - the name of the selected object
	void setCurrentShape(const QString &name);

private Q_SLOTS:
	void loadAnim();
	void loadSwt();
	void resetAnim();
	void addAnim();
	void removeAnim();
	void upAnim();
	void downAnim();

private:

	Ui::CAnimationSetDialog ui;

	friend class CMainWindow;
}; /* CAnimationDialog */

} /* namespace NLQT */

#endif // ANIMATION_SET_DIALOG_H
