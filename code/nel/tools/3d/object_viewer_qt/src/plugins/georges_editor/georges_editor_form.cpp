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

// Project includes
#include "georges_editor_form.h"
#include "georges_editor_constants.h"
#include "georges_dirtree_dialog.h"
#include "georges_treeview_dialog.h"

#include "../core/icore.h"
#include "../core/menu_manager.h"
#include "../core/core_constants.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QSettings>
#include <QFileDialog>
#include <QToolBar>
#include <QDebug>

namespace Plugin
{

	GeorgesEditorForm::GeorgesEditorForm(QWidget *parent)
		: QMainWindow(parent),
		m_georgesDirTreeDialog(0),
		m_mainDock(0),
		m_lastActiveDock(0)
	{
		m_ui.setupUi(this);

		// background for the mainwindow
		QString css = "QWidget#centralwidget {";
		        css += "image: url(:/images/ic_nel_georges_editor.png);";
		        css += "}";

		// add new mainwindow for sheet dockwidgets
		QWidget *widget = new QWidget(this);
		widget->setObjectName("centralwidget");
		widget->setStyleSheet(css);
		setCentralWidget(widget);
		QGridLayout *layout = new QGridLayout(widget);
		layout->setContentsMargins(0,0,0,0);
		widget->setLayout(layout);
		m_mainDock = new QMainWindow(this);
		m_mainDock->setDockNestingEnabled(true);
		layout->addWidget(m_mainDock);

		m_undoStack = new QUndoStack(this);

		Core::MenuManager *menuManager = Core::ICore::instance()->menuManager();
		m_openAction = menuManager->action(Core::Constants::OPEN);

		m_newAction = new QAction(tr("&New..."), this);
		m_newAction->setIcon(QIcon(Core::Constants::ICON_NEW));
		m_newAction->setShortcut(QKeySequence::New);
		m_newAction->setStatusTip(tr("Create a new file"));
		connect(m_newAction, SIGNAL(triggered()), this, SLOT(newFile()));

		m_saveAction = new QAction(tr("&Save..."), this);
		m_saveAction->setIcon(QIcon(Core::Constants::ICON_SAVE));
		m_saveAction->setShortcut(QKeySequence::Save);
		m_saveAction->setStatusTip(tr("Save the current file"));
		connect(m_saveAction, SIGNAL(triggered()), this, SLOT(save()));

		m_fileToolBar = addToolBar(tr("&File"));
		m_fileToolBar->addAction(m_openAction);
		m_fileToolBar->addAction(m_newAction);
		m_fileToolBar->addAction(m_saveAction);

		m_saveAction->setEnabled(false);

		readSettings();

		// create leveldesign directory tree dockwidget
		m_georgesDirTreeDialog = new CGeorgesDirTreeDialog(m_leveldesignPath, this);
		addDockWidget(Qt::LeftDockWidgetArea, m_georgesDirTreeDialog);
		restoreDockWidget(m_georgesDirTreeDialog);

		connect(Core::ICore::instance(), SIGNAL(changeSettings()),
			this, SLOT(settingsChanged()));
		connect(m_georgesDirTreeDialog, SIGNAL(selectedForm(const QString)), 
			this, SLOT(loadFile(const QString)));
		connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)),
			this, SLOT(focusChanged(QWidget*, QWidget*)));
	}

	GeorgesEditorForm::~GeorgesEditorForm()
	{
		writeSettings();
	}

	QUndoStack *GeorgesEditorForm::undoStack() const
	{
		return m_undoStack;
	}

	void GeorgesEditorForm::open()
	{
		/*qDebug() << "GeorgesEditorForm::open()";
		if (!m_dockedWidgets.size())
		{
			m_dockedWidgets.append(new CGeorgesTreeViewDialog(m_mainDock));
			m_mainDock->addDockWidget(Qt::RightDockWidgetArea, m_dockedWidgets.last());
		}
		else
		{
			m_dockedWidgets.append(new CGeorgesTreeViewDialog(m_mainDock));
			Q_ASSERT(m_dockedWidgets.size() > 1);
			m_mainDock->tabifyDockWidget(m_dockedWidgets.at(m_dockedWidgets.size() - 2), m_dockedWidgets.last());
		}*/
		
		// TODO: FileDialog & loadFile();
		//m_mainDock->addDockWidget(Qt::TopDockWidgetArea, new CGeorgesTreeViewDialog(m_mainDock, true));
		//m_mainDock->addDockWidget(Qt::LeftDockWidgetArea, new CGeorgesTreeViewDialog(m_mainDock, true));
		//QString fileName = QFileDialog::getOpenFileName();
		//loadFile(fileName);
	}

	void GeorgesEditorForm::newFile()
	{

	}

	void GeorgesEditorForm::save()
	{

	}

	void GeorgesEditorForm::readSettings()
	{
		QSettings *settings = Core::ICore::instance()->settings();
		settings->beginGroup(Constants::GEORGES_EDITOR_SECTION);

		restoreGeometry(settings->value("geometry").toByteArray());
		restoreState(settings->value("windowState").toByteArray());

		settings->endGroup();

		settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
		m_leveldesignPath = settings->value(Core::Constants::LEVELDESIGN_PATH, "l:/leveldesign").toString();
		settings->endGroup();
	}

	void GeorgesEditorForm::writeSettings()
	{
		QSettings *settings = Core::ICore::instance()->settings();
		settings->beginGroup(Constants::GEORGES_EDITOR_SECTION);

		settings->setValue("geometry", saveGeometry());
		settings->setValue("windowState", saveState());

		settings->endGroup();
		settings->sync();
	}

	void GeorgesEditorForm::settingsChanged()
	{
		QSettings *settings = Core::ICore::instance()->settings();

		settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
		QString oldLDPath = m_leveldesignPath;
		m_leveldesignPath = settings->value(Core::Constants::LEVELDESIGN_PATH, "l:/leveldesign").toString();
		settings->endGroup();

		if (oldLDPath != m_leveldesignPath)
		{
			m_georgesDirTreeDialog->ldPathChanged(m_leveldesignPath);
		}
	}

	void GeorgesEditorForm::loadFile(const QString fileName)
	{
		QFileInfo info(fileName);

		if (!m_dockedWidgets.size())
		{
			CGeorgesTreeViewDialog *dock = new CGeorgesTreeViewDialog(m_mainDock);
			m_lastActiveDock = dock;
			m_dockedWidgets.append(dock);

			m_mainDock->addDockWidget(Qt::RightDockWidgetArea, m_dockedWidgets.last());
			connect(m_dockedWidgets.last(), SIGNAL(closing()),
				this, SLOT(closingTreeView()));
			connect(m_dockedWidgets.last(), SIGNAL(visibilityChanged(bool)),
				m_dockedWidgets.last(), SLOT(checkVisibility(bool)));
		}
		else
		{

			Q_FOREACH(CGeorgesTreeViewDialog *wgt, m_dockedWidgets)
			{
				if (info.fileName() == wgt->loadedForm)
				{
					wgt->raise();
					return;
				}
			}
			CGeorgesTreeViewDialog *dock = new CGeorgesTreeViewDialog(m_mainDock);
			m_dockedWidgets.append(dock);

			connect(m_dockedWidgets.last(), SIGNAL(closing()),
				this, SLOT(closingTreeView()));
			connect(m_dockedWidgets.last(), SIGNAL(visibilityChanged(bool)),
				m_dockedWidgets.last(), SLOT(checkVisibility(bool)));
			Q_ASSERT(m_dockedWidgets.size() > 1);
			m_mainDock->tabifyDockWidget(m_dockedWidgets.at(m_dockedWidgets.size() - 2), m_dockedWidgets.last());
		}
		CForm *form = m_dockedWidgets.last()->getFormByName(info.fileName());
		if (form)
		{
			m_dockedWidgets.last()->setForm(form);
			m_dockedWidgets.last()->loadFormIntoDialog(form);
			QApplication::processEvents();
			connect(m_dockedWidgets.last(), SIGNAL(modified()), 
				this, SLOT(setModified()));
			m_dockedWidgets.last()->raise();
			connect(m_dockedWidgets.last(), SIGNAL(changeFile(QString)), 
				m_georgesDirTreeDialog, SLOT(changeFile(QString)));
		}
		else
		{
			m_dockedWidgets.last()->close();
		}
	}

	void GeorgesEditorForm::closingTreeView()
	{
		//qDebug() << "closingTreeView";
		m_dockedWidgets.removeAll(qobject_cast<CGeorgesTreeViewDialog*>(sender()));
		if (qobject_cast<CGeorgesTreeViewDialog*>(sender()) == m_lastActiveDock)
			m_lastActiveDock = 0;
	}

	void GeorgesEditorForm::setModified () 
	{
		qDebug() << "setModified";
		if (m_lastActiveDock)
			m_saveAction->setEnabled(m_lastActiveDock->isModified());
		else
			m_saveAction->setEnabled(false);
	}

	void GeorgesEditorForm::focusChanged ( QWidget * old, QWidget * now ) 
	{
		if (now) 
		{
			// ugly, UGLY hack for compensating QDockWidgets failure in focus API
			if (now->objectName() == "treeView" ||
				now->objectName() == "checkBoxDefaults" ||
				now->objectName() == "checkBoxParent" ||
				now->objectName() == "commentEdit") 
			{
				QWidget *dlg = 0;
				QApplication::focusWidget()?
					QApplication::focusWidget()->parentWidget()?
					QApplication::focusWidget()->parentWidget()->parentWidget()?
					QApplication::focusWidget()->parentWidget()->parentWidget()->parentWidget()?
					QApplication::focusWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget()?
					QApplication::focusWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget()?
					dlg=QApplication::focusWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget():dlg=0:dlg=0:dlg=0:dlg=0:dlg=0:dlg=0;
				CGeorgesTreeViewDialog *active = qobject_cast<CGeorgesTreeViewDialog*>(dlg);
				if(active)
				{
					//qDebug() << "focusChanged" << active->loadedForm;
					m_lastActiveDock = active;
					m_saveAction->setEnabled(active->isModified());
				}
			}
		}
	}
} /* namespace Plugin */
