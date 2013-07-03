// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "prop_browser_ctrl.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "nel3d_interface.h"

namespace MaterialEditor
{
	CPropBrowserCtrl::CPropBrowserCtrl( QObject *parent ) :
	QObject( parent )
	{
		browser = NULL;
		nel3dIface = NULL;
	}

	CPropBrowserCtrl::~CPropBrowserCtrl()
	{
	}
	
	void CPropBrowserCtrl::setBrowser( QtTreePropertyBrowser *b )
	{
		browser = b;
	}

	void CPropBrowserCtrl::setNel3DIface( CNel3DInterface *iface )
	{
		nel3dIface = iface;
	}

	void CPropBrowserCtrl::setupConnections()
	{
	}

	void CPropBrowserCtrl::onPropsChanged()
	{
		clearProps();
		loadPropsForPass( currentPass );
	}

	void CPropBrowserCtrl::clearProps()
	{
		browser->clear();
	}

	void CPropBrowserCtrl::loadPropsForPass( const QString &pass )
	{
		currentPass = pass;
		CNelMaterialProxy m = nel3dIface->getMaterial();
		CRenderPassProxy p = m.getPass( pass.toUtf8().data() );
	}

}

