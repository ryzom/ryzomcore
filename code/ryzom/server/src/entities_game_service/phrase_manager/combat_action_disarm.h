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



#ifndef RY_COMBAT_ACTION_DISARM_H
#define RY_COMBAT_ACTION_DISARM_H

#include "combat_action.h"

class CCombatPhrase;

class CCombatActionDisarm : public CCombatAction
{
public:
	CCombatActionDisarm()
	{
	}

	/// build from an ai action
	virtual bool initFromAiAction( const CStaticAiAction *aiAction, CCombatPhrase *phrase );

	// validate
	virtual bool validate(CCombatPhrase * phrase, std::string &errorCode) 
	{ return true; }

	// apply
	virtual void apply(CCombatPhrase *phrase);

	/// apply on entity
	virtual void applyOnEntity( CEntityBase *entity, float successFactor );
protected:	
	/// apply combat action effects on target
	void applyOnTarget( uint8 targetIndex, CCombatPhrase *phrase);

private:
	/// pointer on acting entity
	CEntityBaseRefPtr	_ActingEntity;

};


#endif // RY_COMBAT_ACTION_DISARM_H

/* End of combat_action_disarm.h */





















