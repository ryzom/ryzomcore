// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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

#include "nel3d_interface.h"
#include "nel/3d/dynamic_material.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/file.h"

namespace MaterialEditor
{

	const char *SMatProp::idToString[] =
	{
		"Color",
		"Vector4",
		"Float",
		"Double",
		"Int",
		"Uint",
		"Matrix4",
		"Texture"
	};

	std::string SMatProp::typeIdToString( unsigned char id )
	{
		if( id >= EType_count )
			return std::string();
		else
			return std::string( idToString[ id ] );
	}

	unsigned char SMatProp::typeStringToId( const std::string &s )
	{
		for( unsigned char i = 0; i < EType_count; i++ )
			if( s == idToString[ i ] )
				return i;
		return 0;
	}

	void CRenderPassProxy::getProperties( std::vector< SMatProp > &v )
	{
		uint32 count = pass->count();
		for( uint32 i = 0; i < count; i++ )
		{
			const NL3D::SDynMaterialProp &p = *( pass->getProperty( i ) );
			
			SMatProp prop;
			prop.id = p.prop;
			prop.label = p.label;
			prop.type = p.type;
			prop.value = p.value;

			v.push_back( prop );
		}
	}

	void CRenderPassProxy::setProperties( std::vector< SMatProp > &v )
	{
		pass->clear();
		NL3D::SDynMaterialProp p;
		
		std::vector< SMatProp >::iterator itr = v.begin();
		while( itr != v.end() )
		{
			p.prop = itr->id;
			p.label = itr->label;
			p.type = itr->type;
			p.value = itr->value;

			pass->addProperty( p );

			++itr;
		}
	}

	void CRenderPassProxy::getName( std::string &name )
	{
		pass->getName( name );
	}

	void CRenderPassProxy::setName( const std::string &name )
	{
		pass->setName( name );
	}





	void CNelMaterialProxy::getPassList( std::vector< std::string > &l )
	{
		material->getPassList( l );
	}

	void CNelMaterialProxy::addPass( const char *name )
	{
		NL3D::SRenderPass pass;
		pass.setName( name );
		material->addPass( pass );
	}

	void CNelMaterialProxy::removePass( const char *name )
	{
		material->removePass( name );
	}

	void CNelMaterialProxy::movePassUp( const char *name )
	{
		material->movePassUp( name );
	}

	void CNelMaterialProxy::movePassDown( const char *name )
	{
		material->movePassDown( name );
	}

	void CNelMaterialProxy::renamePass( const char *from, const char *to )
	{
		material->renamePass( from, to );
	}

	CRenderPassProxy CNelMaterialProxy::getPass( unsigned long i )
	{
		if( material->count() >= i )
			return CRenderPassProxy( NULL );
		else
			return CRenderPassProxy( material->getPass( i ) );
	}

	CRenderPassProxy CNelMaterialProxy::getPass( const char *name )
	{
		return CRenderPassProxy( material->getPass( name ) );
	}


	CNel3DInterface::CNel3DInterface()
	{
		mat = new NL3D::CDynMaterial();
	}

	CNel3DInterface::~CNel3DInterface()
	{
	}

	bool CNel3DInterface::loadMaterial( const char *fname )
	{
		NLMISC::CIFile file;
		if( !file.open( fname, true ) )
			return false;

		NLMISC::CIXml xml;		
		if( !xml.init( file ) )
			return false;

		newMaterial();
		mat->serial( xml );
		file.close();

		return true;
	}

	bool CNel3DInterface::saveMaterial( const char *fname )
	{
		NLMISC::COFile file;
		if( !file.open( fname, false, true ) )
			return false;
		
		NLMISC::COXml xml;
		if( !xml.init( &file ) )
			return false;

		mat->serial( xml );
		xml.flush();
		file.close();

		return true;
	}

	void CNel3DInterface::newMaterial()
	{
		delete mat;
		mat = new NL3D::CDynMaterial();
	}

	CNelMaterialProxy CNel3DInterface::getMaterial()
	{
		return CNelMaterialProxy( mat );
	}
}

