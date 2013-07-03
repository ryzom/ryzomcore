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


#include "material_splitter.h"
#include "nel3d_interface.h"
#include "material_widget.h"
#include "prop_browser_ctrl.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"

namespace MaterialEditor
{
	MaterialSplitter::MaterialSplitter( QWidget *parent ) :
	QSplitter( parent )
	{
		materialWidget = new MaterialWidget();
		browserCtrl = new CPropBrowserCtrl();
		browser = new QtTreePropertyBrowser();
		browserCtrl->setBrowser( browser );

		setup();
		setupConnections();
	}

	MaterialSplitter::~MaterialSplitter()
	{
		delete browserCtrl;
		browserCtrl = NULL;
		nl3dIface = NULL;
		materialWidget = NULL;
		browser = NULL;
	}

	void MaterialSplitter::setupConnections()
	{
		connect( materialWidget, SIGNAL( propsChanged() ), this, SLOT( onPropsChanged() ) );
	}

	void MaterialSplitter::setup()
	{
		setOrientation( Qt::Vertical );
		addWidget( materialWidget );
		addWidget( browser );
	}

	void MaterialSplitter::setNel3DIface( CNel3DInterface *iface )
	{
		nl3dIface = iface;
		materialWidget->setNel3DIface( iface );
		browserCtrl->setNel3DIface( iface );
	}

	void MaterialSplitter::onNewMaterial()
	{
		materialWidget->onNewMaterial();
	}

	void MaterialSplitter::onMaterialLoaded()
	{
		materialWidget->onMaterialLoaded();
	}

	void MaterialSplitter::onPassAdded( const char *name )
	{
		materialWidget->onPassAdded( name );
	}

	void MaterialSplitter::onPassRemoved( const char *name )
	{
		materialWidget->onPassRemoved( name );
	}

	void MaterialSplitter::onPassMovedUp( const char *name )
	{
		materialWidget->onPassMovedUp( name );
	}

	void MaterialSplitter::onPassMovedDown( const char *name )
	{
		materialWidget->onPassMovedDown( name );
	}

	void MaterialSplitter::onPassRenamed( const char *from, const char *to )
	{
		materialWidget->onPassRenamed( from, to );
	}

	void MaterialSplitter::onPropsChanged()
	{
		QString pass;
		materialWidget->getCurrentPass( pass );
		browserCtrl->onPropsChanged();
	}
}

