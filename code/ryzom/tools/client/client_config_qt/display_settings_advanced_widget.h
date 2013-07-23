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

#ifndef DISPLAYSETTINGSADVANCEDWIDGET_H
#define DISPLAYSETTINGSADVANCEDWIDGET_H

#include "ui_display_settings_advanced_widget.h"
#include "widget_base.h"

/**
 @brief The advanced display settings page of the configuration tool
*/
class CDisplaySettingsAdvancedWidget : public CWidgetBase, public Ui::display_settings_advanced_widget
{
	Q_OBJECT
public:
	CDisplaySettingsAdvancedWidget( QWidget *parent );
	~CDisplaySettingsAdvancedWidget();

	void load();
	void save();

protected:
	void changeEvent( QEvent *event );

};

#endif // DISPLAYSETTINGSADVANCEDWIDGET_H
