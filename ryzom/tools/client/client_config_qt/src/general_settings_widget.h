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

#ifndef GENERALSETTINGWIDGET_H
#define GENERALSETTINGWIDGET_H

#include "ui_general_settings_widget.h"
#include "widget_base.h"

#include "nel/misc/types_nl.h"

class QTranslator;

/**
 @brief The general settings page of the configuration tool
*/
class CGeneralSettingsWidget : public CWidgetBase, public Ui::general_settings_widget
{
	Q_OBJECT

public:
	CGeneralSettingsWidget( QWidget *parent = NULL );
	virtual	~CGeneralSettingsWidget();

	void load();
	void save();

private:
	/**
	 @brief  Retrieves the language combobox index for the language code provided.
	 @param  languageCode  -  Reference to the language code, we are trying to find.
	 @return Returns the index on success, returns -1 if the language code cannot be found.
    */
	sint32 getIndexForLanguageCode(const QString &languageCode);
};

#endif // GENERALSETTINGWIDGET_H
