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


#ifndef PROP_BROWSER_CTRL
#define PROP_BROWSER_CTRL

#include <QObject>
#include <string>
#include <map>
#include "widget_info.h"

class QtTreePropertyBrowser;
class QtVariantPropertyManager;
class QtEnumPropertyManager;
class QtProperty;
class QVariant;

namespace NLGUI
{
	class CInterfaceElement;
}

namespace GUIEditor
{
	class CWidgetInfoTree;

	/// This class controls the Widget property browser widget.
	/// It receives signals from the widget that draws/manages the Nel GUI widgets, and handles them.
	class CPropBrowserCtrl : public QObject
	{
		Q_OBJECT

	public:
		CPropBrowserCtrl();
		~CPropBrowserCtrl();
		void setBrowser( QtTreePropertyBrowser *b );
		void setupWidgetInfo( CWidgetInfoTree *tree );
		void clear();

	public Q_SLOTS:
		void onSelectionChanged( std::string &id );

	private Q_SLOTS:
		void onPropertyChanged( QtProperty *prop, const QVariant &v );
		void onEnumPropertyChanged( QtProperty *prop, int value );

	private:
		void setupProperties( const std::string &type, const NLGUI::CInterfaceElement *element );
		void setupProperty( const SPropEntry &prop, const NLGUI::CInterfaceElement *element );

		QtTreePropertyBrowser *browser;
		QtVariantPropertyManager *propertyMgr;
		QtEnumPropertyManager *enumMgr;

		std::string currentElement;
		std::map< std::string, SWidgetInfo > widgetInfo;
	};

}

#endif
