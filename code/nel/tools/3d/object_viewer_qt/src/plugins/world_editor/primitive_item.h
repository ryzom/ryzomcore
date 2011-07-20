// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef PRIMITIVE_ITEM_H
#define PRIMITIVE_ITEM_H

// Project includes

// NeL includes
#include <nel/ligo/primitive.h>

// Qt includes
#include <QList>
#include <QIcon>
#include <QVariant>

namespace WorldEditor
{

/*
@class BaseTreeItem
@brief
@details
*/
class BaseTreeItem
{
public:
	BaseTreeItem(BaseTreeItem *parent = 0);
	BaseTreeItem(const QList<QVariant> &data, BaseTreeItem *parent = 0);
	virtual ~BaseTreeItem();

	void appendChild(BaseTreeItem *child);

	BaseTreeItem *child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	void setData(int column, const QVariant &data);
	int row() const;
	BaseTreeItem *parent();
	void setModified(bool value);
	bool isModified() const;

private:

	bool m_modified;
	QList<BaseTreeItem *> m_childItems;
	QList<QVariant> m_itemData;
	BaseTreeItem *m_parentItem;
};

/*
@class PrimitiveItem
@brief
@details
*/
class PrimitiveItem: public BaseTreeItem
{
public:
	PrimitiveItem(NLLIGO::IPrimitive *primitive, BaseTreeItem *parent);
	PrimitiveItem(const PrimitiveItem &other);
	virtual ~PrimitiveItem();

private:
	NLLIGO::IPrimitive *m_primitive;
};

/*
@class PrimitivesItem
@brief
@details
*/
class PrimitivesItem: public PrimitiveItem
{
public:
	PrimitivesItem(const QString &name, NLLIGO::CPrimitives *primitives, BaseTreeItem *parent);
	PrimitivesItem(const PrimitiveItem &other);
	virtual ~PrimitivesItem();

private:
	NLLIGO::CPrimitives *m_primitives;
};

} /* namespace WorldEditor */

#endif // PRIMITIVE_ITEM_H
