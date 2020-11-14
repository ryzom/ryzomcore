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

#include <nel/misc/i18n.h>

CGeneralSettingsWidget::CGeneralSettingsWidget( QWidget *parent ) :
	CWidgetBase( parent )
{
	setupUi( this );
	load();

	connect( languageComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onSomethingChanged() ) );
	connect( saveConfigOnQuitCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( onSomethingChanged() ) );
	connect( lowPriorityProcessCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( onSomethingChanged() ) );
}

CGeneralSettingsWidget::~CGeneralSettingsWidget()
{
}

void CGeneralSettingsWidget::load()
{
	CSystem &s = CSystem::GetInstance();

	std::vector<std::string> codes = NLMISC::CI18N::getLanguageCodes();
	std::vector<ucstring> names = NLMISC::CI18N::getLanguageNames();

	languageComboBox->clear();

	for(uint i = 0; i < codes.size(); ++i)
	{
		languageComboBox->addItem(QString::fromUtf16(names[i].c_str()), QString::fromUtf8(codes[i].c_str()));
	}

	sint32 cbIndex = getIndexForLanguageCode( QString::fromUtf8( s.config.getString( "LanguageCode" ).c_str() ) );

	if (cbIndex != -1)
	{
		languageComboBox->setCurrentIndex( cbIndex );
	}

	if (s.config.getInt("SaveConfig"))
		saveConfigOnQuitCheckBox->setChecked( true );

	if (s.config.getInt("ProcessPriority") == -1)
		lowPriorityProcessCheckBox->setChecked(true);
	else
		lowPriorityProcessCheckBox->setChecked(false);
}

void CGeneralSettingsWidget::save()
{
	CSystem &s = CSystem::GetInstance();

	s.config.setString( "LanguageCode", std::string(languageComboBox->itemData(languageComboBox->currentIndex()).toString().toUtf8()));

	if( saveConfigOnQuitCheckBox->isChecked() )
		s.config.setInt( "SaveConfig", 1 );
	else
		s.config.setInt( "SaveConfig", 0 );

	if( lowPriorityProcessCheckBox->isChecked() )
		s.config.setInt( "ProcessPriority", -1 );
	else
		s.config.setInt( "ProcessPriority", 0 );
}

int CGeneralSettingsWidget::getIndexForLanguageCode(const QString &languageCode)
{
	for( sint32 i = 0; i < languageComboBox->count(); i++ )
		if( languageCode.compare(languageComboBox->itemData(i).toString()) == 0 )
			return i;

	return -1;
}
