// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "gui_editor_window.h"
#include "gui_editor_constants.h"

#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/core.h"
#include "../core/menu_manager.h"

#include <nel/misc/debug.h>

#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QDockWidget>
#include <QMessageBox>
#include "../../3rdparty/qtpropertybrowser/QtTreePropertyBrowser"

#include "widget_properties.h"
#include "widget_info_tree.h"
#include "widget_properties_parser.h"
#include "widget_hierarchy.h"
#include "widget_serializer.h"
#include "link_list.h"
#include "proc_list.h"
#include "project_file_parser.h"
#include "project_file_serializer.h"
#include "project_window.h"
#include "nelgui_ctrl.h"
#include "editor_selection_watcher.h"
#include "editor_message_processor.h"
#include "add_widget_widget.h"
#include "new_gui_dlg.h"

namespace GUIEditor
{
	QString _lastDir;
	std::map< std::string, SWidgetInfo > widgetInfo;
	SProjectFiles projectFiles;
	CProjectFileParser projectParser;

	GUIEditorWindow::GUIEditorWindow(QWidget *parent) :
	QMainWindow(parent)
	{
		menu = NULL;
		m_ui.setupUi(this);
		messageProcessor = new CEditorMessageProcessor;
		m_undoStack   = new QUndoStack(this);
		widgetProps   = new CWidgetProperties;
		linkList      = new LinkList;
		procList      = new ProcList;
		projectWindow = new ProjectWindow;
		addWidgetWidget = new AddWidgetWidget;
		connect( projectWindow, SIGNAL( projectFilesChanged() ), this, SLOT( onProjectFilesChanged() ) );
		GUICtrl      = new NelGUICtrl();
		setCentralWidget( GUICtrl->getViewPort() );

		widgetInfoTree = new CWidgetInfoTree;

		createMenus();
		readSettings();

		CWidgetPropParser parser;

		parser.setWidgetPropMap( &widgetInfo );
		parser.setWidgetInfoTree( widgetInfoTree );
		parser.parseGUIWidgets();
		widgetProps->setupWidgetInfo( widgetInfoTree );
		addWidgetWidget->setupWidgetInfo( widgetInfoTree );
		messageProcessor->setTree( widgetInfoTree );

		QDockWidget *dock = new QDockWidget( "Widget Hierarchy", this );
		dock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
		hierarchyView = new WidgetHierarchy;
		dock->setWidget( hierarchyView );
		addDockWidget( Qt::LeftDockWidgetArea, dock );

		dock = new QDockWidget( "Widget Properties", this );
		dock->setAllowedAreas(  Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

		QtTreePropertyBrowser *propBrowser = new QtTreePropertyBrowser;
		browserCtrl.setupWidgetInfo( widgetInfoTree );
		browserCtrl.setBrowser( propBrowser );
		dock->setWidget( propBrowser );
		addDockWidget( Qt::RightDockWidgetArea, dock );

		GUICtrl->init();

		connect( GUICtrl, SIGNAL( guiLoadComplete() ), this, SLOT( onGUILoaded() ) );
		connect( widgetProps, SIGNAL( treeChanged() ), this, SLOT( onTreeChanged() ) );
		connect(
			addWidgetWidget,
			SIGNAL( adding( const QString&, const QString&, const QString& ) ),
			messageProcessor,
			SLOT( onAdd( const QString&, const QString&, const QString& ) )
			);
	}
	
	GUIEditorWindow::~GUIEditorWindow()
	{
		writeSettings();

		removeMenus();

		delete messageProcessor;
		messageProcessor = NULL;

		delete widgetProps;
		widgetProps = NULL;

		delete linkList;
		linkList = NULL;

		delete procList;
		procList = NULL;

		delete projectWindow;
		projectWindow = NULL;

		delete GUICtrl;
		GUICtrl = NULL;

		delete addWidgetWidget;
		addWidgetWidget = NULL;

		// no deletion needed for these, since dockwidget owns them
		hierarchyView = NULL;
		propBrowser   = NULL;

		delete widgetInfoTree;
		widgetInfoTree = NULL;
	}
	
	QUndoStack *GUIEditorWindow::undoStack() const
	{
		return m_undoStack;
	}
	
	void GUIEditorWindow::open()
	{
		QString fileName = QFileDialog::getOpenFileName( this,
											tr( "Open GUI XML files" ),
											_lastDir,
											tr( "All XML files (*.xml)" ) );
		
		setCursor( Qt::WaitCursor );
		if( fileName.isEmpty() )
		{
			setCursor( Qt::ArrowCursor );
			return;
		}
		
		_lastDir = QFileInfo( fileName ).absolutePath();

		projectParser.clear();

		std::string projectFileName = fileName.toUtf8().constData();
		if( !projectParser.parseProjectFile( projectFileName ) )
		{
			QMessageBox::critical( this,
				tr( "Error parsing project file" ),
				tr( "There was an error while parsing the project file. Not a project file?" ) );
			setCursor( Qt::ArrowCursor );
			return;
		}
		projectFiles.clearAll();
		projectParser.getProjectFiles( projectFiles );
		currentProject = projectFiles.projectName.c_str();
		currentProjectFile = fileName;
		projectWindow->setupFiles( projectFiles );
		GUICtrl->setWorkDir( _lastDir );

		if( GUICtrl->parse( projectFiles ) )
		{
			hierarchyView->buildHierarchy( projectFiles.masterGroup );
		}
		else
		{
			QMessageBox::critical( this,
				tr( "Error parsing GUI XML files" ),
				tr( "There was an error while parsing the GUI XML files. See the log file for details." ) );
		}

		setCursor( Qt::ArrowCursor );
	}

	void GUIEditorWindow::newDocument()
	{
		NewGUIDlg d;
		int result = d.exec();
		
		if( result == QDialog::Rejected )
			return;

		close();

		std::string proj = d.getProjectName().toUtf8().constData();
		std::string wnd = d.getWindowName().toUtf8().constData();
		std::string mg = std::string( "ui:" ) + proj;
		std::string dir = d.getProjectDirectory().toUtf8().constData();
		_lastDir = dir.c_str();
		std::string uiFile = "ui_" + proj + ".xml";

		QList< QString > mapList;
		d.getMapList( mapList );

		bool b = GUICtrl->createNewGUI( proj, wnd );
		if( !b )
		{
			QMessageBox::information( this,
										tr( "Creating new GUI project" ),
										tr( "Failed to create new GUI project :(" ) );
			reset();
			return;
		}

		hierarchyView->buildHierarchy( mg );

		projectFiles.projectName = proj;
		projectFiles.masterGroup = mg;
		projectFiles.activeGroup = std::string( "ui:" ) + proj + ":" + wnd;
		projectFiles.version = SProjectFiles::NEW;
		projectFiles.guiFiles.push_back( uiFile );

		for( int i = 0; i < mapList.size(); i++ )
		{
			projectFiles.mapFiles.push_back( mapList[ i ].toUtf8().constData() );
		}

		b = GUICtrl->loadMapFiles( projectFiles.mapFiles );
		if( !b )
		{
			QMessageBox::information( this,
										tr( "Creating new GUI project" ),
										tr( "Failed to create new GUI project: Couldn't load map files. " ) );
			reset();
			return;
		}
		
		projectWindow->setupFiles( projectFiles );

		currentProject = proj.c_str();
		currentProjectFile = std::string( dir + "/" + proj + ".xml" ).c_str();


		// Save the project file
		CProjectFileSerializer serializer;
		serializer.setFile( currentProjectFile.toUtf8().constData() );
		if( !serializer.serialize( projectFiles ) )
		{
			QMessageBox::critical( this,
							tr( "Failed to save project" ),
							tr( "There was an error while trying to save the project." ) );
			return;
		}

		// Save the GUI file
		WidgetSerializer widgetSerializer;
		widgetSerializer.setFile( dir + "/" + uiFile );
		widgetSerializer.setActiveGroup( projectFiles.activeGroup );
		if( !widgetSerializer.serialize( projectFiles.masterGroup ) )
		{
			QMessageBox::critical( this,
							tr( "Failed to save project" ),
							tr( "There was an error while trying to save the project." ) );
			return;
		}
	}

	void GUIEditorWindow::save()
	{
		if( currentProject.isEmpty() )
			return;

		CProjectFileSerializer serializer;
		serializer.setFile( currentProjectFile.toUtf8().constData() );
		if( !serializer.serialize( projectFiles ) )
		{
			QMessageBox::critical( this,
							tr( "Failed to save project" ),
							tr( "There was an error while trying to save the project." ) );
			return;
		}

		// Can't save old projects any further, since the widgets are in multiple files in them
		// using templates, styles and whatnot. There's no way to restore the original XML structure
		// after it's loaded
		if( projectFiles.version == SProjectFiles::OLD )
			return;

		std::string f = _lastDir.toUtf8().constData();
		f += "/";
		f += projectFiles.guiFiles[ 0 ];

		WidgetSerializer widgetSerializer;
		widgetSerializer.setFile( f );
		widgetSerializer.setActiveGroup( projectFiles.activeGroup );
		if( !widgetSerializer.serialize( projectFiles.masterGroup ) )
		{
			QMessageBox::critical( this,
							tr( "Failed to save project" ),
							tr( "There was an error while trying to save the project." ) );
			return;
		}
	}

	void GUIEditorWindow::saveAs()
	{
		if( currentProject.isEmpty() )
			return;

		QString dir = 
			QFileDialog::getExistingDirectory( this, tr( "Save project as..." ) );

		if( dir.isEmpty() )
			return;
		_lastDir = dir;

		if( projectFiles.version == SProjectFiles::OLD )
		{
			projectFiles.guiFiles.clear();
			projectFiles.guiFiles.push_back( "ui_" + projectFiles.projectName + ".xml"  );
			projectFiles.version = SProjectFiles::NEW;

		}

		currentProjectFile = _lastDir;
		currentProjectFile += "/";
		currentProjectFile += projectFiles.projectName.c_str();
		currentProjectFile += ".xml";

		save();
	}

	void GUIEditorWindow::reset()
	{
		projectFiles.clearAll();
		projectWindow->clear();
		hierarchyView->clearHierarchy();
		GUICtrl->reset();
		browserCtrl.clear();
		linkList->clear();
		procList->clear();
		currentProject = "";
		currentProjectFile = "";
		projectParser.clear();
	}

	bool GUIEditorWindow::close()
	{
		if( currentProject.isEmpty() )
			return true;

		QMessageBox::StandardButton reply = QMessageBox::question( this,
											tr( "Closing project" ),
											tr( "Are you sure you want to close this project?" ),
											QMessageBox::Yes | QMessageBox::No );
		if( reply != QMessageBox::Yes )
			return false;


		CEditorSelectionWatcher *w = GUICtrl->getWatcher();
		disconnect( w, SIGNAL( sgnSelectionChanged() ), hierarchyView, SLOT( onSelectionChanged() ) );
		disconnect( w, SIGNAL( sgnSelectionChanged() ), &browserCtrl, SLOT( onSelectionChanged() ) );

		reset();

		return true;
	}

	void GUIEditorWindow::onProjectFilesChanged()
	{
		setCursor( Qt::WaitCursor );

		projectWindow->updateFiles( projectFiles );
		if( !GUICtrl->parse( projectFiles ) )
		{
			QMessageBox::critical( this,
				tr( "Error parsing GUI XML files" ),
				tr( "There was an error while parsing the GUI XML files. See the log file for details." ) );
		}

		setCursor( Qt::ArrowCursor );
	}

	void GUIEditorWindow::onGUILoaded()
	{
		hierarchyView->onGUILoaded();
		procList->onGUILoaded();
		linkList->onGUILoaded();

		CEditorSelectionWatcher *w = GUICtrl->getWatcher();
		connect( w, SIGNAL( sgnSelectionChanged() ), hierarchyView, SLOT( onSelectionChanged() ) );
		connect( w, SIGNAL( sgnSelectionChanged() ), &browserCtrl, SLOT( onSelectionChanged() ) );
	}

	void GUIEditorWindow::onAddWidgetClicked()
	{
		QString g;
		hierarchyView->getCurrentGroup( g );

		addWidgetWidget->setCurrentGroup( g );
		addWidgetWidget->show();
	}

	void GUIEditorWindow::onTreeChanged()
	{
		addWidgetWidget->setupWidgetInfo( widgetInfoTree );
	}


	void GUIEditorWindow::hideEvent( QHideEvent *evnt )
	{
		QWidget::hideEvent( evnt );
		GUICtrl->hide();
	}

	void GUIEditorWindow::showEvent( QShowEvent *evnt )
	{
		QWidget::showEvent( evnt );
		GUICtrl->show();
	}

	void GUIEditorWindow::createMenus()
	{
		Core::MenuManager *mm = Core::ICore::instance()->menuManager();
		QAction *newAction = mm->action( Core::Constants::NEW );
		QAction *saveAction = mm->action( Core::Constants::SAVE );
		QAction *saveAsAction = mm->action( Core::Constants::SAVE_AS );
		QAction *closeAction = mm->action( Core::Constants::CLOSE );
		QAction *delAction = mm->action( Core::Constants::DEL );

		if( newAction != NULL )
			newAction->setEnabled( true );
		if( saveAction != NULL )
			saveAction->setEnabled( true );
		if( saveAsAction != NULL )
			saveAsAction->setEnabled( true );
		if( closeAction != NULL )
			closeAction->setEnabled( true );
		if( delAction != NULL )
		{
			delAction->setEnabled( true );
			connect( delAction, SIGNAL( triggered( bool ) ), messageProcessor, SLOT( onDelete() ) );
		}

		QMenu *m = mm->menuBar()->addMenu( "GUI Editor" );
		if( m != NULL )
		{
			QAction *a = new QAction( "Widget Properties", this );
			connect( a, SIGNAL( triggered( bool ) ), widgetProps, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Link Editor", this );
			connect( a, SIGNAL( triggered( bool ) ), linkList, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Procedure Editor", this );
			connect( a, SIGNAL( triggered( bool ) ), procList, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Project Window", this );
			connect( a, SIGNAL( triggered( bool ) ), projectWindow, SLOT( show() ) );
			m->addAction( a );

			a = new QAction( "Add Widget", this );
			connect( a, SIGNAL( triggered( bool ) ), this, SLOT( onAddWidgetClicked() ) );
			m->addAction( a );

			a = new QAction( "Group", this );
			connect( a, SIGNAL( triggered() ), messageProcessor, SLOT( onGroup() ) );
			m->addAction( a );

			a = new QAction( "Ungroup", this );
			connect( a, SIGNAL( triggered() ), messageProcessor, SLOT( onUngroup() ) );
			m->addAction( a );

			
			// ----------------------------------------------------------------------------------			
			m->addSeparator();

			a = new QAction( "Select groups", this );
			a->setCheckable( true );
			a->setChecked( false );
			connect( a, SIGNAL( triggered( bool ) ), messageProcessor, SLOT( onSetGroupSelection( bool ) ) );
			m->addAction( a );

			a = new QAction( "Multiselect", this );
			a->setCheckable( true );
			a->setChecked( false );
			connect( a, SIGNAL( triggered( bool ) ), messageProcessor, SLOT( onSetMultiSelection( bool ) ) );
			m->addAction( a );

			menu = m;
		}
	}

	void GUIEditorWindow::removeMenus()
	{
		delete menu;
		menu = NULL;
	}
	
	void GUIEditorWindow::readSettings()
	{
		QSettings *settings = Core::ICore::instance()->settings();
		settings->beginGroup(Constants::GUI_EDITOR_SECTION);
		settings->endGroup();
	}
	
	void GUIEditorWindow::writeSettings()
	{
		QSettings *settings = Core::ICore::instance()->settings();
		settings->beginGroup(Constants::GUI_EDITOR_SECTION);
		settings->endGroup();
		settings->sync();
	}
}
