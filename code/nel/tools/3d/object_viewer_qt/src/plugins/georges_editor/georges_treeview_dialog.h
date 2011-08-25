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

#ifndef GEORGES_TREEVIEWER_DIALOG_H
#define GEORGES_TREEVIEWER_DIALOG_H

#include "ui_georges_treeview_form.h"
#include "expandable_headerview.h"

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

namespace Plugin 
{

	class CGeorges;

	class CGeorgesTreeViewDialog: public QDockWidget
	{
		Q_OBJECT

	public:
		CGeorgesTreeViewDialog(QWidget *parent = 0);
		~CGeorgesTreeViewDialog();

		bool modified() {return m_modified;}
		void setModified(bool m) {m_modified = m;}

		CForm* getFormByName(const QString);
		void addParentForm(CForm *form);

		void write (  );

		QTabWidget* tabWidget() { return m_ui.treeViewTabWidget; }

		QString loadedForm;

	protected:
		void closeEvent(QCloseEvent *event);

	Q_SIGNALS:
		void changeFile(QString);
		void modified(bool);
		void closing();

	public Q_SLOTS:
		void setForm(const CForm*);
		void loadFormIntoDialog(CForm *form = 0);
		void modifiedFile( );

	private Q_SLOTS:
		void doubleClicked ( const QModelIndex & index );
		void filterRows();
		void headerClicked(int);

	private:
		Ui::CGeorgesTreeViewDialog m_ui;
		ExpandableHeaderView *m_header;
		UForm    *m_form;
		CGeorges *m_georges;

		bool m_modified;

	}; /* CGeorgesTreeViewDialog */

} /* namespace NLQT */

#endif // GEORGES_TREEVIEWER_DIALOG_H
