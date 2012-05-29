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

#ifndef BNP_PROXY_MODEL_H
#define BNP_PROXY_MODEL_H

// Qt includes
#include <QSortFilterProxyModel>

namespace BNPManager 
{

	class BNPSortProxyModel : public QSortFilterProxyModel
	{

	public:
		BNPSortProxyModel(QObject *parent = 0): QSortFilterProxyModel(parent)
		{
		}
		~BNPSortProxyModel() 
		{
		}

	protected:
		virtual bool 	lessThan ( const QModelIndex & left, const QModelIndex & right ) const;

	};/* class BNPSortProxyModel */

} // BNPManager

#endif // BNP_PROXY_MODEL_H
