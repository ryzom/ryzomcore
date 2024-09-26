// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

class CPatchAllocator;

#ifdef NEL_3DSMAX_SHARED_EXPORTS
#define NEL_3DSMAX_SHARED_API __declspec(dllexport)
#else
#define NEL_3DSMAX_SHARED_API __declspec(dllimport)
#endif

extern NEL_3DSMAX_SHARED_API CPatchAllocator& GetAllocator();

extern NEL_3DSMAX_SHARED_API NLMISC::INelContext &GetSharedNelContext();

/* end of file */
