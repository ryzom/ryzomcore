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

#ifndef FORMITEM_H
#define FORMITEM_H

// NeL includes
#include <nel/georges/u_form.h>
#include <nel/georges/u_form_elm.h>

// Qt includes
#include <QList>
#include <QVariant>

namespace GeorgesQt 
{

	class CFormItem
	{
	public:
		// What is the sub object ?
        enum TSub
        {
                Null,           // Nothing in this node (root ?)
                Header,         // Header node
                Type,           // This node is a type
                Dfn,            // This node is a dfn
                Form,           // This node is a form
        };

		enum TType
		{
			TYPE_ARRAY,
			TYPE_STRUCT,
			TYPE_VSTRUCT,
			TYPE_ATOM
		};

		CFormItem();
		~CFormItem();

		void appendChild(CFormItem *child);

        CFormItem *add (TSub type, const char *name, uint structId, const char *formName, uint slot, NLGEORGES::UForm *formPtr, TType itemType );

		CFormItem *child(int row);
		int childCount() const;
		int columnCount() const;
		QVariant data(int column) const;
		int row() const;
		CFormItem *parent();
		bool setData(int column, const QVariant &value);

		TSub nodeType() { return _Type; }
		std::string formName() { return _FormName; }
		
		std::string name() { return _Name; }
		void setName(std::string name) { _Name = name; }

		uint structId() { return _StructId; }

		NLGEORGES::UForm *form() { return m_form; }

		bool isEditable(int column);
		bool isArray();
		bool isArrayMember();
		bool isStruct();
		bool isVStruct();
		bool isAtom();

		QIcon getItemImage(CFormItem *rootItem);

		CFormItem* findItem( const QString &formName );

		void clearChildren();

		void removeChild( int idx );

		bool rootItem() const{
			if( parentItem == NULL )
				return true;
			else
				return false;
		}

    private:
		QList<CFormItem*> childItems;
		QList<QVariant> itemData;
		CFormItem *parentItem;
		NLGEORGES::UFormElm* formElm;
		NLGEORGES::UForm *m_form;

		uint                                                            _StructId;
        std::string                                                     _Name;
        std::string                                                     _FormName;
        TSub                                                            _Type;
        uint                                                            _Slot;
		TType															_TType;

	}; // CFormItem

}
#endif // FORMITEM_H
