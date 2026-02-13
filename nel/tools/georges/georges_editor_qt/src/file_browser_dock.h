// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef FILE_BROWSER_DOCK_H
#define FILE_BROWSER_DOCK_H

#include <QDockWidget>
#include <QTabWidget>
#include <QTreeWidget>

class MainWindow;

/**
 * File browser dock widget, ported from CFileBrowserDialog.
 * Provides a tabbed file browser with Type, Dfn, and Form tabs.
 */
class FileBrowserDock : public QDockWidget
{
	Q_OBJECT

public:
	FileBrowserDock(MainWindow *mainWindow, QWidget *parent = nullptr);
	~FileBrowserDock();

	void refresh();

signals:
	void fileDoubleClicked(const QString &path);

private slots:
	void onItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
	void setupUi();
	void populateTab(QTreeWidget *tree, const QString &directory, const QStringList &filters);

	MainWindow *_mainWindow;
	QTabWidget *_tabWidget;
	QTreeWidget *_typeTree;
	QTreeWidget *_dfnTree;
	QTreeWidget *_formTree;
};

#endif // FILE_BROWSER_DOCK_H
