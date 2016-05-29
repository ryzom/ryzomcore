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

#ifndef UNINSTALLWIZARDDIALOG_H
#define UNINSTALLWIZARDDIALOG_H

#include "ui_uninstallwizard.h"

/**
 * Wizard displayed at first launch, that asks user to choose source and destination directories.
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CUninstallWizardDialog : public QDialog, public Ui::UninstallWizardDialog
{
	Q_OBJECT

public:
	CUninstallWizardDialog(QWidget *parent = NULL);
	virtual ~CUninstallWizardDialog();

	QVector<int> getSelectedServers() const;
	QVector<int> getSelectedProfiles() const;

	bool isInstallerSelected() const;

private slots:
	void accept();
	void onItemChanged(QStandardItem *item);

private:
	void updateSizes();
	void updateButtons();

	// key is original ID, value is row index
	QMap<int, int> m_serversIndices;
	QMap<int, int> m_profilesIndices;

	int m_installerIndex;
};

#endif
