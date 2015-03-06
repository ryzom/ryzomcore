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


#ifndef LINK_EDITOR_H
#define LINK_EDITOR_H

#include "ui_link_editor.h"
#include "nel/misc/types_nl.h"

namespace GUIEditor
{
	class LinkEditorPvt;

	class LinkEditor : public QWidget, public Ui::LinkEditor
	{
		Q_OBJECT
	public:
		LinkEditor( QWidget *parent = NULL );
		~LinkEditor();
		void setup();
		void setLinkId( uint32 linkId );
		void clear();

	Q_SIGNALS:
		void okClicked();
		
	private Q_SLOTS:
		void onOKButtonClicked();
		void onTextEditContextMenu( const QPoint &pos );
		void onEE();
		void onEEClosing();

	private:
		uint32 currentLinkId;
		LinkEditorPvt *m_pvt;
	};
}

#endif

