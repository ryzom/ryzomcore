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

#ifndef INSTALLDIALOG_H
#define INSTALLDIALOG_H

#include "ui_installdialog.h"

/**
 * Wizard displayed at first launch, that asks user to choose source and destination directories.
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CInstallDialog : public QDialog, public Ui::InstallDialog
{
	Q_OBJECT

public:
	CInstallDialog();
	virtual ~CInstallDialog();

private slots:
	void onShowAdvancedParameters(int state);
	void onAnotherLocationBrowseButtonClicked();
	void onDestinationBrowseButtonClicked();

	void accept();

private:
	void updateAnotherLocationText();
	void updateDestinationText();

	QString m_oldDirectory;
	QString m_anotherDirectory;
	QString m_dstDirectory;
};

#endif
