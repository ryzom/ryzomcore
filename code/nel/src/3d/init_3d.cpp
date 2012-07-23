// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std3d.h"


#include "nel/3d/init_3d.h"
#include <cfloat>

namespace NL3D
{

void init3d ()
{
	// Init for windows
#ifdef NL_OS_WINDOWS
	// Enable FPU exceptions

	// Enable divid by zero and  overflow exception
	_control87 (_EM_INVALID|_EM_DENORMAL/*|_EM_ZERODIVIDE|_EM_OVERFLOW*/|_EM_UNDERFLOW|_EM_INEXACT, _MCW_EM);
#endif // NL_OS_WINDOWS
}

} // NL3D
