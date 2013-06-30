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
	void SDynMaterialProp::serial( NLMISC::IStream &f )
	{
		f.serial( std::string( prop ) );
		f.serial( std::string( label ) );
		f.serial( uint8( type ) );
		f.serial( std::string( value ) );
	}

	void SRenderPass::serial( NLMISC::IStream &f )
	{
		f.serial( std::string( name ) );

		if( !f.isReading() )
		{
			std::vector< SDynMaterialProp >::iterator itr = properties.begin();
			while( itr != properties.end() )
			{
				itr->serial( f );
				++itr;
			}
		}
		else
		{
			SDynMaterialProp prop;
			prop.serial( f );
			properties.push_back( prop );
		}

	}

	void SRenderPass::addProperty( const SDynMaterialProp &prop )
	{
		std::vector< SDynMaterialProp >::const_iterator itr = properties.begin();
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

	void SRenderPass::removeProperty( const std::string &name )
	{
		std::vector< SDynMaterialProp >::iterator itr = properties.begin();
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

	void SRenderPass::changeProperty( const std::string &name, const SDynMaterialProp &prop )
	{
		std::vector< SDynMaterialProp >::iterator itr = properties.begin();
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



	CDynMaterial::CDynMaterial()
	{
	}

	CDynMaterial::~CDynMaterial()
	{
	}

	void CDynMaterial::serial( NLMISC::IStream &f )
	{
		int version = f.serialVersion( 1 );

		if( !f.isReading() )
		{
			std::vector< SRenderPass >::iterator itr = passes.begin();
			while( itr != passes.end() )
			{
				itr->serial( f );
				++itr;
			}
		}
		else
		{
			SRenderPass pass;
			pass.serial( f );
			passes.push_back( pass );
		}
	}

	void CDynMaterial::addPass( const SRenderPass &pass )
	{
		std::string n;
		std::string name;
		pass.getName( name );

		std::vector< SRenderPass >::iterator itr = passes.begin();
		while( itr != passes.end() )
		{
			itr->getName( n );
			if( n == name )
				break;
			++itr;
		}
		if( itr != passes.end() )
			return;

		passes.push_back( pass );
	}

	void CDynMaterial::removePass( const std::string &name )
	{
		std::string n;
		std::vector< SRenderPass >::iterator itr = passes.begin();
		while( itr != passes.end() )
		{
			itr->getName( n );
			if( n == name )
				break;
			++itr;
		}

		if( itr != passes.end() )
			passes.erase( itr );
	}

	SRenderPass* CDynMaterial::getPass( const std::string &name )
	{
		std::string n;
		std::vector< SRenderPass >::iterator itr = passes.begin();
		while( itr != passes.end() )
		{
			itr->getName( n );
			if( n == name )
				break;
			++itr;
		}
		if( itr == passes.end() )
			return NULL;
		else
			return &( *itr );
	}
}


