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

#include "nel/3d/u_scene.h"
#include "nel/3d/u_instance_group.h"

#include "entity_fx.h"
#include "ig_client.h"


// map of Fx entities
TMapEntityFx EntityFx;

using namespace std;
using namespace NLMISC;
using namespace NL3D;

////////////
// GLOBAL //
////////////

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_FX_Entities )

extern UScene			*Scene;

/*
 * Constructor
 */
CEntityFx::CEntityFx( const std::string& FxName, const std::string& InstanceName, const CVector& Position, const CQuat& Rotation )
{
	_FxName = FxName;
	_InstanceName = InstanceName;
	_InstanceName = _InstanceName + ".ps";
	_Position = Position;
	_Rotation = Rotation;
	_StatusFx = SStatusFx();
}

CEntityFx::~CEntityFx()
{
	deleteInstance();
}

// startFx
void CEntityFx::startFx( void )
{
	UInstance instance = Scene->createInstance( _InstanceName );

	if(!instance.empty())
	{
		instance.setTransformMode( UTransformable::RotQuat );
		instance.setPos( _Position );
		instance.setRotQuat( _Rotation );

		instance.setClusterSystem(NULL);

		_FxInstance.cast (instance);
		if (_FxInstance.empty())
		{
			deleteInstance();
		}
	}
}

/// isTerminated return true if fx is finish
bool CEntityFx::isTerminated( void )
{
	if(!_FxInstance.empty())
	{
		if( _FxInstance.isSystemPresent() )
		{
			return (! _FxInstance.isValid() );
		}
	}
	return false;
}

/// deleteInstance
void CEntityFx::deleteInstance( void)
{
	if(!_FxInstance.empty())
	{
		if(Scene)
			Scene->deleteInstance( _FxInstance );
		_FxInstance = NULL;
	}
}


// create new Fx entity
void newFx(const string& FxName, const string& InstanceName, const CVector& Position, const CQuat& Rotation )
{
	if ( EntityFx.find( FxName ) != EntityFx.end() )
	{
		nlerror( "Fx %s already exist", FxName.c_str() );
	}
	else
	{
		CEntityFx *pEntityFx = new CEntityFx( FxName, InstanceName, Position, Rotation );
		EntityFx.insert( make_pair( FxName, pEntityFx) );
	}
}

// delete Fx entity
void deleteFx(const std::string& FxName)
{
	TMapEntityFx::iterator it = EntityFx.find( FxName );

	if( it != EntityFx.end() )
	{
		delete (*it).second;
		EntityFx.erase( it );
	}
}

// Start Fx entity
void startFx(const std::string& FxName)
{
	TMapEntityFx::iterator it = EntityFx.find( FxName );

	if( it != EntityFx.end() )
	{
		(*it).second->startFx();
	}
}

// managed Fx entities
void manageFxEntities(void)
{
	H_AUTO_USE ( RZ_Client_FX_Entities )
	for( TMapEntityFx::iterator it = EntityFx.begin(); it != EntityFx.end(); ++it )
	{
		if( (*it).second->isTerminated() )
		{
			(*it).second->deleteInstance();
		}
	}
}
