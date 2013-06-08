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

#ifndef CLIENT_CONFIG_DIALOG_H
#define CLIENT_CONFIG_DIALOG_H

#include "ui_client_config_dialog.h"

/**
 @brief The main dialog of the configuration tool
 @details Sets up and controls the configuration pages, sets up navigation,
          sets up the ok, cancel, apply, etc buttons.
*/
class CClientConfigDialog : public QDialog, public Ui::client_config_dialog
{
	Q_OBJECT

public:
	CClientConfigDialog( QWidget *parent = NULL );
	~CClientConfigDialog();

protected:
	void closeEvent( QCloseEvent *event );
	void changeEvent( QEvent *event );

private slots:
	//////////////////////////// Main dialog buttons /////////////////////
	void onClickOK();
	void onClickApply();
	void onClickDefault();
	void onClickPlay();
	//////////////////////////////////////////////////////////////////////
	void onClickCategory( QTreeWidgetItem *item );
	void onSomethingChanged();

private:
	/**
	 @brief Tells the config pages to save their changes into the config file
	*/
	void saveChanges();

	/**
	 @brief Reloads the pages' contents from the config file.
	*/
	void reloadPages();

	/**
	 @brief Checks if it's OK to quit the application, may query the user
	 @return Returns true if it's OK to quit, returns false otherwise.
	*/
	bool isOKToQuit();
};

#endif // CLIENT_CONFIG_DIALOG_H
