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
#include <QtGui/QUndoCommand>
#include <QtGui/QUndoStack>


// STL includes

// NeL includes

// Project includes

class BrowserCtrl;

namespace NLGEORGES
{
	class UForm;
	class CForm;
    class CFormElm;
}

using namespace NLGEORGES;

namespace GeorgesQt 
{

	class CGeorges;
	class CGeorgesFormModel;

	class CGeorgesTreeViewDialog: public QDockWidget
	{
		Q_OBJECT

	public:
		CGeorgesTreeViewDialog(QWidget *parent = 0);
		~CGeorgesTreeViewDialog();

		bool isModified() {return m_modified;}
		void setModified(bool m) {m_modified = m;}

		NLGEORGES::CForm* getFormByName(const QString formName);
		NLGEORGES::CForm* getFormByDfnName(const QString dfnName);

		/// Retrieves the root element based on the slot (document or held elements.)
		NLGEORGES::CFormElm *getRootNode(uint slot);

		/// Returns the form as a CForm pointer.
		NLGEORGES::CForm *getFormPtr();

		void addParentForm(QString parentFormNm);

		void write (  );

		QTabWidget* tabWidget() { return m_ui.treeViewTabWidget; }

		void setUndoStack(QUndoStack *stack) {
			m_undoStack = stack;
		}



		QString loadedForm;

	protected:
		void closeEvent(QCloseEvent *event);

	Q_SIGNALS:
		void changeFile(QString);
		void modified();
		void closing();

	public Q_SLOTS:
		void setForm(const CForm*);
		void loadFormIntoDialog(CForm *form = 0);
		void modifiedFile( );
		void checkVisibility(bool);
		void showContextMenu(const QPoint &pos);

	private Q_SLOTS:
		void doubleClicked ( const QModelIndex & index );
		void filterRows();
		void headerClicked(int);

		void onArrayResized( const QString &name, int size );
		void onAppendArray();
		void onDeleteArrayEntry();
		void onValueChanged( const QString &key, const QString &value );
		void onRenameArrayEntry();

	private:
		void log( const QString &msg );

		Ui::CGeorgesTreeViewDialog m_ui;
		ExpandableHeaderView *m_header;
		UForm    *m_form;
		CGeorges *m_georges;

		QUndoStack *m_undoStack;

		/// Contains a record of the last directory a sheet file dialog was opened for.
		QString m_lastSheetDir;

		bool m_modified;

		BrowserCtrl *m_browserCtrl;
		CGeorgesFormModel *m_model;

	}; /* CGeorgesTreeViewDialog */

} /* namespace GeorgesQt */

#endif // GEORGES_TREEVIEWER_DIALOG_H
