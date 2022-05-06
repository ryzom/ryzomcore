// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian JAEKEL <aj@elane2k.com>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
#include "georges_dock_widget.h"
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

	class CGeorgesTreeViewDialog: public GeorgesDockWidget
	{
		Q_OBJECT

	public:
		CGeorgesTreeViewDialog(QWidget *parent = 0);
		~CGeorgesTreeViewDialog();

		NLGEORGES::CForm* getFormByName(const QString formName);
		NLGEORGES::CForm* getFormByDfnName(const QString dfnName);

		/// Retrieves the root element based on the slot (document or held elements.)
		NLGEORGES::CFormElm *getRootNode(uint slot);

		/// Returns the form as a CForm pointer.
		NLGEORGES::CForm *getFormPtr();

		void addParentForm(QString parentFormNm);

		bool load( const QString &fileName );
		void write (  );
		bool newDocument( const QString &fileName, const QString &dfn );

		QTabWidget* tabWidget() { return m_ui.treeViewTabWidget; }

		QString loadedForm;

	Q_SIGNALS:
		void changeFile(QString);
		void modified();

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
		void onVStructChanged( const QString &name );
		void onRenameArrayEntry();
		void onCommentsEdited();

	private:
		void log( const QString &msg );

		Ui::CGeorgesTreeViewDialog m_ui;
		ExpandableHeaderView *m_header;
		UForm    *m_form;
		CGeorges *m_georges;

		/// Contains a record of the last directory a sheet file dialog was opened for.
		QString m_lastSheetDir;

		BrowserCtrl *m_browserCtrl;
		CGeorgesFormModel *m_model;

	}; /* CGeorgesTreeViewDialog */

} /* namespace GeorgesQt */

#endif // GEORGES_TREEVIEWER_DIALOG_H
