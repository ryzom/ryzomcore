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



#ifndef CL_CONTEXTUAL_CURSOR_H
#define CL_CONTEXTUAL_CURSOR_H

/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"


/**
 * Class to manage contextual cursor.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CContextualCursor
{
public:
	/// Callback function to call per context.
	typedef void (*TFunc) (void);
	typedef void (*TFunc2) (bool,bool);

protected:
	typedef struct
	{
		float	distMax;
		TFunc	checkFunc;
		TFunc2	execFunc;
		std::string cursor;
		bool		isString;		// True if this cursor is a string cursor and not an icon cursor
	}TFunctions;

	typedef std::map<std::string, TFunctions> TContext;

	TContext _Contexts;

	/// Current Context.
	std::string _Context;

public:
	/// Constructor
	CContextualCursor();

	void release();

	/// Insert a context and associate the function. If the context already exist -> replace the function if 'replace' = true.
	void add(bool isString, const std::string &contextName, const std::string &texture, float distMax, TFunc checkFunc, TFunc2 execFunc, bool replace = true);
	/// Remove a context.
	void del(const std::string &contextName);

	// Select a nex context.
	bool context(const std::string &contextName, float dist = 0, const ucstring &cursName = ucstring(""));
	inline const std::string &context() const {return _Context;}

	// Check if there is an entity under the cursor.
	void check();
	/// Execute the Action according to the cursor state.
	void execute(bool rightClick, bool dblClick);
};


#endif // CL_CONTEXTUAL_CURSOR_H

/* End of contextual_cursor.h */
