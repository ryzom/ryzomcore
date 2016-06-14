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

#ifndef UNINSTALLDIALOG_H
#define UNINSTALLDIALOG_H

#include "ui_uninstalldialog.h"
#include "operation.h"

/**
 * Wizard displayed when uninstalling components from Add/Remove Program under Windows
 * or when user clicks on Uninstall in main menu.
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CUninstallDialog : public QDialog, public Ui::UninstallDialog
{
	Q_OBJECT

public:
	CUninstallDialog(QWidget *parent = NULL);
	virtual ~CUninstallDialog();

	void setSelectedComponents(const SUninstallComponents &components);
	SUninstallComponents getSelectedCompenents() const;

signals:
	void updateSize(int row, const QString &text);
	void updateLayout();

private slots:
	void accept();
	void onItemChanged(QStandardItem *item);
	void onUpdateSize(int row, const QString &text);
	void onUpdateLayout();

private:
	void showEvent(QShowEvent *event);

	void updateSizes();
	void updateButtons();

	// key is original ID, value is row index
	typedef QMap<QString, int> IDIndicesMap;

	IDIndicesMap m_serversIndices;
	IDIndicesMap m_profilesIndices;

	int m_installerIndex;
};

#endif
