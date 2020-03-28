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



#ifndef NL_ACTION_FACTORY_H
#define NL_ACTION_FACTORY_H

//
// Includes
//

#include <map>
#include <vector>

#include <nel/misc/types_nl.h>

#include "entity_types.h"
#include "action.h"
#include "action_sint64.h"
#include "action_position.h"


namespace CLFECOMMON {

class CActionPosition;
//class CActionSint64;

//
// Classes
//

class CActionFactory
{
public:

	/// Singleton access
	static CActionFactory *getInstance ()
	{
		if (Instance == NULL)
			Instance = new CActionFactory;
		return Instance;
	}

	static bool isInstanceAllocated()
	{
		return Instance != NULL;
	}

	/// Singleton release
	static void release()
	{
		if (Instance)
		{
			Instance->releaseVolatileProperties();
			delete Instance;
			Instance = NULL;
		}
	}

	~CActionFactory();

	/** Register an action with a code.
	 * \param code the code associate with this action. at the end, it should be an huffman code (todo)
	 * \param creator a function that returns a new instances of this registered class
	 */
	void registerAction (uint32 code, CAction *(*creator)());

	//uint getNbBits( TProperty pr

	/** Create an instance of the action associated with the code. The allocation will be done with a special memory
	 * management so you could *not* delete this pointer. Use the factory function to delete them.
	 * \param code the code of the action you want to instanciate
	 * \return a pointer to the new instance of this code
	 */
	CAction *create ( TCLEntityId slot, TActionCode code );

	/** Create the action from a property code, fills property index and fill the internal propindex if needed
	 * (it assumes the frontend and the client have the same mapping property/propindex).
	 */
	CAction *createByPropIndex( TCLEntityId slot, TPropIndex index );

	/** Pack an action to a bit stream.
	 */
	void pack (CAction *action, NLMISC::CBitMemStream &message, NLMISC::TGameCycle currentCycle=0);

	/* Pack an action to a bit stream, for server-->client, actioncode < 4 only
	*/
	void packFast( CAction *action, NLMISC::CBitMemStream& message/*, NLMISC::TGameCycle currentCycle*/ )
	{
		//nlassert( action->Code < 4 );

		/*uint32 timestampDelta = currentCycle - action->GameCycle;
		action->TransmitTimeStamp = true;
		if ( timestampDelta > 15 ) // clamp to 4bit
		{
			timestampDelta = 15;
		}
		uint32 value = (1 << 14) | (action->Code << 12) | (timestampDelta << 8) | action->CLEntityId;
		message.serial( value, 15 );	*/
		// TODO: for properties, remove all (except timestamp?)

		action->pack (message);
	}

	/** Unpack some actions from a bit stream. Set transmitTimestamp=true for server-->client,
	 * false for client-->server. If true, set the current gamecycle.
	 */
	void unpack (NLMISC::CBitMemStream &message, std::vector <CAction *>& actions, NLMISC::TGameCycle currentCycle=0);

	/** Unpack an action from a bit stream. Set transmitTimestamp=true for server-->client,
	 * false for client-->server. If true, set the current gamecycle.
	 */
	CAction *unpack (NLMISC::CBitMemStream &message, NLMISC::TGameCycle currentCycle=0);

	/**
	 * Return the size of the action IN BITS, not in bytes
	 */
	uint32 size (CAction *action);

	/*uint32 sizeFast( CAction *action )
	{
		if ( action->Code == ACTION_POSITION_CODE )
#ifdef TEST_POSITION_CORRECTNESS
			return 1+2+4+8 + 16+16+16 + 32+32;
#else
			return 1+2+4+8 + 16+16+16;
#endif
		else // ACTION_SINT64:
			return 1+2+4+8 + 4+64;
	}*/

	/** Release (delete) the action and set action to NULL.
	 * \param action pointer to the action instance
	 */
	void remove (CAction *&action);

	/** Release (delete) the impulse action
	 * \param impulse action pointer to the action instance
	 */
	void remove (CActionImpulsion *&action);

	uint getNbActionsInStore();
	uint getNbActionsInStore(uint code);


public:

	void	initVolatileProperties();

	CActionSint64*		getVolatilePropAction(TCLEntityId slot, TPropIndex index)
	{
		CActionSint64*	action = _VolatileActions[index];
		action->Slot = slot;
		return action;
	}

	CActionPosition*	getVolatilePositionAction(TCLEntityId slot)
	{
		CActionPosition*	action = _VolatilePosition;
		action->Slot = slot;
		return action;
	}

	void	releaseVolatileProperties();

private:

	/** The ctor is private because CActionFactory is a singelton and you can't instanciate it, use getInstance() instead.
	 */
	CActionFactory ();

//	typedef std::map<uint32, CAction *(*)()> CRegisteredAction;

	typedef std::pair<CAction *(*)(), std::vector<CAction*> >	TActionStore;
	typedef std::vector<TActionStore>		CRegisteredAction;

	CRegisteredAction RegisteredAction;

	static CActionFactory *Instance;

	CActionSint64*		_VolatileActions[NB_VISUAL_PROPERTIES];
	CActionPosition*	_VolatilePosition;
};

}

#endif // NL_ACTION_FACTORY_H

/* End of action_factory.h */
