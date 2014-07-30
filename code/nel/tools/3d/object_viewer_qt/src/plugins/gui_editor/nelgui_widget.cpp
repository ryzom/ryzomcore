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


#include "nelgui_widget.h"
#include "nel/misc/path.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/lua_manager.h"
#include "nel/misc/path.h"
#include "nel/misc/i18n.h"
#include <set>
#include <string>
#include <QTimerEvent>
#include "editor_selection_watcher.h"

namespace GUIEditor
{
	std::set< std::string > hwCursors;

	NelGUIWidget::NelGUIWidget( QWidget *parent ) :
	Nel3DWidget( parent )
	{
		timerID = 0;
		guiLoaded = false;
		watcher = NULL;
	}

	NelGUIWidget::~NelGUIWidget()
	{
		guiLoaded = false;
		if( timerID != 0 )
			killTimer( timerID );

		NLGUI::CViewRenderer::release();
		NLMISC::CI18N::setNoResolution( false );
		
	}

	void NelGUIWidget::init()
	{
		NLMISC::CI18N::setNoResolution( true );
		NLMISC::CPath::remapExtension( "dds", "tga", true );
		NLMISC::CPath::remapExtension( "dds", "png", true );
		NLMISC::CPath::remapExtension( "png", "tga", true );

		Nel3DWidget::init();
		createTextContext( "Ryzom.ttf" );

		NLGUI::CAHManager::setEditorMode( true );
		NLGUI::CLuaManager::setEditorMode( true );
		NLGUI::CInterfaceElement::setEditorMode( true );

		NLGUI::CViewRenderer::setDriver( getDriver() );
		NLGUI::CViewRenderer::setTextContext( getTextContext() );
		NLGUI::CViewRenderer::hwCursors = &hwCursors;
		NLGUI::CViewRenderer::getInstance()->init();

		CWidgetManager::getInstance()->getParser()->setEditorMode( true );

		watcher = new CEditorSelectionWatcher();
	}

	bool NelGUIWidget::parse( SProjectFiles &files )
	{
		reset();
		IParser *parser = CWidgetManager::getInstance()->getParser();

		std::vector< std::string >::iterator itr;
		for( itr = files.mapFiles.begin(); itr != files.mapFiles.end(); ++itr )
		{
			std::string &file = *itr;
			std::string::size_type i = file.find_last_of( '.' );
			std::string mapFile = file.substr( 0, i );
			mapFile.append( ".txt" );

			if( !CViewRenderer::getInstance()->loadTextures( file, mapFile, false ) )
			{
				CViewRenderer::getInstance()->reset();
				return false;
			}
		}

		if( !parser->parseInterface( files.guiFiles, false ) )
			return false;

		CWidgetManager::getInstance()->updateAllLocalisedElements();
		CWidgetManager::getInstance()->activateMasterGroup( files.masterGroup, true );
		
		CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId( files.activeGroup );
		if( e != NULL )
			e->setActive( true );

		timerID = startTimer( 200 );
		guiLoaded = true;
		Q_EMIT guiLoadComplete();

		CWidgetManager::getInstance()->registerSelectionWatcher( watcher );

		return true;
	}

	void NelGUIWidget::reset()
	{
		guiLoaded = false;
		if( timerID != 0 )
			killTimer( timerID );
		timerID = 0;
		CWidgetManager::getInstance()->unregisterSelectionWatcher( watcher );
		CWidgetManager::getInstance()->reset();
		CWidgetManager::getInstance()->getParser()->removeAll();
		CViewRenderer::getInstance()->reset();
		clear();
	}

	void NelGUIWidget::draw()
	{
		getDriver()->clearBuffers( NLMISC::CRGBA::Black );
		CWidgetManager::getInstance()->checkCoords();
		CWidgetManager::getInstance()->drawViews( 0 );
		getDriver()->swapBuffers();
	}

	void NelGUIWidget::paintEvent( QPaintEvent *evnt )
	{
		if( !guiLoaded )
			clear();
	}

	void NelGUIWidget::timerEvent( QTimerEvent *evnt )
	{
		if( evnt->timerId() == timerID )
		{
			if( guiLoaded )
			{
				getDriver()->EventServer.pump();
				draw();
			}
		}
	}

	void NelGUIWidget::showEvent( QShowEvent *evnt )
	{
		if( timerID == 0 )
			timerID = startTimer( 200 );
	}

	void NelGUIWidget::hideEvent( QHideEvent *evnt )
	{
		if( timerID != 0 )
			killTimer( timerID );
	}
}

