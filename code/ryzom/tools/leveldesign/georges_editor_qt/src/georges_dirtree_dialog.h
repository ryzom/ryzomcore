/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

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

#ifndef GEORGES_DIRTREE_DIALOG_H
#define GEORGES_DIRTREE_DIALOG_H

// Qt includes
#include <QtGui/QWidget>

// STL includes

// NeL includes

// Project includes
#include "ui_georges_dirtree_form.h"
#include "filesystem_model.h"

namespace NLQT 
{

	class CGeorgesDirTreeDialog: public QDockWidget
	{
		Q_OBJECT

	public:
		CGeorgesDirTreeDialog(QString ldPath, QWidget *parent = 0);
		~CGeorgesDirTreeDialog();

	private:
		Ui::CGeorgesDirTreeDialog _ui;

		CFileSystemModel *_dirModel;
		QString _ldPath;

	Q_SIGNALS:
		void selectedForm(const QString);

	public Q_SLOTS:
		void ldPathChanged(QString path);

	private Q_SLOTS:
		void fileSelected(QModelIndex index);
		void changeFile(QString file);

	friend class CMainWindow;
	}; /* CGEorgesDirTreeDialog */

} /* namespace NLQT */

#endif // GEORGES_DIRTREE_DIALOG_H
