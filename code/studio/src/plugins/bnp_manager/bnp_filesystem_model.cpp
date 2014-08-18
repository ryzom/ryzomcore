// Object Viewer Qt - BNP Manager Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Roland Winklmeier <roland.m.winklmeier@googlemail.com>
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>

#include "bnp_filesystem_model.h"

#include <QApplication>
#include <QStyle>

namespace BNPManager
{

BNPFileSystemModel::BNPFileSystemModel(QObject *parent)
	: QFileSystemModel(parent)
{
}
// ***************************************************************************
BNPFileSystemModel::~BNPFileSystemModel()
{

}
// ***************************************************************************
int BNPFileSystemModel::columnCount(const QModelIndex &) const
{
	return 1;
}
// ***************************************************************************
QVariant BNPFileSystemModel::data(const QModelIndex& index, int role) const
{

	if (role == Qt::DecorationRole)
	{
		if (isDir(index))
			return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
	}
	return QFileSystemModel::data(index, role);
}
// ***************************************************************************
} // namespace BNPManager
