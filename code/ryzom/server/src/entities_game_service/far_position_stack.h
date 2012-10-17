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

#ifndef NL_FAR_POSITION_STACK_H
#define NL_FAR_POSITION_STACK_H

#include "nel/misc/types_nl.h"
#include "game_share/far_position.h"
#include "game_share/persistent_data.h"
#include <vector>


/**
 * Stack of far positions
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2006
 */
class CFarPositionStack
{
public:

	DECLARE_PERSISTENCE_METHODS

	typedef CFarPosition T;

	/// Constructor
	CFarPositionStack() {}

	/// Return true if the stack is empty
	bool			empty() const { return _Vec.empty(); }

	/// Return the number of elements in the stack
	uint			size() const { return (uint)_Vec.size(); }

	/// Return the latest pushed position (no bound check)
	const T&		top() const { return _Vec.back(); }
	
	/// Push a position on top of the stack
	void			push( const T& pos ) { _Vec.push_back( pos ); }

	/// Pop (remove the top position) off the stack
	void			pop() { _Vec.pop_back(); }

	/// Return the latest push position (non const) (no bound check)
	T&				topToModify() { return _Vec.back(); }

	/// Return the specified position in the stack (no bound check)
	const T&		operator[](uint i) const { return _Vec[i]; }

	/// Replace a position in the stack (no bound check) (debug only)
	void			substFarPosition( uint index, const CFarPosition& newFarPos );

private:

	// Not a real stack because of the persistence methods
	std::vector<T>	_Vec;
};


#endif // NL_FAR_POSITION_STACK_H

/* End of far_position_stack.h */
