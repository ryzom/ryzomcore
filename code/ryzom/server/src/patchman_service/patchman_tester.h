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

#ifndef PATCHMAN_TESTER_H
#define PATCHMAN_TESTER_H

//-----------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/sstring.h"


//-----------------------------------------------------------------------------
// namespace PATCHMAN
//-----------------------------------------------------------------------------

namespace PATCHMAN
{
	//-----------------------------------------------------------------------------
	// class CPatchmanTester
	//-----------------------------------------------------------------------------

	class CPatchmanTester
	{
	public:
		virtual ~CPatchmanTester() {}

		// this is a singleton so it has a getInstance() method to get to the singleton instance
		static CPatchmanTester& getInstance();

		// public interface...

		// clear everything
		virtual void clear() =0;

		// clear loaded script(s) but leave state intact
		virtual void clearScript() =0;

		// clear state variables but leave scripts intact
		virtual void clearState() =0;

		// load a script file
		virtual void loadScript(const NLMISC::CSString& fileName) =0;

		// set a state variable
		virtual void set(const NLMISC::CSString& variableName,const NLMISC::CSString& value) =0;
		virtual void set(const NLMISC::CSString& variableName,sint32 value) =0;

		// trigger an event
		virtual void trigger(const NLMISC::CSString& eventName) =0;

		// debug routine - display internal state info
		virtual void dump(NLMISC::CLog& log) =0;

		// debugging routine - displays the syntax help for the patchtest script files
		virtual void help(NLMISC::CLog& log) =0;
	};

} // end of namespace

#endif

