// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef IAPP_PAGE_H
#define IAPP_PAGE_H

#include <QtCore/QObject>

class QWidget;

namespace Core
{
/**
@interface IAppPage
@brief The IAppPage is an interface for providing app pages in main window.
@details You need to subclass this interface and put an instance of your subclass
  into the plugin manager object pool.
*/
class IAppPage
{
public:
	virtual ~IAppPage() {}

	/// id() is a unique identifier for referencing this page
	virtual QString id() const = 0;

	/// trName() is the (translated) name for display.
	virtual QString trName() const = 0;

	/// icon() is the icon for display
	virtual QIcon icon() const = 0;

	/// The widget will be destroyed by the widget hierarchy when the main window closes
	virtual QWidget *widget(QWidget *parent) = 0;
};

} // namespace Core

Q_DECLARE_INTERFACE(Core::IAppPage, "dev.ryzom.com.IAppPage/0.1")

#endif // IAPP_PAGE_H
