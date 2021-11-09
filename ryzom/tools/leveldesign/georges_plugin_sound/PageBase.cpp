// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "std_sound_plugin.h"
#include "PageBase.h"



#undef new
IMPLEMENT_DYNCREATE(CPageBase, CPropertyPage)
#define new NL_NEW


std::set<CPageBase*>	CPageBase::_AllPages;


void CPageBase::docChanged()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	std::set<CPageBase*>::iterator first(_AllPages.begin()), last(_AllPages.end());
	for(; first != last; ++first)
	{
		CPageBase *page = (*first);
		if (page->m_hWnd != NULL)
			(*first)->onDocChanged();
	}
}
