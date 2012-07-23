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

#ifndef FILESYSTEM_MODEL_H
#define FILESYSTEM_MODEL_H

#include <QtGui/QFileSystemModel>

namespace NLQT 
{

	class CFileSystemModel : public QFileSystemModel
	{
		QString _ldPath;

	public:
		CFileSystemModel(QString ldPath, QObject *parent = 0);
		~CFileSystemModel();

		int columnCount(const QModelIndex &/*parent*/) const;
		int rowCount(const QModelIndex &/*parent*/) const;

		QVariant data(const QModelIndex& index, int role) const ;

	};/* class CFileSystemModel */

} /* namespace NLQT */

#endif // FILESYSTEM_MODEL_H