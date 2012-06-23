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

#ifndef NL_MEM_DISPLAYER_H
#define NL_MEM_DISPLAYER_H

#include "types_nl.h"
#include "displayer.h"

#include <deque>
#include <string>


namespace NLMISC {


/**
 * Display into a string vector
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CMemDisplayer : public IDisplayer
{
public:
	/// Constructor
	CMemDisplayer (const char *displayerName = "");

	/// Set Parameter of the displayer if not set at the ctor time
	void			setParam (uint32 maxStrings = 50);

	/// Write N last line into a displayer (InfoLog by default)
	void			write (CLog *log = NULL, bool quiet=true);
	void			write (std::string &str, bool crLf=false);

	const std::deque<std::string>	&lockStrings () { _CanUseStrings = false; return _Strings; }

	void unlockStrings() { _CanUseStrings = true; }

	void clear () { if (_CanUseStrings) _Strings.clear (); }

protected:
	/// Put the string into the file.
    virtual void	doDisplay ( const CLog::TDisplayInfo& args, const char *message );

	bool						_NeedHeader;

	uint32						_MaxStrings;	// number of string in the _Strings queue (default is 50)

	bool						_CanUseStrings;

	std::deque<std::string>		_Strings;

};

/**
 * Same as CMemDisplayer but only display the text (no line, no date, no process...)
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2002
 */
class CLightMemDisplayer : public CMemDisplayer
{
public:
	/// Constructor
	CLightMemDisplayer (const char *displayerName = "") : CMemDisplayer(displayerName) { }

protected:
	/// Put the string into the file.
    virtual void	doDisplay ( const CLog::TDisplayInfo& args, const char *message );
};



} // NLMISC


#endif // NL_MEM_DISPLAYER_H

/* End of mem_displayer.h */
