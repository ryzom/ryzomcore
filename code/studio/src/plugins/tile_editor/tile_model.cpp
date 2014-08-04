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

#include "tile_model.h"
#include "tile_item.h"
#include "tile_bank.h"

#include <QStringList>

#include <nel/misc/debug.h>

// Initialize the static members
TileModel::TTileZoomFactor TileModel::CurrentZoomFactor;

TileModel::TileModel(const QStringList &headers, QObject *parent) : QAbstractItemModel(parent)
{
	QVector<QVariant> rootData;
	Q_FOREACH(QString header, headers)
		rootData << header;

	rootItem = new Node(rootData);

	TileModel::CurrentZoomFactor = TileModel::TileZoom100;
	m_indexDisplay = true;
	m_fileDisplay = true;

	m_tileBank = new TileBank();
}

TileModel::~TileModel()
{
	delete rootItem;
}

Node *TileModel::getItem(const QModelIndex &index) const
{
	if(index.isValid())
	{
		Node *item = static_cast<Node*>(index.internalPointer());
		if(item) return item;
	}
	return rootItem;
}

QModelIndex TileModel::index(int row, int column, const QModelIndex &parent) const
{
	if(parent.isValid() && parent.column() != 0)
		return QModelIndex();

	Node *parentItem = getItem(parent);

	Node *childItem = parentItem->child(row);
	if(childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex TileModel::parent(const QModelIndex &index) const
{
	if(!index.isValid())
		return QModelIndex();

	Node *childItem = getItem(index);
	Node *parentItem = childItem->parent();

	if(parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->childNumber(), 0, parentItem);
}

int TileModel::rowCount(const QModelIndex &parent) const
{
	Node *parentItem = getItem(parent);
	return parentItem->childCount();
}

int TileModel::columnCount(const QModelIndex &parent) const
{
	Node *parentItem = getItem(parent);
	return parentItem->columnCount();
}

QVariant TileModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();

	Node *item = static_cast<Node*>(index.internalPointer());
	
	// Translate the display role to the settings-specific role.
	
	if(role == Qt::DisplayRole)
	{
		if(m_indexDisplay && m_fileDisplay)
			role = TileFilenameIndexRole;
		else if(m_fileDisplay)
			role = TileFilenameRole;
		else if(m_indexDisplay)
			role = TileIndexRole;
	}
	return item->data(index.column(), role);
}

Qt::ItemFlags TileModel::flags(const QModelIndex &index) const
{
	if(!index.isValid())
		return Qt::ItemIsEnabled;

	return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QVariant TileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section,role);

	return QVariant();
}

void TileModel::appendRow(const QList<Node*> &items)
{
	rootItem->appendRow(items);
}


void TileModel::appendRow(Node *item)
{
	int c = rootItem->childCount();
	
	beginInsertRows( QModelIndex(), c, c );
	
	rootItem->appendRow(item);

	endInsertRows();
}

bool TileModel::removeRows( int row, int count, const QModelIndex &parent )
{
	Node *parentNode = NULL;

	if( !parent.isValid() )
		parentNode = rootItem;
	else
		parentNode = getItem( parent );

	if( parentNode == NULL )
		return false;

	int c = parentNode->childCount();
	if( row + count > c )
		return false;

	beginRemoveRows( parent, row, row + count - 1 );

	bool ok = parentNode->removeChildren( row, count );
	
	endRemoveRows();

	return ok;
}

void TileModel::swapRows( int a, int b )
{
	rootItem->swapRows( a, b );
}

TileSetNode *TileModel::createTileSetNode(QString tileSetName)
{
	// Create the new tile set.
	TileSetNode *tileSet = new TileSetNode(tileSetName);
			
	// child for 128x128 tiles
	TileTypeNode *tile128= new TileTypeNode( TileConstants::Tile128);
	tileSet->appendRow(tile128);

	// child for 256x256 tiles
	TileTypeNode *tile256= new TileTypeNode( TileConstants::Tile256);
	tileSet->appendRow(tile256);

	// child for transition tiles.
	TileTypeNode *tileTrans= new TileTypeNode( TileConstants::TileTransition);
	tileSet->appendRow(tileTrans);

	// Add the default transition tiles.
	// TODO tie this to CTileSet::count from NeL
	for(int transPos=0; transPos<48; transPos++)
	{				
		TileItemNode *transTile= new TileItemNode( TileConstants::TileTransition, transPos, TileConstants::TileDiffuse, QString("empty"));
		tileTrans->appendRow(transTile);
	}

	// child for displacement tiles
	TileTypeNode *tileDisp= new TileTypeNode( TileConstants::TileDisplacement);
	tileSet->appendRow(tileDisp);

	// Add the default displacement tiles.
	// TODO tie this to CTileSet::CountDisplace from NeL
	for(int dispPos=0; dispPos<16; dispPos++)
	{
		TileItemNode *dispTile= new TileItemNode( TileConstants::TileDisplacement, dispPos, TileConstants::TileDiffuse, QString("empty"));
		tileDisp->appendRow(dispTile);
	}

	// Append them in the correct order to the tile set.
	this->appendRow(tileSet);
	m_tileBank->addTileSet(tileSetName);

	return tileSet;
}

TileItemNode *TileModel::createItemNode( TileConstants::TNodeTileType type, int id, TileConstants::TTileChannel channel, const QString &fileName )
{
	return new TileItemNode( type, id, channel, fileName );
}

TileItemNode *TileModel::createItemNode( int idx, TileConstants::TNodeTileType type, int id, TileConstants::TTileChannel channel, const QString &fileName )
{
	TileItemNode *n = new TileItemNode( type, id, channel, fileName );

	bool b = m_tileBank->addTile( idx, fileName, n->pixmap( channel ), channel, type );
	if( !b )
	{
		delete n;
		return NULL;
	}

	return n;
}

const char *TileModel::getTileTypeName(TileConstants::TNodeTileType type)
{
	switch(type)
	{
	case TileConstants::Tile128:
		return "128";
	case TileConstants::Tile256:
		return "256";
	case TileConstants::TileTransition:
		return "Transition";
	case TileConstants::TileDisplacement:
		return "Displacement";
	default:
		break;
	}
	return "UNKNOWN";
}

uint32 TileModel::getTileTypeSize(TileConstants::TNodeTileType type)
{
	switch(type)
	{
	case TileConstants::Tile128:
		return 128;
	case TileConstants::Tile256:
		return 256;
	case TileConstants::TileTransition:
		return 64;
	case TileConstants::TileDisplacement:
		return 32;
	default:
		break;
	}
	return 0;
}

bool TileModel::hasTileSet( const QString &name )
{
	for( int i = 0; i < rowCount(); i++ )
	{
		QModelIndex idx = index( i, 0 );
		if( !idx.isValid() )
		{
			continue;
		}

		TileSetNode *n = reinterpret_cast< TileSetNode* >( idx.internalPointer() );
		if( n->getTileSetName() == name )
			return true;
	}

	return false;
}

void TileModel::clear()
{
	int c = rootItem->childCount();
	if( c == 0 )
		return;

	removeRows( 0, c );
}

void TileModel::addLand( const QString &name )
{
	m_tileBank->addLand( name );
}

void TileModel::removeLand( int idx )
{
	m_tileBank->removeLand( idx );
}

void TileModel::setLandSets( int idx, const QStringList &l )
{
	m_tileBank->setLandSets( idx, l );
}

void TileModel::getLandSets( int idx, QStringList &l )
{
	m_tileBank->getLandSets( idx, l );
}

void TileModel::removeTileSet( int idx )
{
	TileSetNode *set = static_cast< TileSetNode* >( rootItem->child( idx ) );
	if( set == NULL )
		return;

	removeRow( idx );

	m_tileBank->removeTileSet( idx );
}

void TileModel::renameTileSet( int idx, const QString &newName )
{
	m_tileBank->renameTileSet( idx, newName );
}

void TileModel::removeTile( int ts, int type, int tile )
{
	TileSetNode *set = static_cast< TileSetNode* >( rootItem->child( ts ) );
	if( set == NULL )
		return;

	TileTypeNode *typeNode = static_cast< TileTypeNode* >( set->child( type ) );
	if( typeNode == NULL )
		return;

	TileItemNode *tileNode = static_cast< TileItemNode* >( typeNode->child( tile ) );
	if( tileNode == NULL )
		return;

	QModelIndex tileIdx = createIndex( tile, 0, tileNode );
	removeRow( tile, tileIdx.parent() );

	m_tileBank->removeTile( ts, type, tile );
}

bool TileModel::replaceImage( int ts, int type, int tile, TileConstants::TTileChannel channel, const QString &name )
{
	Node *set = rootItem->child( ts );
	Node *tn = set->child( type );
	Node *n = tn->child( tile );

	TileItemNode *tin = static_cast< TileItemNode* >( n );
	QString old = tin->getTileFilename( channel );

	bool b = tin->setTileFilename( channel, name );
	if( !b )
		return false;
	
	m_tileBank->replaceImage( ts, type, tile, channel, name, tin->pixmap( channel ) );
	if( m_tileBank->hasError() )
	{
		tin->setTileFilename( channel, old );
		return false;
	}

	return true;
}

void TileModel::clearImage( int ts, int type, int tile, TileConstants::TTileChannel channel )
{
	Node *set = rootItem->child( ts );
	Node *tn = set->child( type );
	Node *n = tn->child( tile );

	TileItemNode *tin = static_cast< TileItemNode* >( n );
	tin->setTileFilename( channel, "" );

	m_tileBank->clearImage( ts, type, tile, channel );
}

void TileModel::setVegetation( int tileSet, const QString &vegetation )
{
	m_tileBank->setVegetation( tileSet, vegetation );
}

QString TileModel::getVegetation( int tileSet ) const
{
	return m_tileBank->getVegetation( tileSet );
}

void TileModel::setOriented( int tileSet, bool b )
{
	m_tileBank->setOriented( tileSet, b );
}

bool TileModel::getOriented( int tileSet ) const
{
	return m_tileBank->getOriented( tileSet );
}

void TileModel::setSurfaceData( int tileSet, unsigned long data )
{
	m_tileBank->setSurfaceData( tileSet, data );
}

unsigned long TileModel::getSurfaceData( int tileSet ) const
{
	return m_tileBank->getSurfaceData( tileSet );
}

void TileModel::setTexturePath( const QString &path )
{
	m_tileBank->setTexturePath( path );
}

QString TileModel::getTexturePath() const
{
	return m_tileBank->getTexturePath();
}

QString TileModel::getLastError() const{
	return m_tileBank->getLastError();
}

bool TileModel::hasError() const
{
	return m_tileBank->hasError();
}

void TileModel::selectFilenameDisplay(bool selected)
{
	m_fileDisplay = selected;
}

void TileModel::selectIndexDisplay(bool selected)
{
	m_indexDisplay = selected;
}