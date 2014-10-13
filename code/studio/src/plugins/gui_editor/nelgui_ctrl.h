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

#include <QObject>
#include "project_files.h"

class QWidget;
class Nel3DWidget;

namespace NLGUI
{
	class CEventListener;
}

namespace GUIEditor
{
	class CEditorSelectionWatcher;

	/// Qt viewport controller for the Nel GUI library
	class NelGUICtrl : public QObject
	{
		Q_OBJECT
	public:
		NelGUICtrl( QObject *parent = NULL );
		~NelGUICtrl();

		void init();
		bool parse( SProjectFiles &files );
		bool loadMapFiles( const std::vector< std::string > &v );
		bool createNewGUI( const std::string &project, const std::string &window );
		void draw();
		void reset();
		CEditorSelectionWatcher* getWatcher(){ return watcher; }

		QWidget* getViewPort();

		void show();
		void hide();

		void setWorkDir( const QString &dir );

Q_SIGNALS:
		void guiLoadComplete();

	protected:
		void timerEvent( QTimerEvent *evnt );

	private:
		void onGUILoaded();

		int timerID;
		bool guiLoaded;
		CEditorSelectionWatcher *watcher;

		NLGUI::CEventListener *eventListener;

		Nel3DWidget *w;

		bool listening;
	};
}

#endif
