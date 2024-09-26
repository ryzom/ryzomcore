// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
class QtVariantEditorFactory;
class QtEnumEditorFactory;
class QtProperty;
class QVariant;

class ActionPropertyManager;
class ActionEditorFactory;
class TexturePropertyManager;
class TextureEditorFactory;

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
		void onSelectionChanged();

	private Q_SLOTS:
		void onPropertyChanged( QtProperty *prop, const QVariant &v );
		void onEnumPropertyChanged( QtProperty *prop, int value );
		void onActionPropertyChanged( QtProperty *p, const QString &v );
		void onTexturePropertyChanged( QtProperty *p, const QString &v );

	private:
		void enablePropertyWatchers();
		void disablePropertyWatchers();

		void setupProperties( const std::string &type, const NLGUI::CInterfaceElement *element );
		void setupProperty( const SPropEntry &prop, const NLGUI::CInterfaceElement *element );

		QtTreePropertyBrowser *browser;
		QtVariantPropertyManager *propertyMgr;
		QtEnumPropertyManager *enumMgr;
		ActionPropertyManager *actionMgr;
		TexturePropertyManager *textureMgr;

		QtVariantEditorFactory *variantFactory;
		QtEnumEditorFactory *enumFactory;
		ActionEditorFactory *actionFactory;
		TextureEditorFactory *textureFactory;

		std::string currentElement;
		std::map< std::string, SWidgetInfo > widgetInfo;
		std::map< std::string, std::string > nameToType;
		std::map< std::string, QtProperty * > ttPosRefProps; // Tooltip posref properties
		std::map< std::string, std::string > ttPairs;
	};

}

#endif
