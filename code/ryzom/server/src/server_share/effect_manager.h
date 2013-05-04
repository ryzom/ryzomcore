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



#ifndef RY_EFFECT_MANAGER_H
#define RY_EFFECT_MANAGER_H

#include "nel/misc/types_nl.h"

#include "game_share/base_types.h"
#include "basic_effect.h"

class TEffectVector : public std::vector<CBasicEffect>
{
	NL_INSTANCE_COUNTER_DECL(TEffectVector);
public:
};

//typedef std::vector< CBasicEffect >	TEffectVector;
typedef std::map< TDataSetRow, TEffectVector* > TEntitiesEffectMap;

/**
 * Singleton managing effects for EGS and AIS
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CEffectManager
{
public:
	/// Constructor
	CEffectManager();

	/// Destructor
	~CEffectManager();

	/// release method
	static void release();

	/// update method (should be called each tick)
	static void update();

	/// add a effect to given entity
	inline static void addEffect( const TDataSetRow &entityRow, const CBasicEffect &effect )
	{
		TEntitiesEffectMap::iterator it = _Effects.find(entityRow);
		if ( it != _Effects.end())
		{
			if ( (*it).second != NULL)
			{
				(*it).second->push_back(effect);
			}
			else
			{
				nlwarning("<CEffectManager::addEffect> NULL pointer instead of effect vector for entity RowId %u", entityRow.getIndex() );
				(*it).second = new TEffectVector();
				if ( (*it).second) 
					(*it).second->push_back(effect);				
			}
		}
		else
		{
			// create new vector
			TEffectVector *vector = new TEffectVector();
			if (vector)
			{
				vector->push_back(effect);
				_Effects.insert( std::make_pair(entityRow, vector) );
			}
		}

		_NewEffects.push_back(effect);
	}

	/// add a vector of spells to given entity
	inline static void addEffects( const TDataSetRow &entityRow, const std::vector<CBasicEffect> &spells )
	{
		TEntitiesEffectMap::iterator it = _Effects.find(entityRow);
		if ( it != _Effects.end())
		{
			if ( (*it).second != NULL)
			{
				(*it).second->insert( (*it).second->end(), spells.begin(), spells.end() );
			}
			else
			{
				nlwarning("<CEffectManager::addEffect> NULL pointer instead of effect vector for entity RowId %u", entityRow.getIndex() );
				(*it).second = new TEffectVector();
				if ( (*it).second) 
					(*it).second->insert( (*it).second->end(), spells.begin(), spells.end() );
			}
		}
		else
		{
			// create new vector
			TEffectVector *vector = new TEffectVector();
			if (vector)
			{
				vector->insert( vector->end(), spells.begin(), spells.end() );
				_Effects.insert( std::make_pair(entityRow, vector) );
			}
		}

		_NewEffects.insert(_NewEffects.end(), spells.begin(), spells.end() );
	}

	/// remove a effect
	inline static void removeEffect( const TDataSetRow &entityRow, uint32 effectId )
	{
		TEntitiesEffectMap::iterator it = _Effects.find(entityRow);
		if ( it == _Effects.end())
		{
			return;
		}

		if ( (*it).second == NULL)
		{
			nlwarning("<CEffectManager::addEffect> NULL pointer instead of effect vector for entity RowId %u", entityRow.getIndex() );
			return;
		}
		
		// parse vector
		TEffectVector::iterator itEffect;
		const TEffectVector::const_iterator itEnd = (*it).second->end();
		for ( itEffect = (*it).second->begin() ; itEffect != itEnd ; ++itEffect)
		{
			if ( (*itEffect).effectId() == effectId )
			{
				_RemovedEffects.push_back(*itEffect);
				(*it).second->erase(itEffect);

				// if last effect, erase entry
				if ((*it).second->empty())
				{
					delete (*it).second;
					(*it).second = NULL;
					_Effects.erase(it);
				}

				return;
			}
		}
	}

	/// remove an entity
	inline void removeEntity( const TDataSetRow &entityRow )
	{
		TEntitiesEffectMap::iterator it = _Effects.find(entityRow);
		if ( it != _Effects.end())
		{
			if ( (*it).second != NULL)
			{
				delete (*it).second;
				(*it).second = NULL;					
			}
			_Effects.erase(it);
		}
	}

	/// register a service
	inline void static registerService( NLNET::TServiceId serviceId ) 
	{
		_RegisteredServices.insert( serviceId );
	}

	/// unregister a service
	inline void static unregisterService( NLNET::TServiceId serviceId ) 
	{
		_RegisteredServices.erase( serviceId );
	}

private:
	/// map entities to their active spells
	static TEntitiesEffectMap			_Effects;

	/// the spells created since last update
	static std::vector< CBasicEffect >	_NewEffects;

	/// the spells deleted since last update
	static std::vector< CBasicEffect >	_RemovedEffects;

	/// list of services registered to new and removed spells/effects
	static std::set<NLNET::TServiceId>	_RegisteredServices;

};


#endif // RY_EFFECT_MANAGER_H

/* End of effect_manager.h */
