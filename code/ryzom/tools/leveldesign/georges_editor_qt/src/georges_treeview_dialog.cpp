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

#include "georges_treeview_dialog.h"

// Qt includes
#include <QtGui/QWidget>
#include <QSettings>
#include <QFileDialog>

// NeL includes
#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/o_xml.h>
#include <nel/georges/form.h>

// Project includes
#include "modules.h"
#include "georges.h"
#include "georgesform_model.h"
#include "georgesform_proxy_model.h"
#include "formitem.h"
#include "formdelegate.h"

using namespace NLMISC;
using namespace NLGEORGES;

namespace NLQT 
{

	CGeorgesTreeViewDialog::CGeorgesTreeViewDialog(QWidget *parent /*= 0*/, bool emptyView /*= false*/)
		: QDockWidget(parent)
	{
		_georges = new NLQT::CGeorges;

		loadedForm = "";
		_modified = false;

		_ui.setupUi(this);
		_ui.treeViewTabWidget->setTabEnabled (2,false);

		if (emptyView) 
		{
			_ui.treeViewTabWidget->clear();
			setWindowTitle("Form Area");
		}

		_ui.checkBoxParent->setStyleSheet("background-color: rgba(0,255,0,30)");
		_ui.checkBoxDefaults->setStyleSheet("background-color: rgba(255,0,0,30)");
		_form = 0;

		FormDelegate *formdelegate = new FormDelegate(this);
		_ui.treeView->setItemDelegateForColumn(1, formdelegate);


		connect(_ui.treeView, SIGNAL(doubleClicked (QModelIndex)),
			this, SLOT(doubleClicked (QModelIndex)));
		connect(_ui.checkBoxParent, SIGNAL(toggled(bool)),
			this, SLOT(filterRows()));
		connect(_ui.checkBoxDefaults, SIGNAL(toggled(bool)),
			this, SLOT(filterRows()));
	}

	CGeorgesTreeViewDialog::~CGeorgesTreeViewDialog()
	{
		delete _ui.treeView->itemDelegateForColumn(1);
		delete _form;
		deleteLater();
		//QSettings settings("RyzomCore", "GeorgesQt");
		//settings.setValue("dirViewGeometry", saveGeometry());
	}

	void CGeorgesTreeViewDialog::setForm(const CForm *form) 
	{
		_form = (UForm*)form;
	}

	CForm* CGeorgesTreeViewDialog::getFormByName(const QString formName) 
	{
		if(NLMISC::CPath::exists(formName.toUtf8().constData()))
		{
			return (CForm*)_georges->loadForm(formName.toUtf8().constData());
		}
		else
		{
			CForm *form = 0;
			// Load the DFN
			std::string extStr = NLMISC::CFile::getExtension( formName.toUtf8().constData() );
			std::string dfnName = extStr + ".dfn";
			UFormDfn *formdfn;
			if (NLMISC::CPath::exists(dfnName))
			{
				formdfn = _georges->loadFormDfn (dfnName);
				if (!formdfn)
				{
					nlwarning("Failed to load dfn: %s", dfnName.c_str());
					return 0;
				}
			}
			else
			{
				nlwarning("Cannot find dfn: %s", dfnName.c_str());
				return 0;
			}

			form = new CForm;

			// Build the root element
			((CFormElmStruct*)&form->getRootNode())->build((CFormDfn*)formdfn);

			uint i;
			for (i=0; i<CForm::HeldElementCount; i++)
			{
				((CFormElmStruct*)(((CForm*)form)->HeldElements[i]))->build ((CFormDfn*)formdfn);
			}
			return form;
		}
		return 0;
	}

	void CGeorgesTreeViewDialog::loadFormIntoDialog(CForm *form) 
	{

		if(form)
			_form = form;

		if (_form) 
		{
			UFormElm *root = 0;
			root = &_form->getRootNode();

			QStringList parents;
			for (uint i = 0; i < _form->getNumParent(); i++) 
			{
				UForm *u = _form->getParentForm(i);
				parents << u->getFilename().c_str();
			}

			QString comments;
			comments = _form->getComment().c_str();

			if (!comments.isEmpty()) 
			{
				_ui.treeViewTabWidget->setTabEnabled (1,true);
				_ui.commentEdit->setPlainText(comments);
			}

			QStringList strList;
			std::set<std::string> dependencies;
			_form->getDependencies(dependencies);

			QMap< QString, QStringList> deps;
			Q_FOREACH(std::string str, dependencies) 
			{
				QString file = str.c_str();
				if (str == _form->getFilename()) continue;
				deps[file.remove(0,file.indexOf(".")+1)] << str.c_str();
			}
			nlinfo("typ's %d",deps["typ"].count());
			nlinfo("dfn's %d",deps["dfn"].count());

			//nlwarning(strList.join(";").toUtf8().constData());
			if (root) 
			{
				loadedForm = _form->getFilename().c_str();

				CGeorgesFormModel *model = new CGeorgesFormModel(root,deps,comments,parents);
				CGeorgesFormProxyModel *proxyModel = new CGeorgesFormProxyModel();
				proxyModel->setSourceModel(model);
				_ui.treeView->setModel(proxyModel);
				_ui.treeView->expandAll();
				_ui.treeView->resizeColumnToContents(0);
				_ui.treeView->resizeColumnToContents(1);
				_ui.treeView->resizeColumnToContents(2);
				//_ui.treeView->hideColumn(3);

				filterRows();

				//_ui.treeView->setRowHidden(0,QModelIndex(),true);
				connect(model, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
					this, SLOT(modifiedFile()));

				Modules::mainWin().setWindowTitle("Qt Georges Editor - " + loadedForm);
				//Modules::mainWin().getTabBar();			
			}		
		}
	}

	void CGeorgesTreeViewDialog::addParentForm(CForm *form) 
	{
		((CForm*)_form)->insertParent(((CForm*)_form)->getParentCount(), form->getFilename().c_str(), form);
	}

	void CGeorgesTreeViewDialog::modifiedFile( ) 
	{
		if (!_modified) 
		{
			_modified = true;
			setWindowTitle(windowTitle()+"*");
			Modules::mainWin().setWindowTitle(Modules::mainWin().windowTitle()+"*");
			Q_EMIT modified(_modified);
		}
	}

	void CGeorgesTreeViewDialog::write( ) 
	{

		COFile file;
		std::string s = CPath::lookup(loadedForm.toUtf8().constData(), false);
		if (file.open (s)) 
		{
			try	
			{
				if (loadedForm.contains(".typ")) 
				{
					//nlassert (Type != NULL);

					//// Write the file
					//// Modified ?
					//if (IsModified ())
					//{
					//	Type->Header.MinorVersion++;
					//	flushValueChange ();
					//}
					//Type->write (xmlStream.getDocument (), theApp.Georges4CVS);
					//modify (NULL, NULL, false);
					//flushValueChange ();
					//UpdateAllViews (NULL);
					//return TRUE;
				}
				else if (loadedForm.contains(".dfn"))	
				{
					//nlassert (Dfn != NULL);

					//// Write the file
					//if (IsModified ())
					//{
					//	Dfn->Header.MinorVersion++;
					//	flushValueChange ();
					//}
					//Dfn->write (xmlStream.getDocument (), lpszPathName, theApp.Georges4CVS);
					//modify (NULL, NULL, false);
					//UpdateAllViews (NULL);
					//return TRUE;
				}
				else 
				{
					nlassert (_form != NULL);

					// Write the file
					/*if (IsModified ())
					{
					((CForm*)(UForm*)Form)->Header.MinorVersion++;
					}*/
					//((CForm*)(UForm*)Form)->write (xmlStream.getDocument (), lpszPathName, theApp.Georges4CVS);
					_form->write(file);
					setWindowTitle(windowTitle().remove("*"));
					_modified = false;
					//if (strcmp (xmlStream.getErrorString (), "") != 0)
					//{
					//	char message[512];
					//	smprintf (message, 512, "Error while saving file: %s", xmlStream.getErrorString ());
					//theApp.outputError (message);
					//}
					//modify (NULL, NULL, false);
					//flushValueChange ();
					//UpdateAllViews (NULL);

					// Get the left view
					//CView* pView = getLeftView ();
				}
			}
			catch (Exception &e)
			{
				nlerror("Error while loading file: %s", e.what());
			}
		}
		else
		{ //if (!file.open())
			nlerror("Can't open the file %s for writing.", s.c_str());
		}
	}

	void CGeorgesTreeViewDialog::doubleClicked ( const QModelIndex & index ) 
	{
		// TODO: this is messy :( perhaps this can be done better
		CGeorgesFormProxyModel * mp = 
			dynamic_cast<CGeorgesFormProxyModel *>(_ui.treeView->model());
		CGeorgesFormModel *m = 
			dynamic_cast<CGeorgesFormModel *>(mp->sourceModel());
		QModelIndex in = mp->mapToSource(index);

		// col containing additional stuff like icons
		if (index.column() == 2) 
		{
			QModelIndex in2 = m->index(in.row(),in.column()-1,in.parent());
			CFormItem *item = m->getItem(in2);
			QString value = item->data(1).toString();

			QString path = CPath::lookup(value.toUtf8().constData(),false).c_str();

			if(value.contains(".tga") || value.contains(".png")) 
			{
				QString file = QFileDialog::getOpenFileName(
					this,
					"Select a new image",
					path,
					"Images (*.png *.tga)"
					);
				if (file.isNull())
					return;
				QFileInfo info = QFileInfo(file);

				// TODO?
				// right way would be another delegate but im too lazy :)
				// so for now i just call it directly
				m->setData(in2, info.fileName());
				return;
			}
			else 
			{
				if (path.contains(".shape") || path.contains(".ps"))
				{
					if (Modules::objViewInt()) 
					{
						Modules::objViewInt()->resetScene();
						//Modules::config().configRemapExtensions();
						Modules::objViewInt()->loadMesh(path.toUtf8().constData(),"");
					}
					return;
				}
			} 

			// open eg parent files
			if (!path.isEmpty())
				Q_EMIT changeFile(path);

		}
	}

	void CGeorgesTreeViewDialog::closeEvent(QCloseEvent *event) 
	{
		if (Modules::mainWin().getEmptyView() == this)
		{
			event->ignore();
		}
		else
		{
			if(Modules::mainWin().getTreeViewList().size() == 1)
			{
				Modules::mainWin().createEmptyView(
					Modules::mainWin().getTreeViewList().takeFirst());
			}
			Modules::mainWin().getTreeViewList().removeOne(this);
			deleteLater();
		}
	}

	void CGeorgesTreeViewDialog::filterRows()
	{
		nlinfo("CGeorgesTreeViewDialog::filterRows");
		CGeorgesFormProxyModel * mp = dynamic_cast<CGeorgesFormProxyModel *>(_ui.treeView->model());
		CGeorgesFormModel *m = dynamic_cast<CGeorgesFormModel *>(mp->sourceModel());
		if (m) {
			m->setShowParents(_ui.checkBoxParent->isChecked());
			m->setShowDefaults(_ui.checkBoxDefaults->isChecked());
		}

		//CGeorgesFormProxyModel * mp = dynamic_cast<CGeorgesFormProxyModel *>(_ui.treeView->model());
		//CGeorgesFormModel *m = dynamic_cast<CGeorgesFormModel *>(mp->sourceModel());

		//for (int i = 0; i < m->rowCount(); i++) 
		//{
		//	const QModelIndex in = m->index(i,0);
		//	if (m->getItem(in)->nodeFrom() == UFormElm::NodeParentForm) 
		//	{
		//		if (newState == Qt::Checked) 
		//		{
		//			_ui.treeView->setRowHidden(in.row(),in.parent(),false);
		//		}
		//		else
		//		{
		//			_ui.treeView->setRowHidden(in.row(),in.parent(),true);
		//		}
		//	} 
		//	else 
		//	{ // search childs // recursive?
		//		for (int j = 0; j < m->rowCount(in); j++) 
		//		{
		//			const QModelIndex in2 = m->index(j,0,in);
		//			if (m->getItem(in2)->nodeFrom() == UFormElm::NodeParentForm) 
		//			{
		//				if (newState == Qt::Checked) 
		//				{
		//					_ui.treeView->setRowHidden(in2.row(),in,false);
		//				}
		//				else 
		//				{
		//					_ui.treeView->setRowHidden(in2.row(),in,true);
		//				}
		//			}
		//		}
		//	} // end of search childs
		//}
	}

} /* namespace NLQT */
