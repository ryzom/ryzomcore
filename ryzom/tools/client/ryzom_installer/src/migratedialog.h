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

#ifndef MIGRATEDIALOG_H
#define MIGRATEDIALOG_H

#include "ui_migratedialog.h"

/**
 * Wizard displayed at first launch, that asks user to choose source and destination directories.
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CMigrateDialog : public QDialog, public Ui::MigrateDialog
{
	Q_OBJECT

public:
	CMigrateDialog();
	virtual ~CMigrateDialog();

private slots:
	void onShowAdvancedParameters(int state);
	void onDestinationDefaultButtonClicked();
	void onDestinationBrowseButtonClicked();

	void accept();

private:
	void updateDestinationText();

	QString m_currentDirectory;
	QString m_dstDirectory;
};

#endif
