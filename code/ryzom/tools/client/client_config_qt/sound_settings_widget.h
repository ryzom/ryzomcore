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

#ifndef SOUNDSETTINGSWIDGET_H
#define SOUNDSETTINGSWIDGET_H

#include "ui_sound_settings_widget.h"
#include "widget_base.h"

/**
 @brief The sound settings page of the configuration tool
*/
class CSoundSettingsWidget : public CWidgetBase, public Ui::sound_settings_widget
{
	Q_OBJECT
public:
	CSoundSettingsWidget( QWidget *parent = NULL );
	~CSoundSettingsWidget();

	void load();
	void save();

protected:
	void changeEvent( QEvent *event );

private slots:
	void onTracksSliderChange();

private:
	/**
	 @brief Updates the text on the tracks label.
	*/
	void updateTracksLabel();
};

#endif // SOUNDSETTINGSWIDGET_H
