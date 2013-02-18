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

#ifndef WIDGET_INFO_TREE_H
#define WIDGET_INFO_TREE_H

#include "widget_info_tree_node.h"
#include "nel/misc/debug.h"

namespace GUIEditor
{
	class CWidgetInfoTree
	{
	public:
		CWidgetInfoTree()
		{
			root = NULL;
		}

		~CWidgetInfoTree()
		{
			delete root;
			root = NULL;
		}

		/// Find a node by it's name
		CWidgetInfoTreeNode* findNodeByName( const std::string &name )
		{
			if( root == NULL )
				return NULL;

			return root->findNodeByName( name );
		}

		void addRootNode( SWidgetInfo &info )
		{
			nlassert( root == NULL );
			root = CWidgetInfoTreeNode::create( info );
		}

		/// Add a new node as a child to it's ancestor
		bool addNode( SWidgetInfo &info )
		{
			CWidgetInfoTreeNode *node = findNodeByName( info.ancestor );
			if( node == NULL )
				return false;
			else
				node->addChild( info );
			return true;
		}

		/// Finds a node by it's name and removes it
		bool removeNode( const std::string &name )
		{
			CWidgetInfoTreeNode *node = findNodeByName( name );
			if( node == NULL )
				return false;

			if( node == root )
				root = NULL;
			if( node->getParent() != NULL )
				node->getParent()->removeChildByNameND( node->getInfo().name );
			delete node;

			return true;
		}

		/// Finds a node by it's info entry and removes it
		bool removeNode( SWidgetInfo *info )
		{
			return removeNode( info->name );
		}

		/// Removes this property from all of the nodes
		void removePropertyFromAll( const SPropEntry &prop )
		{
			if( root == NULL )
				return;
			root->removePropertyFromAll( prop );
		}

		/// Get the node names and put them into the vector
		void getNames( std::vector< std::string > &v ) const
		{
			if( root == NULL )
				return;
			root->getNames( v );
		}


		/// Accepts a visitor
		void accept( CWidgetInfoTreeVisitor *visitor )
		{
			root->accept( visitor );
		}

	private:
		CWidgetInfoTreeNode *root;
	};
}

#endif
