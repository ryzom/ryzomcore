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


#ifndef NELGUI_WIDGET_H
#define NELGUI_WIDGET_H

#include "nel3d_widget.h"
#include "project_files.h"

namespace GUIEditor
{
	class CEditorSelectionWatcher;

	/// Qt viewport for the Nel GUI library
	class NelGUIWidget : public Nel3DWidget
	{
		Q_OBJECT
	public:
		NelGUIWidget( QWidget *parent = NULL );
		~NelGUIWidget();

		void init();
		bool parse( SProjectFiles &files );
		void draw();
		void reset();
		CEditorSelectionWatcher* getWatcher(){ return watcher; }

Q_SIGNALS:
		void guiLoadComplete();

	protected:
		void paintEvent( QPaintEvent *evnt );
		void timerEvent( QTimerEvent *evnt );
		void showEvent( QShowEvent *evnt );
		void hideEvent( QHideEvent *evnt );


	private:
		int timerID;
		bool guiLoaded;
		CEditorSelectionWatcher *watcher;
	};
}

#endif
