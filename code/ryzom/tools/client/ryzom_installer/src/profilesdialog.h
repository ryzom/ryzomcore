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

#ifndef PROFILESDIALOG_H
#define PROFILESDIALOG_H

#include "ui_profilesdialog.h"

class CProfilesModel;
class CServersModel;

/**
 * Dialog displayed when editing existing profiles.
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CProfilesDialog : public QDialog, public Ui::ProfilesDialog
{
	Q_OBJECT

public:
	CProfilesDialog(QWidget *parent = NULL);
	virtual ~CProfilesDialog();

private slots:
	void accept();

	void onAddProfile();
	void onDeleteProfile();
	void onProfileClicked(const QModelIndex &index);
	void onProfileDirectoryClicked();

	void displayProfile(int index);
	void saveProfile(int index);
	void deleteProfile(int index);
	void addProfile();

	void updateExecutableVersion(int index);

	void onExecutableDefaultClicked();
	void onExecutableBrowseClicked();

private:
	CProfilesModel *m_model;
	CServersModel *m_serversModel;

	int m_currentProfileIndex;
};

#endif
