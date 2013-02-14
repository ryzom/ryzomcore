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

#ifndef INTERFACE_COMMON_H
#define INTERFACE_COMMON_H

enum WindowsPriority
{
	WIN_PRIORITY_WORLD_SPACE  = 0,
	WIN_PRIORITY_LOWEST       = 1,
	WIN_PRIORITY_LOW          = 2,
	WIN_PRIORITY_NORMAL       = 3,
	WIN_PRIORITY_HIGH         = 4,
	WIN_PRIORITY_HIGHEST      = 5,
	WIN_PRIORITY_MAX          = 8
};

enum THotSpot
{
	Hotspot_BL = 36,	// 100100,
	Hotspot_BM = 34,	// 100010,
	Hotspot_BR = 33,	// 100001,
	Hotspot_ML = 20,	// 010100,
	Hotspot_MM = 18,	// 010010
	Hotspot_MR = 17,	// 010001
	Hotspot_TL = 12,	// 001100
	Hotspot_TM = 10,	// 001010
	Hotspot_TR = 9,		// 001001
	Hotspot_xR = 1,		// 000001
	Hotspot_xM = 2,		// 000010
	Hotspot_xL = 4,		// 000100
	Hotspot_Bx = 32,	// 100000
	Hotspot_Mx = 16,	// 010000
	Hotspot_Tx = 8,		// 001000
	Hotspot_TTAuto = 0,	// Special For Tooltip PosRef. Auto mode. see CCtrlBase and tooltip info
};

#define	DECLARE_UI_CLASS(_class_)					\
	virtual std::string	getClassName() {return #_class_;}		\
	static	NLMISC::IClassable	*creator() {return new _class_(CViewBase::TCtorParam());}
#define	REGISTER_UI_CLASS(_class_)  \
	class CRegisterUIClassHelper_##_class_ \
	{ \
	public: \
		CRegisterUIClassHelper_##_class_() \
		{ \
			NLMISC::CClassRegistry::init(); \
			NLMISC::CClassRegistry::registerClass(#_class_, _class_::creator, typeid(_class_).name()); \
		} \
	} RegisterUIClassHelper_##_class_;



#endif

