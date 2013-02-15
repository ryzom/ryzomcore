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



#ifndef RYZOM_FABER_ACTION_H
#define RYZOM_FABER_ACTION_H

//
#include "game_share/brick_families.h"
#include "game_share/item_type.h"
#include "egs_sheets/egs_static_game_item.h"
#include "egs_sheets/egs_sheets.h"

class CFaberPhrase;

/// Macro used to declare a Sabrina Faber action ( i.e. : Sword Faber ) 
#define FABER_ACTION_FACTORY(_class_,_type_) \
class _class_##Factory : public IFaberActionFactory \
{\
public:\
	_class_##Factory()\
	{\
		for (uint i = 0; i < Factories.size(); i++ ){ \
		if ( Factories[i].first == _type_){nlerror("<IFaberActionFactory buildPhrase> item type %s is affected to more than one class",ITEM_TYPE::toString(_type_).c_str() );}} \
		Factories.push_back(std::make_pair(_type_,this));\
	};\
protected:\
	IFaberAction * build( const TDataSetRow & actorRowId, CFaberPhrase * phrase )\
	{\
		_class_ *inst = new _class_;\
		if ( !inst->build( actorRowId, phrase  ) ){delete inst;return NULL;} \
		return inst;\
	}\
};\
_class_##Factory* _class_##FactoryInstance = new _class_##Factory;

// CFaber action, interface and common members
class IFaberAction
{
public:
	bool build( const TDataSetRow & actorRowId, CFaberPhrase* phrase )
	{
		if ( !checkSentenceValidity( phrase ) ) return false;
		return true;
	}
	virtual void apply(CFaberPhrase * phrase) = 0;
	virtual void systemApply(CFaberPhrase * phrase) = 0;
	
protected:
	//check sentence validity
	virtual bool checkSentenceValidity( CFaberPhrase * phrase ) = 0;
};

/**
 * class factory for sabrina Faber action
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class IFaberActionFactory
{
public:
	/// clear the class factory
	static void clear();

	/**
	 * Build the desired step from a primitive node
	 * \param prim : the primitive node used to build the step
	 * \return a pointer on the built step (NULL if failure)
	 */
	inline static IFaberAction * buildAction( const TDataSetRow & actorRowId, CFaberPhrase * phrase, ITEM_TYPE::TItemType type )
	{
		for ( uint i = 0; i < Factories.size(); i++ )
		{
			if ( Factories[i].first == type )
			{
				return Factories[i].second->build( actorRowId, phrase );
			}
		}
		nlwarning( "<IFaberActionFactory buildAction> the item type %s has no corresponding faber action class", ITEM_TYPE::toString( type ).c_str() );
		return NULL;
	}

protected:
	/**
	 * Create a step from parameters
	 * \param params : a vector of vector of strings describing the step params
	 * \return a pointer on the built step (NULL if failure)
	 */
	virtual IFaberAction * build( const TDataSetRow & actorRowId, CFaberPhrase * phrase ) = 0;

	static std::vector< std::pair< ITEM_TYPE::TItemType , IFaberActionFactory* > > Factories;
};

#endif // RYZOM_FABER_ACTION_H

/* End of faber_action.h */
