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
#include <nel/georges/u_form_elm.h>

// Qt includes
#include <QList>
#include <QVariant>

namespace GeorgesQt 
{

	class CFormItem

	{
	public:
		CFormItem(NLGEORGES::UFormElm *elm, const QList<QVariant> &data, 
			CFormItem *parent = 0,
            NLGEORGES::UFormElm::TWhereIsValue wV = NLGEORGES::UFormElm::ValueForm,
            NLGEORGES::UFormElm::TWhereIsNode wN = NLGEORGES::UFormElm::NodeForm);
		~CFormItem();

		void appendChild(CFormItem *child);
        //CFormItem *add (/*TSub type,*/ const char *name, uint structId, const char *formName, uint slot);
        CFormItem *add(NLGEORGES::UFormElm* root, std::string elmName);

		CFormItem *child(int row);
		int childCount() const;
		int columnCount() const;
		QVariant data(int column) const;
		int row() const;
		CFormItem *parent();
		bool setData(int column, const QVariant &value);
		NLGEORGES::UFormElm* getFormElm() {return formElm;}
		NLGEORGES::UFormElm::TWhereIsValue valueFrom() 
		{
			return whereV;
		}
		NLGEORGES::UFormElm::TWhereIsNode nodeFrom() 
		{
			return whereN;
		}

        void setValueFrom(NLGEORGES::UFormElm::TWhereIsValue wV) { whereV = wV; }
        void setNodeFrom(NLGEORGES::UFormElm::TWhereIsNode wN) { whereN = wN; }


    private:
        //CFormItem() { whereV = NLGEORGES::UFormElm::ValueForm; whereN = NLGEORGES::UFormElm::NodeForm; }
		QList<CFormItem*> childItems;
		QList<QVariant> itemData;
		CFormItem *parentItem;
		NLGEORGES::UFormElm* formElm;
        NLGEORGES::UFormElm::TWhereIsValue whereV;
        NLGEORGES::UFormElm::TWhereIsNode whereN;
	}; // CFormItem

}
#endif // FORMITEM_H
