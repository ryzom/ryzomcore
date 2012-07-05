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

#include "widget_properties.h"

namespace GUIEditor
{
	QString _lastDir;
	std::map< std::string, SWidgetInfo > widgetInfo;
	
	GUIEditorWindow::GUIEditorWindow(QWidget *parent) :
	QMainWindow(parent)
	{
		m_ui.setupUi(this);
		m_undoStack = new QUndoStack(this);
		widgetProps = new CWidgetProperties;
		createMenus();
		readSettings();
		parseGUIWidgets();
		widgetProps->setupWidgetInfo( &widgetInfo );
	}
	
	GUIEditorWindow::~GUIEditorWindow()
	{
		writeSettings();
		delete widgetProps;
		widgetProps = NULL;
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
	
	void GUIEditorWindow::createMenus()
	{
		Core::MenuManager *mm = Core::ICore::instance()->menuManager();
		QMenu *menu = mm->menu( Core::Constants::M_TOOLS );
		if( menu != NULL )
		{
			QAction *a = new QAction( "Widget Properties", this );
			connect( a, SIGNAL( triggered( bool ) ), widgetProps, SLOT( show() ) );
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

	void GUIEditorWindow::parseGUIWidgets()
	{
		QDir d( "widgets" );
		if( !d.exists() )
		{
			nlwarning( "GUI widgets directory doesn't exist!" );
			return;
		}

		QStringList nameFilters;
		nameFilters.push_back( "*.xml" );

		QStringList files = d.entryList( nameFilters, QDir::Files );
		if( files.empty() )
		{
			nlwarning( "GUI widgets directory has no files!" );
			return;
		}

		QStringListIterator itr( files );
		while( itr.hasNext() )
			parseGUIWidget( "widgets/" + itr.next() );
	}

	void GUIEditorWindow::parseGUIWidget( const QString &file )
	{
		QFile f( file );
		if( f.open( QIODevice::ReadOnly ) )
		{
			parseGUIWidgetXML( f );
			f.close();
		}
		else
			nlwarning( QString( "File %1 cannot be opened!" ).arg( file ).toStdString().c_str() );
	}

	void GUIEditorWindow::parseGUIWidgetXML( QFile &file )
	{
		QXmlStreamReader reader;
		reader.setDevice( &file );

		reader.readNext();
		if( reader.atEnd() )
			return;

		while( !reader.atEnd() && !( reader.isStartElement() && ( reader.name() == "widget" ) ) )
			reader.readNext();
		if( reader.atEnd() )
			return;

		while( !reader.atEnd() && !( reader.isStartElement() && ( reader.name() == "header" ) ) )
			reader.readNext();
		if( reader.atEnd() )
			return;

		QString name = parseGUIWidgetHeader( reader );
		if( name.isEmpty() )
		{
			nlwarning( "malformed XML." );
			return;
		}

		while( !reader.atEnd() && !( reader.isStartElement() && ( reader.name() == "properties" ) ) )
			reader.readNext();
		if( reader.atEnd() )
			return;

		parseGUIWidgetProperties( reader, name );
	}

	QString GUIEditorWindow::parseGUIWidgetHeader( QXmlStreamReader &reader )
	{
		reader.readNext();
		if( reader.atEnd() )
			return QString( "" );

		SWidgetInfo info;

		while( !reader.atEnd() && !( reader.isEndElement() && ( reader.name() == "header" ) ) )
		{
			if( reader.isStartElement() )
			{
				QString key = reader.name().toString();
				QString value = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );

				if( !reader.hasError() )
				{
					if( key == "name" )
						info.name = value.toStdString();
					else
					if( key == "guiname" )
						info.GUIName = value.toStdString();
					else
					if( key == "description" )
						info.description = value.toStdString();
					else
					if( key == "icon" )
						info.icon == value.toStdString();
					else
					if( key == "abstract" )
					{
						info.isAbstract = false;
						if( value == "true" )
							info.isAbstract = true;
					}
					else
						nlwarning( "Malformed XML." );
				}
			}
			
			reader.readNext();
		}
		if( reader.atEnd() )
			return QString( "" );
		if( info.name.empty() )
			return QString( "" );

		widgetInfo[ info.name.c_str() ] = info;
		return QString( info.name.c_str() );
	}

	void GUIEditorWindow::parseGUIWidgetProperties( QXmlStreamReader &reader, const QString &widgetName )
	{
		reader.readNext();
		if( reader.atEnd() )
			return;

		std::map< std::string, SWidgetInfo >::iterator itr =
			widgetInfo.find( widgetName.toStdString() );
		if( itr == widgetInfo.end() )
			return;

		std::vector< SPropEntry > &v = itr->second.props;

		while( !reader.atEnd() && !( reader.isEndElement() && ( reader.name() == "properties" ) ) )
		{
			if( reader.isStartElement() && reader.name() == "property" )
			{
				SPropEntry prop;
				reader.readNext();

				while( !reader.atEnd() && !( reader.isEndElement() && ( reader.name() == "property" ) ) )
				{
					if( reader.isStartElement() )
					{
						QString key = reader.name().toString();
						QString value = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );

						if( !reader.hasError() )
						{
							if( key == "name" )
								prop.propName = value.toStdString();
							else
							if( key == "type" )
								prop.propType = value.toStdString();
							else
							if( key == "default" )
								prop.propDefault = value.toStdString();
							else
								nlwarning( QString( "Unknown tag %1 within a property" ).arg( key ).toStdString().c_str() );

						}
						else
							nlwarning( "Malformed XML." );
					}
					
					reader.readNext();
				}
				if( reader.atEnd() )
					return;
				
				v.push_back( prop );
			}

			reader.readNext();
		}
	}	
}
