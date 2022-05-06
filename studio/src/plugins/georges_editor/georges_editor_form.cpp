// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian JAEKEL <aj@elane2k.com>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "georges_typ_dialog.h"

#include "../core/icore.h"
#include "../core/menu_manager.h"
#include "../core/core_constants.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/path.h>

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

		m_newTypAction = new QAction(tr("New Type"), this );
		m_newTypAction->setIcon(QIcon(Core::Constants::ICON_NEW));
		m_newTypAction->setStatusTip(tr("Create a new type file"));
		connect( m_newTypAction, SIGNAL(triggered()), this, SLOT(newTyp()));

		m_newDfnAction = new QAction(tr("New DFN"), this );
		m_newDfnAction->setIcon(QIcon(Core::Constants::ICON_NEW));
		m_newDfnAction->setStatusTip(tr("Create a new definition file"));
		connect( m_newDfnAction, SIGNAL(triggered()), this, SLOT(newDfn()));

		m_newFormAction = new QAction(tr("New Form"), this );
		m_newFormAction->setIcon(QIcon(Core::Constants::ICON_NEW));
		m_newFormAction->setStatusTip(tr("Create a new form file"));
		connect( m_newFormAction, SIGNAL(triggered()), this, SLOT(newForm()));

		m_saveAction = new QAction(tr("&Save..."), this);
		m_saveAction->setIcon(QIcon(Core::Constants::ICON_SAVE));
		m_saveAction->setShortcut(QKeySequence::Save);
		m_saveAction->setStatusTip(tr("Save the current file"));
		connect(m_saveAction, SIGNAL(triggered()), this, SLOT(save()));

		m_fileToolBar = addToolBar(tr("&File"));
		m_fileToolBar->addAction(m_newTypAction);
		m_fileToolBar->addAction(m_newDfnAction);
		m_fileToolBar->addAction(m_newFormAction);
		m_fileToolBar->addAction(m_openAction);
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
        m_lastSheetDir = settings->value(Core::Constants::LEVELDESIGN_PATH, "R:/leveldesign").toString();
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

	void GeorgesEditorForm::newTyp()
	{
		QString fileName = QFileDialog::getSaveFileName( this, 
															tr( "New Type" ),
															"",
															"Type files (*.typ)" );
		if( fileName.isEmpty() )
			return;

		
		GeorgesTypDialog *d = new GeorgesTypDialog();
		d->newDocument( fileName );
		addGeorgesWidget( d );
		setModified();
	}

	void GeorgesEditorForm::newDfn()
	{
		QString fileName = QFileDialog::getSaveFileName( this, 
															tr( "New Definition" ),
															"",
															"Definition files (*.dfn)" );
		if( fileName.isEmpty() )
			return;

		GeorgesDFNDialog *d = new GeorgesDFNDialog();
		d->newDocument( fileName );
		addGeorgesWidget( d );
		setModified();
	}

	void GeorgesEditorForm::newForm()
	{
		QString dfnFileName = QFileDialog::getOpenFileName( this, 
															tr( "New Form" ),
															"",
															"Definition files (*.dfn)" );
		if( dfnFileName.isEmpty() )
			return;

		QFileInfo dfnInfo( dfnFileName );
		QString baseName = dfnInfo.baseName();
		QString filter;
		filter += baseName;
		filter += " files (*.";
		filter += baseName;
		filter += ")";

		QString fileName = QFileDialog::getSaveFileName( this, 
															tr( "New Form" ),
															"",
															filter );
		if( fileName.isEmpty() )
			return;

		CGeorgesTreeViewDialog *d = new CGeorgesTreeViewDialog();
		if( !d->newDocument( fileName, dfnFileName ) )
		{
			QMessageBox::information( this,
										tr( "Failed to create new form" ),
										tr( "Failed to create new form!" ) );
			return;
		}

		addGeorgesWidget( d );
		setModified();
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
		m_leveldesignPath = settings->value(Core::Constants::LEVELDESIGN_PATH, "R:/leveldesign").toString();
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

	void GeorgesEditorForm::addGeorgesWidget( GeorgesDockWidget *w )
	{
		w->setUndoStack(UndoStack);
        m_lastActiveDock = w;
        m_dockedWidgets.append(w);

		connect( w, SIGNAL( closing( GeorgesDockWidget* ) ), this, SLOT( dialogClosing( GeorgesDockWidget* ) ) );
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

		QCoreApplication::processEvents();
		w->raise();
	}

	void GeorgesEditorForm::settingsChanged()
	{
		QSettings *settings = Core::ICore::instance()->settings();

		settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
		QString oldLDPath = m_leveldesignPath;
		m_leveldesignPath = settings->value(Core::Constants::LEVELDESIGN_PATH, "R:/leveldesign").toString();
		settings->endGroup();

		if (oldLDPath != m_leveldesignPath)
		{
			m_georgesDirTreeDialog->ldPathChanged(m_leveldesignPath);
		}
	}

    void GeorgesEditorForm::loadFile(const QString &fileName)
	{
		std::string path = NLMISC::CPath::lookup( fileName.toUtf8().constData(), false );
		if( path.empty() )
		{
			QMessageBox::information( this,
									tr( "Failed to load file..." ),
									tr( "Failed to load file '%1': File doesn't exist!" ).arg( fileName ) );
			return;
		}

		QFileInfo info( path.c_str() );

        // Check to see if the form is already loaded, if it is just raise it.
        if (m_dockedWidgets.size())
		{
			Q_FOREACH(GeorgesDockWidget *wgt, m_dockedWidgets)
			{
				if ( QString( path.c_str() ) == wgt->fileName())
				{
					wgt->raise();
					return;
				}
			}
        }

		GeorgesDockWidget *w = NULL;

		if( info.suffix() == "dfn" )
		{
			w = loadDfnDialog( path.c_str() );
		}
		else
		if( info.suffix() == "typ" )
		{
			w = loadTypDialog( path.c_str() );
		}
		else
		{
			w = loadFormDialog( path.c_str() );
		}

		if( w == NULL )
		{
			QMessageBox::information( this,
										tr( "Failed to load file..." ),
										tr( "Failed to load file '%1': Not a typ, dfn, or form file!" ).arg( info.fileName() ) );
			return;
		}

		addGeorgesWidget( w );
	}

	void GeorgesEditorForm::dialogClosing( GeorgesDockWidget *d )
	{
		m_dockedWidgets.removeAll( d );
		
		if( m_dockedWidgets.size() == 0 )
			m_lastActiveDock = NULL;
		else
			m_lastActiveDock = m_dockedWidgets.last();

		delete d;
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
		GeorgesTypDialog *d = new GeorgesTypDialog();
		if( !d->load( fileName ) )
		{
			delete d;
			return NULL;
		}

		connect( d, SIGNAL( modified() ), this, SLOT( setModified() ) );

		return d;
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

	GeorgesDockWidget* GeorgesEditorForm::loadFormDialog( const QString &fileName )
	{
		CGeorgesTreeViewDialog *d = new CGeorgesTreeViewDialog();
		if( !d->load( fileName ) )
		{
			delete d;
			return NULL;
		}

		connect(d, SIGNAL(modified()), this, SLOT(setModified()));
		connect(d, SIGNAL(changeFile(QString)), m_georgesDirTreeDialog, SLOT(changeFile(QString)));

		return d;
	}

} /* namespace GeorgesQt */
