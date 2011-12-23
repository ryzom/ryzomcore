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

Node::Node() : m_parentItem(0)
{
}

Node::Node(const QVector<QVariant> &data, Node *parent)
{
	m_parentItem = parent;
	m_itemData = data;
}

Node::~Node()
{
	qDeleteAll(m_childItems);
}

void Node::appendChild(Node *item)
{
	m_childItems.append(item);
}

Node *Node::child(int row)
{
	//nlinfo("row %d and size %d", row, childItems.size());
	return m_childItems.value(row);
}

int Node::childCount() const
{
	return m_childItems.count();
}

int Node::childNumber() const
{
	if(m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<Node*>(this));
	return 0;
}

bool Node::insertChildren(int position, int count, int columns)
{
	if(position<0 || position>m_childItems.size())
		return false;

	for(int row = 0; row < count; row++)
	{
		QVector<QVariant> data(columns);
		Node *item = new Node(data, this);
		m_childItems.insert(position, item);
	}

	return true;
}

bool Node::removeChildren(int position, int count)
{
	if(position<0 || position+count>m_childItems.size())
		return false;

	for(int row=0; row<count; row++)
		delete m_childItems.takeAt(position);
	
	return true;
}

bool Node::insertColumns(int position, int columns)
{
	if(position<0 || position > m_itemData.size())
		return false;

	for(int column=0; column<columns; column++)
		m_itemData.insert(position, columns);

	Q_FOREACH(Node *child, m_childItems)
		child->insertColumns(position, columns);

	return true;
}

int Node::row() const
{
	if(m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<Node*>(this));

	return 0;
}

int Node::columnCount() const
{
	return m_itemData.count();
}

QVariant Node::data(int column) const
{
	return m_itemData.value(column);
}

bool Node::setData(int column, const QVariant &value)
{
	if(column < 0 || column >= m_itemData.size())
		return false;

	m_itemData[column] = value;
	return true;
}

Node *Node::parent()
{
	return m_parentItem;
}

void Node::setParent(Node *parent)
{
	m_parentItem = parent;
}

void Node::appendRow(const QList<Node*> &items)
{
	Q_FOREACH(Node *item, items)
		appendRow(item);
}

void Node::appendRow(Node *item)
{
	item->setParent(this);
	m_childItems.append(item);
}

///////////////////////////////////////////////////

TileSetNode::TileSetNode(QString tileSetName, Node *parent) : m_tileSetName(tileSetName)
{
	m_parentItem = parent;
}

TileSetNode::~TileSetNode()
{
	qDeleteAll(m_childItems);
}

QVariant TileSetNode::data(int column) const
{
	return QVariant(m_tileSetName);
}

///////////////////////////////////////////////////

TileTypeNode::TileTypeNode(TNodeTileType type, Node *parent) : m_nodeTileType(type)
{
	m_parentItem = parent;
}

TileTypeNode::~TileTypeNode()
{
	qDeleteAll(m_childItems);
}

QVariant TileTypeNode::data(int column) const
{
	return QVariant(getTileTypeName(m_nodeTileType));
}

TileTypeNode::TNodeTileType TileTypeNode::getTileType()
{
	return m_nodeTileType;
}


const char *TileTypeNode::getTileTypeName(TNodeTileType type)
{
	switch(type)
	{
	case Tile128:
		return "128";
	case Tile256:
		return "256";
	case TileTransition:
		return "Transition";
	case TileDisplacement:
		return "Displacement";
	default:
		break;
	}
	return "UNKNOWN";
}

///////////////////////////////////////////////////

TileItemNode::TileItemNode(int tileId, TTileChannel channel, QString filename, Node *parent) : m_tileId(tileId)
{
	m_tileFilename[channel] = filename;
	m_parentItem = parent;
	nlinfo("dispalying tile %d - %s", m_tileId, m_tileFilename[TileDiffuse].toStdString().c_str());
}

TileItemNode::~TileItemNode()
{
	qDeleteAll(m_childItems);
}

void TileItemNode::setTileFilename(TTileChannel channel, QString filename)
{
	m_tileFilename[channel] = filename;
}

QVariant TileItemNode::data(int column) const
{
	nlinfo("dispalying tile %d - %s", m_tileId, m_tileFilename[TileDiffuse].toStdString().c_str());
	// find some way to know which file/bitmap to display
	return QVariant(m_tileFilename[TileDiffuse]);
}
