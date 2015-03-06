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

#ifndef WIDGET_INFO_SERIALIZER_H
#define WIDGET_INFO_SERIALIZER_H

#include "widget_info_tree_visitor.h"

namespace GUIEditor
{
	class CWidgetInfoTree;

	class CWidgetInfoSerializer : public CWidgetInfoTreeVisitor
	{
	public:
		void serialize( CWidgetInfoTree *tree );
		void visit( CWidgetInfoTreeNode *node );

	private:
	};
}

#endif

