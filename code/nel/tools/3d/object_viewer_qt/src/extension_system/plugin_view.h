/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef PLUGIN_VIEW_H
#define PLUGIN_VIEW_H

#include "ui_plugin_view.h"


// STL includes

// Qt includes

// NeL includes

// Project includes


namespace NLQT
{

class IPluginManager;

class CPluginView: public QDialog
{
	Q_OBJECT

public:
	CPluginView(IPluginManager *pluginManager, QWidget *parent = 0);
	~CPluginView();

private Q_SLOTS:
	void updateList();

private:

	IPluginManager *_pluginManager;
	Ui::CPluginView _ui;
}; /* class CPluginView */

} /* namespace NLQT */

#endif // PLUGIN_VIEW_H
