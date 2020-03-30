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

#include "filesystem_model.h"

#include <QtGui/QApplication>
#include <QtGui/QStyle>

namespace NLQT 
{

	CFileSystemModel::CFileSystemModel(QString ldPath, QObject *parent)
		: QFileSystemModel(parent),
		_ldPath(ldPath)
	{

	}
	CFileSystemModel::~CFileSystemModel()
	{

	}

	QVariant CFileSystemModel::data(const QModelIndex& index, int role) const {

		if (role == Qt::DecorationRole) 
		{
			if (_ldPath.isEmpty())
				return QVariant();
			if (isDir(index))
				return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
		}
		if (_ldPath.isEmpty() && role == Qt::DisplayRole) 
		{
			if (index.parent().isValid())
				return QVariant();
			return QString("Set a correct leveldesign path ...");
		}
		return QFileSystemModel::data(index, role);
	}

	int CFileSystemModel::columnCount(const QModelIndex &/*parent*/) const
	{
		return 1;
	}

	int CFileSystemModel::rowCount(const QModelIndex &parent) const
	{
		if (_ldPath.isEmpty()) 
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

} /* namespace NLQT */

/* end of file */
