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
		f.xmlPush( "property" );

		f.xmlPush( "id" );
		f.serial( prop );
		f.xmlPop();

		f.xmlPush( "label" );
		f.serial( label );
		f.xmlPop();

		f.xmlPush( "type" );
		f.serial( type );
		f.xmlPop();

		f.xmlPush( "value" );
		f.serial( value );
		f.xmlPop();

		f.xmlPop();
	}

	void SRenderPass::serial( NLMISC::IStream &f )
	{
		f.xmlPush( "pass" );

		f.xmlPush( "name" );
		f.serial( name );
		f.xmlPop();

		f.xmlPush( "shader" );
		f.serial( shaderRef );
		f.xmlPop();

		f.xmlPush( "properties" );

		if( !f.isReading() )
		{
			uint32 n = properties.size();
			f.xmlPush( "count" );
			f.serial( n );
			f.xmlPop();

			std::vector< SDynMaterialProp >::iterator itr = properties.begin();
			while( itr != properties.end() )
			{
				itr->serial( f );
				++itr;
			}
		}
		else
		{
			uint32 n;
			f.xmlPush( "count" );
			f.serial( n );
			f.xmlPop();

			for( uint32 i = 0; i < n; i++ )
			{
				SDynMaterialProp prop;
				prop.serial( f );
				properties.push_back( prop );
			}
		}

		f.xmlPop();

		f.xmlPop();
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

	bool SRenderPass::changeProperty( const std::string &name, const SDynMaterialProp &prop )
	{
		std::vector< SDynMaterialProp >::iterator itr = properties.begin();
		while( itr != properties.end() )
		{
			if( itr->prop == name )
				break;
			++itr;
		}
		if( itr == properties.end() )
			return false;

		itr->prop  = prop.prop;
		itr->label = prop.label;
		itr->type  = prop.type;
		itr->value = prop.value;

		return true;
	}


	const SDynMaterialProp* SRenderPass::getProperty( uint32 i ) const
	{
		if( i >= properties.size() )
			return NULL;

		return &( properties[ i ] );
	}

	CDynMaterial::CDynMaterial()
	{
		reconstruct();
	}

	CDynMaterial::~CDynMaterial()
	{
		clear();
	}

	CDynMaterial& CDynMaterial::operator=( const CDynMaterial &other )
	{
		if( &other != this )
		{
			clear();

			std::vector< SRenderPass* >::const_iterator itr = other.passes.begin();
			while( itr != other.passes.end() )
			{
				SRenderPass *pass = new SRenderPass();
				*pass = *(*itr);
				passes.push_back( pass );
				++itr;
			}
		}

		return *this;
	}

	void CDynMaterial::reconstruct()
	{
		clear();
		SRenderPass *p = new SRenderPass();
		p->setName( "pass1" );
		passes.push_back( p );
	}

	void CDynMaterial::clear()
	{
		std::vector< SRenderPass* >::iterator itr = passes.begin();
		while( itr != passes.end() )
		{
			delete *itr;
			++itr;
		}
		passes.clear();
	}

	void CDynMaterial::serial( NLMISC::IStream &f )
	{
		f.xmlPush( "Material" );

		int version = f.serialVersion( 1 );

		f.xmlPush( "passes" );

		if( !f.isReading() )
		{
			uint32 n = passes.size();
			f.xmlPush( "count" );
			f.serial( n );
			f.xmlPop();

			std::vector< SRenderPass* >::iterator itr = passes.begin();
			while( itr != passes.end() )
			{
				(*itr)->serial( f );
				++itr;
			}
		}
		else
		{
			clear();
			uint32 n;
			f.xmlPush( "count" );
			f.serial( n );
			f.xmlPop();

			for( uint32 i = 0; i < n; i++ )
			{
				SRenderPass *pass = new SRenderPass();
				pass->serial( f );
				passes.push_back( pass );
			}
		}

		f.xmlPop();

		f.xmlPop();
	}

	void CDynMaterial::addPass( const SRenderPass &pass )
	{
		std::string n;
		std::string name;
		pass.getName( name );

		std::vector< SRenderPass* >::iterator itr = passes.begin();
		while( itr != passes.end() )
		{
			(*itr)->getName( n );
			if( n == name )
				break;
			++itr;
		}
		if( itr != passes.end() )
			return;

		SRenderPass *p = new SRenderPass();
		*p = pass;
		passes.push_back( p );
	}

	void CDynMaterial::removePass( const std::string &name )
	{
		std::string n;
		std::vector< SRenderPass* >::iterator itr = passes.begin();
		while( itr != passes.end() )
		{
			(*itr)->getName( n );
			if( n == name )
				break;
			++itr;
		}

		if( itr != passes.end() )
		{
			delete *itr;
			passes.erase( itr );
		}
	}

	void CDynMaterial::renamePass( const std::string &from, const std::string &to )
	{
		std::string n;
		std::vector< SRenderPass* >::iterator itr = passes.begin();
		while( itr != passes.end() )
		{
			(*itr)->getName( n );
			if( n == from )
				break;
			++itr;
		}

		if( itr != passes.end() )
			(*itr)->setName( to );
	}

	void CDynMaterial::movePassUp( const std::string &name )
	{
		std::string n;
		uint32 i = 0;
		for( i = 0; i < passes.size(); i++ )
		{
			passes[ i ]->getName( n );
			if( n == name )
				break;
		}

		if( i >= passes.size() )
			return;

		if( i == 0 )
			return;

		SRenderPass *temp = passes[ i ];
		passes[ i ] = passes[ i - 1 ];
		passes[ i - 1 ] = temp;
	}

	void CDynMaterial::movePassDown( const std::string &name )
	{
		std::string n;
		uint32 i = 0;
		for( i = 0; i < passes.size(); i++ )
		{
			passes[ i ]->getName( n );
			if( n == name )
				break;
		}

		if( i >= passes.size() )
			return;

		if( i == ( passes.size() - 1 ) )
			return;

		SRenderPass *temp = passes[ i ];
		passes[ i ] = passes[ i + 1 ];
		passes[ i + 1 ] = temp;
	}

	SRenderPass* CDynMaterial::getPass( const std::string &name )
	{
		std::string n;
		std::vector< SRenderPass* >::iterator itr = passes.begin();
		while( itr != passes.end() )
		{
			(*itr)->getName( n );
			if( n == name )
				break;
			++itr;
		}
		if( itr == passes.end() )
			return NULL;
		else
			return *itr;
	}

	SRenderPass* CDynMaterial::getPass( uint32 i )
	{
		if( i >= passes.size() )
			return NULL;
		else
			return passes[ i ];
	}

	void CDynMaterial::getPassList( std::vector< std::string > &l )
	{
		std::vector< SRenderPass* >::iterator itr = passes.begin();
		while( itr != passes.end() )
		{
			std::string name;
			
			(*itr)->getName( name );
			l.push_back( name );
			
			++itr;
		}
	}
}


