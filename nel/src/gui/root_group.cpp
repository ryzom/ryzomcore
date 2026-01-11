// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#include "stdpch.h"
#include "nel/gui/root_group.h"
#include <vector>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	CRootGroup::CRootGroup(const TCtorParam &param) :
	CInterfaceGroup(param)
	{
	}

	CRootGroup::~CRootGroup()
	{
	}

	CInterfaceElement* CRootGroup::getElement (const std::string &id)
	{
		if (_Id == id)
		return this;

		if (id.substr(0, _Id.size()) != _Id)
			return NULL;

		std::vector<CViewBase*>::const_iterator itv;
		for (itv = _Views.begin(); itv != _Views.end(); itv++)
		{
			CViewBase *pVB = *itv;
			if (pVB->getId() == id)
				return pVB;
		}

		std::vector<CCtrlBase*>::const_iterator itc;
		for (itc = _Controls.begin(); itc != _Controls.end(); itc++)
		{
			CCtrlBase* ctrl = *itc;
			if (ctrl->getId() == id)
				return ctrl;
		}

		// Accelerate
		std::string sTmp = id;
		sTmp = sTmp.substr(_Id.size()+1,sTmp.size());
		std::string::size_type pos = sTmp.find(':');
		if (pos != std::string::npos)
			sTmp = sTmp.substr(0,pos);

		std::map<std::string,CInterfaceGroup*>::iterator it = _Accel.find(sTmp);
		if (it != _Accel.end())
		{
			CInterfaceGroup *pIG = it->second;
			return pIG->getElement(id);
		}
		return NULL;
	}

	void CRootGroup::addGroup (CInterfaceGroup *child, sint eltOrder)
	{
		std::string sTmp = child->getId();
		sTmp = sTmp.substr(_Id.size()+1,sTmp.size());
		_Accel.insert(std::pair<std::string,CInterfaceGroup*>(sTmp, child));
		CInterfaceGroup::addGroup(child,eltOrder);
	}

	bool CRootGroup::delGroup (CInterfaceGroup *child, bool dontDelete)
	{
		std::string sTmp = child->getId();
		sTmp = sTmp.substr(_Id.size()+1,sTmp.size());
		std::map<std::string,CInterfaceGroup*>::iterator it = _Accel.find(sTmp);
		if (it != _Accel.end())
		{
			_Accel.erase(it);
		}
		return CInterfaceGroup::delGroup(child,dontDelete);
	}

}

