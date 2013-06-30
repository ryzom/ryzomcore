// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "nel/3d/dynamic_material.h"

namespace NL3D
{
	CDynMaterial::CDynMaterial()
	{
	}

	CDynMaterial::~CDynMaterial()
	{
	}

	void CDynMaterial::serial( NLMISC::IStream &f )
	{
		int version = f.serialVersion( 1 );

		std::vector< CDynMaterialProp >::const_iterator itr = properties.begin();
		while( itr != properties.end() )
		{
			f.serial( std::string( itr->prop ) );
			f.serial( std::string( itr->label ) );
			f.serial( uint8( itr->type ) );
			f.serial( std::string( itr->value ) );

			++itr;
		}
	}

	void CDynMaterial::addProperty( const CDynMaterialProp &prop )
	{
		std::vector< CDynMaterialProp >::const_iterator itr = properties.begin();
		while( itr != properties.end() )
		{
			if( itr->prop == prop.prop )
				break;
			++itr;
		}
		if( itr != properties.end() )
			return;

		properties.push_back( prop );
	}

	void CDynMaterial::removeProperty( const std::string &name )
	{
		std::vector< CDynMaterialProp >::iterator itr = properties.begin();
		while( itr != properties.end() )
		{
			if( itr->prop == name )
				break;
			++itr;
		}
		if( itr == properties.end() )
			return;

		properties.erase( itr );
	}

	void CDynMaterial::changeProperty( const std::string &name, const CDynMaterialProp &prop )
	{
		std::vector< CDynMaterialProp >::iterator itr = properties.begin();
		while( itr != properties.end() )
		{
			if( itr->prop == name )
				break;
			++itr;
		}
		if( itr == properties.end() )
			return;

		itr->prop  = prop.prop;
		itr->label = prop.label;
		itr->type  = prop.type;
		itr->value = prop.value;
	}

}


