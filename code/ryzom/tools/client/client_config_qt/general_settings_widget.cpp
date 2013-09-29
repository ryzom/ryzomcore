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
#include "general_settings_widget.h"
#include "system.h"
#include <QTranslator>

const QString CGeneralSettingsWidget::languageCodes[ NUM_LANGUAGE_CODES ] =
{
	"en",
	"fr",
	"de",
	"hu"
};

CGeneralSettingsWidget::CGeneralSettingsWidget( QWidget *parent ) :
	CWidgetBase( parent )
{
	currentTranslator = NULL;
	setupUi( this );
	load();

	connect( languageComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onLanguageChanged() ) );
	connect( saveConfigOnQuitCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( onSomethingChanged() ) );
	connect( lowPriorityProcessCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( onSomethingChanged() ) );
}

CGeneralSettingsWidget::~CGeneralSettingsWidget()
{
}

void CGeneralSettingsWidget::load()
{
	CSystem &s = CSystem::GetInstance();

	sint32 cbIndex = getIndexForLanguageCode( QString::fromUtf8( s.config.getString( "LanguageCode" ).c_str() ) );
	if( cbIndex != -1 ){
		languageComboBox->setCurrentIndex( cbIndex );
		onLanguageChanged();
	}

	if( s.config.getInt( "SaveConfig" ) )
		saveConfigOnQuitCheckBox->setChecked( true );

	if( s.config.getInt( "ProcessPriority" ) == -1 )
		lowPriorityProcessCheckBox->setChecked( true );
	else
		lowPriorityProcessCheckBox->setChecked( false );
}

void CGeneralSettingsWidget::save()
{
	CSystem &s = CSystem::GetInstance();

	s.config.setString( "LanguageCode", std::string( languageCodes[ languageComboBox->currentIndex() ].toUtf8() ) );

	if( saveConfigOnQuitCheckBox->isChecked() )
		s.config.setInt( "SaveConfig", 1 );
	else
		s.config.setInt( "SaveConfig", 0 );

	if( lowPriorityProcessCheckBox->isChecked() )
		s.config.setInt( "ProcessPriority", -1 );
	else
		s.config.setInt( "ProcessPriority", 0 );
}

void CGeneralSettingsWidget::onLanguageChanged()
{
	sint32 i = languageComboBox->currentIndex();

	if( currentTranslator != NULL )
	{
		qApp->removeTranslator( currentTranslator );
		delete currentTranslator;
		currentTranslator = NULL;
	}

	currentTranslator = new QTranslator();
	if( currentTranslator->load( QString( ":/translations/ryzom_configuration_%1" ).arg( languageCodes[ i ] ) ) )
		qApp->installTranslator( currentTranslator );

	emit changed();
}

void CGeneralSettingsWidget::changeEvent( QEvent *event )
{
	if( event->type() == QEvent::LanguageChange )
	{
		sint32 i = languageComboBox->currentIndex();
		// Signals that are emitted on index change need to be disconnected, since retranslation cleans the widget
		disconnect( languageComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onLanguageChanged() ) );
		retranslateUi( this );
		languageComboBox->setCurrentIndex( i );
		connect( languageComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onLanguageChanged() ) );
	}

	QWidget::changeEvent( event );
}

int CGeneralSettingsWidget::getIndexForLanguageCode(const QString &languageCode)
{
	for( sint32 i = 0; i < NUM_LANGUAGE_CODES; i++ )
		if( languageCode.compare( languageCodes[ i ] ) == 0 )
			return i;

	return -1;
}
