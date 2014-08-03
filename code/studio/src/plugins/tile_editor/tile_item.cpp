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

//#include "tile_widget.h"

#include <nel/misc/debug.h>

#include <nel/3d/tile_bank.h>

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
	m_oriented = false;
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

TileTypeNode::TileTypeNode(TileModel::TNodeTileType type, Node *parent) : m_nodeTileType(type)
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

TileModel::TNodeTileType TileTypeNode::getTileType()
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

NL3D::CTile::TBitmap channelToTBitmap( TileModel::TTileChannel channel )
{
	NL3D::CTile::TBitmap bm;

	switch( channel )
	{
	case TileModel::TileDiffuse: bm = NL3D::CTile::diffuse; break;
	case TileModel::TileAdditive: bm = NL3D::CTile::additive; break;
	case TileModel::TileAlpha: bm = NL3D::CTile::alpha; break;
	}

	return bm;
}

class TileItemNodePvt
{
public:

	TileItemNodePvt()
	{
		for( int i = 0; i < TileModel::TileChannelCount; i++ )
			m_borderFirst[ i ] = false;

		m_id = -1;
	}

	bool pixmapToCBGRA( QPixmap &pixmap, std::vector< NLMISC::CBGRA >& pixels )
	{
		QImage img = pixmap.toImage();
		if( img.format() != QImage::Format_ARGB32 )
			img = img.convertToFormat( QImage::Format_ARGB32 );

		if( img.format() != QImage::Format_ARGB32 )
			return false;

		int c = img.width() * img.height();

		const unsigned char *data = img.bits();
		const unsigned int *idata = reinterpret_cast< const unsigned int* >( data );

		NLMISC::CBGRA bgra;
		pixels.clear();

		int i = 0;
		while( i < c )
		{
			bgra.A = ( idata[ i ] & 0xFF000000 ) >> 24;
			bgra.R = ( idata[ i ] & 0x00FF0000 ) >> 16;
			bgra.G = ( idata[ i ] & 0x0000FF00 ) >> 8;
			bgra.B = ( idata[ i ] & 0x000000FF );
			pixels.push_back( bgra );

			i++;
		}

		return true;
	}

	int getWidthForType( TileModel::TNodeTileType type, TileModel::TTileChannel channel )
	{
		int width = -1;

		switch( type )
		{
		case TileModel::Tile128: width = 128; break;
		case TileModel::Tile256: width = 256; break;
		case TileModel::TileTransition:
			{
				if( channel != TileModel::TileAlpha )
					width = 128;
				break;
			}

		case TileModel::TileDisplacement: width = 32; break;
		}

		return width;
	}

	NL3D::CTileSet::TError checkTile( TileModel::TTileChannel channel )
	{
		if( m_type == TileModel::TileDisplacement )
			return NL3D::CTileSet::ok;

		if( channel == TileModel::TileAdditive )
			return NL3D::CTileSet::ok;

		int pixel;
		int component;
		int index;

		NL3D::CTileSet set;	
		NL3D::CTile::TBitmap bm = channelToTBitmap( channel );

		NL3D::CTileSet::TError error = NL3D::CTileSet::ok;

		switch( m_type )
		{
		case TileModel::Tile128:
			error = set.checkTile128( bm, m_border[ channel ], pixel, component );
			break;
		case TileModel::Tile256:
			error = set.checkTile256( bm, m_border[ channel ], pixel, component );
			break;
		case TileModel::TileTransition:
			if( channel != TileModel::TileAlpha )
				error = set.checkTile128( bm, m_border[ channel ], pixel, component );
			else
				error = set.checkTileTransition( NL3D::CTileSet::TTransition( m_id ), bm, m_border[ channel ], index, pixel, component );
			break;
		}

		if( error == NL3D::CTileSet::ok )
			return NL3D::CTileSet::ok;
		if( error == NL3D::CTileSet::addFirstA128128 )
			return NL3D::CTileSet::addFirstA128128;

		static const char* comp[]={"Red", "Green", "Blue", "Alpha", ""};

		if( error != NL3D::CTileSet::ok )
		{
			m_lastError = "ERROR: ";
			m_lastError += NL3D::CTileSet::getErrorMessage( error );
			m_lastError += "\n";

			switch( m_type )
			{
			case TileModel::Tile128:
			case TileModel::Tile256:
				m_lastError += QString( "pixel: %1 component: %2" ).arg( pixel ).arg( comp[ component ] );
				break;

			case TileModel::TileTransition:
				if( channel != TileModel::TileAlpha )
				{
					m_lastError += QString( "pixel: %1 component: %2" ).arg( pixel ).arg( comp[ component ] );
				}
				else
				{
					if ((error==NL3D::CTileSet::topInterfaceProblem)||(error==NL3D::CTileSet::bottomInterfaceProblem)||
						(error==NL3D::CTileSet::leftInterfaceProblem)||(error==NL3D::CTileSet::rightInterfaceProblem)||
						(error==NL3D::CTileSet::topBottomNotTheSame)||(error==NL3D::CTileSet::rightLeftNotTheSame)
						||(error==NL3D::CTileSet::topInterfaceProblem))
					{
						if( index != -1 )
						{
							m_lastError += QString( "tile: %1 pixel: %2 component: %3" ).arg( index ).arg( pixel ).arg( comp[ component ] );
						}
						else
						{
							m_lastError += QString( "incompatible with a 128 tile, pixel: %1 component: %2" ).arg( pixel ).arg( comp[ component ] );
						}
					}

				}
				break;
			}
		}

		return error;
	}

	bool checkPixmap( TileModel::TTileChannel channel, QPixmap &pixmap )
	{
		int w = pixmap.width();
		int h = pixmap.height();

		if( w != h )
		{
			m_lastError = QObject::tr( "Not a square texture." );
			return false;
		}
		
		int width = getWidthForType( m_type, channel );		

		if( width != -1 )
		{
			if( width != w )
			{
				m_lastError = QObject::tr( "Not the proper size." );
				return false;
			}
		}

		std::vector< NLMISC::CBGRA > pixels;

		pixmapToCBGRA( pixmap, pixels );

		m_border[ channel ].set( w, h, pixels );

		NL3D::CTileSet::TError error = checkTile( channel );
		if( error == NL3D::CTileSet::addFirstA128128 )
		{
			m_borderFirst[ channel ] = true;
			return true;
		}

		if( error != NL3D::CTileSet::ok )
			return false;

		return true;
	}
	
	bool loadImage( TileModel::TTileChannel channel, const QString &fn, bool empty = false )
	{
		QPixmap temp;
		bool b = temp.load( fn );

		if( !b )
		{
			m_lastError = QObject::tr( "Cannot open file %1" ).arg( fn );
			return false;
		}

		m_borderFirst[ channel ] = false;
		
		if( !empty )
		{
			if( !checkPixmap( channel, temp ) )
				return false;
		}

		pixmaps[ channel ] = temp;
		
		return true;
	}

	void clearImage( TileModel::TTileChannel channel )
	{
		pixmaps[ channel ] = QPixmap();
	}

	const QPixmap& pixMap( TileModel::TTileChannel channel ) const{
		return pixmaps[ channel ];
	}

	void setType( TileModel::TNodeTileType type ){ m_type = type; }
	void setId( int id ){ m_id = id; }
	int id() const{ return m_id; }

	QString getLastError() const{ return m_lastError; }
	bool borderFirst( TileModel::TTileChannel channel ) const{ return m_borderFirst[ channel ]; }

	const NL3D::CTileBorder &border( TileModel::TTileChannel channel ){ return m_border[ channel ]; }

private:
	QPixmap pixmaps[ TileModel::TileChannelCount ];
	TileModel::TNodeTileType m_type;
	NL3D::CTileBorder m_border[ TileModel::TileChannelCount ];
	int m_id;
	QString m_lastError;
	bool m_borderFirst[ TileModel::TileChannelCount ];
};

TileModel::TTileChannel TileItemNode::s_displayChannel = TileModel::TileDiffuse;

TileItemNode::TileItemNode( TileModel::TNodeTileType type, int tileId, TileModel::TTileChannel channel, QString filename, Node *parent)
{
	m_parentItem = parent;
	//nlinfo("dispalying tile %d - %s", m_tileId, m_tileFilename[TileModel::TileDiffuse].toAscii().data());

	pvt = new TileItemNodePvt();
	pvt->setType( type );
	pvt->setId( tileId );

	m_hasError = false;

	setTileFilename( channel, filename );
}

TileItemNode::~TileItemNode()
{
	delete pvt;
	pvt = NULL;

	qDeleteAll(m_childItems);
}

bool TileItemNode::setTileFilename(TileModel::TTileChannel channel, QString filename)
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

QString TileItemNode::getTileFilename(TileModel::TTileChannel channel)
{
	QMap< TileModel::TTileChannel, QString >::const_iterator itr
		= m_tileFilename.find( channel );
	if( itr == m_tileFilename.end() )
		return "";

	return itr.value();
}

void TileItemNode::setId( int id )
{
	pvt->setId( id );
}

int TileItemNode::id() const
{
	return pvt->id();
}

QString TileItemNode::getLastError() const
{
	return pvt->getLastError();
}

bool TileItemNode::borderFirst( TileModel::TTileChannel channel ) const
{
	return pvt->borderFirst( channel );
}

const NL3D::CTileBorder& TileItemNode::border( TileModel::TTileChannel channel ) const
{
	return pvt->border( channel );
}

int TileItemNode::alphaRot() const
{
	return 0;
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
		return QVariant("("+QString::number(pvt->id())+")");
	}
	else if(role == TileModel::TileFilenameIndexRole)
	{
		return QVariant(tileFilename + " ("+QString::number(pvt->id())+")");
	}

	return QVariant();
}

int TileItemNode::columnCount() const
{
	return 1;
}