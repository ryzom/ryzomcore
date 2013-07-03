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


#ifndef MATERIAL_SPLITTER_H
#define MATERIAL_SPLITTER_H

#include <QSplitter>
#include "material_observer.h"

class QtTreePropertyBrowser;

namespace MaterialEditor
{
	class CNel3DInterface;
	class MaterialWidget;
	class CPropBrowserCtrl;

	class MaterialSplitter : public QSplitter, public CMaterialObserver
	{
		Q_OBJECT
	public:
		MaterialSplitter( QWidget *parent = NULL );
		~MaterialSplitter();

		void setup();

		void setNel3DIface( CNel3DInterface *iface );

		void onNewMaterial();
		void onMaterialLoaded();		
		void onPassAdded( const char *name );
		void onPassRemoved( const char *name );
		void onPassMovedUp( const char *name );
		void onPassMovedDown( const char *name );
		void onPassRenamed( const char *from, const char *to );

	private:
		CNel3DInterface *nl3dIface;
		MaterialWidget *materialWidget;
		CPropBrowserCtrl *browserCtrl;
		QtTreePropertyBrowser *browser;
	};
}

#endif
