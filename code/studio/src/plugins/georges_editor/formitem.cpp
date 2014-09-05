// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian Jaekel <aj at elane2k dot com>
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

// Project includes
#include "formitem.h"
#include "actions.h"
#include "georges_editor_form.h"

// Qt includes
#include <QIcon>

// NeL includes
#include <nel/misc/o_xml.h>
#include <nel/misc/debug.h>
#include <nel/georges/u_type.h>
#include <nel/georges/form.h>

using namespace NLGEORGES;

namespace GeorgesQt 
{
	CFormItem::CFormItem()
	{
		parentItem = NULL;
		formElm = NULL;
		m_form = NULL;
		_StructId = 0;
		_Slot = 0;
		_Type = Null;
		_Array = false;
	}

	CFormItem::~CFormItem() 
	{
		clearChildren();
	}

	void CFormItem::appendChild(CFormItem *item) 
	{
		childItems.append(item);
	}

	CFormItem *CFormItem::child(int row) 
	{
		return childItems.value(row);
	}

	int CFormItem::childCount() const 
	{
		return childItems.count();
	}

	int CFormItem::columnCount() const 
	{
		//nlinfo("columnCount %d",itemData.count());
		//return itemData.count();
		return 1;
	}

	QVariant CFormItem::data(int column) const 
	{
		//return itemData.value(column);
		return QVariant(_Name.c_str());
	}

	CFormItem *CFormItem::parent()
	{
		return parentItem;
	}

	int CFormItem::row() const 
	{
		if (parentItem)
			return parentItem->childItems.indexOf(const_cast<CFormItem*>(this));

		return 0;
	}

	bool CFormItem::setData(int column, const QVariant &value) 
	{
		nlwarning("This should not be called anymore.");
		return false;
	}

	bool CFormItem::isEditable(int column)
	{
		// Ensure only valid types can be edited.
		if(_Type == Null)
			return false;

		// Make sure only the first column (name) can be editted.
		if (column != 0)
			return false;

		if(isArrayMember())
			return true;

		return false;
	}

	bool CFormItem::isArray()
	{
		return _Array;
	}

	bool CFormItem::isArrayMember()
	{
		if( parentItem == NULL )
			return false;

		return parentItem->isArray();
	}

	QIcon CFormItem::getItemImage(CFormItem *rootItem)
	{
		if(_Type == CFormItem::Null)
		{
				return QIcon(":/images/root.ico");
		}
		else if(_Type == CFormItem::Form)
		{
			// If the parent is the root item then this is the content.
			if(parentItem == rootItem)
				return QIcon(":/images/root.ico");

			// If it wasn't a root node then lets check the node type.
			const NLGEORGES::CFormDfn *parentDfn;
			uint indexDfn;
			const NLGEORGES::CFormDfn *nodeDfn;
			const NLGEORGES::CType *nodeType;
			NLGEORGES::CFormElm *node;
			NLGEORGES::UFormDfn::TEntryType type;
			bool array;
			bool parentVDfnArray;
			NLGEORGES::CForm *form = static_cast<CForm*>(m_form);
			NLGEORGES::CFormElm *elm = static_cast<CFormElm*>(&form->getRootNode());
			nlverify ( elm->getNodeByName (_FormName.c_str(), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, true, NLGEORGES_FIRST_ROUND) );

			if(array)
			{
				return QIcon(":/images/array.ico");
			}
			else
			{
				if(type == NLGEORGES::UFormDfn::EntryType)
				{
					if(parentDfn)
					{
						// Not sure what the hell to do with this. Gets filename from parent dfn?
					}
					return QIcon(":/images/zfee51.ico");
				}
				else if(type == NLGEORGES::UFormDfn::EntryDfn)
				{
					if(parentDfn)
					{
						// Not sure what the hell to do with this. Gets filename from parent dfn?
					}
					return QIcon(":/images/struct.ico");
				}
				else if(type == NLGEORGES::UFormDfn::EntryVirtualDfn)
				{
					if(node)
					{
						// Not sure what the hell to do with this. Gets filename from parent dfn?
						std::string dfnName;
						NLMISC::safe_cast<NLGEORGES::CFormElmVirtualStruct*>(node)->getDfnName(dfnName);
						// return dfnName.c_str() ?
					}
					return QIcon(":/images/vstruct.ico");
				}
			}
			//return QIcon(":/images/struct.ico");
		}
		return QIcon();
	}

	CFormItem* CFormItem::findItem( const QString &formName )
	{
		CFormItem *item = NULL;

		if( _FormName.c_str() == formName )
			return this;

		for( int i = 0; i < childItems.count(); i++ )
		{
			item = childItems[ i ]->findItem( formName );
			if( item != NULL )
				return item;
		}

		return item;
	}

	void CFormItem::clearChildren()
	{
		qDeleteAll( childItems );
		childItems.clear();
	}

	CFormItem *CFormItem::add (TSub type, const char *name, uint structId, const char *formName, uint slot, NLGEORGES::UForm *formPtr, bool isArray)
    {
		CFormItem *newNode = new CFormItem();
        newNode->_Type = type;
        newNode->_Name = name;
        newNode->parentItem = this;
        newNode->_StructId = structId;
        newNode->_FormName = formName;
        newNode->_Slot  = slot;		
		newNode->m_form = formPtr;
		newNode->_Array = isArray;

        appendChild(newNode);
        return newNode;
    }

}
