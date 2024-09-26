// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2011  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#if !defined(AFX_STDAFX_H__7B19FB21_D10C_11D4_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_STDAFX_H__7B19FB21_D10C_11D4_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

//#include "nel/misc/types_nl.h"
// Max SDK includes
//#define NOMINMAX
#include <assert.h>
#include <algorithm>
#include <max.h>
#include <stdmat.h>
#include <shaders.h>
#include <iparamb2.h>
#include <maxversion.h>
#if MAX_VERSION_MAJOR >= 14
#	include <maxscript/maxscript.h>
#else
#	include <MaxScrpt/maxscrpt.h>
#endif
//#include <parser.h>

// Character Studio SDK include
#include <bipexp.h>
#include <phyexp.h>

#undef min
#undef max

#include <string>
#include <vector>
#include <map>

#include "../nel_patch_lib/rpo.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/triangle.h"
#include "nel/misc/bsphere.h"
#include "nel/misc/string_common.h"
#include "nel/3d/quad_tree.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/light.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_STDAFX_H__7B19FB21_D10C_11D4_9CD4_0050DAC3A412__INCLUDED_)
