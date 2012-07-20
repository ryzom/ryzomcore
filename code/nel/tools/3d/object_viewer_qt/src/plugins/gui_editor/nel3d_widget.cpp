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


#include "nel3d_widget.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/text_context.h"
#include "nel/misc/rgba.h"
#include "nel/misc/path.h"


namespace GUIEditor
{
	Nel3DWidget::Nel3DWidget( QWidget *parent ) :
	QWidget( parent )
	{
		driver = NULL;
		textContext = NULL;

		// Need to set this attribute with a NULL paintengine returned to Qt
		// so that we can render the widget normally ourselves, without the image
		// disappearing when a widget is resized or shown on top of us
		setAttribute( Qt::WA_PaintOnScreen, true );
	}

	Nel3DWidget::~Nel3DWidget()
	{
		if( driver != NULL )
		{
			if( textContext != NULL )
			{
				driver->deleteTextContext( textContext );
				textContext = NULL;
			}

			driver->release();
			delete driver;
			driver = NULL;
		}
	}

	void Nel3DWidget::init()
	{
		nlassert( driver == NULL );

		driver = NL3D::UDriver::createDriver( 0, false, 0 );
		driver->setMatrixMode2D11();
		driver->setDisplay( winId(), NL3D::UDriver::CMode( width(), height(), 32, true ) );
	}

	void Nel3DWidget::createTextContext( std::string fontFile )
	{
		if( driver == NULL )
			return;
		
		std::string font;

		try
		{
			font = NLMISC::CPath::lookup( fontFile );
		}
		catch( ... )
		{
			nlinfo( "Font %s cannot be found, cannot create textcontext!", fontFile.c_str() );
			exit( EXIT_FAILURE );
		}

		if( textContext != NULL )
		{
			driver->deleteTextContext( textContext );
			textContext = NULL;
		}

		textContext = driver->createTextContext( font );
	}

	void Nel3DWidget::clear()
	{
		if( driver == NULL )
			return;
		driver->clearBuffers( NLMISC::CRGBA::Black );
		driver->swapBuffers();
	}
}


