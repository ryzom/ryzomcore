// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian Jaekel <aj at elane2k dot com>
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

#ifndef GEORGES_DIRTREE_DIALOG_H
#define GEORGES_DIRTREE_DIALOG_H

// Qt includes
#include <QtGui/QWidget>

// STL includes

// NeL includes

// Project includes
#include "ui_georges_dirtree_form.h"
#include "georges_filesystem_model.h"

namespace GeorgesQt
{

class CGeorgesDirTreeDialog: public QDockWidget
{
	Q_OBJECT

public:
	CGeorgesDirTreeDialog(QString ldPath, QWidget *parent = 0);
	~CGeorgesDirTreeDialog();

	void ldPathChanged(QString);

private:
	Ui::CGeorgesDirTreeDialog m_ui;

	CGeorgesFileSystemModel *m_dirModel;
	//CGeorgesFileSystemProxyModel *m_proxyModel;
	QString m_ldPath;

Q_SIGNALS:
	void fileSelected(const QString&);

private Q_SLOTS:
	void fileSelected(QModelIndex index);
	void changeFile(QString file);

}; /* CGEorgesDirTreeDialog */

} /* namespace GeorgesQt */

#endif // GEORGES_DIRTREE_DIALOG_H
