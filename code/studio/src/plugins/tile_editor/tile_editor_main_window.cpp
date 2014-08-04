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

#include "tilebank_saver.h"
#include "tilebank_loader.h"

#include "land_edit_dialog.h"

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
	m_rotateSM = new QSignalMapper();
	m_rotateAG = new QActionGroup(this);
	m_rotateAG->addAction(m_ui->actionRotateTile0);
	m_rotateAG->addAction(m_ui->actionRotateTile90);
	m_rotateAG->addAction(m_ui->actionRotateTile180);
	m_rotateAG->addAction(m_ui->actionRotateTile270);
	m_ui->actionRotateTile0->setChecked( true );

	connect( m_ui->actionRotateTile0, SIGNAL( triggered() ), m_rotateSM, SLOT( map() ) );
	connect( m_ui->actionRotateTile90, SIGNAL( triggered() ), m_rotateSM, SLOT( map() ) );
	connect( m_ui->actionRotateTile180, SIGNAL( triggered() ), m_rotateSM, SLOT( map() ) );
	connect( m_ui->actionRotateTile270, SIGNAL( triggered() ), m_rotateSM, SLOT( map() ) );
	m_rotateSM->setMapping( m_ui->actionRotateTile0, 0 );
	m_rotateSM->setMapping( m_ui->actionRotateTile90, 1 );
	m_rotateSM->setMapping( m_ui->actionRotateTile180, 2 );
	m_rotateSM->setMapping( m_ui->actionRotateTile270, 3 );
	connect( m_rotateSM, SIGNAL( mapped( int ) ), this, SLOT( onRotate( int ) ) );

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

	connect(m_ui->landAddTB, SIGNAL(clicked()), this, SLOT(onLandAdd()));
	connect(m_ui->landRemoveTB, SIGNAL(clicked()), this, SLOT(onLandRemove()));
	connect(m_ui->landEditTB, SIGNAL(clicked()), this, SLOT(onLandEdit()));

	connect(m_ui->chooseVegetPushButton, SIGNAL(clicked()), this, SLOT(onChooseVegetation()));
	connect(m_ui->resetVegetPushButton, SIGNAL(clicked()), this, SLOT(onResetVegetation()));

	connect(m_ui->tileBankTexturePathPB, SIGNAL(clicked()), this, SLOT(onChooseTexturePath()));

	m_tileModel = createTileModel();
	m_ui->tileSetLV->setModel( m_tileModel );

	// 128x128 List View
	//m_ui->listView128->setItemDelegate(m_tileItemDelegate);
	m_ui->listView128->setModel( m_tileModel );
	m_ui->listView128->addAction(m_ui->actionAddTile);
	m_ui->listView128->addAction(m_ui->actionDeleteTile);
	m_ui->listView128->addAction(m_ui->actionReplaceImage);
	m_ui->listView128->addAction(m_ui->actionDeleteImage);

	// 256x256 List View
	//m_ui->listView256->setItemDelegate(m_tileItemDelegate);
	m_ui->listView256->setModel( m_tileModel );
	m_ui->listView256->addAction(m_ui->actionAddTile);
	m_ui->listView256->addAction(m_ui->actionDeleteTile);
	m_ui->listView256->addAction(m_ui->actionReplaceImage);
	m_ui->listView256->addAction(m_ui->actionDeleteImage);

	// Transition List View
	//m_ui->listViewTransition->setItemDelegate(m_tileItemDelegate);
	m_ui->listViewTransition->setModel( m_tileModel );
	m_ui->listViewTransition->addAction(m_ui->actionReplaceImage);
	m_ui->listViewTransition->addAction(m_ui->actionDeleteImage);

	// Displacement List View
	//m_ui->listViewDisplacement->setItemDelegate(m_tileItemDelegate);
	m_ui->listViewDisplacement->setModel( m_tileModel );
	m_ui->listViewDisplacement->addAction(m_ui->actionReplaceImage);
	m_ui->listViewDisplacement->addAction(m_ui->actionDeleteImage);

	
	// Connect context menu actions up.
	connect(m_ui->actionAddTile, SIGNAL(triggered(bool)), this, SLOT(onActionAddTile(bool)));
	connect(m_ui->actionDeleteTile, SIGNAL(triggered(bool)), this, SLOT(onActionDeleteTile(bool)));
	connect(m_ui->actionReplaceImage, SIGNAL(triggered(bool)), this, SLOT(onActionReplaceImage(bool)));
	connect(m_ui->actionDeleteImage, SIGNAL(triggered(bool)), this, SLOT(onActionDeleteImage(bool)));

	//connect(m_ui->tileViewTabWidget, SIGNAL(currentChanged(int)), m_tileItemDelegate, SLOT(currentTab(int)));
	connect( m_ui->tileSetLV->selectionModel(), SIGNAL( currentChanged( const QModelIndex &, const QModelIndex & ) ),
		this, SLOT( changeActiveTileSet( const QModelIndex &, const QModelIndex & ) ) );

	// Connect the zoom buttons.
	connect(m_ui->actionZoom50, SIGNAL(triggered()), m_zoomSignalMapper, SLOT(map()));
	m_zoomSignalMapper->setMapping(m_ui->actionZoom50, 0);
	connect(m_ui->actionZoom100, SIGNAL(triggered()), m_zoomSignalMapper, SLOT(map()));
	m_zoomSignalMapper->setMapping(m_ui->actionZoom100, 1);
	connect(m_ui->actionZoom200, SIGNAL(triggered()), m_zoomSignalMapper, SLOT(map()));
	m_zoomSignalMapper->setMapping(m_ui->actionZoom200, 2);
	connect(m_zoomSignalMapper, SIGNAL(mapped(int)), this, SLOT(onZoomFactor(int)));

	QAction *saveAction = Core::ICore::instance()->menuManager()->action( Core::Constants::SAVE );
	saveAction->setEnabled( true );
	QAction *saveAsAction = Core::ICore::instance()->menuManager()->action( Core::Constants::SAVE_AS );
	saveAsAction->setEnabled( true );
	QAction *openAction = Core::ICore::instance()->menuManager()->action( Core::Constants::OPEN );
	openAction->setEnabled( true );

	connect( m_ui->actionSaveTileBank, SIGNAL( triggered() ), this, SLOT( save() ) );
	connect( m_ui->actionSaveTileBankAs, SIGNAL( triggered() ), this, SLOT( saveAs() ) );
	connect( m_ui->actionOpenTileBank, SIGNAL( triggered() ), this, SLOT( open() ) );

	connect( m_ui->orientedCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( onOrientedStateChanged( int ) ) );
	connect( m_ui->surfaceDataLineEdit, SIGNAL( textEdited( const QString& ) ), this, SLOT( onSurfaceDataChanged( const QString& ) ) );

	connect( m_ui->diffuse128BT, SIGNAL( toggled( bool ) ), this, SLOT( onDiffuseToggled( bool ) ) );
	connect( m_ui->diffuse256BT, SIGNAL( toggled( bool ) ), this, SLOT( onDiffuseToggled( bool ) ) );
	connect( m_ui->diffuseTrBT, SIGNAL( toggled( bool ) ), this, SLOT( onDiffuseToggled( bool ) ) );
	connect( m_ui->additive128BT, SIGNAL( toggled( bool ) ), this, SLOT( onAdditiveToggled( bool ) ) );
	connect( m_ui->additive256BT, SIGNAL( toggled( bool ) ), this, SLOT( onAdditiveToggled( bool ) ) );
	connect( m_ui->additiveTrBT, SIGNAL( toggled( bool ) ), this, SLOT( onAdditiveToggled( bool ) ) );
	connect( m_ui->alphaTrBT, SIGNAL( toggled( bool ) ), this, SLOT( onAlphaToggled( bool ) ) );

	connect( m_ui->tileViewTabWidget, SIGNAL( currentChanged( int ) ), this, SLOT( onTabChanged( int ) ) );
	
}

TileEditorMainWindow::~TileEditorMainWindow()
{
	delete m_ui;
	delete m_undoStack;
	delete m_rotationMenu;
	delete m_rotateSM;
	delete m_rotateAG;
		
	delete m_tileDisplayMenu;
	delete m_tileEditorMenu;

	delete m_zoomMenu;
	delete m_zoomActionGroup;
	delete m_zoomSignalMapper;

	delete m_tileModel;
	m_tileModel = NULL;
}

void TileEditorMainWindow::save()
{
	if( m_fileName.isEmpty() )
		saveAs();
	else
		saveAs( m_fileName );
}

void TileEditorMainWindow::saveAs()
{
	QString fn = QFileDialog::getSaveFileName( this,
													tr( "Save TileBank as..." ),
													"",
													tr( "TileBank files (*.tilebank)" ) );

	if( fn.isEmpty() )
		return;

	saveAs( fn );

}

void TileEditorMainWindow::saveAs( const QString &fn )
{
	QList< QString > landNames;

	int c = m_ui->landLW->count();
	for( int i = 0; i < c; i++ )
	{
		QListWidgetItem *item = m_ui->landLW->item( i );
		landNames.push_back( item->text() );
	}

	TileBankSaver saver;
	bool ok = saver.save( fn.toUtf8().constData(), m_tileModel );

	if( !ok )
	{
		QMessageBox::critical( this,
								tr( "Saving tilebank" ),
								tr( "Failed to save tilebank :(" ) );
	}
}

void TileEditorMainWindow::open()
{
	QString fn = QFileDialog::getOpenFileName( this,
												tr( "Loading tilebank" ),
												"",
												tr( "tilebank files (*.tilebank)" ) );

	if( fn.isEmpty() )
		return;

	TileBankLoader loader;
	bool b = true;
	//loader.load( fn.toUtf8().constData(), m_tileModel, m_lands );

	if( !b )
	{
		QMessageBox::critical( this,
								tr( "Loading tilebank" ),
								tr( "Failed to load tilebank %1" ).arg( fn ) );
	}

	// Put the loaded data into the GUI
	onTileBankLoaded();

	m_fileName = fn;
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

void TileEditorMainWindow::onRotate( int id )
{
	TileItemNode::setAlphaRot( id * 90 );
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
	onActionReplaceImage(m_ui->tileViewTabWidget->currentIndex());
}

void TileEditorMainWindow::onActionDeleteImage(bool triggered)
{
	onActionDeleteImage(m_ui->tileViewTabWidget->currentIndex());
}

void TileEditorMainWindow::onTileSetAdd()
{
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

	QString set = reinterpret_cast< TileSetNode* >( idx.internalPointer() )->getTileSetName();

	m_tileModel->removeTileSet( idx.row() );
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

	QString oldName = node->getTileSetName();
	node->setTileSetName( newName );
	m_ui->tileSetLV->reset();

	m_tileModel->renameTileSet( idx.row(), newName );
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
	
	m_tileModel->addLand( name );
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

	delete item;

	m_tileModel->removeLand( idx );
}

void TileEditorMainWindow::onLandEdit()
{
	QListWidgetItem *item = m_ui->landLW->currentItem();
	if( item == NULL )
		return;

	QStringList ts;
	int c = m_tileModel->rowCount();
	for( int i = 0; i < c; i++ )
	{
		QModelIndex idx = m_tileModel->index( i, 0 );
		if( !idx.isValid() )
			continue;

		TileSetNode *n = reinterpret_cast< TileSetNode* >( idx.internalPointer() );
		ts.push_back( n->getTileSetName() );
	}
	
	int r = m_ui->landLW->currentRow();

	QStringList sts;
	m_tileModel->getLandSets( r, sts );

	LandEditDialog d;
	d.setSelectedTileSets( sts );
	d.setTileSets( ts );
	int result = d.exec();

	if( result != QDialog::Accepted )
		return;

	// Update the tileset of the land
	sts.clear();
	d.getSelectedTileSets( sts );
	
	m_tileModel->setLandSets( r, sts );
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

	m_tileModel->setVegetation( idx.row(), vegetSet );

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

	m_tileModel->setVegetation( idx.row(), "" );	
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
	m_tileModel->setTexturePath( path );
}

void TileEditorMainWindow::onOrientedStateChanged( int state )
{
	QModelIndex idx = m_ui->tileSetLV->currentIndex();
	if( !idx.isValid() )
		return;

	int row = idx.row();

	if( state == Qt::Checked )
		m_tileModel->setOriented( row, true );
	else
		m_tileModel->setOriented( row, false );
}

void TileEditorMainWindow::onSurfaceDataChanged( const QString &text )
{
	QModelIndex idx = m_ui->tileSetLV->currentIndex();
	if( !idx.isValid() )
		return;

	bool ok = false;
	unsigned long data = text.toUInt( &ok );
	if( !ok )
		return;

	m_tileModel->setSurfaceData( idx.row(), data );
}

void TileEditorMainWindow::onDiffuseToggled( bool b )
{
	if( !b )
		return;

	TileItemNode::setDisplayChannel( TileConstants::TileDiffuse );
	updateTab();
}

void TileEditorMainWindow::onAdditiveToggled( bool b )
{
	if( !b )
		return;

	TileItemNode::setDisplayChannel( TileConstants::TileAdditive );
	updateTab();
}

void TileEditorMainWindow::onAlphaToggled( bool b )
{
	if( !b )
		return;

	TileItemNode::setDisplayChannel( TileConstants::TileAlpha );
	updateTab();
}

void TileEditorMainWindow::onTabChanged( int tab )
{
	if( tab == -1 )
		return;

	m_ui->diffuse128BT->setChecked( true );
	m_ui->diffuse256BT->setChecked( true );
	m_ui->diffuseTrBT->setChecked( true );
}

TileConstants::TNodeTileType tabToType( int tabId )
{
	if( tabId >= TileConstants::TileNodeTypeCount )
		return TileConstants::TileNodeTypeCount;

	return TileConstants::TNodeTileType( tabId );
}

void TileEditorMainWindow::onActionAddTile(int tabId)
{
	QModelIndex idx = m_ui->tileSetLV->currentIndex();
	if( !idx.isValid() )
	{
		QMessageBox::information( this,
									tr( "Adding new tiles" ),
									tr( "You need to have a tileset selected before you can add tiles!" ) );
		return;
	}

	int tileSet = idx.row();
	
	idx = m_tileModel->index( tileSet, 0 );
	if( !idx.isValid() )
		return;

	int setId = idx.row();

	TileSetNode *tsn = reinterpret_cast< TileSetNode* >( idx.internalPointer() );

	Node *n = tsn->child( tabId );

	QFileDialog::Options options;
	QString selectedFilter;
	QStringList fileNames = QFileDialog::getOpenFileNames(this, "Choose Tile Texture", "." , "Images (*.png);;All Files (*.*)", &selectedFilter, options);

	int c = n->childCount();

	TileConstants::TNodeTileType type = tabToType( tabId );

	QStringListIterator itr( fileNames );
	QString error;

	while( itr.hasNext() )
	{
		TileItemNode *newNode = m_tileModel->createItemNode( setId, type, c, TileConstants::TileDiffuse, itr.next() );
		if( newNode == NULL )
		{
			if( m_tileModel->hasError() )
				error = m_tileModel->getLastError();

			int reply = QMessageBox::question( this,
												tr( "Error adding tile" ),
												error + "\nContinue?",
												QMessageBox::Yes, QMessageBox::No );
			if( reply != QMessageBox::Yes )
				break;
			else
				continue;
		}
	
		n->appendRow( newNode );
		c++;
	}

	QModelIndex rootIdx = m_tileModel->index( tabId, 0, m_ui->tileSetLV->currentIndex());

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

	QModelIndex tsidx = m_ui->tileSetLV->currentIndex();
	if( !tsidx.isValid() )
		return;
	int ts = tsidx.row();
	int tile = idx.row();

	m_tileModel->removeTile( ts, tabId, tile );	
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

	QModelIndex tsidx = m_ui->tileSetLV->currentIndex();
	if( !tsidx.isValid() )
		return;
	int ts = tsidx.row();

	TileItemNode *n = reinterpret_cast< TileItemNode* >( idx.internalPointer() );
	int tile = n->id();

	m_tileModel->clearImage( ts, tabId, tile, TileItemNode::displayChannel() );
}

void TileEditorMainWindow::onActionReplaceImage( int tabId )
{
	QListView *lv = getListViewByTab( tabId );

	QModelIndex tsidx = m_ui->tileSetLV->currentIndex();
	if( !tsidx.isValid() )
		return;
	int set = tsidx.row();
	
	QModelIndex idx = lv->currentIndex();
	if( !idx.isValid() )
	{
		QMessageBox::information( this,
									tr( "Replacing tile image" ),
									tr( "No tile selected!" ) );
		return;
	}

	QString fileName = QFileDialog::getOpenFileName( this,
														tr( "Select tile image" ),
														"",
														tr( "PNG files (*.png)" ) );
	if( fileName.isEmpty() )
		return;
	
	TileItemNode *n = reinterpret_cast< TileItemNode* >( idx.internalPointer() );
	int tile = n->id();

	m_tileModel->replaceImage( set, tabId, tile, TileItemNode::displayChannel(), fileName );
	if( m_tileModel->hasError() )
	{
		QString error = m_tileModel->getLastError();
		QMessageBox::information( this,
								tr( "Error replacing tile image" ),
								error );
	}
}

void TileEditorMainWindow::onTileBankLoaded()
{
	m_ui->landLW->clear();
	// load lands

	m_ui->listView128->reset();
	m_ui->listView256->reset();
	m_ui->listViewTransition->reset();

	QString path = m_tileModel->getTexturePath();
	if( path.isEmpty() )
		m_ui->tileBankTexturePathPB->setText( "..." );
	else
		m_ui->tileBankTexturePathPB->setText( path );

	QModelIndex idx = m_tileModel->index( 0, 0 );
	if( idx.isValid() )
		m_ui->tileSetLV->setCurrentIndex( idx );

	if( m_ui->landLW->count() > 0 )
		m_ui->landLW->setCurrentRow( 0 );
}

void TileEditorMainWindow::updateTab()
{
	m_ui->tileViewTabWidget->currentWidget()->repaint();
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

	if( newIndex.isValid() )
	{
		QString vegetSet = m_tileModel->getVegetation( newIndex.row() );
		
		if( !vegetSet.isEmpty() )
			m_ui->chooseVegetPushButton->setText( vegetSet );
		else
			m_ui->chooseVegetPushButton->setText( "..." );

		m_ui->orientedCheckBox->setChecked( m_tileModel->getOriented( newIndex.row() ) );
		m_ui->surfaceDataLineEdit->setText( QString::number( m_tileModel->getSurfaceData( newIndex.row() ) ) );
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


