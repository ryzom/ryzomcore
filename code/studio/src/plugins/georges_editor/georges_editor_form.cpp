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
#include "stdpch.h"
#include "georges_editor_form.h"
#include "georges_editor_constants.h"
#include "georges_dirtree_dialog.h"
#include "georges_treeview_dialog.h"
#include "georges_dfn_dialog.h"

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

namespace GeorgesQt
{
	QUndoStack *GeorgesEditorForm::UndoStack = NULL;

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

		UndoStack = new QUndoStack(this);

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

        // Set the default sheet dir dir to the level design path.
        m_lastSheetDir = ".";
        QSettings *settings = Core::ICore::instance()->settings();
        settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
        m_lastSheetDir = settings->value(Core::Constants::LEVELDESIGN_PATH, "l:/leveldesign").toString();
        settings->endGroup();

		connect(Core::ICore::instance(), SIGNAL(changeSettings()),
			this, SLOT(settingsChanged()));
		connect(m_georgesDirTreeDialog, SIGNAL(fileSelected(const QString&)), 
			this, SLOT(loadFile(const QString&)));
		connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)),
			this, SLOT(focusChanged(QWidget*, QWidget*)));
	}

	GeorgesEditorForm::~GeorgesEditorForm()
	{
		writeSettings();
	}

	QUndoStack *GeorgesEditorForm::undoStack() const
	{
		return UndoStack;
	}

	void GeorgesEditorForm::open()
	{
		QString fileName = QFileDialog::getOpenFileName(this, tr("Open Form"));
		if(!fileName.isNull())
			loadFile(fileName);
	}

    void GeorgesEditorForm::newFile()
	{
        // Assume it is a form, for now. We'll have to retrieve the DFN we'll be using as a base.
        QString fileName = QFileDialog::getOpenFileName(this, tr("Select Base Form Definition"), m_lastSheetDir, "Form Definition (*.dfn)");
        if(!fileName.isNull())
        {
            // Use the file loader to create the new form.
            loadFile(fileName, true);

            // Save the folder we just opened for future dialogs.
            QFileInfo pathInfo( fileName );
            m_lastSheetDir = pathInfo.absolutePath();
        }
	}

	void GeorgesEditorForm::save()
	{
        m_lastActiveDock->write();


        m_saveAction->setEnabled(false);
		QAction *saveAction = Core::ICore::instance()->menuManager()->action( Core::Constants::SAVE );
		if( saveAction != NULL )
			saveAction->setEnabled(false);
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

    void GeorgesEditorForm::loadFile(const QString &fileName)
    {
        loadFile(fileName, false);
    }

    void GeorgesEditorForm::loadFile(const QString &fileName, bool loadFromDfn)
	{
		QFileInfo info(fileName);

        // Check to see if the form is already loaded, if it is just raise it.
        if (m_dockedWidgets.size())
		{
			Q_FOREACH(GeorgesDockWidget *wgt, m_dockedWidgets)
			{
				if (info.fileName() == wgt->fileName())
				{
					wgt->raise();
					return;
				}
			}
        }

		GeorgesDockWidget *w = NULL;

		if( info.suffix() == "dfn" )
		{
			w = loadDfnDialog( fileName );
		}
		else
		if( info.suffix() == "typ" )
		{
			w = loadTypDialog( fileName );
		}
		else
		{
			w = loadFormDialog( fileName, loadFromDfn );
		}

		if( w == NULL )
		{
			QMessageBox::information( this,
										tr( "Failed to load file..." ),
										tr( "Failed to load file '%1'" ).arg( info.fileName() ) );
			return;
		}

        w->setUndoStack(UndoStack);
        m_lastActiveDock = w;
        m_dockedWidgets.append(w);

        connect(m_dockedWidgets.last(), SIGNAL(closing()), this, SLOT(closingTreeView()));
        connect(m_dockedWidgets.last(), SIGNAL(visibilityChanged(bool)), m_dockedWidgets.last(), SLOT(checkVisibility(bool)));

        // If there is more than one form open - tabify the new form. If this is the first form open add it to the dock.
        if(m_dockedWidgets.size() > 1)
        {
			m_mainDock->tabifyDockWidget(m_dockedWidgets.at(m_dockedWidgets.size() - 2), m_dockedWidgets.last());
		}
        else
        {
            m_mainDock->addDockWidget(Qt::RightDockWidgetArea, m_dockedWidgets.last());
        }

		w->raise();

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
		bool saveEnabled = false;
		if (m_lastActiveDock)
			saveEnabled = m_lastActiveDock->isModified();
		else
			saveEnabled = false;
		
		m_saveAction->setEnabled( saveEnabled );

		QAction *saveAction = Core::ICore::instance()->menuManager()->action( Core::Constants::SAVE );
		saveAction->setEnabled( saveEnabled );
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

	GeorgesDockWidget* GeorgesEditorForm::loadTypDialog( const QString &fileName )
	{
		return NULL;
	}

	GeorgesDockWidget* GeorgesEditorForm::loadDfnDialog( const QString &fileName )
	{
		GeorgesDFNDialog *d = new GeorgesDFNDialog();
		bool b = d->load( fileName );
		if( !b )
		{
			delete d;
			return NULL;
		}

		connect( d, SIGNAL( modified() ), this, SLOT( setModified() ) );

		return d;
	}

	GeorgesDockWidget* GeorgesEditorForm::loadFormDialog( const QString &fileName, bool loadFromDFN )
	{
		QFileInfo info( fileName );

		CGeorgesTreeViewDialog *d = new CGeorgesTreeViewDialog();

		// Retrieve the form and load the form.
        NLGEORGES::CForm *form;
        if(loadFromDFN)
        {
            // Get the form by DFN name.
            form = d->getFormByDfnName(info.fileName());
        }
        else
        {
            form = d->getFormByName(info.fileName());
        }

		if (form)
		{
			d->setForm(form);
			d->loadFormIntoDialog(form);
			QApplication::processEvents();
			connect(d, SIGNAL(modified()), 
				this, SLOT(setModified()));
			connect(d, SIGNAL(changeFile(QString)), 
				m_georgesDirTreeDialog, SLOT(changeFile(QString)));
		}
		else
		{
			delete d;
			d = NULL;
		}

		return d;
	}

} /* namespace GeorgesQt */
