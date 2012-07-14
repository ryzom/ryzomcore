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
#include "../../3rdparty/qtpropertybrowser/QtTreePropertyBrowser"

#include "widget_properties.h"
#include "widget_properties_parser.h"
#include "widget_hierarchy.h"
#include "link_editor.h"
#include "proc_editor.h"

#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_renderer.h"

namespace GUIEditor
{
	QString _lastDir;
	std::map< std::string, SWidgetInfo > widgetInfo;
	std::set< std::string > hwCurs;

	GUIEditorWindow::GUIEditorWindow(QWidget *parent) :
	QMainWindow(parent)
	{
		m_ui.setupUi(this);
		m_undoStack = new QUndoStack(this);
		widgetProps = new CWidgetProperties;
		linkEditor  = new LinkEditor;
		procEditor  = new ProcEditor;
		createMenus();
		readSettings();

		CWidgetPropParser parser;

		parser.setWidgetPropMap( &widgetInfo );
		parser.parseGUIWidgets();
		widgetProps->setupWidgetInfo( &widgetInfo );

		QDockWidget *dock = new QDockWidget( "Widget Hierarchy", this );
		dock->setAllowedAreas( Qt::LeftDockWidgetArea );
		WidgetHierarchy *ha = new WidgetHierarchy;
		dock->setWidget( ha );
		addDockWidget( Qt::LeftDockWidgetArea, dock );

		dock = new QDockWidget( "Widget Properties", this );
		dock->setAllowedAreas( Qt::RightDockWidgetArea );
		QtTreePropertyBrowser *tb = new QtTreePropertyBrowser;
		browserCtrl.setBrowser( tb );
		browserCtrl.setup();
		dock->setWidget( tb );
		addDockWidget( Qt::RightDockWidgetArea, dock );
		
		CWidgetManager::getInstance();
		NLMISC::CPath::addSearchPath( "z:/ryzom/data", true, false, NULL );
		NLMISC::CPath::remapExtension ("dds", "tga", true);
		NLMISC::CPath::remapExtension ("dds", "png", true);
		NLMISC::CPath::remapExtension ("png", "tga", true);
		
		NLGUI::_UIStringMapper =
			NLMISC::CStringMapper::createLocalMapper();
		
		NL3D::UDriver *driver = NL3D::UDriver::createDriver();
		CViewRenderer::setDriver( driver );
		CViewRenderer::setTextContext( driver->createTextContext( NLMISC::CPath::lookup( "ryzom.ttf" ) ) );
		hwCurs.insert( "curs_default.tga" );
		CViewRenderer::hwCursors = &hwCurs;
		CViewRenderer::getInstance();
	}
	
	GUIEditorWindow::~GUIEditorWindow()
	{
		writeSettings();

		delete widgetProps;
		widgetProps = NULL;

		delete linkEditor;
		linkEditor = NULL;

		delete procEditor;
		procEditor = NULL;

		CWidgetManager::release();
	}
	
	QUndoStack *GUIEditorWindow::undoStack() const
	{
		return m_undoStack;
	}
	
	void GUIEditorWindow::open()
	{
		QStringList fileNames = QFileDialog::getOpenFileNames(this,
											tr("Open GUI XML files"),
											_lastDir,
											tr("All XML files (*.xml)"));
		
		setCursor(Qt::WaitCursor);
		if(!fileNames.isEmpty())
		{
			QStringList list = fileNames;
			_lastDir = QFileInfo(list.front()).absolutePath();
		}
		setCursor(Qt::ArrowCursor);
	}


	void GUIEditorWindow::parse()
	{
		std::vector< std::string > files;
		files.push_back( "login_config.xml" );
		files.push_back( "login_widgets.xml" );
		files.push_back( "login_main.xml" );
		files.push_back( "login_keys.xml" );
		
		CViewRenderer::getInstance()->loadTextures( 
			"texture_interfaces_v3_login.tga",
			"texture_interfaces_v3_login.txt",
			false );
		
		setCursor( Qt::WaitCursor );
		CWidgetManager::getInstance()->getParser()->parseInterface( files, false );
		setCursor( Qt::ArrowCursor );
	}
	
	void GUIEditorWindow::createMenus()
	{
		Core::MenuManager *mm = Core::ICore::instance()->menuManager();
		QMenu *menu = mm->menu( Core::Constants::M_TOOLS );
		if( menu != NULL )
		{
			QAction *a = new QAction( "Widget Properties", this );
			connect( a, SIGNAL( triggered( bool ) ), widgetProps, SLOT( show() ) );
			menu->addAction( a );

			a = new QAction( "Link Editor", this );
			connect( a, SIGNAL( triggered( bool ) ), linkEditor, SLOT( show() ) );
			menu->addAction( a );

			a = new QAction( "Proc Editor", this );
			connect( a, SIGNAL( triggered( bool ) ), procEditor, SLOT( show() ) );
			menu->addAction( a );
			
			a = new QAction( "parse", this );
			connect( a, SIGNAL( triggered( bool ) ), this, SLOT( parse() ) );
			menu->addAction( a );
		}
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
