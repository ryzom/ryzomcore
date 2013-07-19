// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef WIDGETBASE_H
#define WIDGETBASE_H

#include <QWidget>

/**
 @brief Base class for the config tool's settings page widgets.
*/
class CWidgetBase : public QWidget
{
	Q_OBJECT

public:
	CWidgetBase( QWidget *parent = NULL ) : QWidget( parent ) {}
	~CWidgetBase() {}

	/**
	 @brief Tells the widget to load it's values from the config.
	*/
	virtual void load() = 0;

	/**
	 @brief Tells the widget to save it's values into the config.
	*/
	virtual void save() = 0;

signals:
	/**
	 @brief Emitted when the user changes a setting.
	*/
	void changed();


private slots:
	/**
	 @brief Triggered when something changes in the widget, emits the Changed() signal.
	*/
	void onSomethingChanged()
	{
		emit changed();
	}
};


#endif
