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


#include "property_browser_ctrl.h"
#include "../../3rdparty/qtpropertybrowser/QtVariantPropertyManager"
#include "../../3rdparty/qtpropertybrowser/QtTreePropertyBrowser"

namespace GUIEditor
{
	CPropBrowserCtrl::CPropBrowserCtrl()
	{
		browser = NULL;
		propertyMgr = new QtVariantPropertyManager;
	}

	CPropBrowserCtrl::~CPropBrowserCtrl()
	{
		delete propertyMgr;
		propertyMgr = NULL;
		browser = NULL;
	}

	void CPropBrowserCtrl::setBrowser( QtTreePropertyBrowser *b )
	{
		browser = b;
	}

	void CPropBrowserCtrl::setup()
	{
		if( browser == NULL )
			return;

		QtVariantProperty *p;
		
		p = propertyMgr->addProperty( QVariant::String, "Id" );
		p->setValue( "ExampleId" );
		browser->addProperty( p );

		p = propertyMgr->addProperty( QVariant::Bool, "Active" );
		p->setValue( true );
		browser->addProperty( p );

		p = propertyMgr->addProperty( QVariant::String, "on_enter" );
		p->setValue( "on_enter_handler" );
		browser->addProperty( p );

		p = propertyMgr->addProperty( QVariant::String, "on_enter_params" );
		p->setValue( "someparams" );
		browser->addProperty( p );
	}
}