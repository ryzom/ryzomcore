// Ryzom Core - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#include "startup_settings_dlg.h"
#include <QFileDialog>
#include <QSettings>
#include <QStringList>

int findListItem( QListWidget *l, const QString &s )
{
	for( int i = 0; i < l->count(); i++ )
	{
		QListWidgetItem *item = l->item( i );
		if( item->text() == s )
			return i;
	}

	return -1;
}

StartupSettingsDlg::StartupSettingsDlg( QDialog *parent ) :
QDialog( parent )
{
	setupUi( this );
	setupConnections();
	settings = NULL;
}

StartupSettingsDlg::~StartupSettingsDlg()
{
}

void StartupSettingsDlg::load()
{
	pluginsLE->setText( settings->value( "PluginPath" ).toString() );

	settings->beginGroup( "DataPath" );

	sheetsLE->setText( settings->value( "LevelDesignPath" ).toString() );
	assetsLE->setText( settings->value( "AssetsPath" ).toString() );
	primitivesLE->setText( settings->value( "PrimitivesPath" ).toString() );
	ligoLE->setText( settings->value( "LigoConfigFile" ).toString() );

	QStringList l = settings->value( "SearchPaths" ).toStringList();
	{
		QStringListIterator itr( l );
		while( itr.hasNext() )
		{
			QString p = itr.next();
			if( findListItem( searchLW, p ) == -1 )
				searchLW->addItem( p );
		}
	}

	l.clear();

	l = settings->value( "RecursiveSearchPathes" ).toStringList();
	{
		QStringListIterator itr( l );
		while( itr.hasNext() )
		{
			QString p = itr.next();
			if( findListItem( recursiveLW, p ) == -1 )
				recursiveLW->addItem( p );
		}
	}

	settings->endGroup();
}

void StartupSettingsDlg::saveSearchPaths()
{
	QStringList l;
	for( int i = 0; i < searchLW->count(); i++ )
	{
		l.push_back( searchLW->item( i )->text() );
	}
	
	settings->setValue( "SearchPaths", l );
}

void StartupSettingsDlg::saveRecursivePaths()
{
	QStringList l;
	for( int i = 0; i < recursiveLW->count(); i++ )
	{
		l.push_back( recursiveLW->item( i )->text() );
	}
	
	settings->setValue( "RecursiveSearchPathes", l );
}

void StartupSettingsDlg::save()
{
	settings->setValue( "PluginPath", pluginsLE->text() );

	settings->beginGroup( "DataPath" );

	settings->setValue( "LevelDesignPath", sheetsLE->text() );
	settings->setValue( "AssetsPath", assetsLE->text() );
	settings->setValue( "PrimitivesPath", primitivesLE->text() );
	settings->setValue( "LigoConfigFile", ligoLE->text() );

	saveSearchPaths();
	saveRecursivePaths();

	settings->endGroup();

	settings->sync();
}

void StartupSettingsDlg::accept()
{
	save();
	QDialog::accept();
}

void StartupSettingsDlg::reject()
{
	QDialog::reject();
}

void StartupSettingsDlg::onOKClicked()
{
	accept();
}

void StartupSettingsDlg::onCancelClicked()
{
	reject();
}

void StartupSettingsDlg::onPluginBClicked()
{
	QString p = QFileDialog::getExistingDirectory( this, tr( "Plugins directory" ), "" );
	pluginsLE->setText( p );
}

void StartupSettingsDlg::onSheetsBClicked()
{
	QString p = QFileDialog::getExistingDirectory( this, tr( "Sheets directory" ), "" );
	sheetsLE->setText( p );
}

void StartupSettingsDlg::onAssetsBClicked()
{
	QString p = QFileDialog::getExistingDirectory( this, tr( "Assets directory" ), "" );
	assetsLE->setText( p );
}

void StartupSettingsDlg::onPrimitivesBClicked()
{
	QString p = QFileDialog::getExistingDirectory( this, tr( "Primitives directory" ), "" );
	primitivesLE->setText( p );
}

void StartupSettingsDlg::onLigoBClicked()
{
    QString p;
    p = QFileDialog::getOpenFileName( this, tr( "LIGO config file" ), "" );
	ligoLE->setText( p );
}

void StartupSettingsDlg::onPathAddClicked()
{
	QString p = QFileDialog::getExistingDirectory( this, tr( "Search path" ), "" );
	if( p.isEmpty() )
		return;

	if( findListItem( searchLW, p ) != -1 )
		return;

	searchLW->addItem( p );
}

void StartupSettingsDlg::onPathRemoveClicked()
{
	QListWidgetItem *i = searchLW->currentItem();
	if( i == NULL )
		return;
	
	delete i;
}

void StartupSettingsDlg::onRecursiveAddClicked()
{
	QString p = QFileDialog::getExistingDirectory( this, tr( "Recursive search path" ), "" );
	if( p.isEmpty() )
		return;

	if( findListItem( recursiveLW, p ) != -1 )
		return;

	recursiveLW->addItem( p );
}

void StartupSettingsDlg::onRecursiveRemoveClicked()
{
	QListWidgetItem *i = recursiveLW->currentItem();
	if( i == NULL )
		return;
	
	delete i;
}


void StartupSettingsDlg::setupConnections()
{
	connect( bb, SIGNAL( accepted() ), this, SLOT( onOKClicked() ) );
	connect( bb, SIGNAL( rejected() ), this, SLOT( onCancelClicked() ) );

	connect( pluginsB, SIGNAL( clicked( bool ) ), this, SLOT( onPluginBClicked() ) );	
	connect( sheetsB, SIGNAL( clicked( bool ) ), this, SLOT( onSheetsBClicked() ) );
	connect( assetsB, SIGNAL( clicked( bool ) ), this, SLOT( onAssetsBClicked() ) );
	connect( primitivesB, SIGNAL( clicked( bool ) ), this, SLOT( onPrimitivesBClicked() ) );
	connect( ligoB, SIGNAL( clicked( bool ) ), this, SLOT( onLigoBClicked() ) );

	connect( pathAddB, SIGNAL( clicked( bool ) ), this, SLOT( onPathAddClicked() ) );
	connect( pathRemoveB, SIGNAL( clicked( bool ) ), this, SLOT( onPathRemoveClicked() ) );
	connect( recAddB, SIGNAL( clicked( bool ) ), this, SLOT( onRecursiveAddClicked() ) );
	connect( recRemoveB, SIGNAL( clicked( bool ) ), this, SLOT( onRecursiveRemoveClicked() ) );
}

