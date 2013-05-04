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


#ifndef GUI_EDITOR_CONTEXT_H
#define GUI_EDITOR_CONTEXT_H

#include "../core/icontext.h"

#include "nel/misc/app_context.h"

#include <QtCore/QObject>
#include <QtGui/QIcon>

namespace GUIEditor
{
	class GUIEditorWindow;
	
	class GUIEditorContext: public Core::IContext
	{
		Q_OBJECT
	public:
		GUIEditorContext(QObject *parent = 0);
		virtual ~GUIEditorContext() {}
		
		virtual QString id() const{ return QLatin1String("GUIEditorContext"); }

		virtual QString trName() const{ return tr("GUI Editor"); }

		virtual QIcon icon() const{ return QIcon(); }

		void open();

		void newDocument();

		void save();

		void saveAs();

		void close();
		
		virtual QUndoStack *undoStack();
		
		virtual QWidget *widget();
		
		GUIEditorWindow *m_guiEditorWindow;
	};
	
}

#endif
