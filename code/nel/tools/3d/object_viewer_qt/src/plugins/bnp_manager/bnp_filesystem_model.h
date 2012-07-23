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

#ifndef BNP_FILESYSTEM_MODEL_H
#define BNP_FILESYSTEM_MODEL_H

#include <QtGui/QFileSystemModel>

namespace BNPManager
{

class BNPFileSystemModel : public QFileSystemModel
{
	Q_OBJECT

public:

	/**
	 * Constructor
	 */
	BNPFileSystemModel(QObject *parent = 0);

	/**
	 * Destructor
	 */
	~BNPFileSystemModel();

	int columnCount(const QModelIndex &) const;

	QVariant data(const QModelIndex& index, int role) const ;

};

}

#endif