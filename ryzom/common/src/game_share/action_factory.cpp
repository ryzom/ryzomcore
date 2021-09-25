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



#include "stdpch.h"
#include "action_factory.h"


//
// Using
//

using namespace std;

using namespace NLMISC;
using namespace NLNET;

namespace CLFECOMMON {

//
// Variables
//

CActionFactory *CActionFactory::Instance = NULL;


//
// Functions
//

CActionFactory::CActionFactory ()
{
	CActionSint64::init();

	_VolatilePosition = NULL;
	uint	i;
	for (i=0; i<NB_VISUAL_PROPERTIES; ++i)
		_VolatileActions[i] = NULL;
}


CActionFactory::~CActionFactory()
{
	uint nbDeleted = 0;
	for ( std::vector<TActionStore>::iterator iv=RegisteredAction.begin(); iv!=RegisteredAction.end(); ++iv )
	{
		TActionStore& actionStore = (*iv);
		for ( std::vector<CAction*>::iterator ias=actionStore.second.begin(); ias!=actionStore.second.end(); ++ias )
		{
			delete (*ias);
			++nbDeleted;
		}
	}
	//nldebug( "Deleted %u actions", nbDeleted );
}


uint CActionFactory::getNbActionsInStore()
{
	uint nb = 0;
	for ( std::vector<TActionStore>::iterator iv=RegisteredAction.begin(); iv!=RegisteredAction.end(); ++iv )
	{
		TActionStore& actionStore = (*iv);
		nb += (uint)actionStore.second.size();
	}
	return nb;
}

uint CActionFactory::getNbActionsInStore(uint code)
{
	return (uint)RegisteredAction[code].second.size();
}


void CActionFactory::registerAction (uint32 code, CAction *(*creator)())
{
/*
	CRegisteredAction::iterator it = RegisteredAction.find (code);
	if (it != RegisteredAction.end ())
	{
		nlerror ("the code %u already registered in the CActionFactory", code);
	}

	RegisteredAction.insert (CRegisteredAction::value_type(code, creator));
*/

	if (RegisteredAction.size() > code && RegisteredAction[code].first != NULL)
	{
		nlerror ("The code %u already registered in the CActionFactory", code);
	}

	if (code >= 256)
	{
		nlerror ("Cannot register action code %d because it exceeds 255", code);
	}

	if (RegisteredAction.size() <= code)
		RegisteredAction.resize(code+1, make_pair((CAction *(*)())NULL, vector<CAction*>()));

	RegisteredAction[code].first = creator;
}

CAction *CActionFactory::create (TCLEntityId slot, TActionCode code)
{
	if (RegisteredAction.size() <= code || RegisteredAction[code].first == NULL)
	{
		nlwarning ("CActionFactory::create() try to create an unknown action (%u)", code);
		return NULL;
	}
	else if (RegisteredAction[code].second.empty())
	{
		// no action left in the store
		CAction		*action = RegisteredAction[code].first (); // execute the factory function
		//nlinfo( "No action in store for code %u, creating action (total %u, total for code %u)", code, getNbActionsInStore(), getNbActionsInStore(action->Code) );
		action->Code = code;
		action->PropertyCode = code;	// default, set the property code to the action code (see create(TProperty,TPropIndex))
		action->Slot = slot;
		action->reset();
		return action;
	}
	else
	{
		// pop an action off the store
		CAction		*action = RegisteredAction[code].second.back();
		//nlinfo( "Found action in store for code %u (total %u, total for code %u)", code, getNbActionsInStore(), getNbActionsInStore(action->Code) );
		RegisteredAction[code].second.pop_back();
		action->reset();
		action->Slot = slot;
		action->PropertyCode = code;
		return action;
	}
}


/* Create the action from a property code, fills property index and fill the internal propindex if needed
 * (it assumes the frontend and the client have the same mapping property/propindex).
 */
CAction *CActionFactory::createByPropIndex( TCLEntityId slot, TPropIndex propIndex )
{
	CAction *action;

	switch ( propIndex )
	{
	case PROPERTY_POSITION: // same as propertyId
		{
			action = create( slot, ACTION_POSITION_CODE );
			break;
		}
	default:
		{
#ifdef NL_DEBUG
			nlassert( propIndex < NB_VISUAL_PROPERTIES );
#endif
			action = create( slot, ACTION_SINT64 );
			((CActionSint64*)action)->setNbBits( propIndex );
			break;
		}
	}
	action->PropertyCode = propIndex;
	return action;
}


void CActionFactory::remove (CAction *&action)
{
	if (action != NULL)
	{
		RegisteredAction[action->Code].second.push_back(action);
		//nlinfo( "Inserting action in store for code %u (total %u, total for code %u)", action->Code, getNbActionsInStore(), getNbActionsInStore(action->Code) );
		action = NULL;
	}
}

void CActionFactory::remove (CActionImpulsion *&action)
{
	if (action != NULL)
	{
		CAction*	ptr = static_cast<CAction*>(action);
		remove(ptr);
		action = NULL;
	}
}


/* Pack an action to a bit stream. Set transmitTimestamp=true for server-->client,
 * false for client-->server. If true, set the current gamecycle.
 */
void CActionFactory::pack (CAction *action, NLMISC::CBitMemStream &message, NLMISC::TGameCycle /* currentCycle */ )
{
	//H_BEFORE(FactoryPack);
	//sint32 val = message.getPosInBit ();

	//

	if (action->Code < 4)
	{
		// short code (0 1 2 3)
		bool shortcode = true;
		uint32 code = action->Code;
		message.serialAndLog1 (shortcode);
		message.serialAndLog2 (code, 2);
	}
	else
	{
		bool shortcode = false;
		message.serialAndLog1 (shortcode);
		message.serialAndLog1 (action->Code);
	}

	action->pack (message);
	//H_AFTER(FactoryPack);

	//OLIV: nlassertex (message.getPosInBit () - val == (sint32)CActionFactory::getInstance()->size (action), ("CActionFactory::pack () : action %d packed %u bits, should be %u, size() is wrong", action->Code, message.getPosInBit () - val, CActionFactory::getInstance()->size (action)));

//	nlinfo ("ac:%p pack one action in message %d %hu %u %d", action, action->Code, (uint16)(action->CLEntityId), val, message.getPosInBit()-val);
}


/* Pack an action to a bit stream, for server-->client, actioncode < 4 only
 */
//void CActionFactory::packFast( CAction *action, NLMISC::CBitMemStream& message, NLMISC::TGameCycle currentCycle )


/* Unpack some actions from a bit stream. Set transmitTimestamp=true for server-->client,
 * false for client-->server. If true, set the current gamecycle.
 */
void CActionFactory::unpack (NLMISC::CBitMemStream &message, std::vector <CAction *>& actions, NLMISC::TGameCycle /* currentCycle */ )
{
	actions.clear ();

	static int n = 0;
	n++;
	while ((sint32)message.length() * 8 - message.getPosInBit () >= 8)
	{
		TActionCode code;

		bool shortcode;
		message.serial (shortcode);

		if (shortcode)
		{
			code = 0;
			uint32 val;
			message.serial (val, 2);
			code = (TActionCode) val;
		}
		else
		{
			message.serial (code);
		}

		CAction *action = create (INVALID_SLOT, code);

		//nlinfo ("m%d size: p:%d s:%d c:%d (actionsize: %d) slot:%hu", n, message.getPosInBit (), message.length() * 8, code, action->size(), (uint16)action->CLEntityId);

		if (action == NULL)
		{
			nlwarning ("Unpacking an action with unknown code, skip it (%u)", code);
		}
		else
		{
			action->unpack (message);
			actions.push_back (action);
		}
	}
}


/* Unpack an action from a bit stream.
 */
CAction *CActionFactory::unpack (NLMISC::CBitMemStream &message, NLMISC::TGameCycle /* currentCycle */ )
{
	CAction	*action = NULL;

	if ((sint32)message.length() * 8 - message.getPosInBit () >= 8)
	{
		TActionCode code;

		bool shortcode;
		message.serial (shortcode);

		if (shortcode)
		{
			code = 0;
			uint32 val;
			message.serial (val, 2);
			code = (TActionCode) val;
		}
		else
		{
			message.serial (code);
		}

		action = create (INVALID_SLOT, (TActionCode)code);

		if (action == NULL)
		{
			nlwarning ("Unpacking an action with unknown code, skip it (%u)", code);
		}
		else
		{
			action->unpack (message);
		}
	}

	return action;
}


/*
 * Return the size IN BITS, not in bytes
 */
uint32 CActionFactory::size (CAction *action)
{
	// If you change this size, please update IMPULSE_ACTION_HEADER_SIZE in the front-end

	/*
	 * Warning: when calculating bit sizes, don't forget to multiply sizeof by 8
	 */
	uint32 headerBitSize;

	// size of the code

	if (action->Code < 4)
		headerBitSize = 1 + 2;
	else
		headerBitSize = 1 + (sizeof (action->Code) * 8);

	return headerBitSize + action->size ();
}


void	CActionFactory::initVolatileProperties()
{
	for (uint i=PROPERTY_ORIENTATION; i<NB_VISUAL_PROPERTIES; ++i)
		_VolatileActions[i] = (CActionSint64*)createByPropIndex( 0, i );

	_VolatilePosition = (CActionPosition*)(createByPropIndex( 0, PROPERTY_POSITION ));
}

void	CActionFactory::releaseVolatileProperties()
{
	for (uint i=PROPERTY_ORIENTATION; i<NB_VISUAL_PROPERTIES; ++i)
	{
		CAction*	action = _VolatileActions[i];
		remove(action);
		_VolatileActions[i] = NULL;
	}

	CAction*	action = _VolatilePosition;
	remove(action);
	_VolatilePosition = NULL;
}


}


NLMISC_COMMAND( displayActionVectors, "Display the size of action vector in factory", "" )
{
	uint nbActionsInStore = CLFECOMMON::CActionFactory::getInstance()->getNbActionsInStore();
	log.displayNL( "%u actions in factory store", nbActionsInStore );
	return true;
}

