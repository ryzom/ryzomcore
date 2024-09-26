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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "ui_settingsdialog.h"

/**
 * Settings dialog
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CSettingsDialog : public QDialog, public Ui::SettingsDialog
{
	Q_OBJECT

public:
	CSettingsDialog(QWidget *parent = NULL);
	virtual ~CSettingsDialog();

	QString getInstallationDirectory() const;

public slots:
	void onInstallationDirectoryButtonClicked();

private slots:
	void accept();

private:
	void updateInstallationDirectoryLabel();

	QString m_installationDirectory;
};

#endif
