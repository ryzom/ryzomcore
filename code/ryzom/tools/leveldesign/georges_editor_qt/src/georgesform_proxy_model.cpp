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

#include "georgesform_proxy_model.h"

// NeL includes
#include <nel/misc/debug.h>

namespace NLQT {

	bool CGeorgesFormProxyModel::filterAcceptsRow(int sourceRow,
         const QModelIndex &sourceParent) const
	{
		nlinfo("CGeorgesFormProxyModel::filterAcceptsRow");
     //QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
     //QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
     //QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent);

     //return (sourceModel()->data(index0).toString().contains(filterRegExp())
     //        || sourceModel()->data(index1).toString().contains(filterRegExp()))
     //       && dateInRange(sourceModel()->data(index2).toDate());
			
		//	if (getItem(p_index)->valueFrom() == UFormElm::ValueDefaultDfn)
		//				return QBrush(QColor(255,0,0,30));
		//			if (getItem(p_index)->nodeFrom() == UFormElm::NodeParentForm)
		//				return QBrush(QColor(0,255,0,30));
		//			return QVariant();
		return true;
	}
	
/******************************************************************************/

	bool CGeorgesFormProxyModel::filterAcceptsColumn(int sourceRow,
         const QModelIndex &sourceParent) const
	{
		nlinfo("CGeorgesFormProxyModel::filterAcceptsColumn");
		return QSortFilterProxyModel::filterAcceptsColumn(sourceRow, sourceParent);
	}
} /* namespace NLQT */

/* end of file */
