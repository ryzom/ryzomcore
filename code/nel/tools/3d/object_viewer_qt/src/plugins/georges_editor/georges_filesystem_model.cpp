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

#include "georges_filesystem_model.h"

#include <QApplication>
#include <QStyle>

namespace Plugin
{

CGeorgesFileSystemModel::CGeorgesFileSystemModel(QString ldPath, QObject *parent)
	: QFileSystemModel(parent),
	  m_ldPath(ldPath),
	  m_correct(false)
{
	checkLDPath();
}

CGeorgesFileSystemModel::~CGeorgesFileSystemModel()
{

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
} /* namespace NLQT */

/* end of file */
