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

#ifndef STARTUP_SETTINGS_DIALOG_H
#define STARTUP_SETTINGS_DIALOG_H

#include "ui_startup_settings_dlg.h"

class QSettings;

class StartupSettingsDlg : public QDialog, public Ui::StartupSettingsDlg
{
	Q_OBJECT
public:
	StartupSettingsDlg( QDialog *parent = NULL );
	~StartupSettingsDlg();

	void setSettings( QSettings *s ){ settings = s; }

	void load();

public Q_SLOTS:
	void accept();
	void reject();

	void onOKClicked();
	void onCancelClicked();

	void onPluginBClicked();
	void onSheetsBClicked();
	void onAssetsBClicked();
	void onPrimitivesBClicked();
	void onLigoBClicked();

	void onPathAddClicked();
	void onPathRemoveClicked();
	void onRecursiveAddClicked();
	void onRecursiveRemoveClicked();

private:
	void setupConnections();
	void save();
	void saveSearchPaths();
	void saveRecursivePaths();

	QSettings *settings;
};

#endif
