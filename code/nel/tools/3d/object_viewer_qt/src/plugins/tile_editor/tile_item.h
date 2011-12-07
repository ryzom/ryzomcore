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

#ifndef TILE_ITEM_H
#define TILE_ITEM_H

#include "nel/misc/types_nl.h"

#include <QAbstractListModel>
#include <QVector>

class TileItem
{
public:

	TileItem(const QVector<QVariant> &data, TileItem *parent=0);
	virtual ~TileItem();

	void appendChild(TileItem *child);

	TileItem *child(int row);
	int childCount() const;
	int childNumber() const;
	int columnCount() const;
	bool setData(int column, const QVariant &value);
	QVariant data(int column) const;

	bool insertChildren(int position, int count, int columns);
	bool removeChildren(int position, int count);
	bool insertColumns(int position, int columns);

	int row() const;
	TileItem *parent();
	void setParent(TileItem *parent);

	void appendRow(const QList<TileItem*> &items);
	void appendRow(TileItem *item);

	//QImage *getTileImageFromChannel(int channel);

	//int getTileIndex() { return m_tileIndex; }
	//QString getTileFilename() { return m_tileFilename; }

protected:
	QList<TileItem*> childItems;
	QVector<QVariant> itemData;
	TileItem *parentItem;

	//QMap<int, QImage*> m_tileChannels;
	//int m_tileIndex;
	//QString m_tileFilename;
};

class TileTypeTileItem : public TileItem
{
public:
	TileTypeTileItem(const QVector<QVariant> &data, TileItem *parent=0);
	virtual ~TileTypeTileItem();
	QVariant data(int column) const;
};

#endif // TILE_ITEM_H
