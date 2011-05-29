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

#ifndef GEORGES_FILESYSTEM_MODEL_H
#define GEORGES_FILESYSTEM_MODEL_H

#include <QtGui/QFileSystemModel>

namespace Plugin
{

class CGeorgesFileSystemModel : public QFileSystemModel
{
	QString m_ldPath;

public:
	CGeorgesFileSystemModel(QString ldPath, QObject *parent = 0);
	~CGeorgesFileSystemModel();

	int columnCount(const QModelIndex &/*parent*/) const;
	int rowCount(const QModelIndex &/*parent*/) const;

	QVariant data(const QModelIndex& index, int role) const ;

	bool isCorrectLDPath()
	{
		return m_correct;
	}
	void checkLDPath();

private:
	bool m_correct;
};/* class CGeorgesFileSystemModel */

} /* namespace NLQT */

#endif // GEORGES_FILESYSTEM_MODEL_H