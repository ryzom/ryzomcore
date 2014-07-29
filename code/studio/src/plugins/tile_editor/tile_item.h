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

#include "tile_model.h"

class TileWidget;

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
	virtual int columnCount() const;
	bool setData(int column, const QVariant &value);
	virtual QVariant data(int column, int role) const;

	bool insertChildren(int position, int count, int columns);
	virtual bool removeChildren(int position, int count);
	bool insertColumns(int position, int columns);

	int row() const;
	Node *parent();
	void setParent(Node *parent);

	void appendRow(const QList<Node*> &items);
	void appendRow(Node *item);

	void swapRows( int a, int b );

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
	QVariant data(int column, int role) const;
	int columnCount() const;

	const QString &getTileSetName(){ return m_tileSetName; }
	void setTileSetName( const QString &name ){ m_tileSetName = name; }

	void setVegetSet( const QString &s ){ m_vegetSet = s; }
	QString vegetSet() const{ return m_vegetSet; }

	bool isOriented() const{ return m_oriented; }
	void setOriented( bool b ){ m_oriented = b; }

private:
	QString m_tileSetName;
	QString m_vegetSet;
	bool m_oriented;
};

class TileTypeNode : public Node
{
public:

	TileTypeNode(TileModel::TNodeTileType type, Node *parent=0);
	virtual ~TileTypeNode();
	QVariant data(int column, int role) const;
	int columnCount() const;

	TileModel::TNodeTileType getTileType();

	bool removeChildren( int position, int count );

private:
	void reindex();
	TileModel::TNodeTileType m_nodeTileType;
};

class TileItemNode : public Node
{
public:
	TileItemNode(int tileId, TileModel::TTileChannel channel, QString filename, Node *parent=0);
	virtual ~TileItemNode();
	QVariant data(int column, int role) const;
	int columnCount() const;
	void setTileFilename(TileModel::TTileChannel channel, QString filename);
	QString getTileFilename(TileModel::TTileChannel channel);
	void setId( int id ){ m_tileId = id; }
	int id() const{ return m_tileId; }
private:
	int m_tileId;
	QMap<TileModel::TTileChannel, QString> m_tileFilename;
	QMap<TileModel::TTileChannel, TileWidget*> m_tileWidget;
};

#endif // TILE_ITEM_H
