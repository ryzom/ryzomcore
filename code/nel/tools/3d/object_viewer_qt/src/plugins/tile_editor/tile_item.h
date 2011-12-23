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

class Node
{
public:
	Node();
	Node(const QVector<QVariant> &data, Node *parent=0);
	virtual ~Node();

	void appendChild(Node *child);

	Node *child(int row);
	int childCount() const;
	int childNumber() const;
	int columnCount() const;
	bool setData(int column, const QVariant &value);
	virtual QVariant data(int column) const;

	bool insertChildren(int position, int count, int columns);
	bool removeChildren(int position, int count);
	bool insertColumns(int position, int columns);

	int row() const;
	Node *parent();
	void setParent(Node *parent);

	void appendRow(const QList<Node*> &items);
	void appendRow(Node *item);

protected:
	QList<Node*> m_childItems;
	QVector<QVariant> m_itemData;
	Node *m_parentItem;
};

class TileSetNode : public Node
{
public:
	TileSetNode(QString tileSetName, Node *parent=0);
	virtual ~TileSetNode();
	QVariant data(int column) const;

	const QString &getTileSetName();
private:
	QString m_tileSetName;
};

class TileTypeNode : public Node
{
public:
	enum TNodeTileType
	{
		Tile128 = 0,
		Tile256 = 1,
		TileTransition = 2,
		TileDisplacement = 3
	};

	TileTypeNode(TNodeTileType type, Node *parent=0);
	virtual ~TileTypeNode();
	QVariant data(int column) const;

	TNodeTileType getTileType();

	static const char *getTileTypeName(TNodeTileType type);
private:
	TNodeTileType m_nodeTileType;
};

class TileItemNode : public Node
{
public:
	enum TTileChannel
	{
		TileDiffuse = 0,
		TileAdditive = 1,
		TileAlpha = 2,
	};

	TileItemNode(int tileId, TTileChannel channel, QString filename, Node *parent=0);
	virtual ~TileItemNode();
	QVariant data(int column) const;
	void setTileFilename(TTileChannel channel, QString filename);
private:
	int m_tileId;
	QMap<TTileChannel, QString> m_tileFilename;
};

#endif // TILE_ITEM_H
