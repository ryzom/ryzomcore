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

#ifndef GEORGESFORM_PROXY_MODEL_H
#define GEORGESFORM_PROXY_MODEL_H

// Qt includes
#include <QSortFilterProxyModel>

namespace GeorgesQt 
{

	class CGeorgesFormProxyModel : public QSortFilterProxyModel 
	{

	public:
		CGeorgesFormProxyModel(QObject *parent = 0): QSortFilterProxyModel(parent)
		{
		}
		~CGeorgesFormProxyModel() 
		{
		}

	protected:
		virtual bool  filterAcceptsColumn ( int source_column, const QModelIndex & source_parent ) const ;
		virtual bool  filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const ;

	};/* class CGeorgesFormProxyModel */

} /* namespace GeorgesQt */

#endif // GEORGESFORM_PROXY_MODEL_H
