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



#ifndef RY_COMBAT_ACTION_H
#define RY_COMBAT_ACTION_H

#include "nel/misc/types_nl.h"
//
#include "game_share/egs_sheets/egs_static_brick.h"
//

class CCombatPhrase;


/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatAction
{
public:
	/// Constructor
	CCombatAction() : _CombatPhrase(NULL)
	{}

	/// build
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, uint &brickIndex, CCombatPhrase * phrase ) = 0;

	/// validate the combat action
	virtual bool validate(CCombatPhrase *phrase, std::string &errorCode) { return true; }

	/// apply combat action effects
	virtual void	apply(CCombatPhrase *phrase) {}

	/// set target
	inline void setTarget( const TDataSetRow &entityRowId) { _TargetRowId = entityRowId; }

protected:
	TDataSetRow		_ActorRowId;
	TDataSetRow		_TargetRowId;
	CCombatPhrase	*_CombatPhrase;

};


/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
/*
class CCombatActionFactory
{
public:
	/// clear the class factory
	static void clear();

	/**
	 * Build the desired step from a primitive node
	 * \param prim : the primitive node used to build the step
	 * \return a pointer on the built step (NULL if failure)
	 */
/*	inline static CCombatAction * buildAction( const TDataSetRow & actorRowId, const TDataSetRow &targetRowId, const std::vector< const CStaticBrick* >& bricks, uint & brickIndex, CCombatPhrase * phrase )
	{
		//get appropriate factory
		for ( uint i = 0; i < Factories->size(); i++ )
		{
			if ( (*Factories)[i].first == bricks[brickIndex]->Family )
			{
				return (*Factories)[i].second->build( actorRowId, bricks, brickIndex, phrase);
			}
		}
		nlwarning( "<CCombatActionFactory buildAction> the brick family %s has no corresponding magic action class", BRICK_FAMILIES::toString( bricks[0]->Family ).c_str() );
		return NULL;
	}
protected:
	///\init the factories
	inline static void init()
	{
//		if( !Factories )
//			Factories = new std::vector< std::pair< BRICK_FAMILIES::TBrickFamily , CCombatActionFactory* > >;
	}
	/**
	 * Create a step from parameters
	 * \param params : a vector of vector of strings describing the step params
	 * \return a pointer on the built step (NULL if failure)
	 */
//	virtual IMagicAction * build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& brickIds, uint & index, CMagicPhrase * phrase ) = 0;

	///the phrase factories. We use a pointer here because we cant control the order inwhich ctor of static members are called
//	static std::vector< std::pair< BRICK_FAMILIES::TBrickFamily , CCombatActionFactory* > >* Factories;
/*
};


/**
 * Combat action template factory
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
/*
template <class T> class CCombatActionTFactory : public CCombatActionFactory
{
public:
	/// build method
	T *build( const TDataSetRow & actorRowId, const TDataSetRow &targetRowId, const std::vector< const CStaticBrick* >& bricks, uint &brickIndex, CCombatPhrase * phrase )
	{
		T *instance = new T;
		if (!instance)
		{
			nlwarning("<CCombatActionFactory> failed to allocate element of template type");
			return 0;
		}
		if ( !instance->build(actorRowId, targetRowId, bricks, brickIndex, phrase) )
		{
			delete instance;
			return 0;
		}
		return instance:
	}
};


*/



#endif // RY_COMBAT_ACTION_H

/* End of combat_action.h */
