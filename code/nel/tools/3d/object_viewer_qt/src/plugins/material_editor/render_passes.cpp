// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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

#include "render_passes.h"
#include "nel3d_interface.h"
#include "material_observer.h"
#include <QInputDialog>
#include <QMessageBox>

namespace MaterialEditor
{
	RenderPassesWidget::RenderPassesWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		setupConnections();
		nl3dIface = NULL;
		observer = NULL;
	}

	RenderPassesWidget::~RenderPassesWidget()
	{
	}

	void RenderPassesWidget::fillList( const QStringList &list )
	{
		listWidget->clear();

		QStringListIterator itr( list );
		while( itr.hasNext() )
		{
			listWidget->addItem( itr.next() );
		}
	}

	void RenderPassesWidget::getList( QStringList &list )
	{
		for( int i = 0; i < listWidget->count(); i++ )
		{
			list.push_back( listWidget->item( i )->text() );
		}
	}

	void RenderPassesWidget::clear()
	{
		listWidget->clear();
	}

	void RenderPassesWidget::onMaterialLoaded()
	{
		clear();
		CNelMaterialProxy m = nl3dIface->getMaterial();
		if( m.isEmpty() )
			return;

		std::vector< std::string > pl;
		m.getPassList( pl );

		std::vector< std::string >::const_iterator itr = pl.begin();
		while( itr != pl.end() )
		{
			listWidget->addItem( QString( itr->c_str() ) );
			++itr;
		}
	}

	void RenderPassesWidget::setupConnections()
	{
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) );
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
		connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
		connect( editButton, SIGNAL( clicked( bool ) ), this, SLOT( onEditClicked() ) );
		connect( upButton, SIGNAL( clicked( bool ) ), this, SLOT( onUpClicked() ) );
		connect( downButton, SIGNAL( clicked( bool ) ), this, SLOT( onDownClicked() ) );
	}

	bool RenderPassesWidget::passExists( const QString &label )
	{
		int c = listWidget->count();

		for( int i = 0; i < c; i++ )
		{
			if( label == listWidget->item( i )->text() )
				return true;
		}
		
		return false;
	}

	void RenderPassesWidget::onOKClicked()
	{
		close();
	}

	void RenderPassesWidget::onAddClicked()
	{
		QString label =
			QInputDialog::getText(
			NULL,
			tr( "Pass label" ),
			tr( "Please enter the new pass' label" ) 
			);

		if( label.isEmpty() )
			return;

		if( passExists( label ) )
		{
			QMessageBox::warning( 
				NULL,
				tr( "Pass label" ),
				tr( "Pass label already exists!" ) 
				);

			return;
		}

		listWidget->addItem( label );

		CNelMaterialProxy material = nl3dIface->getMaterial();
		material.addPass( label.toUtf8().data() );
		
		if( observer != NULL )
			observer->onPassAdded( label.toUtf8().data() );
	}

	void RenderPassesWidget::onRemoveClicked()
	{
		int row = listWidget->currentRow();
		if( row == -1 )
			return;
		QString pass;

		QListWidgetItem *item = listWidget->takeItem( row );
		pass = item->text();
		delete item;

		CNelMaterialProxy material = nl3dIface->getMaterial();
		if( material.isEmpty() )
			return;

		material.removePass( pass.toUtf8().data() );

		if( observer != NULL )
			observer->onPassRemoved( pass.toUtf8().data() );
	}

	void RenderPassesWidget::onEditClicked()
	{
		QListWidgetItem *item = listWidget->currentItem();
		if( item == NULL )
			return;

		QString from = item->text();
		QString to =
			QInputDialog::getText(
			NULL,
			tr( "Editing pass label" ),
			tr( "Please enter the new label" ),
			QLineEdit::Normal,
			from
			);

		if( to.isEmpty() )
			return;

		if( from == to )
			return;

		if( passExists( to ) )
		{
			QMessageBox::warning( 
				NULL,
				tr( "Pass label" ),
				tr( "Pass label already exists!" ) 
				);
			return;
		}

		item->setText( to );
		
		CNelMaterialProxy material = nl3dIface->getMaterial();
		if( material.isEmpty() )
			return;

		material.renamePass( from.toUtf8().data(), to.toUtf8().data() );

		if( observer != NULL )
			observer->onPassRenamed( from.toUtf8().data(), to.toUtf8().data() );
	}

	void RenderPassesWidget::onUpClicked()
	{
		QListWidgetItem *item = listWidget->currentItem();
		if( item == NULL )
			return;

		int row = listWidget->currentRow();
		if( row == 0 )
			return;

		item = listWidget->takeItem( row );
		listWidget->insertItem( row - 1, item );
		listWidget->setCurrentRow( row - 1 );

		QString s = item->text();

		CNelMaterialProxy material = nl3dIface->getMaterial();
		if( material.isEmpty() )
			return;

		material.movePassUp( s.toUtf8().data() );

		if( observer != NULL )
			observer->onPassMovedUp( s.toUtf8().data() );
	}

	void RenderPassesWidget::onDownClicked()
	{
		QListWidgetItem *item = listWidget->currentItem();
		if( item == NULL )
			return;

		int row = listWidget->currentRow();
		if( row == ( listWidget->count() - 1 ) )
			return;

		item = listWidget->takeItem( row );
		listWidget->insertItem( row + 1, item );
		listWidget->setCurrentRow( row + 1 );

		QString s = item->text();
		
		CNelMaterialProxy material = nl3dIface->getMaterial();
		if( material.isEmpty() )
			return;

		material.movePassDown( s.toUtf8().data() );

		if( observer != NULL )
			observer->onPassMovedDown( s.toUtf8().data() );
	}
}

