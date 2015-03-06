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

#ifndef SLOT_MANAGER_DIALOG_H
#define SLOT_MANAGER_DIALOG_H

#include <nel/misc/types_nl.h>
#include "ui_slot_form.h"

#include <QtGui/QDockWidget>
#include <QtGui/QScrollArea>

#include "nel/3d/channel_mixer.h"

namespace NLQT
{

class CSlotGroupBox: public QGroupBox
{
	Q_OBJECT
public:
	CSlotGroupBox(QWidget *parent = 0);
	~CSlotGroupBox();
	void resetUi();
	void updateUi();

public Q_SLOTS:
	void saveSlotInfo();

private Q_SLOTS:
	void selectAnim();
	void selectSkel();
	void alignAblend();

private:

	QString _animName, _skelName;
	int _numSlot;
	Ui::CSlotGroupBox _ui;

	friend class CSlotManagerDialog;
};

class CSlotManagerDialog: public QDockWidget
{
	Q_OBJECT

public:
	CSlotManagerDialog(QWidget *parent = 0);
	~CSlotManagerDialog();

public Q_SLOTS:
	void updateUiSlots();

private:
	QWidget *_dockWidgetContents;
	QGridLayout *_gridLayout;
	QGridLayout *_slotGridLayout;
	QScrollArea *_scrollArea;
	QWidget *_scrollAreaWidgetContents;
	QTabWidget *_tabWidget;
	QSpacerItem *_verticalSpacers[NL3D::CChannelMixer::NumAnimationSlot];
	QGridLayout *_gridLayouts[NL3D::CChannelMixer::NumAnimationSlot];
	QWidget *_tabs[NL3D::CChannelMixer::NumAnimationSlot];
	CSlotGroupBox *_slotGroupBoxs[NL3D::CChannelMixer::NumAnimationSlot];

}; /* CSlotManager */

} /* namespace NLQT */

#endif // SLOT_MANAGER_DIALOG_H
