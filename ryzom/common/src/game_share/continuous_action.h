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



#ifndef NL_CONTINUOUS_ACTION_H
#define NL_CONTINUOUS_ACTION_H

//
// Includes
//

#include "action.h"


namespace CLFECOMMON {

//
// Classes
//

class CContinuousAction : public CAction
{
public:
	/// Returns true always (continuous action by essence)
	bool			isContinuous() const { return true; }

	/*virtual bool	isDelta() const = 0;
	virtual bool	hasGaranty() const = 0;
	virtual void	setGaranty(bool g=true) = 0;
	virtual void	packDelta(const CAction::TValue &origin) = 0;
	virtual void	unpackDelta(const CAction::TValue &origin) = 0;*/
	virtual TValue	getValue() const = 0;
	//virtual TValue	getValue(const CAction::TValue &origin) const = 0;

protected:

	/// Default ctor that initialize Timeout value
	CContinuousAction ();

	friend class CActionFactory;
};

}

#endif // NL_CONTINUOUS_ACTION_H

/* End of continuous_action.h */
