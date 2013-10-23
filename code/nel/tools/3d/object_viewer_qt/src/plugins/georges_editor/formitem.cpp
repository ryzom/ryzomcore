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

#include "stdpch.h"
#include "formitem.h"

// Qt includes

// NeL includes
#include <nel/misc/o_xml.h>
#include <nel/georges/u_type.h>
#include <nel/georges/form.h>

namespace GeorgesQt 
{

	CFormItem::CFormItem(NLGEORGES::UFormElm* elm, const QList<QVariant> &data, CFormItem *parent,
		NLGEORGES::UFormElm::TWhereIsValue wV, NLGEORGES::UFormElm::TWhereIsNode wN) 
	{
		parentItem = parent;
		itemData = data;
		formElm = elm;
		whereV = wV;
		whereN = wN;
	}

	CFormItem::~CFormItem() 
	{
		qDeleteAll(childItems);
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
		return itemData.count();
	}

	QVariant CFormItem::data(int column) const 
	{
		return itemData.value(column);
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
		if (column < 0 || column >= itemData.size())
			return false;

		// TODO: default values
		if (!formElm)
			return false;

		itemData[column] = value;
		if (formElm->isAtom()) 
		{
			const NLGEORGES::UType *type = formElm->getType();
			if (type) 
			{
				switch (type->getType()) 
				{
				case NLGEORGES::UType::UnsignedInt:
				case NLGEORGES::UType::SignedInt:
				case NLGEORGES::UType::Double:
				case NLGEORGES::UType::String:
					if (parentItem->formElm->isArray())
					{
						//((NLGEORGES::CFormElm*)parentItem->formElm);//->arrayInsertNodeByName(
						//if(parentItem->formElm->getArrayNode(elmName, num))
						//{
						//}

						bool ok;
						// TODO: the node can be renamed from eg "#0" to "foobar"
						int arrayIndex = itemData[0].toString().remove("#").toInt(&ok);
						if(ok)
						{
							NLGEORGES::UFormElm *elmt = 0;
							if(parentItem->formElm->getArrayNode(&elmt, arrayIndex) && elmt)
							{
								if (elmt->isAtom()) 
								{
									((NLGEORGES::CFormElmAtom*)elmt)->setValue(value.toString().toUtf8().constData());
									nldebug(QString("array element string %1 %2")
									.arg(itemData[0].toString()).arg(value.toString())
									.toUtf8().constData());
								}
							}
						}
					}
					else
					{
						if(parentItem->formElm->setValueByName(
							value.toString().toUtf8().constData(),
							itemData[0].toString().toUtf8().constData()))
						{
							nldebug(QString("string %1 %2")
							.arg(itemData[0].toString()).arg(value.toString())
							.toUtf8().constData());
						}
						else
						{
							nldebug(QString("FAILED string %1 %2")
							.arg(itemData[0].toString()).arg(value.toString())
							.toUtf8().constData());
						}
					}
					break;
				case NLGEORGES::UType::Color:
					nldebug("Color is TODO");
					break;
				default:
					break;
				}
			}
		}
		else
		{
			nldebug("setting sth other than Atom");
		}
		//formElm->setValueByName();
		return true;
	}
}
