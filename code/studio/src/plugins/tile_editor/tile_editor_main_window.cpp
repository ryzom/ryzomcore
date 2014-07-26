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

#include "tile_editor_main_window.h"

#include "nel/misc/path.h"

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QFileDialog>
#include <QInputDialog>

#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/menu_manager.h"

#include "tile_model.h"
#include "tile_item.h"
#include "tile_item_delegate.h"

TileEditorMainWindow::TileEditorMainWindow(QWidget *parent)
	: QMainWindow(parent),
	m_ui(new Ui::TileEditorMainWindow)
{
	m_ui->setupUi(this);
	m_undoStack = new QUndoStack(this);

	// Retrieve the menu manager
	Core::ICore *core = Core::ICore::instance();
	Core::MenuManager *menuManager = core->menuManager();
	
	// Create tile rotation drop down toolbar menu.
	m_rotationMenu = new QMenu(tr("Rotate Tile"), m_ui->toolBar);
	m_rotationMenu->setIcon(QIcon(":/tileRotation/images/rotation0.png"));
	QList<QAction*> rotateActions;
	rotateActions.push_back(m_ui->actionRotateTile0);
	rotateActions.push_back(m_ui->actionRotateTile90);
	rotateActions.push_back(m_ui->actionRotateTile180);
	rotateActions.push_back(m_ui->actionRotateTile270);
	m_rotationMenu->addActions(rotateActions);
	m_ui->toolBar->addAction(m_rotationMenu->menuAction());

	// Create the tile zoom menu.
	m_zoomMenu = new QMenu(tr("Zoom"), m_ui->toolBar);
	m_zoomActionGroup = new QActionGroup(this);
	m_zoomSignalMapper = new QSignalMapper(this);
	QList<QAction*> zoomActions;
	zoomActions.push_back(m_ui->actionZoom50);
	zoomActions.push_back(m_ui->actionZoom100);
	zoomActions.push_back(m_ui->actionZoom200);
	m_zoomActionGroup->addAction(m_ui->actionZoom50);
	m_zoomActionGroup->addAction(m_ui->actionZoom100);
	m_zoomActionGroup->addAction(m_ui->actionZoom200);
	m_zoomMenu->addActions(zoomActions);
	m_ui->toolBar->addAction(m_zoomMenu->menuAction());

	m_tileEditorMenu = new QMenu(tr("Tile Editor"), core->menuManager()->menuBar());
	m_tileDisplayMenu = new QMenu(tr("Tile Display"), m_ui->toolBar);
	QList<QAction*> displayActions;
	displayActions.push_back(m_ui->actionTileDisplayFilename);
	displayActions.push_back(m_ui->actionTileDisplayIndex);
	m_ui->actionTileDisplayIndex->setChecked(true);
	m_tileDisplayMenu->addActions(displayActions);
	m_tileEditorMenu->addMenu(m_tileDisplayMenu);
	core->menuManager()->menuBar()->addMenu(m_tileEditorMenu);

	// Set up the list views.
	m_tileItemDelegate = new TileItemDelegate();

	// Set up the tile set list view.
	//m_ui->tileSetLV->setModel(m_model);
	//m_ui->tileSetLV->setRootIndex(m_model->index(0,0));

	connect(m_ui->tileSetAddTB, SIGNAL(clicked()), this, SLOT(onTileSetAdd()));
	connect(m_ui->tileSetDeleteTB, SIGNAL(clicked()), this, SLOT(onTileSetDelete()));
	connect(m_ui->tileSetEditTB, SIGNAL(clicked()), this, SLOT(onTileSetEdit()));
	connect(m_ui->tileSetUpTB, SIGNAL(clicked()), this, SLOT(onTileSetUp()));
	connect(m_ui->tileSetDownTB, SIGNAL(clicked()), this, SLOT(onTileSetDown()));

	connect(m_ui->landAddTB, SIGNAL(clicked()), this, SLOT(onLandAdd()));
	connect(m_ui->landRemoveTB, SIGNAL(clicked()), this, SLOT(onLandRemove()));
	connect(m_ui->landEditTB, SIGNAL(clicked()), this, SLOT(onLandEdit()));
	connect(m_ui->landLW, SIGNAL(currentRowChanged(int)), this, SLOT(onLandRowChanged(int)));

	connect(m_ui->chooseVegetPushButton, SIGNAL(clicked()), this, SLOT(onChooseVegetation()));
	connect(m_ui->resetVegetPushButton, SIGNAL(clicked()), this, SLOT(onResetVegetation()));

	connect(m_ui->tileBankTexturePathPB, SIGNAL(clicked()), this, SLOT(onChooseTexturePath()));

	// 128x128 List View
	//m_ui->listView128->setItemDelegate(m_tileItemDelegate);
	m_ui->listView128->addAction(m_ui->actionAddTile);
	m_ui->listView128->addAction(m_ui->actionDeleteTile);
	m_ui->listView128->addAction(m_ui->actionReplaceImage);
	m_ui->listView128->addAction(m_ui->actionDeleteImage);

	// 256x256 List View
	//m_ui->listView256->setItemDelegate(m_tileItemDelegate);
	m_ui->listView256->addAction(m_ui->actionAddTile);
	m_ui->listView256->addAction(m_ui->actionDeleteTile);
	m_ui->listView256->addAction(m_ui->actionReplaceImage);
	m_ui->listView256->addAction(m_ui->actionDeleteImage);

	// Transition List View
	//m_ui->listViewTransition->setItemDelegate(m_tileItemDelegate);
	m_ui->listViewTransition->addAction(m_ui->actionReplaceImage);
	m_ui->listViewTransition->addAction(m_ui->actionDeleteImage);

	// Displacement List View
	//m_ui->listViewDisplacement->setItemDelegate(m_tileItemDelegate);
	m_ui->listViewDisplacement->addAction(m_ui->actionReplaceImage);
	m_ui->listViewDisplacement->addAction(m_ui->actionDeleteImage);

	
	// Connect context menu actions up.
	connect(m_ui->actionAddTile, SIGNAL(triggered(bool)), this, SLOT(onActionAddTile(bool)));
	connect(m_ui->actionDeleteTile, SIGNAL(triggered(bool)), this, SLOT(onActionDeleteTile(bool)));
	connect(m_ui->actionReplaceImage, SIGNAL(triggered(bool)), this, SLOT(onActionReplaceImage(bool)));
	connect(m_ui->actionDeleteImage, SIGNAL(triggered(bool)), this, SLOT(onActionDeleteImage(bool)));

	//connect(m_ui->tileViewTabWidget, SIGNAL(currentChanged(int)), m_tileItemDelegate, SLOT(currentTab(int)));

	// Connect the zoom buttons.
	connect(m_ui->actionZoom50, SIGNAL(triggered()), m_zoomSignalMapper, SLOT(map()));
	m_zoomSignalMapper->setMapping(m_ui->actionZoom50, 0);
	connect(m_ui->actionZoom100, SIGNAL(triggered()), m_zoomSignalMapper, SLOT(map()));
	m_zoomSignalMapper->setMapping(m_ui->actionZoom100, 1);
	connect(m_ui->actionZoom200, SIGNAL(triggered()), m_zoomSignalMapper, SLOT(map()));
	m_zoomSignalMapper->setMapping(m_ui->actionZoom200, 2);
	connect(m_zoomSignalMapper, SIGNAL(mapped(int)), this, SLOT(onZoomFactor(int)));
}

TileEditorMainWindow::~TileEditorMainWindow()
{
	delete m_ui;
	delete m_undoStack;
	delete m_rotationMenu;
		
	delete m_tileDisplayMenu;
	delete m_tileEditorMenu;

	delete m_zoomMenu;
	delete m_zoomActionGroup;
	delete m_zoomSignalMapper;

	qDeleteAll( m_tileModels );
	m_tileModels.clear();
}

void TileEditorMainWindow::onZoomFactor(int level)
{
	int tile128Scaled=TileModel::TILE_128_BASE_SIZE;
	int tile256Scaled=TileModel::TILE_256_BASE_SIZE;
	int tileTransScaled=TileModel::TILE_TRANSITION_BASE_SIZE;
	int tileDispScaled=TileModel::TILE_DISPLACE_BASE_SIZE;
	switch(level)
	{
	// Zoom Level 50%
	case 0:
		nlinfo("zooming to 50%");
		TileModel::CurrentZoomFactor = TileModel::TileZoom50;
		tile128Scaled /= 2;
		tile256Scaled /= 2;
		tileTransScaled /= 2;
		tileDispScaled /= 2;
		break;
	case 1:
		nlinfo("zooming to 100%");
		TileModel::CurrentZoomFactor = TileModel::TileZoom100;
		break;
	case 2:
		nlinfo("zooming to 200%");
		TileModel::CurrentZoomFactor = TileModel::TileZoom200;
		tile128Scaled *= 2;
		tile256Scaled *= 2;
		tileTransScaled *= 2;
		tileDispScaled *= 2;
		break;
	default:
		nlwarning("Invalid Time Zoom Factor passed.");
		break;
	};

	nlinfo("resizing transition view. base size: %d factor %d to: %d", TileModel::TILE_TRANSITION_BASE_SIZE, level, tileTransScaled);

	m_ui->listView128->setIconSize(QSize(tile128Scaled, tile128Scaled));
	m_ui->listView128->setCurrentIndex(m_ui->listView128->model()->index(0, 0, m_ui->listView128->rootIndex()));
	m_ui->listView256->setIconSize(QSize(tile256Scaled, tile256Scaled));
	m_ui->listView256->setCurrentIndex(m_ui->listView256->model()->index(0, 0, m_ui->listView256->rootIndex()));
	m_ui->listViewTransition->setIconSize(QSize(tileTransScaled, tileTransScaled));
	m_ui->listViewTransition->setCurrentIndex(m_ui->listViewTransition->model()->index(0, 0, m_ui->listViewTransition->rootIndex()));
	m_ui->listViewDisplacement->setIconSize(QSize(tileDispScaled, tileDispScaled));
	m_ui->listViewDisplacement->setCurrentIndex(m_ui->listViewDisplacement->model()->index(0, 0, m_ui->listViewDisplacement->rootIndex()));
	m_ui->listViewTransition->repaint();
}

void TileEditorMainWindow::onActionAddTile(bool triggered)
{
	onActionAddTile(m_ui->tileViewTabWidget->currentIndex());
}

void TileEditorMainWindow::onActionDeleteTile(bool triggered)
{
	onActionDeleteTile(m_ui->tileViewTabWidget->currentIndex());
}

void TileEditorMainWindow::onActionReplaceImage(bool triggered)
{
}

void TileEditorMainWindow::onActionDeleteImage(bool triggered)
{
	onActionDeleteImage(m_ui->tileViewTabWidget->currentIndex());
}

void TileEditorMainWindow::onTileSetAdd()
{
	if( m_ui->landLW->count() == 0 )
	{
		QMessageBox::information( this,
									tr( "Error adding tile set" ),
									tr( "You need to add a land before adding a tileset!" ) );
		return;
	}

	bool ok;
    QString text = QInputDialog::getText(this, tr("Add Tile Set"), tr("Enter Tile Set name:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
	{
		TileModel *model = static_cast<TileModel*>(m_ui->tileSetLV->model());
		
		if( model->hasTileSet( text ) )
		{
			QMessageBox::information( this, tr("Error Adding Tile Set"), tr("This name already exists") );
			return;
		}

		// Create and append the new tile set to the model.
		TileSetNode *tileSet = model->createTileSetNode(text);
		
		// Retrieve how many rows there currently are and set the current index using that.
		m_ui->tileSetLV->reset();
		uint32 rows = model->rowCount();
		m_ui->tileSetLV->setCurrentIndex(model->index(rows-1, 0));
	}
}

void TileEditorMainWindow::onTileSetDelete()
{
	QModelIndex idx = m_ui->tileSetLV->currentIndex();
	if( !idx.isValid() )
		return;

	int reply = QMessageBox::question( this,
										tr( "Removing tile set" ),
										tr( "Are you sure you want to remove this tile set?" ),
										QMessageBox::Yes | QMessageBox::Cancel );

	if( reply != QMessageBox::Yes )
		return;

	TileModel *model = static_cast<TileModel*>(m_ui->tileSetLV->model());
	bool ok = model->removeRow( idx.row() );
}

void TileEditorMainWindow::onTileSetEdit()
{
	QModelIndex idx = m_ui->tileSetLV->currentIndex();
	if( !idx.isValid() )
		return;

	TileSetNode *node = reinterpret_cast< TileSetNode* >( idx.internalPointer() );
	QString name = node->getTileSetName();

	bool ok = false;

	QString newName = QInputDialog::getText( this,
										tr( "Edit tileset" ),
										tr( "Enter tileset name" ),
										QLineEdit::Normal,
										name,
										&ok );

	if( !ok )
		return;

	TileModel *model = static_cast<TileModel*>(m_ui->tileSetLV->model());
	if( model->hasTileSet( newName ) )
	{
		QMessageBox::information( this,
									tr("Tileset already exists"),
									tr("A tileset with that name already exists!") );
		return;
	}

	node->setTileSetName( newName );
	m_ui->tileSetLV->reset();
}

void TileEditorMainWindow::onTileSetUp()
{
	QModelIndex idx = m_ui->tileSetLV->currentIndex();
	if( !idx.isValid() )
		return;

	if( idx.row() == 0 )
		return;

	TileModel *model = static_cast<TileModel*>(m_ui->tileSetLV->model());
	if( model->rowCount() < 2 )
		return;

	int r = idx.row();
	model->swapRows( r, r - 1 );

	m_ui->tileSetLV->reset();
	m_ui->tileSetLV->setCurrentIndex( model->index( r - 1, 0 ) );
}

void TileEditorMainWindow::onTileSetDown()
{
	QModelIndex idx = m_ui->tileSetLV->currentIndex();
	if( !idx.isValid() )
		return;

	TileModel *model = static_cast<TileModel*>(m_ui->tileSetLV->model());
	if( model->rowCount() < idx.row() )
		return;
	if( model->rowCount() < 2 )
		return;
	
	int r = idx.row();
	model->swapRows( r, r + 1 );

	m_ui->tileSetLV->reset();
	m_ui->tileSetLV->setCurrentIndex( model->index( r + 1, 0 ) );
}

void TileEditorMainWindow::onLandAdd()
{
	QString name = QInputDialog::getText( this,
											tr("Adding new land"),
											tr("Please specify the new land's name") );

	if( name.isEmpty() )
		return;

	for( int i = 0; i < m_ui->landLW->count(); i++ )
	{
		QListWidgetItem *item = m_ui->landLW->item( i );
		if( item->text() == name )
		{
			QMessageBox::information( this,
										tr( "Error adding new land" ),
										tr( "A land with that name already exists." ) );
			return;
		}
	}

	m_ui->landLW->addItem( name );

	TileModel *m = createTileModel();

	m_tileModels.push_back( m );

	if( m_tileModels.count() == 1 )
		m_ui->landLW->setCurrentRow( 0 );
}

void TileEditorMainWindow::onLandRemove()
{
	QListWidgetItem *item = m_ui->landLW->currentItem();
	if( item == NULL )
		return;

	int idx = m_ui->landLW->currentRow();

	int reply = QMessageBox::question( this,
										tr( "Removing land" ),
										tr( "Are you sure you want to remove this land?" ),
										QMessageBox::Yes | QMessageBox::Cancel );

	if( reply != QMessageBox::Yes )
		return;

	QList< TileModel* >::iterator itr = m_tileModels.begin() + idx;
	delete m_tileModels[ idx ];
	m_tileModels[ idx ] = NULL;
	m_tileModels.erase( itr );

	delete item;
}

void TileEditorMainWindow::onLandEdit()
{
	QListWidgetItem *item = m_ui->landLW->currentItem();
	if( item == NULL )
		return;

	QString name = item->text();

	QString newName = QInputDialog::getText( this,
												tr( "Editing land" ),
												tr( "Please specify the new name of the selected land" ),
												QLineEdit::Normal,
												name );

	if( newName.isEmpty() )
		return;
	if( newName == name )
		return;
	item->setText( newName );
}

void TileEditorMainWindow::onLandRowChanged( int row )
{
	m_ui->chooseVegetPushButton->setText( "..." );

	if( row == -1 )
	{
		disconnect( m_ui->tileSetLV->selectionModel(), SIGNAL( currentChanged( const QModelIndex &, const QModelIndex & ) ),
			this, SLOT( changeActiveTileSet( const QModelIndex &, const QModelIndex & ) ) );

		m_ui->tileSetLV->setModel( NULL );
		m_ui->listView128->setModel( NULL );
		m_ui->listView256->setModel( NULL );
		m_ui->listViewTransition->setModel( NULL );
		m_ui->listViewDisplacement->setModel( NULL );
	}
	else
	{
		//disconnect( m_ui->tileSetLV->selectionModel(), SIGNAL( currentChanged( const QModelIndex &, const QModelIndex & ) ),
		//	this, SLOT( changeActiveTileSet( const QModelIndex &, const QModelIndex & ) ) );

		m_ui->tileSetLV->setModel( m_tileModels[ row ] );
		m_ui->listView128->setModel( m_tileModels[ row ] );
		m_ui->listView256->setModel( m_tileModels[ row ] );
		m_ui->listViewTransition->setModel( m_tileModels[ row ] );
		m_ui->listViewDisplacement->setModel( m_tileModels[ row ] );

		connect( m_ui->tileSetLV->selectionModel(), SIGNAL( currentChanged( const QModelIndex &, const QModelIndex & ) ),
			this, SLOT( changeActiveTileSet( const QModelIndex &, const QModelIndex & ) ) );

		if( m_ui->tileSetLV->model()->rowCount() != 0 )
		{
			QModelIndex idx = m_ui->tileSetLV->model()->index( 0, 0 );
			m_ui->tileSetLV->setCurrentIndex( idx );
		}
	}

	m_ui->tileSetLV->reset();
}

void TileEditorMainWindow::onChooseVegetation()
{
	QModelIndex idx = m_ui->tileSetLV->currentIndex();
	if( !idx.isValid() )
	{
		QMessageBox::information( this,
									tr("Choosing a vegetation set"),
									tr("You need to select a tileset before choosing a vegetation set!") );
		return;
	}

	QString vegetSet = QFileDialog::getOpenFileName( this,
														tr( "Choose vegetation set" ),
														"",
														tr( "Nel vegetset files (*.vegetset)" ) );

	if( vegetSet.isEmpty() )
		return;

	TileSetNode *node = reinterpret_cast< TileSetNode* >( idx.internalPointer() );
	node->setVegetSet( vegetSet );

	m_ui->chooseVegetPushButton->setText( vegetSet );
}

void TileEditorMainWindow::onResetVegetation()
{
	QModelIndex idx = m_ui->tileSetLV->currentIndex();
	if( !idx.isValid() )
	{
		QMessageBox::information( this,
									tr("Resetting a vegetation set"),
									tr("You need to select a tileset before resetting a vegetation set!") );
		return;
	}
	m_ui->chooseVegetPushButton->setText( "..." );

	TileSetNode *node = reinterpret_cast< TileSetNode* >( idx.internalPointer() );
	node->setVegetSet( "" );
}

void TileEditorMainWindow::onChooseTexturePath()
{
	QString path = QFileDialog::getExistingDirectory( this,
														tr("Choose tilebank absolute texture path "),
														"" );

	if( path.isEmpty() )
		return;

	int reply = QMessageBox::question( this,
										tr("tilebank texture path"),
										tr("Are you sure you want to make '%1' the tilebank absolute texture path?").arg( path ),
										QMessageBox::Yes | QMessageBox::Cancel );

	if( reply != QMessageBox::Yes )
		return;

	m_texturePath = path;
	m_ui->tileBankTexturePathPB->setText( path );
}

void TileEditorMainWindow::onActionAddTile(int tabId)
{
	int land = m_ui->landLW->currentRow();
	if( land == -1 )
	{
		QMessageBox::information( this,
									tr( "Adding new tile" ),
									tr( "You need to have a land and a tileset selected before you can add tiles!" ) );
		return;
	}

	QModelIndex idx = m_ui->tileSetLV->currentIndex();
	if( !idx.isValid() )
	{
		QMessageBox::information( this,
									tr( "Adding new tiles" ),
									tr( "You need to have a tileset selected before you can add tiles!" ) );
		return;
	}

	int tileSet = idx.row();
	
	TileModel *model = static_cast< TileModel* >( m_tileModels[ land ] );
	idx = model->index( tileSet, 0 );
	if( !idx.isValid() )
		return;

	TileSetNode *tsn = reinterpret_cast< TileSetNode* >( idx.internalPointer() );

	Node *n = tsn->child( tabId );

	QFileDialog::Options options;
	QString selectedFilter;
	QStringList fileNames = QFileDialog::getOpenFileNames(this, "Choose Tile Texture", "." , "Images (*.png);;All Files (*.*)", &selectedFilter, options);

	int c = n->childCount();

	QStringListIterator itr( fileNames );
	while( itr.hasNext() )
	{
		Node *newNode = TileModel::createItemNode( c, TileModel::TileDiffuse, itr.next() );
		n->appendRow( newNode );
		c++;
	}

	QModelIndex rootIdx = model->index( tabId, 0, m_ui->tileSetLV->currentIndex());

	QListView *lv = getListViewByTab( tabId );

	lv->reset();
	lv->setRootIndex( rootIdx );
	lv->setCurrentIndex( lv->model()->index( 0, 0, rootIdx ) );
}

void TileEditorMainWindow::onActionDeleteTile( int tabId )
{
	QListView *lv = getListViewByTab( tabId );

	QModelIndex idx = lv->currentIndex();
	if( !idx.isValid() )
	{
		QMessageBox::information( this,
									tr( "Deleting a tile" ),
									tr( "You need to select a tile to delete is!" ) );
		return;
	}

	int row = idx.row();

	QModelIndex parent = idx.parent();
	lv->model()->removeRow( row, parent );

	//lv->reset();
}

void TileEditorMainWindow::onActionDeleteImage( int tabId )
{
	QListView *lv = getListViewByTab( tabId );
	
	QModelIndex idx = lv->currentIndex();
	if( !idx.isValid() )
	{
		QMessageBox::information( this,
									tr( "Deleting tile image" ),
									tr( "No tile selected!" ) );
		return;
	}

	TileItemNode *n = reinterpret_cast< TileItemNode* >( idx.internalPointer() );
	n->setTileFilename( TileModel::TileDiffuse, "" );
}

TileModel* TileEditorMainWindow::createTileModel()
{
	QStringList headers;
	headers << "Tile Set";
	TileModel *m = new TileModel( headers );

	connect( m_ui->actionTileDisplayFilename, SIGNAL( toggled( bool )), m, SLOT( selectFilenameDisplay( bool ) ) );
	connect( m_ui->actionTileDisplayIndex, SIGNAL( toggled( bool )), m, SLOT( selectIndexDisplay( bool ) ) );

	return m;
}

void TileEditorMainWindow::changeActiveTileSet(const QModelIndex &newIndex, const QModelIndex &oldIndex)
{
	TileModel *model = static_cast<TileModel*>(m_ui->tileSetLV->model());

	QModelIndex tile128Idx = model->index(0, 0, newIndex);
	QModelIndex tile256Idx = model->index(1, 0, newIndex);
	QModelIndex tileTransIdx = model->index(2, 0, newIndex);
	QModelIndex tileDispIdx = model->index(3, 0, newIndex);

	m_ui->listView128->setRootIndex(tile128Idx);
	m_ui->listView128->setCurrentIndex(m_ui->listView128->model()->index(0, 0, m_ui->listView128->rootIndex()));
	m_ui->listView256->setRootIndex(tile256Idx);
	m_ui->listView256->setCurrentIndex(m_ui->listView256->model()->index(0, 0, m_ui->listView256->rootIndex()));
	m_ui->listViewTransition->setRootIndex(tileTransIdx);
	m_ui->listViewTransition->setCurrentIndex(m_ui->listViewTransition->model()->index(0, 0, m_ui->listViewTransition->rootIndex()));
	m_ui->listViewDisplacement->setRootIndex(tileDispIdx);
	m_ui->listViewDisplacement->setCurrentIndex(m_ui->listViewDisplacement->model()->index(0, 0, m_ui->listViewDisplacement->rootIndex()));

	TileSetNode *oldNode = NULL;
	TileSetNode *newNode = NULL;

	if( oldIndex.isValid() )
		oldNode = reinterpret_cast< TileSetNode* >( oldIndex.internalPointer() );
	if( newIndex.isValid() )
		newNode = reinterpret_cast< TileSetNode* >( newIndex.internalPointer() );

	if( newNode != NULL )
	{
		QString vegetSet = newNode->vegetSet();
		
		if( !vegetSet.isEmpty() )
			m_ui->chooseVegetPushButton->setText( vegetSet );
		else
			m_ui->chooseVegetPushButton->setText( "..." );
	}
	else
	{
		m_ui->chooseVegetPushButton->setText( "..." );
	}

	//nlinfo("number of rows in displacement: %d", tileDispIdx.model()->rowCount(tileDispIdx));

	//m_ui->listView128->reset();
	//m_ui->listView256->reset();
	//m_ui->listViewTransition->reset();
	//m_ui->listViewDisplacement->reset();
}


QListView* TileEditorMainWindow::getListViewByTab( int tab ) const
{
	QListView *lv = NULL;

	switch( tab )
	{
	case TAB_128: lv = m_ui->listView128; break;
	case TAB_256: lv = m_ui->listView256; break;
	case TAB_TRANSITION: lv = m_ui->listViewTransition; break;
	case TAB_DISPLACEMENT: lv = m_ui->listViewDisplacement; break;
	}

	return lv;
}


