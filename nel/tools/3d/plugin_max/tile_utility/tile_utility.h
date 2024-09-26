// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef __TILE_UTILITY__H
#define __TILE_UTILITY__H

#pragma conform(forScope, push)
#pragma conform(forScope, off)

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <assert.h>
#include <Max.h>
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <utilapi.h>
#include <plugapi.h>
#include <stdmat.h>

#undef _CRT_SECURE_NO_DEPRECATE

#pragma conform(forScope, pop)

#undef min
#undef max

#include "resource.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define RGBAddClassID (Class_ID(0x5621932, 0x565a6387))

#include "../nel_3dsmax_shared/string_common.h"

#endif // __TILE_UTILITY__H
