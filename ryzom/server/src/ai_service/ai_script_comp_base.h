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

#ifndef _SCRIPT_COMP_BASE_H_
#define _SCRIPT_COMP_BASE_H_

// The abstract fight script component, split from ai_script_comp.h so that
// sheets.h (compiled into the ai sheets library) does not see any concrete
// component class: with the concrete definitions in view, the optimizer may
// speculatively devirtualize destructor calls in CCreature and emit
// references to vtables that only exist in the AI service objects.

class	CSpawnBot;

class	CFightScriptComp
		:public	NLMISC::CRefCount
{
public:
	CFightScriptComp()
	{}
	virtual ~CFightScriptComp()
	{}
	virtual	std::string	toString() const = 0;

	virtual	bool	update(CSpawnBot	&bot)	const = 0;	//	returns true if it behaves normally, false if there a problem and callers may not consider it behaves normally.
																	//	for instance ONCE may not consider that this call happened.
	virtual	void	remove(CFightScriptComp	*child)
	{}
protected:
private:
};

#endif
