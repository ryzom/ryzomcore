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

#include <QPixmap>

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
	m_childItems.clear();
}

void Node::appendChild(Node *item)
{
	item->setParent( this );
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

QVariant Node::data(int column, int role) const
{
	if(role == Qt::DisplayRole || 
		role == TileModel::TileFilenameIndexRole || 
		role == TileModel::TileFilenameRole || 
		role == TileModel::TileIndexRole)
		return m_itemData.value(column);
	return QVariant();
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

void Node::swapRows( int a, int b )
{
	Node *temp = m_childItems[ a ];
	m_childItems[ a ] = m_childItems[ b ];
	m_childItems[ b ] = temp;
}

void Node::clear()
{
	qDeleteAll( m_childItems );
	m_childItems.clear();
	m_itemData.clear();
}

///////////////////////////////////////////////////

TileSetNode::TileSetNode(QString tileSetName, Node *parent) : m_tileSetName(tileSetName)
{
	m_parentItem = parent;
}

TileSetNode::~TileSetNode()
{
	qDeleteAll(m_childItems);
	m_childItems.clear();
}

QVariant TileSetNode::data(int column, int role) const
{
	if(role == Qt::DisplayRole || 
		role == TileModel::TileFilenameIndexRole || 
		role == TileModel::TileFilenameRole || 
		role == TileModel::TileIndexRole)
		return QVariant(m_tileSetName);
	return QVariant();
}

int TileSetNode::columnCount() const
{
	return 1;
}

///////////////////////////////////////////////////

TileTypeNode::TileTypeNode(TileConstants::TNodeTileType type, Node *parent) : m_nodeTileType(type)
{
	m_parentItem = parent;
}

TileTypeNode::~TileTypeNode()
{
	qDeleteAll(m_childItems);
	m_childItems.clear();
}

QVariant TileTypeNode::data(int column, int role) const
{
	if(role == Qt::DisplayRole || 
		role == TileModel::TileFilenameIndexRole || 
		role == TileModel::TileFilenameRole || 
		role == TileModel::TileIndexRole)
		return QVariant(TileModel::getTileTypeName(m_nodeTileType));
	return QVariant();
	
}

int TileTypeNode::columnCount() const
{
	return 1;
}

TileConstants::TNodeTileType TileTypeNode::getTileType()
{
	return m_nodeTileType;
}

bool TileTypeNode::removeChildren( int position, int count )
{
	bool ok = Node::removeChildren( position, count );
	if( !ok )
		return false;

	reindex();

	return true;
}

void TileTypeNode::reindex()
{
	int i = 0;

	QListIterator< Node* > itr( m_childItems );
	while( itr.hasNext() )
	{
		TileItemNode *n = dynamic_cast< TileItemNode* >( itr.next() );
		if( n == NULL )
			continue;

		n->setId( i );

		i++;
	}
}


///////////////////////////////////////////////////

class TileItemNodePvt
{
public:

	TileItemNodePvt()
	{
	}

	bool loadImage( TileConstants::TTileChannel channel, const QString &fn, bool empty = false )
	{
		QPixmap temp;
		bool b = temp.load( fn );

		if( !b )
		{
			return false;
		}

		pixmaps[ channel ] = temp;
		
		return true;
	}

	void clearImage( TileConstants::TTileChannel channel )
	{
		pixmaps[ channel ] = QPixmap();
	}

	const QPixmap& pixMap( TileConstants::TTileChannel channel ) const{
		return pixmaps[ channel ];
	}

private:
	QPixmap pixmaps[ TileConstants::TileChannelCount ];
};

TileConstants::TTileChannel TileItemNode::s_displayChannel = TileConstants::TileDiffuse;
int TileItemNode::s_alphaRot = 0;

TileItemNode::TileItemNode( TileConstants::TNodeTileType type, int tileId, TileConstants::TTileChannel channel, QString filename, Node *parent)
{
	m_id = tileId;
	m_parentItem = parent;
	//nlinfo("dispalying tile %d - %s", m_tileId, m_tileFilename[TileModel::TileDiffuse].toAscii().data());

	pvt = new TileItemNodePvt();
	m_hasError = false;

	setTileFilename( channel, filename );
}

TileItemNode::TileItemNode( TileConstants::TNodeTileType type, int tileId, Node *parent )
{
	m_id = tileId;
	m_parentItem = parent;
	pvt = new TileItemNodePvt();
	m_hasError = false;
}

TileItemNode::~TileItemNode()
{
	delete pvt;
	pvt = NULL;

	qDeleteAll(m_childItems);
}

bool TileItemNode::setTileFilename(TileConstants::TTileChannel channel, QString filename)
{
	QString fn = filename;
	bool empty = false;

	if( filename.isEmpty() || ( filename == "empty" ) )
	{
		fn = ":/placeHolder/images/empty_image.png";
		empty = true;
	}

	bool b = pvt->loadImage( channel, fn, empty );
	m_hasError = !b;
	if( !b )
		return false;

	m_tileFilename[channel] = filename;
	return true;
}

QString TileItemNode::getTileFilename(TileConstants::TTileChannel channel)
{
	QMap< TileConstants::TTileChannel, QString >::const_iterator itr
		= m_tileFilename.find( channel );
	if( itr == m_tileFilename.end() )
		return "";

	return itr.value();
}

QVariant TileItemNode::data(int column, int role) const
{	
	QString tileFilename = m_tileFilename[ TileItemNode::s_displayChannel ];

	if(role == TileModel::TilePixmapRole || role == Qt::DecorationRole)
	{
		TileTypeNode *parent = dynamic_cast<TileTypeNode*>(m_parentItem);
		if(parent == NULL)
			return QVariant();

		// Retrieve the target tile size.
		uint32 tileSize = TileModel::getTileTypeSize(parent->getTileType());

		if(TileModel::CurrentZoomFactor == TileModel::TileZoom200)
			tileSize *= 2;
		else if(TileModel::CurrentZoomFactor == TileModel::TileZoom50)
			tileSize /= 2;

		QPixmap pixmap = pvt->pixMap( TileItemNode::s_displayChannel );

		pixmap = pixmap.scaled(tileSize, tileSize);

		return pixmap;
	}
	else if(role == Qt::DisplayRole)
	{
		return QVariant(tileFilename);
	}
	else if(role == TileModel::TileFilenameRole)
	{
		return QVariant(tileFilename);
	}
	else if(role == TileModel::TileIndexRole)
	{
		return QVariant("("+QString::number(m_id)+")");
	}
	else if(role == TileModel::TileFilenameIndexRole)
	{
		return QVariant(tileFilename + " ("+QString::number(m_id)+")");
	}

	return QVariant();
}

int TileItemNode::columnCount() const
{
	return 1;
}

QVariant TileItemNode::pixmap( TileConstants::TTileChannel channel ) const
{
	return pvt->pixMap( channel );
}

