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

#include <nel/misc/types_nl.h>
#include "profiling.h"

// NeL includes
#include <nel/3d/u_driver.h>

// Project includes
#include "misc.h"
#include "sound_manager.h"

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;

////////////
// EXTERN //
////////////
extern UDriver *Driver;

/////////////
// GLOBALS //
/////////////
bool Profiling = false; // Are we in Profile mode?
uint ProfileNumFrame = 0;
bool WantProfiling = false;
bool ProfilingVBLock = false;
bool WantProfilingVBLock = false;

// ********************************************************************

/// Test Profiling and run?
void testLaunchProfile()
{
	if(!WantProfiling)
		return;

	// comes from ActionHandler
	WantProfiling= false;

#ifdef _PROFILE_ON_
	if( !Profiling )
	{
		// start the bench.
		NLMISC::CHTimer::startBench();
		ProfileNumFrame = 0;
		Driver->startBench();
		if (SoundMngr)
			SoundMngr->getMixer()->startDriverBench();
		// state
		Profiling= true;
	}
	else
	{
		// end the bench.
		if (SoundMngr)
			SoundMngr->getMixer()->endDriverBench();
		NLMISC::CHTimer::endBench();
		Driver->endBench();


		// Display and save profile to a File.
		CLog	log;
		CFileDisplayer	fileDisplayer(NLMISC::CFile::findNewFile(getLogDirectory() + "profile.log"));
		CStdDisplayer	stdDisplayer;
		log.addDisplayer(&fileDisplayer);
		log.addDisplayer(&stdDisplayer);
		// diplay
		NLMISC::CHTimer::displayHierarchicalByExecutionPathSorted(&log, CHTimer::TotalTime, true, 48, 2);
		NLMISC::CHTimer::displayHierarchical(&log, true, 48, 2);
		NLMISC::CHTimer::displayByExecutionPath(&log, CHTimer::TotalTime);
		NLMISC::CHTimer::display(&log, CHTimer::TotalTime);
		NLMISC::CHTimer::display(&log, CHTimer::TotalTimeWithoutSons);
		Driver->displayBench(&log);

		if (SoundMngr)
			SoundMngr->getMixer()->displayDriverBench(&log);

		// state
		Profiling= false;
	}
#endif	// #ifdef _PROFILE_ON_
}

// ********************************************************************

/// Test ProfilingVBLock and run?
void testLaunchProfileVBLock()
{
	// If running, must stop for this frame.
	if(ProfilingVBLock)
	{
		std::vector<string> strs;
		Driver->endProfileVBHardLock(strs);
		nlinfo("Profile VBLock");
		nlinfo("**************");
		for(uint i=0;i<strs.size();i++)
		{
			nlinfo(strs[i].c_str());
		}
		ProfilingVBLock= false;

		// Display additional info on allocated VBHards
		nlinfo("VBHard list");
		nlinfo("**************");
		Driver->profileVBHardAllocation(strs);
		for(uint i=0;i<strs.size();i++)
		{
			nlinfo(strs[i].c_str());
		}
		strs.clear();
		Driver->endProfileIBLock(strs);
		nlinfo("Profile Index Buffer Lock");
		nlinfo("**************");
		for(uint i=0;i<strs.size();i++)
		{
			nlinfo(strs[i].c_str());
		}
		ProfilingVBLock= false;

		// Display additional info on allocated VBHards
		/*
		nlinfo("Index Buffer list");
		nlinfo("**************");
		Driver->profileIBAllocation(strs);
		for(uint i=0;i<strs.size();i++)
		{
			nlinfo(strs[i].c_str());
		}
		*/
	}

	// comes from ActionHandler
	if(WantProfilingVBLock)
	{
		WantProfilingVBLock= false;
		ProfilingVBLock= true;
		Driver->startProfileVBHardLock();
		Driver->startProfileIBLock();
	}
}

/* end of file */