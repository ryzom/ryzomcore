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
#include "../core/imenu_manager.h"
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
		m_emptyDock(0),
		m_mainDock(0)
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

		Core::IMenuManager *menuManager = Core::ICore::instance()->menuManager();
		_openAction = menuManager->action(Core::Constants::OPEN);

		/*_openAction = new QAction(tr("&Open..."), this);
		_openAction->setIcon(QIcon(Core::Constants::ICON_OPEN));
		_openAction->setShortcut(QKeySequence::Open);
		_openAction->setStatusTip(tr("Open an existing file"));
		connect(_openAction, SIGNAL(triggered()), this, SLOT(open()));*/

		_newAction = new QAction(tr("&New..."), this);
		_newAction->setIcon(QIcon(Core::Constants::ICON_NEW));
		_newAction->setShortcut(QKeySequence::New);
		_newAction->setStatusTip(tr("Create a new file"));
		connect(_newAction, SIGNAL(triggered()), this, SLOT(newFile()));

		_saveAction = new QAction(tr("&Save..."), this);
		_saveAction->setIcon(QIcon(Core::Constants::ICON_SAVE));
		_saveAction->setShortcut(QKeySequence::Save);
		_saveAction->setStatusTip(tr("Save the current file"));
		connect(_saveAction, SIGNAL(triggered()), this, SLOT(save()));

		_fileToolBar = addToolBar(tr("&File"));
		_fileToolBar->addAction(_openAction);
		_fileToolBar->addAction(_newAction);
		_fileToolBar->addAction(_saveAction);

		//_openAction->setEnabled(false);
		//_newAction->setEnabled(false);
		//_saveAction->setEnabled(false);

		readSettings();

		// create leveldesign directory tree dockwidget
		m_georgesDirTreeDialog = new CGeorgesDirTreeDialog(m_leveldesignPath, this);
		addDockWidget(Qt::LeftDockWidgetArea, m_georgesDirTreeDialog);
		restoreDockWidget(m_georgesDirTreeDialog);

		connect(Core::ICore::instance(), SIGNAL(changeSettings()),
			this, SLOT(settingsChanged()));
		connect(m_georgesDirTreeDialog, SIGNAL(selectedForm(const QString)), 
			this, SLOT(loadFile(const QString)));
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
			m_dockedWidgets.append(new CGeorgesTreeViewDialog(m_mainDock));
			m_mainDock->addDockWidget(Qt::RightDockWidgetArea, m_dockedWidgets.last());
		}
		else
		{
			m_dockedWidgets.append(new CGeorgesTreeViewDialog(m_mainDock));
			Q_ASSERT(m_dockedWidgets.size() > 1);
			m_mainDock->tabifyDockWidget(m_dockedWidgets.at(m_dockedWidgets.size() - 2), m_dockedWidgets.last());
		}
		CForm *form = m_dockedWidgets.last()->getFormByName(info.fileName());
		if (form)
		{
		//	delete newView;
		//	newView = createTreeView(fileName);
		m_dockedWidgets.last()->setForm(form);
		m_dockedWidgets.last()->loadFormIntoDialog(form);
		//setCurrentFile(info.fileName());
		//	return;
		}
	}

} /* namespace Plugin */
