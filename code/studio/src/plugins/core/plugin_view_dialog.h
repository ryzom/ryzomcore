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

#ifndef PLUGIN_VIEW_H
#define PLUGIN_VIEW_H

#include "ui_plugin_view_dialog.h"

#include <QtCore/QMap>
#include <QtCore/QStringList>

namespace ExtensionSystem
{
class IPluginManager;
class IPluginSpec;
}

namespace Core
{

class PluginView: public QDialog
{
	Q_OBJECT

public:
	PluginView(ExtensionSystem::IPluginManager *pluginManager, QWidget *parent = 0);
	~PluginView();

private Q_SLOTS:
	void updateList();
	void updateSettings();

	void onItemClicked();
	void onUnloadClicked();
	void onLoadClicked();

private:

	const int m_checkStateColumn;
	QMap<ExtensionSystem::IPluginSpec *, QTreeWidgetItem *> m_specToItem;
	QStringList m_whiteList;
	ExtensionSystem::IPluginManager *m_pluginManager;
	Ui::PluginView m_ui;
}; /* class PluginView */

} /* namespace Core */

#endif // PLUGIN_VIEW_H
