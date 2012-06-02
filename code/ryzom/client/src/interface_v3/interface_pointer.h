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



#ifndef NL_INTERFACE_POINTER_H
#define NL_INTERFACE_POINTER_H

#include "nel/misc/resource_ptr.h"

class CInterfaceElement *getInterfaceResource(const std::string &key);

/** Interface element ptr
 *  This pointer uses the NLMISC::CResourcePtr
 */
template<class TPtr>
class CInterfacePtr
{
public:
	static void *getResource (const std::string &key)
	{
		if (key.empty())
			return NULL;
		else
			return dynamic_cast<TPtr*>(getInterfaceResource(key));
	}

	/* In FINAL_VERSION, use an interface pointer without memory or cpu overhead because we don't need the runtime reloading feature. */
#if FINAL_VERSION
	typedef NLMISC::CStaticResourcePtr<TPtr, std::string, CInterfacePtr<TPtr> > TInterfacePtr;
#else // FINAL_VERSION
	typedef NLMISC::CResourcePtr<TPtr, std::string, CInterfacePtr<TPtr> > TInterfacePtr;
#endif // FINAL_VERSION
};

// Some pointers
typedef CInterfacePtr<class CInterfaceElement>::TInterfacePtr	CInterfaceElementPtr;
typedef CInterfacePtr<class CInterfaceGroup>::TInterfacePtr		CInterfaceGroupPtr;
typedef CInterfacePtr<class CCtrlTextButton>::TInterfacePtr		CCtrlTextButtonPtr;
typedef CInterfacePtr<class CViewText>::TInterfacePtr			CViewTextPtr;
typedef CInterfacePtr<class CViewTextMenu>::TInterfacePtr		CViewTextMenuPtr;
typedef CInterfacePtr<class CViewTextMenu>::TInterfacePtr		CViewTextMenuPtr;
typedef CInterfacePtr<class CCtrlBase>::TInterfacePtr			CCtrlBasePtr;
typedef CInterfacePtr<class CCtrlBaseButton>::TInterfacePtr		CCtrlBaseButtonPtr;
typedef CInterfacePtr<class CGroupContainer>::TInterfacePtr		CGroupContainerPtr;

#endif // NL_INTERFACE_POINTER_H

/* End of interface_pointer.h */
