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

#include "file_browser_dock.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>

#include "main_window.h"
#include "../../3d/shared_widgets/configuration.h"

FileBrowserDock::FileBrowserDock(MainWindow *mainWindow, QWidget *parent)
	: QDockWidget(tr("File Browser"), parent), _mainWindow(mainWindow)
{
	setupUi();
}

FileBrowserDock::~FileBrowserDock()
{
}

void FileBrowserDock::setupUi()
{
	QWidget *container = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);

	_tabWidget = new QTabWidget(container);

	// Type tab
	_typeTree = new QTreeWidget(_tabWidget);
	_typeTree->setHeaderLabel(tr("Type Files"));
	_typeTree->setRootIsDecorated(true);
	_tabWidget->addTab(_typeTree, tr("Type"));

	// Dfn tab
	_dfnTree = new QTreeWidget(_tabWidget);
	_dfnTree->setHeaderLabel(tr("DFN Files"));
	_dfnTree->setRootIsDecorated(true);
	_tabWidget->addTab(_dfnTree, tr("DFN"));

	// Form tab
	_formTree = new QTreeWidget(_tabWidget);
	_formTree->setHeaderLabel(tr("Form Files"));
	_formTree->setRootIsDecorated(true);
	_tabWidget->addTab(_formTree, tr("Form"));

	layout->addWidget(_tabWidget);
	setWidget(container);

	connect(_typeTree, &QTreeWidget::itemDoubleClicked, this, &FileBrowserDock::onItemDoubleClicked);
	connect(_dfnTree, &QTreeWidget::itemDoubleClicked, this, &FileBrowserDock::onItemDoubleClicked);
	connect(_formTree, &QTreeWidget::itemDoubleClicked, this, &FileBrowserDock::onItemDoubleClicked);
}

void FileBrowserDock::refresh()
{
	_typeTree->clear();
	_dfnTree->clear();
	_formTree->clear();

	// Read paths from CConfiguration. Config values are relative to the project
	// root (the folder containing .nel), matching the planar_reflection pattern.
	NLQT::CConfiguration &config = _mainWindow->getConfiguration();
	std::string projectRoot = _mainWindow->getProjectRoot();
	QString rootPath = QString::fromUtf8(config.getValue("RootSearchDirectory", std::string("")).c_str());
	QString subDir = QString::fromUtf8(config.getValue("TypDfnSubFolder", std::string("")).c_str());

	// Resolve relative to project root
	if (!rootPath.isEmpty() && !projectRoot.empty())
		rootPath = QDir::cleanPath(QString::fromUtf8(projectRoot.c_str()) + "/" + rootPath);

	QString searchDir = rootPath;
	if (!subDir.isEmpty())
	{
		searchDir = QDir::cleanPath(rootPath + QDir::separator() + subDir);
	}

	if (!searchDir.isEmpty() && QDir(searchDir).exists())
	{
		populateTab(_typeTree, searchDir, QStringList() << "*.typ");
		populateTab(_dfnTree, searchDir, QStringList() << "*.dfn");

		// Form files: search root directory
		if (QDir(rootPath).exists())
			populateTab(_formTree, rootPath, QStringList() << "*.*");
	}
}

void FileBrowserDock::populateTab(QTreeWidget *tree, const QString &directory, const QStringList &filters)
{
	QDir dir(directory);
	if (!dir.exists())
		return;

	QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
	for (const QFileInfo &fi : files)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(tree);
		item->setText(0, fi.fileName());
		item->setData(0, Qt::UserRole, fi.absoluteFilePath());
	}

	// Also scan subdirectories
	QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	for (const QFileInfo &di : dirs)
	{
		QTreeWidgetItem *dirItem = new QTreeWidgetItem(tree);
		dirItem->setText(0, di.fileName());
		dirItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));

		QDir subDir(di.absoluteFilePath());
		QFileInfoList subFiles = subDir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
		for (const QFileInfo &fi : subFiles)
		{
			QTreeWidgetItem *item = new QTreeWidgetItem(dirItem);
			item->setText(0, fi.fileName());
			item->setData(0, Qt::UserRole, fi.absoluteFilePath());
		}
	}
}

void FileBrowserDock::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);
	if (!item)
		return;

	QString path = item->data(0, Qt::UserRole).toString();
	if (!path.isEmpty())
		emit fileDoubleClicked(path);
}
