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

#include "game_share/brick_families.h"
#include "game_share/mode_and_behaviour.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "progression/progression_pve.h"
#include "phrase_manager/area_effect.h"
#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_ai_action.h"

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
	IMagicAction * build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, uint & brickIndex, CBuildParameters &buildParams, CMagicPhrase * phrase )\
	{\
		_class_ *inst = new _class_;\
		if ( !inst->build( actorRowId, bricks, brickIndex, buildParams, phrase ) ){delete inst;return NULL;} \
		return inst;\
	}\
};\
_class_##Factory* _class_##FactoryInstance = new _class_##Factory;

/**
 * class use to store parameters during phrase building from a set of bricks
 */
struct CBuildParameters
{
	uint16		BreakResistBrickPower;

	CBuildParameters() : BreakResistBrickPower(0)
	{
	}
};

/**
 * Interface for magic actions
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class IMagicAction
{
protected:
	enum TMagicActionState
	{
		New,
		Launched,
		Applied
	};
	
public:
	/// build the action from an ai action object
	virtual bool buildFromAiAction( const CStaticAiAction *aiAction ) { return false; }
	
	/// build the action basic params, the call the addBrick virtual method to retrieve the specific params
	bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, uint & index, CBuildParameters &buildParams, CMagicPhrase* phrase )
	{
		// get the skill
		if ( ! bricks.empty()  )
		{
			_Skill = SKILLS::unknown;
			uint idx = index;
			while ( _Skill == SKILLS::unknown && idx >= 0 )
			{
				if ( idx >= bricks.size() )
				{
					nlwarning("<IMagicAction build> index %u > bricks.size() %u",idx, bricks.size());
					break;
				}
				if ( !bricks[idx] )
				{
					nlwarning("<IMagicAction build> index  %u> bricks.size() %u",idx, bricks.size());
					break;
				}

				_Skill = bricks[idx]->getSkill(0);
				// as we have problems with brick skills, let's check them. All mandatory must have valid skills
				if ( _Skill == SKILLS::unknown && bricks[idx]->Family >= BRICK_FAMILIES::BeginMagicMandatory && bricks[idx]->Family <= BRICK_FAMILIES::EndMagicMandatory )
				{
					nlwarning( "<IMagicAction build> no valid skill in a mandatory magic brick %s.",bricks[idx]->SheetId.toString().c_str() );
					return false;
				}
				idx--;
			}
		}
		else
		{
			nlwarning("<IMagicAction build> no brick in action...");
			return false;
		}
		while(true)
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
			if ( !addBrick( *bricks[index], phrase,end, buildParams ) )
				return false;
			index++;
			if ( end )
				break;
		}
		return true;
	}
	
	// set skill used
	inline void setSkill(SKILLS::ESkills skill) { _Skill = skill; }
	
	///\name virtual action methods
	//@{
	/// validate the action
	virtual bool validate(CMagicPhrase * phrase, std::string &errorCode) = 0;
	
	/// launch the action
	virtual void launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						 const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport
					   ) = 0;
	
	/// apply the action
	virtual void apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
						sint32 vamp, float vampRatio, bool reportXp
					  ) = 0;
	
	/// add a brick to build the action
	virtual bool addBrick( const CStaticBrick & brick, CMagicPhrase * phrase, bool &effectEnd, CBuildParameters &buildParams ) = 0;
	//@}
	
protected:
	/// the skill used in that action
	SKILLS::ESkills			_Skill;
	/// the sheet id of the action brick if any
	NLMISC::CSheetId		_ActionBrickSheetId;
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
	inline static IMagicAction * buildAction( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, uint & brickIndex, CBuildParameters &buildParams, CMagicPhrase * phrase )
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

		const TBrickParam::IId* param = bricks[brickIndex]->Params[0];

		if ( param->id() != TBrickParam::MA )
		{
			nlwarning("<IMagicActionFactory buildAction>NASTY BUG: no param in brick %u : if we are there the first param should be a TBrickParam::MA",brickIndex);
			return NULL;
		}
		INFOLOG("MA: %s",((CSBrickParamMaType *)param)->Type.c_str());
		const std::string & type = ((CSBrickParamMaType *)param)->Type;

		//get appropriate factory
		for ( uint i = 0; i < Factories->size(); i++ )
		{
			if ( !NLMISC::nlstricmp( (*Factories)[i].first , type)  )
			{
				INFOLOG("MA: %s is managed by the system. Building action...",((CSBrickParamMaType *)param)->Type.c_str());
				return (*Factories)[i].second->build( actorRowId, bricks, brickIndex, buildParams, phrase);
			}
		}
		nlwarning( "<IMagicActionFactory buildAction> the type MA:%s has no corresponding magic action class", type.c_str() );
		return NULL;
	}
protected:
	///\init the factories
	inline static void init()
	{	
		if( !Factories )
			Factories = new std::vector< std::pair< std::string , IMagicActionFactory* > >;
	}
	/**
	 * Create a step from parameters
	 * \param params : a vector of vector of strings describing the step params
	 * \return a pointer on the built step (NULL if failure)
	 */
	virtual IMagicAction * build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& brickIds, uint & index, CBuildParameters &buildParams, CMagicPhrase * phrase ) = 0;
	
	///the phrase factories. We use a pointer here because we cant control the order inwhich ctor of static members are called
	static std::vector< std::pair< std::string , IMagicActionFactory* > >* Factories;
};

/**
 * class factory for sabrina Magic Effect, built from AI Actions
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class IMagicAiActionFactory
{
	NL_INSTANCE_COUNTER_DECL(IMagicAiActionFactory);
public:

	/// clear the class factory
	static void clear();

	/**
	 * Build the desired step from a primitive node
	 * \param prim : the primitive node used to build the step
	 * \return a pointer on the built step (NULL if failure)
	 */
	inline static IMagicAction * buildActionFromAiAction(const CStaticAiAction *aiAction, CMagicPhrase *phrase)
	{
#ifdef NL_DEBUG
		nlassert(aiAction);
		nlassert(phrase);
#endif
		const AI_ACTION::TAiActionType actionType = aiAction->getType();

		if ( actionType != AI_ACTION::EffectSpell && actionType != AI_ACTION::EoTSpell )
		{
			//get appropriate factory
			for ( uint i = 0; i < Factories->size(); i++ )
			{
				if ( (*Factories)[i].first == actionType )
				{
					INFOLOG(" Action type %s is managed by the system. Building action...",AI_ACTION::toString(actionType).c_str());
					return (*Factories)[i].second->buildFromAiAction(aiAction, phrase);
				}
			}
			nlwarning( "<IMagicAiActionFactory buildFromAiAction> the Ai Action type %s has no corresponding magic action class", AI_ACTION::toString(actionType).c_str() );
			return NULL;
		}
		else
		{
			AI_ACTION::TAiEffectType aiEffectType;
			if (actionType == AI_ACTION::EoTSpell)
			{
				aiEffectType = aiAction->getData().OTEffectSpell.EffectFamily;
			}
			else
			{
				aiEffectType = aiAction->getData().EffectSpell.EffectFamily;
			}
			
			if (aiEffectType != AI_ACTION::UnknownEffect)
			{
				//get appropriate factory
				for ( uint i = 0; i < SpecializedActionFactories->size(); i++ )
				{
					if ( (*SpecializedActionFactories)[i].first == aiEffectType )
					{
						INFOLOG(" AI Effect type %s is managed by the system. Building action...",AI_ACTION::toString(aiEffectType).c_str());
						return (*SpecializedActionFactories)[i].second->buildFromAiAction(aiAction, phrase);
					}
				}
				nlwarning( "<IMagicAiActionFactory buildFromAiAction> the Ai Effect type %s has no corresponding magic action class", AI_ACTION::toString(aiEffectType).c_str() );				
			}

			return NULL;
		}
	}
protected:
	///\init the factories
	inline static void init()
	{	
		if( !Factories )
			Factories = new std::vector< std::pair< AI_ACTION::TAiActionType , IMagicAiActionFactory* > >;
		if( !SpecializedActionFactories )
			SpecializedActionFactories = new std::vector< std::pair< AI_ACTION::TAiEffectType , IMagicAiActionFactory* > >;
	}
	/**
	 * Create a step from parameters
	 * \param params : a vector of vector of strings describing the step params
	 * \return a pointer on the built step (NULL if failure)
	 */
	virtual IMagicAction * buildFromAiAction( const CStaticAiAction *aiAction, CMagicPhrase *phrase ) = 0;
	
	///the phrase factories. We use a pointer here because we cant control the order inwhich ctor of static members are called
	static std::vector< std::pair< AI_ACTION::TAiActionType , IMagicAiActionFactory* > >* Factories;

	///the phrase factories. We use a pointer here because we cant control the order inwhich ctor of static members are called
	static std::vector< std::pair< AI_ACTION::TAiEffectType , IMagicAiActionFactory* > >* SpecializedActionFactories;
};

/**
 * Magic AI action template factory
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
template <class T> class CMagicAiActionTFactory : public IMagicAiActionFactory
{
public:
	explicit CMagicAiActionTFactory(AI_ACTION::TAiActionType type)
	{
		IMagicAiActionFactory::init();
		
#ifdef NL_DEBUG
		// check this type isn't used yet
		for (uint i = 0; i < Factories->size(); i++ )
		{
			if ( (*Factories)[i].first == type )
			{
				nlstop;
			}
		}
#endif
		// add factory
		Factories->push_back(std::make_pair( type ,this));
	};

	/// buildFromAiAction method
	IMagicAction * buildFromAiAction(const CStaticAiAction *aiAction, CMagicPhrase *phrase)
	{
		T *instance = new T;
		if (!instance)
		{
			nlwarning("<CMagicAiActionTFactory> failed to allocate element of template type");
			return 0;
		}
		if ( !instance->initFromAiAction(aiAction, phrase) )
		{
			delete instance;
			return 0;
		}
		return instance;
	}
};


/**
 * Magic AI action template factory
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
template <class T> class CMagicAiSpecializedActionTFactory : public IMagicAiActionFactory
{
public:
	explicit CMagicAiSpecializedActionTFactory(AI_ACTION::TAiEffectType effectType)
	{
		IMagicAiActionFactory::init();
		
#ifdef NL_DEBUG
		// check this type isn't used yet
		for (uint i = 0; i < SpecializedActionFactories->size(); i++ )
		{
			if ( (*SpecializedActionFactories)[i].first == effectType )
			{
				nlstop;
			}
		}
#endif
		// add factory
		SpecializedActionFactories->push_back(std::make_pair( effectType,this));		
	};

	/// buildFromAiAction method
	IMagicAction * buildFromAiAction(const CStaticAiAction *aiAction, CMagicPhrase *phrase)
	{
		T *instance = new T;
		if (!instance)
		{
			nlwarning("<CMagicAiSpecializedActionTFactory> failed to allocate element of template type");
			return 0;
		}
		if ( !instance->initFromAiAction(aiAction, phrase) )
		{
			delete instance;
			return 0;
		}
		return instance;
	}
};


#endif // RY_MAGIC_ACTION_H

/* End of magic_action.h */


















