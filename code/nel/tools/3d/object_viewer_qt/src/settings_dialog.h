/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <nel/misc/types_nl.h>
#include "ui_settings_form.h"

// STL includes

// NeL includes
#include <nel/misc/config_file.h>

// Project includes

namespace NLQT {

/**
@class CSettingsDialog
@brief Settings dialog (graphics/sound/search path/vegetable).
*/
class CSettingsDialog: public QDialog
{
     Q_OBJECT

public:
	CSettingsDialog(QWidget *parent = 0);
	~CSettingsDialog();

private Q_SLOTS:
	void addPath();
	void removePath();
	void upPath();
	void downPath();
	void applyPressed();

	void setTileBank();
	void setTileFarBank();
	void setTextureVegetable();
	void addZone();
	void removeZone();

	void setEnableBloom(bool state);
	void setEnableSquareBloon(bool state);
	void setDensityBloom(int density);

private:
	void cfcbGraphicsDrivers(NLMISC::CConfigFile::CVar &var);
	void cfcbSoundDrivers(NLMISC::CConfigFile::CVar &var);
	void cfcbSearchPaths(NLMISC::CConfigFile::CVar &var);
	
	void loadGraphicsSettings();
	void loadSoundSettings();
	void loadPathsSettings();
	void loadVegetableSettings();
	void saveGraphicsSettings();
	void saveSoundSettings();
	void savePathsSettings();
	void saveVegetableSettings();
	
	Ui::CSettingsDialog ui;

}; /* class CSettingsDialog */

} /* namespace NLQT */

#endif // SETTINGS_DIALOG_H
