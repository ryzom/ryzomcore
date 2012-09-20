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

// NeL includes
#include <nel/misc/debug.h>
#include <nel/georges/u_form_elm.h>

// project includes
#include "formitem.h"
#include "georgesform_proxy_model.h"
#include "georgesform_model.h"

#if 0

namespace GeorgesQt 
{

	bool CGeorgesFormProxyModel::filterAcceptsRow(int sourceRow,
		const QModelIndex &sourceParent) const
	{
		// column doesnt matter for item
		CGeorgesFormModel *smodel = dynamic_cast<CGeorgesFormModel *>(sourceModel());
		QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
		CFormItem *item   = smodel->getItem(index);

		// if elm not existing it must be some kind of default or type value
		if(!item->getFormElm())
		{
			return smodel->showDefaults();
		}

		// else it might be some parent elm
		switch (item->nodeFrom())
		{
		case NLGEORGES::UFormElm::NodeParentForm:
			{
				return smodel->showParents();
			}
		case NLGEORGES::UFormElm::NodeForm:
			{
				switch (item->valueFrom())
				{
				case NLGEORGES::UFormElm::ValueParentForm:
					{
						return smodel->showParents();
					}
				default:
					{
						CFormItem *parent = item->parent();
						if (parent && (parent->nodeFrom() == NLGEORGES::UFormElm::NodeParentForm))
						{
							return smodel->showParents();
						}
					}
				}
			}
		}
		return true;
	}
	
/******************************************************************************/

	bool CGeorgesFormProxyModel::filterAcceptsColumn(int sourceRow,
         const QModelIndex &sourceParent) const
	{
		return QSortFilterProxyModel::filterAcceptsColumn(sourceRow, sourceParent);
	}
} /* namespace GeorgesQt */

#endif // 0
/* end of file */
