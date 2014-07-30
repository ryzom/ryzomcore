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

#ifndef WIDGET_INFO_TREE_NODE
#define WIDGET_INFO_TREE_NODE

#include "widget_info.h"
#include "widget_info_tree_visitor.h"

namespace GUIEditor
{

	/// Widget Tree Info Node
	class CWidgetInfoTreeNode
	{
	public:
		CWidgetInfoTreeNode( SWidgetInfo &info )
		{
			this->info = info;
			parent = NULL;
			children.clear();
		}

		~CWidgetInfoTreeNode()
		{
			removeChildren();
			parent = NULL;
		}

		/// Set the parent of this node
		void setParent( CWidgetInfoTreeNode *newParent )
		{
			parent = newParent;
		}


		/// Returns the parent of this node
		CWidgetInfoTreeNode* getParent() const
		{
			return parent;
		}

		/// Create a new node
		static CWidgetInfoTreeNode* create( SWidgetInfo &info )
		{
			return new CWidgetInfoTreeNode( info );
		}

		/// Get the WidgetInfo of this node
		SWidgetInfo& getInfo()
		{
			return info;
		}

		/// Add a new child node
		void addChild( SWidgetInfo &info )
		{
			CWidgetInfoTreeNode *node = CWidgetInfoTreeNode::create( info );
			node->setParent( this );
			children.push_back( node );
			
			// copy the properties to the child, since they inherit them
			for( std::vector< SPropEntry >::const_iterator itr = this->info.props.begin(); itr != this->info.props.end(); ++itr )
			{
				node->addProperty( *itr );
			}
		}

		/// Remove child by name
		bool removeChildByName( const std::string &name )
		{
			for( std::vector< CWidgetInfoTreeNode* >::iterator itr = children.begin(); itr != children.end(); ++itr )
			{
				if( ( *itr )->getInfo().name == name )
				{
					delete ( *itr );
					children.erase( itr );
					return true;
				}
			}

			return false;
		}

		/// Remove child by name, but don't delete the child
		bool removeChildByNameND( const std::string &name )
		{
			for( std::vector< CWidgetInfoTreeNode* >::iterator itr = children.begin(); itr != children.end(); ++itr )
			{
				if( ( *itr )->getInfo().name == name )
				{
					children.erase( itr );
					return true;
				}
			}

			return false;
		}

		/// Remove child by ancestor's name
		bool removeChildByAncestor( const std::string &ancestor )
		{
			for( std::vector< CWidgetInfoTreeNode* >::iterator itr = children.begin(); itr != children.end(); ++itr )
			{
				if( ( *itr )->getInfo().ancestor == ancestor )
				{
					children.erase( itr );
					delete ( *itr );
					return true;
				}
			}

			return false;
		}

		/// Remove all children
		void removeChildren()
		{
			std::vector< CWidgetInfoTreeNode* >::iterator itr = children.begin();
			while( itr != children.end() )
			{
				CWidgetInfoTreeNode *node = *itr;
				++itr;
				delete node;
			}
			children.clear();
		}

		/// Checks if this node has this property
		bool hasProperty( const std::string &name )
		{
			std::vector< SPropEntry >::const_iterator itr = info.findProp( name );
			if( itr != info.props.end() )
				return true;
			else
				return false;
		}

		/// Add new property to the node
		void addProperty( const SPropEntry &prop )
		{
			info.props.push_back( prop );
		}

		/// Add new property to the node and all children
		void addPropertyToAll( const SPropEntry &prop )
		{
			addProperty( prop );

			std::vector< CWidgetInfoTreeNode* >::iterator itr = children.begin();
			while( itr != children.end() )
			{
				( *itr )->addPropertyToAll( prop );
				++itr;
			}

		}

		/// Removes this property from the node
		void removeProperty( const SPropEntry &prop )
		{
			std::vector< SPropEntry >::iterator itr = info.props.begin();
			while( itr != info.props.end() )
			{
				if( ( itr->propName    == prop.propName ) &&
					( itr->propType    == prop.propType ) &&
					( itr->propDefault == prop.propDefault ) )
					break;
				++itr;
			}
			if( itr != info.props.end() )
				info.props.erase( itr );
		}

		/// Removes this property from all of the nodes recursively
		void removePropertyFromAll( const SPropEntry &prop )
		{
			removeProperty( prop );
			for( std::vector< CWidgetInfoTreeNode* >::iterator itr = children.begin(); itr != children.end(); ++itr )
			{
				( *itr )->removePropertyFromAll( prop );
			}

		}

		/// Find the node by it's name and then return a pointer to it
		CWidgetInfoTreeNode* findNodeByName( const std::string &name )
		{
			if( info.name == name )
				return this;

			CWidgetInfoTreeNode *node = NULL;

			for( std::vector< CWidgetInfoTreeNode* >::iterator itr = children.begin(); itr != children.end(); ++itr )
			{
				node = ( *itr )->findNodeByName( name );
				if( node != NULL )
					return node;
			}

			return NULL;
		}

		/// Get the node names and put them into the vector
		void getNames( std::vector< std::string > &v, bool includeAbstract = true ) const
		{
			if( !info.isAbstract || ( info.isAbstract && includeAbstract ) )
				v.push_back( info.name );

			for( std::vector< CWidgetInfoTreeNode* >::const_iterator itr = children.begin(); itr != children.end(); ++itr )
				( *itr )->getNames( v, includeAbstract );
		}

		/// Accepts a visitor to itself and to the children nodes
		void accept( CWidgetInfoTreeVisitor *visitor )
		{
			visitor->visit( this );
			for( std::vector< CWidgetInfoTreeNode* >::iterator itr = children.begin(); itr != children.end(); ++itr )
				( *itr )->accept( visitor );
		}

	private:
		SWidgetInfo info;
		CWidgetInfoTreeNode *parent;
		std::vector< CWidgetInfoTreeNode* > children;

	};

}

#endif
