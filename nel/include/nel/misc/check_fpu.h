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


#ifndef NL_CHECK_FPU_H
#define NL_CHECK_FPU_H


namespace NLMISC
{

class CFpuChecker
{
private:
	static int	_RefFpuCtrl;
	void check();
	void dumpFpu(int value);
public:
	CFpuChecker();
	~CFpuChecker();
};

}


// Enable define. Normal State is 0, to save CPU speed.
#define NL_CHECK_FPU_CONTROL_WORD 0

// Use those Macros
#if NL_CHECK_FPU_CONTROL_WORD
#define FPU_CHECKER NLMISC::CFpuChecker __fpc__;
#define FPU_CHECKER_ONCE { NLMISC::CFpuChecker __fpc__; }
#else
#define FPU_CHECKER
#define FPU_CHECKER_ONCE
#endif


#endif	// NL_CHECK_FPU_H

