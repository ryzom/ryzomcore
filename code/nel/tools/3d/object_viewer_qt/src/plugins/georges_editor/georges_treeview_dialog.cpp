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

#include "stdpch.h"
#include "georges_treeview_dialog.h"
#include "georges.h"
#include "georgesform_model.h"
#include "georgesform_proxy_model.h"
#include "formitem.h"
#include "formdelegate.h"
#include "expandable_headerview.h"

// Qt includes
#include <QtGui/QWidget>
#include <QSettings>
#include <QFileDialog>
#include <QDebug>
#include <QMenu>

// NeL includes
#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/o_xml.h>
#include <nel/georges/u_form_loader.h>
#include <nel/georges/form.h>
#include <nel/georges/u_form.h>
#include <nel/georges/u_type.h>

// OVQT Includes
#include "../core/icore.h"
#include "../core/core_constants.h"

using namespace NLMISC;
using namespace NLGEORGES;

namespace GeorgesQt 
{

	CGeorgesTreeViewDialog::CGeorgesTreeViewDialog(QWidget *parent /*= 0*/)
		: QDockWidget(parent),
		m_header(0),
		m_modified(false)
	{
		m_georges = new CGeorges;

		loadedForm = "";
		// Set the default sheet dir dir to the level design path.
		m_lastSheetDir = ".";
		QSettings *settings = Core::ICore::instance()->settings();
        settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
		m_lastSheetDir = settings->value(Core::Constants::LEVELDESIGN_PATH, "l:/leveldesign").toString();
		settings->endGroup();

		m_ui.setupUi(this);
		m_header = new ExpandableHeaderView(Qt::Horizontal, m_ui.treeView);
		m_ui.treeView->setHeader(m_header);
		m_ui.treeView->header()->setResizeMode(QHeaderView::ResizeToContents);
		m_ui.treeView->header()->setStretchLastSection(true);
		m_ui.treeViewTabWidget->setTabEnabled (2,false);

		m_ui.checkBoxParent->setStyleSheet("background-color: rgba(0,255,0,30)");
		m_ui.checkBoxDefaults->setStyleSheet("background-color: rgba(255,0,0,30)");
		m_form = 0;

		FormDelegate *formdelegate = new FormDelegate(this);
		m_ui.treeView->setItemDelegateForColumn(1, formdelegate);

		// Set up custom context menu.
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

		connect(m_ui.treeView, SIGNAL(doubleClicked (QModelIndex)),
			this, SLOT(doubleClicked (QModelIndex)));
		connect(m_ui.checkBoxParent, SIGNAL(toggled(bool)),
			this, SLOT(filterRows()));
		connect(m_ui.checkBoxDefaults, SIGNAL(toggled(bool)),
			this, SLOT(filterRows()));
		connect(m_header, SIGNAL(headerClicked(int)),
			this, SLOT(headerClicked(int)));
	}

	CGeorgesTreeViewDialog::~CGeorgesTreeViewDialog()
	{
		delete m_form;
		qDebug() << "DTOR";
	}

	void CGeorgesTreeViewDialog::headerClicked(int section)
	{
		if (section == 0)
        {
			if (*(m_header->expanded()))
				m_ui.treeView->expandAll();
			else
				m_ui.treeView->collapseAll();
        }
	}

	void CGeorgesTreeViewDialog::setForm(const CForm *form) 
	{
		m_form = (UForm*)form;
	}

    NLGEORGES::CForm* CGeorgesTreeViewDialog::getFormByName(const QString formName)
	{
		if(NLMISC::CPath::exists(formName.toUtf8().constData()))
		{
            //NLGEORGES::CForm *form = dynamic_cast<NLGEORGES::CForm*>(m_georges->loadForm(formName.toUtf8()));
            return (NLGEORGES::CForm *)m_georges->loadForm(formName.toUtf8().constData());
		}
		//else
		//{
		//	CForm *form = 0;
		//	// Load the DFN
		//	std::string extStr = NLMISC::CFile::getExtension( formName.toUtf8() );
		//	QString dfnName = QString("%1.dfn").arg(extStr.c_str());
		//	UFormDfn *formdfn;
		//	if (NLMISC::CPath::exists(dfnName.toUtf8()))
		//	{
		//		formdfn = _georges->loadFormDfn (dfnName.toUtf8());
		//		if (!formdfn)
		//		{
		//			nlwarning("Failed to load dfn: %s", dfnName.toUtf8());
		//			return 0;
		//		}
		//	}
		//	else
		//	{
		//		nlwarning("Cannot find dfn: %s", dfnName.toUtf8());
		//		return 0;
		//	}

		//	form = new CForm;

		//	// Build the root element
		//	((CFormElmStruct*)&form->getRootNode())->build((CFormDfn*)formdfn);

		//	uint i;
		//	for (i=0; i<CForm::HeldElementCount; i++)
		//	{
		//		((CFormElmStruct*)(((CForm*)form)->HeldElements[i]))->build ((CFormDfn*)formdfn);
		//	}
		//	return form;
		//}
        nlinfo("File '%s' does not exist!", formName.toUtf8().constData());
		return 0;
	}

    NLGEORGES::CForm* CGeorgesTreeViewDialog::getFormByDfnName(const QString dfnName)
    {
        if(NLMISC::CPath::exists(dfnName.toUtf8().constData()))
        {
            // Create a new form object.
            NLGEORGES::CForm *form = new NLGEORGES::CForm();
            m_form = form;

            // Retrieve a copy of the root definition.
            NLGEORGES::CFormDfn *formDfn = dynamic_cast<NLGEORGES::CFormDfn *>(m_georges->loadFormDfn(dfnName.toUtf8().constData()));

            // Next we'll use the root node to build a new form.
            NLGEORGES::CFormElmStruct *fes = dynamic_cast<NLGEORGES::CFormElmStruct *>(getRootNode(0));
            fes->build(formDfn);

            // And then initialize the held elements;
            for(uint i = 0; i<NLGEORGES::CForm::HeldElementCount; i++)
            {
                fes = dynamic_cast<NLGEORGES::CFormElmStruct *>(getRootNode(i+1));
                fes->build(formDfn);
            }

            return form;
        }
        nlinfo("File '%s' does not exist!", dfnName.toUtf8().constData());
        return NULL;
    }

    NLGEORGES::CFormElm *CGeorgesTreeViewDialog::getRootNode(uint slot)
    {
        NLGEORGES::CForm *form = getFormPtr();

        if(slot == 0)
        {
            const NLGEORGES::UFormElm &formElm = form->getRootNode();
            return (NLGEORGES::CFormElm *)&formElm;
        }

        // Make sure the slot value is valid and then return the corresponding element.
        nlassert(slot < NLGEORGES::CForm::HeldElementCount+1);
        return getFormPtr()->HeldElements[slot-1];
    }

    NLGEORGES::CForm *CGeorgesTreeViewDialog::getFormPtr()
    {
        return dynamic_cast<NLGEORGES::CForm *>(m_form);
    }

	void CGeorgesTreeViewDialog::loadFormIntoDialog(CForm *form) 
	{

		if(form)
			m_form = form;
		else
			return;

		UFormElm *root = 0;
		root = &m_form->getRootNode();

		QStringList parents;
        uint cnt = form->getParentCount();
        for (uint i = 0; i < cnt /*form->getParentCount()*/; i++)
		{
			UForm *u = m_form->getParentForm(i);
			parents << u->getFilename().c_str();
		}

		QString comments;
		comments = m_form->getComment().c_str();

		if (!comments.isEmpty()) 
		{
			m_ui.treeViewTabWidget->setTabEnabled (1,true);
			m_ui.commentEdit->setPlainText(comments);
		}

		QStringList strList;
		std::set<std::string> dependencies;
		m_form->getDependencies(dependencies);

		QMap< QString, QStringList> deps;
		Q_FOREACH(std::string str, dependencies) 
		{
			QString file = str.c_str();
			if (str == m_form->getFilename()) continue;
			deps[file.remove(0,file.indexOf(".")+1)] << str.c_str();
		}
		nlinfo("typ's %d",deps["typ"].count());
		nlinfo("dfn's %d",deps["dfn"].count());

		//nlwarning(strList.join(";").toUtf8());
		if (root) 
		{
			loadedForm = m_form->getFilename().c_str();

			CGeorgesFormModel *model = new CGeorgesFormModel(root,deps,comments,parents,m_header->expanded());
			CGeorgesFormProxyModel *proxyModel = new CGeorgesFormProxyModel();
			proxyModel->setSourceModel(model);
			m_ui.treeView->setModel(proxyModel);
			m_ui.treeView->expandAll();
			// this is a debug output row
			m_ui.treeView->hideColumn(3);

			filterRows();

		//		//_ui.treeView->setRowHidden(0,QModelIndex(),true);
			connect(model, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
				this, SLOT(modifiedFile()));

			setWindowTitle(loadedForm);
		//		//Modules::mainWin().getTabBar();			
		}
	}

	void CGeorgesTreeViewDialog::addParentForm(QString parentFormNm)
	{
		// Try to load the form
		NLGEORGES::UForm *uParentForm = m_georges->loadForm(parentFormNm.toUtf8().constData());
		NLGEORGES::CForm *parentForm = dynamic_cast<NLGEORGES::CForm*>(uParentForm);
		NLGEORGES::CForm *mainForm = static_cast<NLGEORGES::CForm*>(m_form);

		CGeorgesFormProxyModel * proxyModel = dynamic_cast<CGeorgesFormProxyModel *>(m_ui.treeView->model());
		CGeorgesFormModel *model = dynamic_cast<CGeorgesFormModel *>(proxyModel->sourceModel());

		if(parentForm)
		{
			if (mainForm != parentForm)
			{
				// Check it is the same dfn
				if (parentForm->Elements.FormDfn ==  mainForm->Elements.FormDfn)
				{
					// This is the parent form selector
					if(!mainForm->insertParent(mainForm->getParentCount(),parentFormNm.toUtf8(), parentForm))
						nlwarning("Failed to add parent form: %s", parentFormNm.toUtf8().constData());
					else
					{
						nlinfo("Successfullyadded parent form: %s", parentFormNm.toUtf8().constData());
						model->addParentForm(parentFormNm);
					}
				}
			}
		}
	}

	void CGeorgesTreeViewDialog::modifiedFile( ) 
	{
		if (!m_modified) 
		{
			m_modified = true;
			setWindowTitle(windowTitle() + "*");
		}
		Q_EMIT modified();
	}

	void CGeorgesTreeViewDialog::write( ) 
	{

        NLMISC::COFile file;
        std::string s = NLMISC::CPath::lookup(loadedForm.toUtf8().constData(), false);
        if(file.open (s))
        {
            try
            {
		//		if (loadedForm.contains(".typ")) 
		//		{
		//			//nlassert (Type != NULL);

		//			//// Write the file
		//			//// Modified ?
		//			//if (IsModified ())
		//			//{
		//			//	Type->Header.MinorVersion++;
		//			//	flushValueChange ();
		//			//}
		//			//Type->write (xmlStream.getDocument (), theApp.Georges4CVS);
        //			//modify (NULL, NULL, false);
		//			//flushValueChange ();
		//			//UpdateAllViews (NULL);
		//			//return TRUE;
		//		}
		//		else if (loadedForm.contains(".dfn"))	
		//		{
		//			//nlassert (Dfn != NULL);

		//			//// Write the file
		//			//if (IsModified ())
		//			//{
		//			//	Dfn->Header.MinorVersion++;
		//			//	flushValueChange ();
		//			//}
		//			//Dfn->write (xmlStream.getDocument (), lpszPathName, theApp.Georges4CVS);
		//			//modify (NULL, NULL, false);
		//			//UpdateAllViews (NULL);
		//			//return TRUE;
		//		}
		//		else 
		//		{
                    nlassert (m_form != NULL);

                    // Write the file
		//			/*if (IsModified ())
		//			{
		//			((CForm*)(UForm*)Form)->Header.MinorVersion++;
		//			}*/
		//			//((CForm*)(UForm*)Form)->write (xmlStream.getDocument (), lpszPathName, theApp.Georges4CVS);
                    m_form->write(file, false);
                    setWindowTitle(windowTitle().remove("*"));
                    m_modified = false;
		//			//if (strcmp (xmlStream.getErrorString (), "") != 0)
		//			//{
		//			//	char message[512];
		//			//	smprintf (message, 512, "Error while saving file: %s", xmlStream.getErrorString ());
		//			//theApp.outputError (message);
		//			//}
		//			//modify (NULL, NULL, false);
		//			//flushValueChange ();
		//			//UpdateAllViews (NULL);

		//			// Get the left view
		//			//CView* pView = getLeftView ();
		//		}
            }
            catch (Exception &e)
            {
                nlerror("Error while loading file: %s", e.what());
            }
        }
        else
        {
            nlerror("Can't open the file %s for writing.", s.c_str());
        }
	}

	void CGeorgesTreeViewDialog::doubleClicked ( const QModelIndex & index ) 
	{
		// TODO: this is messy :( perhaps this can be done better
		CGeorgesFormProxyModel * proxyModel = 
			dynamic_cast<CGeorgesFormProxyModel *>(m_ui.treeView->model());
		CGeorgesFormModel *model = 
			dynamic_cast<CGeorgesFormModel *>(proxyModel->sourceModel());
		QModelIndex sourceIndex = proxyModel->mapToSource(index);

		CFormItem *item = model->getItem(sourceIndex);

		if (item->parent() && item->parent()->data(0) == "parents")
		{
			Q_EMIT changeFile(CPath::lookup(item->data(0).toString().toUtf8().constData(),false).c_str());
		}

		//// col containing additional stuff like icons
		//if (index.column() == 2) 
		//{
		//	QModelIndex in2 = m->index(in.row(),in.column()-1,in.parent());
		//	CFormItem *item = m->getItem(in2);
		//	QString value = item->data(1).toString();

		//	QString path = CPath::lookup(value.toUtf8(),false).c_str();

		//	if(value.contains(".tga") || value.contains(".png")) 
		//	{
		//		QString file = QFileDialog::getOpenFileName(
		//			this,
		//			"Select a new image",
		//			path,
		//			"Images (*.png *.tga)"
		//			);
		//		if (file.isNull())
		//			return;
		//		QFileInfo info = QFileInfo(file);

		//		// TODO?
		//		// right way would be another delegate but im too lazy :)
		//		// so for now i just call it directly
		//		m->setData(in2, info.fileName());
		//		return;
		//	}
		//	else 
		//	{
		//		if (path.contains(".shape") || path.contains(".ps"))
		//		{
		//			if (Modules::objViewInt()) 
		//			{
		//				Modules::objViewInt()->resetScene();
		//				//Modules::config().configRemapExtensions();
		//				Modules::objViewInt()->loadMesh(path.toUtf8(),"");
		//			}
		//			return;
		//		}
		//	} 

		//	// open eg parent files
		//	if (!path.isEmpty())
		//		Q_EMIT changeFile(path);

		//}
	}

	void CGeorgesTreeViewDialog::closeEvent(QCloseEvent *event) 
	{
		Q_EMIT closing();
		deleteLater();
	}

	void CGeorgesTreeViewDialog::checkVisibility(bool visible) {
		// this prevents invisible docks from getting tab focus
		qDebug() << "checkVisibility" << visible;
		setEnabled(visible);
		//if (visible)
			Q_EMIT modified();
	}

	void CGeorgesTreeViewDialog::filterRows()
	{
		CGeorgesFormProxyModel * mp = dynamic_cast<CGeorgesFormProxyModel *>(m_ui.treeView->model());
		CGeorgesFormModel *m = dynamic_cast<CGeorgesFormModel *>(mp->sourceModel());
		if (m) {
			m->setShowParents(m_ui.checkBoxParent->isChecked());
			m->setShowDefaults(m_ui.checkBoxDefaults->isChecked());
		}
	}

	void CGeorgesTreeViewDialog::showContextMenu(const QPoint &pos)
	{
		QMenu contextMenu;
		QMenu *structContext = NULL;
		QPoint globalPos = this->mapToGlobal(pos);
		
		// Fisrt we're going to see if we've right clicked on a new item and select it.
		const QModelIndex &index = this->m_ui.treeView->currentIndex();

		if(!index.isValid())
			return;

		CGeorgesFormProxyModel * mp = dynamic_cast<CGeorgesFormProxyModel *>(m_ui.treeView->model());
		CGeorgesFormModel *m = dynamic_cast<CGeorgesFormModel *>(mp->sourceModel());
		QModelIndex sourceIndex = mp->mapToSource(index);

		if (m) 
		{
			
			CFormItem *item = m->getItem(sourceIndex);

			// Right click on the "parents" item
			if (item->data(0) == "parents")
				contextMenu.addAction("Add parent...");
			// Right click on a parent item
			else if(item->parent() && item->parent()->data(0) == "parents")
			{
				contextMenu.addAction("Add parent...");
				contextMenu.addAction("Remove parent");
			}
			else if(item->getFormElm()->isArray())
				contextMenu.addAction("Add array entry...");
			else if(item->getFormElm()->isStruct())
			{
				QMenu *structContext = new QMenu("Add struct element...", this);
				contextMenu.addMenu(structContext);

				NLGEORGES::UFormDfn *defn = item->getFormElm()->getStructDfn();
				if(defn)
				{
					for(uint defnNum=0; defnNum < defn->getNumEntry(); defnNum++)
					{
						std::string entryName;
						std::string dummy;
						UFormElm::TWhereIsValue *whereV = new UFormElm::TWhereIsValue;
						bool result = defn->getEntryName(defnNum, entryName);
						bool result2 = item->getFormElm()->getValueByName(dummy, entryName.c_str(), NLGEORGES::UFormElm::Eval, whereV);

						
						if(result2 && *whereV != UFormElm::ValueForm)
						{
							structContext->addAction(entryName.c_str());
						}
						delete whereV;
					}
				}
			}
			else if(item->getFormElm()->isAtom() && item->valueFrom() == NLGEORGES::UFormElm::ValueForm)
				contextMenu.addAction("Revert to parent/default...");

			QAction *selectedItem = contextMenu.exec(globalPos);
			if(selectedItem)
			{
				if(selectedItem->text() == "Add parent...")
				{
					// Get the file extension of the form so we can build a dialog pattern.
					QString file = m_form->getFilename().c_str();
					file = file.remove(0,file.indexOf(".")+1);
					QString filePattern = "Parent Sheets (*."+file+")";
					
					nlinfo("parent defn name '%s'", file.toUtf8().constData());
					QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Select parent sheets..."), m_lastSheetDir, filePattern);
					if(!fileNames.isEmpty())
					{
						Q_FOREACH(QString fileToParent, fileNames)
						{
							// Get just the filename. Georges doesn't want the path.
							QFileInfo pathInfo( fileToParent );
							QString tmpFileName( pathInfo.fileName() );

							nlinfo("requesting to add parent form '%s'", tmpFileName.toUtf8().constData());
							
							// Call to add the form and load it into the Georges form.
							addParentForm(tmpFileName);

							// Save the file lookup path for future dialog boxes.
							m_lastSheetDir = pathInfo.absolutePath();
						}						
					}
					m_ui.treeView->expandAll();
				}
				else if(selectedItem->text() == "Remove parent")
				{
					NLGEORGES::CForm *form = static_cast<NLGEORGES::CForm *>(m_form);
					QString parentFileName = item->data(0).toString();

					for(uint num = 0; num < form->getParentCount(); num++)
					{
						QString curParentName = form->getParent(num)->getFilename().c_str();
						if(parentFileName == curParentName)
						{
							form->removeParent(num);
							m->removeParentForm(parentFileName);
							break;
						}
					}

					m_ui.treeView->expandAll();
				}

			} // if selected context menu item is valid.
		} // if 'm' model valid.

		if(structContext)
			delete structContext;
	}

} /* namespace GeorgesQt */
