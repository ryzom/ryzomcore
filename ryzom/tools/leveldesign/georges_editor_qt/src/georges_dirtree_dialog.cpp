/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

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

#include "georges_dirtree_dialog.h"

// Qt includes
#include <QtGui/QWidget>
#include <QSettings>

// NeL includes

// Project includes
#include "modules.h"

using namespace NLMISC;

namespace NLQT 
{

	CGeorgesDirTreeDialog::CGeorgesDirTreeDialog(QString ldPath, QWidget *parent)
		:QDockWidget(parent), _ldPath(ldPath)
	{

		_ui.setupUi(this);

		//QStyleOptionViewItem myButtonOption;
		//       const QStyleOptionViewItem *buttonOption =
		//		 qstyleoption_cast<const QStyleOptionViewItem *>(_ui.dirTree->style());

		/*setStyleSheet("                     \
		QTreeView {                         \
		alternate-background-color: yellow; \
		}                                   \
		QTreeView::item {                   \
		border: 1px solid #d9d9d9;        \
		border-top-color: transparent;     \
		border-bottom-color: transparent;  \
		}                                   \
		QTreeView::item:hover {             \
		background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e7effd, stop: 1 #cbdaf1);\
		border: 1px solid #bfcde4;         \
		}                                   \
		QTreeView::item:selected {          \
		border: 1px solid #567dbc;         \
		}                                   \
		QTreeView::item:selected:active{    \
		background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6ea1f1, stop: 1 #567dbc);\
		}                                   \
		QTreeView::item:selected:!active {  \
		background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6b9be8, stop: 1 #577fbf);\
		}\
		");*/
		//QString leveldata = Modules::config().getConfigFile().getVar("SearchPaths").asString().c_str();

		_dirModel = new CFileSystemModel(_ldPath);
		_ui.dirTree->setModel(_dirModel);

		QFileInfo info(_ldPath);

		if (!_ldPath.isEmpty() && info.isDir()) 
		{
			_dirModel->setRootPath(_ldPath);	
			_ui.dirTree->setRootIndex(_dirModel->index(_ldPath));
		}
		else 
		{
			_dirModel->setRootPath(QDir::currentPath());	
		}

		_ui.dirTree->setAnimated(false);
		_ui.dirTree->setIndentation(20);
		//_ui.dirTree->setSortingEnabled(true);

		/*connect(_ui.dirTree, SIGNAL(clicked(QModelIndex)),
		this, SLOT(fileSelected(QModelIndex)));*/
		connect(_ui.dirTree, SIGNAL(activated(QModelIndex)),
			this, SLOT(fileSelected(QModelIndex)));
	}

	CGeorgesDirTreeDialog::~CGeorgesDirTreeDialog() 
	{
		delete _dirModel;
	}

	void CGeorgesDirTreeDialog::fileSelected(QModelIndex index) 
	{
		QString name;
		if (index.isValid() && !_dirModel->isDir(index)) 
		{
			Q_EMIT selectedForm(_dirModel->fileName(index));
		}
	}

	void CGeorgesDirTreeDialog::changeFile(QString file) 
	{
		QModelIndex index = _dirModel->index(file);
		//_dirModel->;
		_ui.dirTree->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect);
		_ui.dirTree->scrollTo(index,QAbstractItemView::PositionAtCenter);
		fileSelected(index);
	}

	void CGeorgesDirTreeDialog::ldPathChanged(QString path) 
	{
		_ldPath = path;
		QFileInfo info(_ldPath);

		delete _dirModel;

		if (!_ldPath.isEmpty() && info.isDir()) 
		{
			_dirModel = new CFileSystemModel(_ldPath);
			_ui.dirTree->setModel(_dirModel);
			_dirModel->setRootPath(_ldPath);
			_ui.dirTree->setRootIndex(_dirModel->index(_ldPath));
		}
		else 
		{
			_dirModel = new CFileSystemModel("");
			_ui.dirTree->setModel(_dirModel);
			_dirModel->setRootPath(QDir::currentPath());
			_ldPath = "";
		}
	}

} /* namespace NLQT */