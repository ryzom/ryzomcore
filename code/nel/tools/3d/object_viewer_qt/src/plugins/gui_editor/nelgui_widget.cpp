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
#include <set>
#include <string>

namespace GUIEditor
{
	std::set< std::string > hwCursors;

	NelGUIWidget::NelGUIWidget( QWidget *parent ) :
	Nel3DWidget( parent )
	{
	}

	NelGUIWidget::~NelGUIWidget()
	{
		NLGUI::CViewRenderer::release();
		
	}

	void NelGUIWidget::init()
	{
		NLMISC::CPath::addSearchPath( "fonts" );

		Nel3DWidget::init();
		createTextContext( "Ryzom.ttf" );

		NLGUI::CViewRenderer::setDriver( getDriver() );
		NLGUI::CViewRenderer::setTextContext( getTextContext() );
		NLGUI::CViewRenderer::hwCursors = &hwCursors;
		NLGUI::CViewRenderer::getInstance()->init();
	}
}

