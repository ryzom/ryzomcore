// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
// Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#ifndef IOPTIONS_PAGE_H
#define IOPTIONS_PAGE_H

// Project includes
#include "core_global.h"

// Qt includes
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE
class QWidget;
class QIcon;
QT_END_NAMESPACE

namespace Core
{
/**
@interface IOptionsPage
@brief The IOptionsPage is an interface for providing options pages.
@details You need to subclass this interface and put an instance of your subclass
  into the plugin manager object pool.
*/
class CORE_EXPORT IOptionsPage: public QObject
{
	Q_OBJECT
public:
	IOptionsPage(QObject *parent = 0): QObject(parent) {}
	virtual ~IOptionsPage() {}

	/// id() is a unique identifier for referencing this page
	virtual QString id() const = 0;

	/// trName() is the (translated) name for display.
	virtual QString trName() const = 0;

	/// category() is the unique id for the category that the page should be displayed in
	virtual QString category() const = 0;

	/// trCategory() is the translated category
	virtual QString trCategory() const = 0;

	virtual QIcon categoryIcon() const = 0;

	/// createPage() is called to retrieve the widget to show in the preferences dialog
	/// The widget will be destroyed by the widget hierarchy when the dialog closes
	virtual QWidget *createPage(QWidget *parent) = 0;

	/// apply() is called to store the settings. It should detect if any changes have been made and store those.
	virtual void apply() = 0;

	/// finish() is called directly before the preferences dialog closes
	virtual void finish() = 0;
};

} // namespace Core

#endif // IOPTIONS_PAGE_H
