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

#ifndef GEORGES_TREEVIEWER_DIALOG_H
#define GEORGES_TREEVIEWER_DIALOG_H

#include "ui_georges_treeview_form.h"

// Qt includes
#include <QtGui/QDockWidget>

// STL includes

// NeL includes

// Project includes

namespace NLGEORGES
{
	class UForm;
	class CForm;
}

using namespace NLGEORGES;

namespace NLQT 
{

	class CGeorges;

	class CGeorgesTreeViewDialog: public QDockWidget
	{
		Q_OBJECT

	public:
		CGeorgesTreeViewDialog(QWidget *parent = 0, bool empty = false);
		~CGeorgesTreeViewDialog();

		bool modified() {return _modified;}
		void setModified(bool m) {_modified = m;}

		CForm* getFormByName(const QString);
		void addParentForm(CForm *form);

		void write (  );

		QString loadedForm;

	protected:
		void closeEvent(QCloseEvent *event);

    Q_SIGNALS:
		void changeFile(QString);
		void modified(bool);
	public Q_SLOTS:
		void setForm(const CForm*);
		void loadFormIntoDialog(CForm *form = 0);
		void modifiedFile( );
	private Q_SLOTS:
		void doubleClicked ( const QModelIndex & index );
		void filterRows();

	private:
		Ui::CGeorgesTreeViewDialog _ui;
		UForm          *_form;
		NLQT::CGeorges *_georges;

		bool _modified;

	}; /* CGeorgesTreeViewDialog */

} /* namespace NLQT */

#endif // GEORGES_TREEVIEWER_DIALOG_H
