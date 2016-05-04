// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "client_config_dialog.h"

#include "general_settings_widget.h"
#include "display_settings_widget.h"
#include "display_settings_details_widget.h"
#include "display_settings_advanced_widget.h"
#include "sound_settings_widget.h"
#include "sys_info_widget.h"
#include "sys_info_opengl_widget.h"
#include "sys_info_d3d_widget.h"

#include "system.h"

#include <QtGui>
#include <QMessageBox>

#include "nel/misc/cmd_args.h"

extern NLMISC::CCmdArgs Args;

CClientConfigDialog::CClientConfigDialog( QWidget *parent ) :
	QDialog( parent )
{
	setupUi( this );
	connect( buttonBox->button( QDialogButtonBox::Cancel ), SIGNAL( clicked() ), this, SLOT( close() ) );
	connect( buttonBox->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), this, SLOT( onClickOK() ) );
	connect( applyButton, SIGNAL( clicked() ), this, SLOT( onClickApply() ) );
	connect( defaultButton, SIGNAL( clicked() ), this, SLOT( onClickDefault() ) );
	connect( playButton, SIGNAL( clicked() ), this, SLOT( onClickPlay() ) );
	connect( treeWidget, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
			 this, SLOT( onClickCategory( QTreeWidgetItem * ) ) );

	// General
	QTreeWidgetItem *item = treeWidget->topLevelItem( 0 );
	item->setData( 0, Qt::UserRole, 0 );

	// Display
	item = treeWidget->topLevelItem( 1 );
	item->setData( 0, Qt::UserRole, 1 );

	// Display details
	item = treeWidget->topLevelItem( 1 )->child( 0 );
	item->setData( 0, Qt::UserRole, 2 );

	// Display advanced
	item = treeWidget->topLevelItem( 1 )->child( 1 );
	item->setData( 0, Qt::UserRole, 3 );

	// Sound
	item = treeWidget->topLevelItem( 2 );
	item->setData( 0, Qt::UserRole, 4 );

	// System information
	item = treeWidget->topLevelItem( 3 );
	item->setData( 0, Qt::UserRole, 5 );

	// OpenGL info
	item = treeWidget->topLevelItem( 3 )->child( 0 );
	item->setData( 0, Qt::UserRole, 6 );

	// Direct3D info
	item = treeWidget->topLevelItem( 3 )->child( 1 );
	item->setData( 0, Qt::UserRole, 7 );

#ifndef Q_OS_WIN
	// Hide Direct3D page under Linux and OS X
	item->setHidden(true);
#endif

#ifdef RZ_USE_STEAM
	// Hide Play Ryzom button if using Steam because we need to launch it from Steam
	playRyzom->setHidden(true);
#endif

	CategoryStackedWidget->addWidget( new CGeneralSettingsWidget( CategoryStackedWidget ) );
	CategoryStackedWidget->addWidget( new CDisplaySettingsWidget( CategoryStackedWidget ) );
	CategoryStackedWidget->addWidget( new CDisplaySettingsDetailsWidget( CategoryStackedWidget ) );
	CategoryStackedWidget->addWidget( new CDisplaySettingsAdvancedWidget( CategoryStackedWidget ) );
	CategoryStackedWidget->addWidget( new CSoundSettingsWidget( CategoryStackedWidget ) );
	CategoryStackedWidget->addWidget( new CSysInfoWidget( CategoryStackedWidget ) );
	CategoryStackedWidget->addWidget( new CSysInfoOpenGLWidget( CategoryStackedWidget ) );

#ifdef Q_OS_WIN
	// Add Direct3D widget only under Windows
	CategoryStackedWidget->addWidget( new CSysInfoD3DWidget( CategoryStackedWidget ) );
#endif

	for( sint32 i = 0; i < CategoryStackedWidget->count();  i++ )
	{
		QWidget *w = CategoryStackedWidget->widget( i );

		// The sysinfo pages are not derived from CWidgetBase, so they don't have this signal!
		if( qobject_cast< CWidgetBase * >( w ) == NULL )
			continue;

		connect( w, SIGNAL( changed() ), this, SLOT( onSomethingChanged() ) );
	}

	applyButton->setEnabled( false );
}

CClientConfigDialog::~CClientConfigDialog()
{
}

void CClientConfigDialog::closeEvent( QCloseEvent *event )
{
	if( isOKToQuit() )
		event->accept();
	else
		event->ignore();
}

void CClientConfigDialog::onClickOK()
{
	saveChanges();
	// Since we use the apply button's enabled state to check for unsaved changes on quit, we disable it
	applyButton->setEnabled( false );
	close();
}

void CClientConfigDialog::onClickApply()
{
	saveChanges();
	applyButton->setEnabled( false );
}

void CClientConfigDialog::onClickDefault()
{
	CSystem::GetInstance().config.revertToDefault();
	CSystem::GetInstance().config.save();
	reloadPages();
	applyButton->setEnabled( false );
}

void CClientConfigDialog::onClickPlay()
{
	bool started = false;

	QStringList arguments;

	if (Args.haveArg("p"))
	{
		arguments << "-p" << QString::fromUtf8(Args.getArg("p").front().c_str());
	}

	QString clientFullPath = QString::fromUtf8(Args.getProgramPath().c_str());

#ifdef Q_OS_WIN32
#ifdef _DEBUG
	clientFullPath += "ryzom_client_d.exe";
#else
	clientFullPath += "ryzom_client_r.exe";
#endif
#elif defined(Q_OS_MAC)
	clientFullPath += "Ryzom";
#else
	clientFullPath += "ryzom_client";
#endif

	started = QProcess::startDetached(clientFullPath, arguments);

	onClickOK();
}

void CClientConfigDialog::onClickCategory( QTreeWidgetItem *item )
{
	if( item == NULL )
		return;

	static const char *iconNames[] =
	{
		":/resources/general_icon.png",
		":/resources/display_icon.png",
		":/resources/display_properties_icon.png",
		":/resources/display_config_icon.png",
		":/resources/sound_icon.png",
		":/resources/general_icon.png",
		":/resources/card_icon.png",
		":/resources/card_icon.png"
	};

	sint32 index = item->data( 0, Qt::UserRole ).toInt();

	if( ( index < 0 ) || ( index > 7 ) )
		return;

	CategoryStackedWidget->setCurrentIndex( index );
	categoryLabel->setText( item->text( 0 ) );
	topleftIcon->setPixmap( QPixmap( QString::fromUtf8( iconNames[ index ] ) ) );
}

void CClientConfigDialog::onSomethingChanged()
{
	applyButton->setEnabled( true );
}

void CClientConfigDialog::saveChanges()
{
	// First we tell the pages to save their changes into the cached config file
	for( sint32 i = 0; i < CategoryStackedWidget->count(); i++ )
	{
		QWidget *w = CategoryStackedWidget->widget( i );
		CWidgetBase *wb = qobject_cast< CWidgetBase * >( w );

		// The system information pages are not derived from CWidgetBase, so they can't save!
		if( wb == NULL )
			continue;

		wb->save();
	}

	// Then we write the cache to the disk
	CSystem::GetInstance().config.save();
}

void CClientConfigDialog::reloadPages()
{
	for( sint32 i = 0; i < CategoryStackedWidget->count(); i++ )
	{
		QWidget *w = CategoryStackedWidget->widget( i );
		CWidgetBase *wb = qobject_cast< CWidgetBase * >( w );

		// The system information pages are not derived from CWidgetBase, so they can't load!
		if( wb == NULL )
			continue;

		wb->load();
	}
}

bool CClientConfigDialog::isOKToQuit()
{
	// If the apply button is enabled we have unsaved changes
	if( applyButton->isEnabled() )
	{
		sint32 r = QMessageBox::warning(
										this,
										tr( "Ryzom configuration" ),
										tr( "Are you sure you want to quit without saving the configuration?" ),
										QMessageBox::Yes | QMessageBox::No
										);

		if( r == QMessageBox::No )
			return false;
	}

	return true;
}

