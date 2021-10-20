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

#include "std_afx.h"
#include "range_manager.h"


// here instanciate static fields of tange_manager

// the range manager for floats
CRangeManager<float>::TRangeMap CRangeManager<float>::_RangeMap ;

// the range manager for unsigned ints
CRangeManager<uint32>::TRangeMap CRangeManager<uint32>::_RangeMap ;

// the range manager for signed ints
CRangeManager<sint32>::TRangeMap CRangeManager<sint32>::_RangeMap ;
