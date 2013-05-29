// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#ifndef MATERIAL_EDITOR_CONTEXT_H
#define MATERIAL_EDITOR_CONTEXT_H

#include "../core/icontext.h"

#include "nel/misc/app_context.h"

#include <QtCore/QObject>
#include <QtGui/QIcon>

namespace MaterialEditor
{
	class MaterialEditorWindow;
	
	class MaterialEditorContext: public Core::IContext
	{
		Q_OBJECT
	public:
		MaterialEditorContext(QObject *parent = 0);
		virtual ~MaterialEditorContext() {}
		
		virtual QString id() const{ return QLatin1String("MaterialEditorContext"); }

		virtual QString trName() const{ return tr("Material Editor"); }

		virtual QIcon icon() const{ return QIcon(); }

		void open();

		void newDocument();

		void save();

		void saveAs();

		void close();
		
		virtual QUndoStack *undoStack(){ return NULL; }
		
		virtual QWidget *widget();
		
		MaterialEditorWindow *m_materialEditorWindow;
	};
	
}

#endif
