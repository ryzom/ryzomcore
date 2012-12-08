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
#include "georges_filesystem_model.h"

#include <QApplication>
#include <QStyle>

namespace GeorgesQt
{

CGeorgesFileSystemModel::CGeorgesFileSystemModel(QString ldPath, QObject *parent)
	: QFileSystemModel(parent),
	  m_ldPath(ldPath),
	  m_correct(false)
{
	checkLDPath();

	// this yielded no relevant performance boost on my observations
	//connect(this, SIGNAL(directoryLoaded(QString)),
	//		this, SLOT(dir(const QString)));
}

CGeorgesFileSystemModel::~CGeorgesFileSystemModel()
{

}

void CGeorgesFileSystemModel::dir(const QString &dir)
{
	// in theory this should prefetch all directory entries for the 
	// filesystem model to speed up later work
	QModelIndex i = index(dir);

	if (hasChildren(i)) {
		int childCount = rowCount(i);
		for (int c=0; c<childCount; c++) {
			const QModelIndex child = index(c, 0, i);
			if (child.isValid()) {
				fetchMore(child);
			}
		}
	}
}

QVariant CGeorgesFileSystemModel::data(const QModelIndex& index, int role) const
{

	if (role == Qt::DecorationRole)
	{
		if (!m_correct)
			return QVariant();
		if (isDir(index))
			return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
	}
	if (!m_correct && role == Qt::DisplayRole)
	{
		if (index.parent().isValid())
			return QVariant();
		return tr("Set a correct leveldesign path ...");
	}
	return QFileSystemModel::data(index, role);
}

int CGeorgesFileSystemModel::columnCount(const QModelIndex &/*parent*/) const
{
	return 1;
}

int CGeorgesFileSystemModel::rowCount(const QModelIndex &parent) const
{

	if (!m_correct)
	{
		if(parent.isValid())
		{
			return 0;
		}
		else
		{
			return qMin(QFileSystemModel::rowCount(parent),1);
		}
	}
	return QFileSystemModel::rowCount(parent);
}

void CGeorgesFileSystemModel::checkLDPath()
{
	QFileInfo check1(QString("%1/game_element").arg(m_ldPath));
	QFileInfo check2(QString("%1/DFN").arg(m_ldPath));

	if (check1.exists() && check2.exists())
	{
		m_correct = true;
	}
	else
	{
		m_correct = false;
	}
}
//
//bool CGeorgesFileSystemModel::canFetchMore(const QModelIndex &parent) const
//{
//	return true;
//	
//   /* Q_D(const QFileSystemModel);
//    const QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(parent);
//    return (!indexNode->populatedChildren);*/
//}

//CGeorgesFileSystemProxyModel::CGeorgesFileSystemProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
//{
//	setFilterCaseSensitivity(Qt::CaseInsensitive);
//}

//bool CGeorgesFileSystemProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
//{
	// TODO this is not perfect as it could be
	// eg it should filter all dirs which have no entry
	//QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
	//if (sourceModel()->hasChildren(idx))
	//{
	//	QString d = sourceModel()->data(idx).toString();
	//	//QModelIndex i = mapFromSource(source_parent);
	//	//if (hasChildren(i)) {
	//	int childCount = sourceModel()->rowCount(idx);
	//	for (int c=0; c<childCount; c++) {
	//		/*const QModelIndex child = sourceModel()->index(c, 0, idx);
	//		if (child.isValid()) {
	//			bool test = filterAcceptsRow(c, child);
	//		}*/
	//	}
	//	return true;
	//}
	//return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
//}

//QVariant CGeorgesFileSystemProxyModel::data ( const QModelIndex & index, int role ) const
//{
//	if (role == Qt::DisplayRole)
//	{
//		QString test = QSortFilterProxyModel::data(index, role).toString();
//		return test.append(QString(" (%1/%2)")).
//			arg(rowCount(index)).
//			arg(sourceModel()->rowCount(mapToSource(index)));
//	}
//	return QSortFilterProxyModel::data(index, role);
//}
} /* namespace GeorgesQt */

/* end of file */
