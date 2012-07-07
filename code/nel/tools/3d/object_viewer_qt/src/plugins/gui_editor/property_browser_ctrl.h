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

class QtTreePropertyBrowser;
class QtVariantPropertyManager;

namespace GUIEditor
{

	/// This class controls the Widget property browser widget.
	/// It receives signals from the widget that draws/manages the Nel GUI widgets, and handles them.
	class CPropBrowserCtrl : public QObject
	{
		Q_OBJECT

	public:
		CPropBrowserCtrl();
		~CPropBrowserCtrl();
		void setBrowser( QtTreePropertyBrowser *b );
		void setup();

	private:
		QtTreePropertyBrowser *browser;
		QtVariantPropertyManager *propertyMgr;
	};

}

#endif
