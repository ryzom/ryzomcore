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
#include "georgesform_model.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/georges/u_form_elm.h>

// project includes
#include "formitem.h"

#include <QDebug>

namespace NLQT 
{

	bool CGeorgesFormProxyModel::filterAcceptsRow(int sourceRow,
		const QModelIndex &sourceParent) const
	{
		//nlinfo("CGeorgesFormProxyModel::filterAcceptsRow");

		// column doesnt matter for item
		CGeorgesFormModel *smodel = dynamic_cast<CGeorgesFormModel *>(sourceModel());
		QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
		CFormItem *item   = smodel->getItem(index);

		//qDebug() << smodel->showParents() << (item->valueFrom() == NLGEORGES::UFormElm::NodeParentForm);
		//nlinfo("%s %d %d %d %d", item->data(index.column()).toString().toUtf8().constData(), 
		//	item->valueFrom(),
		//	item->nodeFrom(),
		//	smodel->showParents(), 
		//	(item->valueFrom() == NLGEORGES::UFormElm::NodeParentForm));

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
		default:
			break;
		}
		return true;
	}
	
/******************************************************************************/

	bool CGeorgesFormProxyModel::filterAcceptsColumn(int sourceRow,
         const QModelIndex &sourceParent) const
	{
		//nlinfo("CGeorgesFormProxyModel::filterAcceptsColumn");
		return QSortFilterProxyModel::filterAcceptsColumn(sourceRow, sourceParent);
	}
} /* namespace NLQT */

/* end of file */
