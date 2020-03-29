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
#include "effect_manager.h"
#include "effect_message.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(TEffectVector);

// init static members
TEntitiesEffectMap		CEffectManager::_Effects;
vector< CBasicEffect >	CEffectManager::_NewEffects;
vector< CBasicEffect >	CEffectManager::_RemovedEffects;
set< NLNET::TServiceId >	CEffectManager::_RegisteredServices;


//--------------------------------------------------------------
//					CEffectManager constructor
//--------------------------------------------------------------
CEffectManager::CEffectManager()
{
} // constructor //


//--------------------------------------------------------------
//				CEffectManager destructor
//--------------------------------------------------------------
CEffectManager::~CEffectManager()
{
	release();
} // destructor //


//--------------------------------------------------------------
//					CEffectManager::release
//--------------------------------------------------------------
void CEffectManager::release()
{
	TEntitiesEffectMap::iterator it = _Effects.begin();
	const TEntitiesEffectMap::iterator itEnd = _Effects.end();

	for ( ; it != itEnd ; ++it)
	{
		if ( (*it).second != NULL)
		{
			delete (*it).second;
		}
	}

	_Effects.clear();
	_RegisteredServices.clear();
	_RemovedEffects.clear();
	_NewEffects.clear();
} // release //


//--------------------------------------------------------------
//					CEffectManager::update
//--------------------------------------------------------------
void CEffectManager::update()
{
	H_AUTO(EffectManagerUpdate);

	if (  ! _RegisteredServices.empty() )
	{
		CAddEffectsMessage		addMsg;
		CRemoveEffectsMessage	removeMsg;

		std::vector< CBasicEffect >::const_iterator itNew = _NewEffects.begin();
		const std::vector< CBasicEffect >::const_iterator itNewEnd = _NewEffects.end();
		for ( ; itNew != itNewEnd ; ++itNew )
		{
			addMsg.addEffect( *itNew );
		}

		std::vector< CBasicEffect >::const_iterator itRemove = _RemovedEffects.begin();
		const std::vector< CBasicEffect >::const_iterator itRemoveEnd = _RemovedEffects.end();
		for ( ; itRemove != itRemoveEnd ; ++itRemove )
		{
			removeMsg.addEffect( *itRemove );
		}		

		set< NLNET::TServiceId >::const_iterator it;
		const set< NLNET::TServiceId >::const_iterator itEnd = _RegisteredServices.end();
		for (it = _RegisteredServices.begin() ; it != itEnd ; ++it)
		{
			if (!_NewEffects.empty())
				addMsg.send( (*it) );

			if (!_RemovedEffects.empty())
				removeMsg.send( (*it) );
		}
	}

	_NewEffects.clear();
	_RemovedEffects.clear();
} // update //>
