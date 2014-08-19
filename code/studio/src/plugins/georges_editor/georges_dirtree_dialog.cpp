// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian Jaekel <aj at elane2k dot com>
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

#include "stdpch.h"
// Project includes
#include "georges_dirtree_dialog.h"

// Qt includes
#include <QtGui/QWidget>
#include <QSettings>

// NeL includes

namespace GeorgesQt
{

CGeorgesDirTreeDialog::CGeorgesDirTreeDialog(QString ldPath, QWidget *parent)
	:QDockWidget(parent),
	m_ldPath(ldPath)
{

	m_ui.setupUi(this);

	m_ui.filterResetButton->setIcon(
		QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));

	m_dirModel = new CGeorgesFileSystemModel(m_ldPath);
	//m_proxyModel = new CGeorgesFileSystemProxyModel(this);

	//m_proxyModel->setSourceModel(m_dirModel);
	m_ui.dirTree->setModel(m_dirModel);

	// TODO: filtering in tree model is ... complicated - so hide it for now
	m_ui.filterLineEdit->hide();
	m_ui.filterResetButton->hide();
	m_ui.label->hide();

	if (m_dirModel->isCorrectLDPath())
	{
		m_dirModel->setRootPath(m_ldPath);
		m_ui.dirTree->setRootIndex(m_dirModel->index(m_ldPath));
	}
	else
	{
		m_dirModel->setRootPath(QDir::currentPath());
	}

	m_ui.dirTree->setAnimated(false);
	m_ui.dirTree->setIndentation(20);

	connect(m_ui.dirTree, SIGNAL(activated(QModelIndex)),
			this, SLOT(fileSelected(QModelIndex)));
}

CGeorgesDirTreeDialog::~CGeorgesDirTreeDialog()
{
	delete m_dirModel;
}

void CGeorgesDirTreeDialog::fileSelected(QModelIndex index)
{
	if (index.isValid() && !m_dirModel->isDir(index))
	{
		Q_EMIT selectedForm(m_dirModel->fileName(index));
	}
}

void CGeorgesDirTreeDialog::changeFile(QString file)
{
	QModelIndex index = m_dirModel->index(file);
	m_ui.dirTree->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect);
	m_ui.dirTree->scrollTo(index,QAbstractItemView::PositionAtCenter);
	fileSelected(index);
}

void CGeorgesDirTreeDialog::ldPathChanged(QString path)
{
	m_ldPath = path;

	delete m_dirModel;
	//delete m_proxyModel;

	m_dirModel = new CGeorgesFileSystemModel(m_ldPath);
	//m_proxyModel = new CGeorgesFileSystemProxyModel(this);

	//m_proxyModel->setSourceModel(m_dirModel);
	m_ui.dirTree->setModel(m_dirModel);

	if (m_dirModel->isCorrectLDPath())
	{
		m_dirModel->setRootPath(m_ldPath);
		m_ui.dirTree->setRootIndex(m_dirModel->index(m_ldPath));
	}
	else
	{
		m_dirModel->setRootPath(QDir::currentPath());
	}
}

} /* namespace GeorgesQt */