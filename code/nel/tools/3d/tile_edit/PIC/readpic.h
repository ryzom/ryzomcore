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

#ifndef	_READPIC_H_
#define	_READPIC_H_

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif


#include <string>
#include <vector>

#include <nel/misc/types_nl.h>
#include <nel/misc/rgba.h>

//============================================================
// API.
//============================================================


bool	PIC_LoadPic(std::string Path, std::vector<NLMISC::CBGRA> &Tampon, uint &Width, uint &Height);



#endif