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



#ifndef RY_MAGIC_ACTION_H
#define RY_MAGIC_ACTION_H

#include "nel/misc/types_nl.h"
#include "game_share/brick_families.h"
#include "game_share/egs_sheets/egs_sheets.h"
#include "game_share/mode_and_behaviour.h"
#include "phrase_utilities_functions.h"

class CMagicPhrase;
extern NLMISC::CRandom RandomGenerator;


/// Macro used to declare a Sabrina Magic action ( i.e. : effects represented by active bricks in a spell )
#define BEGIN_MAGIC_ACTION_FACTORY(_class_) \
class _class_##Factory : public IMagicActionFactory \
{\
public:\
	_class_##Factory()\
	{\
		IMagicActionFactory::init(); \

#define ADD_MAGIC_ACTION_TYPE(_type_)\
		for (uint i = 0; i < Factories->size(); i++ )\
		{\
			if ( (*Factories)[i].first == _type_ )\
			{\
				nlstop;\
			}\
		}\
Factories->push_back(std::make_pair( std::string(_type_) ,this));

#define END_MAGIC_ACTION_FACTORY(_class_) \
	};\
	IMagicAction * build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, uint & brickIndex, CMagicPhrase * phrase )\
	{\
		_class_ *inst = new _class_;\
		if ( !inst->build( actorRowId, bricks, brickIndex, phrase  ) ){delete inst;return NULL;} \
		return inst;\
	}\
};\
_class_##Factory* _class_##FactoryInstance = new _class_##Factory;


/**
 * Interface for magic actions
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class IMagicAction
{
public:

	/// build the action basic params, the call the addBrick virtual method to retrieve the specific params
	bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, uint & index, CMagicPhrase* phrase )
	{
		// get the skill
		if ( ! bricks.empty()  )
		{
			_Skill = bricks[index]->Skill;
			if ( _Skill == SKILLS::unknown )
			{
				uint idx = index;
				nlwarning( "<IMagicAction build> invalid skill in brick %s. We will iterate backwards through the brick queue to get a skill",bricks[index]->SheetId.toString().c_str() );
				while ( _Skill == SKILLS::unknown && idx > 0 )
					_Skill = bricks[idx--]->Skill;
				if ( _Skill == SKILLS::unknown)
				{
					nlwarning( "<IMagicAction build> no valid skill found in brick %s.",bricks[idx]->SheetId.toString().c_str() );
					return false;
				}
			}
		}
		else
		{
			nlwarning("<IMagicAction build> no brick in action...");
			return false;
		}
		while(1)
		{
			if ( index >= bricks.size() )
			{
				nlwarning("<IMagicAction build> index %u > bricks.size() %u",index, bricks.size());
				break;
			}
			if ( !bricks[index] )
			{
				nlwarning("<IMagicAction build> index  %u> bricks.size() %u",index, bricks.size());
				break;
			}
			INFOLOG("IN MAGIC ACTIOND : Parsing brick index %u value %s",index,bricks[index]->SheetId.toString().c_str() );

			bool end = false;;
			if ( !addBrick( *bricks[index], phrase,end ) )
				return false;
			index++;
			if ( end )
				break;
		}
		return true;
	}
	///\name virtual action methods
	//@{
	/// validate the action
	virtual bool validate(CMagicPhrase * phrase) = 0;
	/// apply the action
	virtual void apply( CMagicPhrase * phrase, float successFactor,MBEHAV::CBehaviour & behav, bool isMad ) = 0;
	/// add a brick to build the action
	virtual bool addBrick( const CStaticBrick & brick, CMagicPhrase * phrase, bool &effectEnd ) = 0;
	//@}
protected:
	/// the skill used in that action
	SKILLS::ESkills			_Skill;
};

/**
 * class factory for sabrina Magic Effect
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class IMagicActionFactory
{
public:
	/// clear the class factory
	static void clear();

	/**
	 * Build the desired step from a primitive node
	 * \param prim : the primitive node used to build the step
	 * \return a pointer on the built step (NULL if failure)
	 */
	inline static IMagicAction * buildAction( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, uint & brickIndex, CMagicPhrase * phrase )
	{
		// check validity
		if ( bricks.size()<=brickIndex )
		{
			nlwarning("<IMagicActionFactory buildAction> brick index %u and size is %u !",brickIndex, bricks.size());
			return NULL;
		}
		if ( !bricks[brickIndex] )
		{
			nlwarning("<IMagicActionFactory buildAction> brick index %u is NULL",brickIndex);
			return NULL;
		}
		if ( bricks[brickIndex]->Params.empty() )
		{
			nlwarning("<IMagicActionFactory buildAction>NASTY BUG: no param in brick %u : if we are there there is at least1 param for the effect type",brickIndex);
			return NULL;
		}
		if ( bricks[brickIndex]->Params[0]->id() != TBrickParam::MA )
		{
			nlwarning("<IMagicActionFactory buildAction>NASTY BUG: no param in brick %u : if we are there the first param should be a TBrickParam::MA",brickIndex);
			return NULL;
		}
		INFOLOG("MA: %s",((CSBrickParamMaType *)bricks[brickIndex]->Params[0])->Type.c_str());
		const std::string & type = ((CSBrickParamMaType *)bricks[brickIndex]->Params[0])->Type;

		//get appropriate factory
		for ( uint i = 0; i < Factories->size(); i++ )
		{
			if ( !NLMISC::nlstricmp( (*Factories)[i].first , type)  )
			{
				INFOLOG("MA: %s is managed by the system. Building action...",((CSBrickParamMaType *)bricks[brickIndex]->Params[0])->Type.c_str());
				return (*Factories)[i].second->build( actorRowId, bricks, brickIndex, phrase);
			}
		}
		nlwarning( "<IMagicActionFactory buildAction> the type MA:%s has no corresponding magic action class", type.c_str() );
		return NULL;
	}
protected:
	///\init the factories
	inline static void init()
	{		if( !Factories )
			Factories = new std::vector< std::pair< std::string , IMagicActionFactory* > >;
	}
	/**
	 * Create a step from parameters
	 * \param params : a vector of vector of strings describing the step params
	 * \return a pointer on the built step (NULL if failure)
	 */
	virtual IMagicAction * build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& brickIds, uint & index, CMagicPhrase * phrase ) = 0;
	
	///the phrase factories. We use a pointer here because we cant control the order inwhich ctor of static members are called
	static std::vector< std::pair< std::string , IMagicActionFactory* > >* Factories;
};

#endif // RY_MAGIC_ACTION_H

/* End of magic_action.h */


















