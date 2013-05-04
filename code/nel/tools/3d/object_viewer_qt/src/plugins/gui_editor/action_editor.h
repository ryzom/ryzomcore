// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef ACTION_EDITOR_H
#define ACTION_EDITOR_H

#include "ui_action_editor.h"

namespace NLGUI
{
	class CProcAction;
}

namespace GUIEditor
{
	class ActionEditor : public QWidget, public Ui::ActionEditor
	{
		Q_OBJECT
	public:
		ActionEditor( QWidget *parent = NULL );
		~ActionEditor();
		void setCurrentAction( NLGUI::CProcAction *action );
		void clear();

	private Q_SLOTS:
		void onOkButtonClicked();

	private:
		NLGUI::CProcAction *currentAction;
	};
}

#endif

