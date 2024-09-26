// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#include <QObject>

namespace GUIEditor
{
	class CWidgetInfoTree;

	/// Processes the GUI Editor's editor messages like delete, new, etc...
	class CEditorMessageProcessor : public QObject
	{
		Q_OBJECT
	public:
		CEditorMessageProcessor( QObject *parent = NULL ) :
		QObject( parent )
		{
			tree = NULL;
		}

		~CEditorMessageProcessor(){}

		void setTree( CWidgetInfoTree *tree ){ this->tree = tree; }
		
	public Q_SLOTS:
		void onDelete();
		void onAdd( const QString &parentGroup, const QString &widgetType, const QString &name );
		void onSetGroupSelection( bool b );
		void onGroup();
		void onUngroup();
		void onSetMultiSelection( bool b );

	private:
		CWidgetInfoTree *tree;
	};
}

