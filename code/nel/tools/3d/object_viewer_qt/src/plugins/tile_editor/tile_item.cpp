// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "tile_item.h"

#include <nel/misc/debug.h>

TileItem::TileItem(const QVector<QVariant> &data, TileItem *parent)
{
	parentItem = parent;
	itemData = data;
}

TileItem::~TileItem()
{
	qDeleteAll(childItems);
}

void TileItem::appendChild(TileItem *item)
{
	childItems.append(item);
}

TileItem *TileItem::child(int row)
{
	return childItems.value(row);
}

int TileItem::childCount() const
{
	return childItems.count();
}

int TileItem::childNumber() const
{
	if(parentItem)
		return parentItem->childItems.indexOf(const_cast<TileItem*>(this));
	return 0;
}

bool TileItem::insertChildren(int position, int count, int columns)
{
	if(position<0 || position>childItems.size())
		return false;

	for(int row = 0; row < count; row++)
	{
		QVector<QVariant> data(columns);
		TileItem *item = new TileItem(data, this);
		childItems.insert(position, item);
	}

	return true;
}

bool TileItem::removeChildren(int position, int count)
{
	if(position<0 || position+count>childItems.size())
		return false;

	for(int row=0; row<count; row++)
		delete childItems.takeAt(position);
	
	return true;
}

bool TileItem::insertColumns(int position, int columns)
{
	if(position<0 || position > itemData.size())
		return false;

	for(int column=0; column<columns; column++)
		itemData.insert(position, columns);

	Q_FOREACH(TileItem *child, childItems)
		child->insertColumns(position, columns);

	return true;
}

int TileItem::row() const
{
	if(parentItem)
		return parentItem->childItems.indexOf(const_cast<TileItem*>(this));

	return 0;
}

int TileItem::columnCount() const
{
	return itemData.count();
}

QVariant TileItem::data(int column) const
{
	return itemData.value(column);
}

bool TileItem::setData(int column, const QVariant &value)
{
	if(column < 0 || column >= itemData.size())
		return false;

	itemData[column] = value;
	return true;
}

TileItem *TileItem::parent()
{
	return parentItem;
}

void TileItem::setParent(TileItem *parent)
{
	parentItem = parent;
}

void TileItem::appendRow(const QList<TileItem*> &items)
{
	Q_FOREACH(TileItem *item, items)
		appendRow(item);
}

void TileItem::appendRow(TileItem *item)
{
	nlinfo("number of children: %d", childItems.size());
	item->setParent(this);
	childItems.append(item);
	nlinfo("number of children: %d", childItems.size());
}
//QImage *TileItem::getTileImageFromChannel(int channel)
//{
//	return m_tileChannels[channel];
//}


///////////////////////////////////////////////////

TileTypeTileItem::TileTypeTileItem(const QVector<QVariant> &data, TileItem *parent) : TileItem(data,parent)
{
}

TileTypeTileItem::~TileTypeTileItem()
{
	qDeleteAll(childItems);
}

QVariant TileTypeTileItem::data(int column) const
{
	QVariant val = itemData.value(column);

	nlinfo("the column is %d and the value is '%s'. there are %d values",
		column, val.toString().toStdString().c_str(), itemData.size());
	return itemData.value(column);
}