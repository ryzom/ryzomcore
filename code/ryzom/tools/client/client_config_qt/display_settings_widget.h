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

#ifndef DISPLAYSETTINGSWIDGET_H
#define DISPLAYSETTINGSWIDGET_H

#include "ui_display_settings_widget.h"
#include "widget_base.h"
#include <nel/misc/types_nl.h>

struct CVideoMode;

enum E3DDriver
{
	DRV_AUTO,
	DRV_OPENGL,
	DRV_DIRECT3D
};

/**
 @brief The display settings page of the configuration tool
*/
class CDisplaySettingsWidget : public CWidgetBase, public Ui::display_settings_widget
{
	Q_OBJECT
public:
	CDisplaySettingsWidget( QWidget *parent = NULL );
	~CDisplaySettingsWidget();

	void load();
	void save();

protected:
	void changeEvent( QEvent *event );

private slots:
	/**
	 @brief Updates the video modes combo box, based on the current driver selection.
	*/
	void updateVideoModes();

private:
	/**
	 @brief  Finds the proper index for the video mode combobox
	 @param  mode  -  the video mode read from config
	 @return Returns the proper video mode index if found, returns 0 otherwise.
    */
	uint32 findVideoModeIndex( CVideoMode *mode );


	/**
	 @brief Retrieves the driver type from the config string.
	 @param str  -  Reference to the driver string.
	 @return Returns the driver type on success, rReturns E3DDriver::DRV_AUTO otherwise.
    */
	E3DDriver getDriverFromConfigString( std::string &str ) const;
};

#endif // DISPLAYSETTINGSWIDGET_H
