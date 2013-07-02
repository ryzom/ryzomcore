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


#ifndef NEL3D_INTERFACE_H
#define NEL3D_INTERFACE_H

#include <vector>
#include <string>

namespace NL3D
{
	class CDynMaterial;
	struct SRenderPass;
}

namespace MaterialEditor
{

	struct SMatProp
	{
		enum EType
		{
			Color,
			Vector4,
			Float,
			Double,
			Int,
			Uint,
			Matrix4,
			Texture
		};

		std::string id;
		std::string label;
		unsigned char type;
		std::string value;
	};

	class CRenderPassProxy
	{
	public:
		CRenderPassProxy( NL3D::SRenderPass *p )
		{
			pass = p;
		}

		~CRenderPassProxy(){}

		void getProperties( std::vector< SMatProp > &v );
		void setProperties( std::vector< SMatProp > &v );
		void getName( std::string &name );
		void setName( const std::string &name );

	private:
		NL3D::SRenderPass *pass;
	};


	class CNelMaterialProxy
	{
	public:
		CNelMaterialProxy( NL3D::CDynMaterial *mat )
		{
			material = mat;
		}

		~CNelMaterialProxy(){}

		void getPassList( std::vector< std::string > &l );

		void addPass( const char *name );
		void removePass( const char *name );
		void movePassUp( const char *name );
		void movePassDown( const char *name );
		void renamePass( const char *from, const char *to );

		CRenderPassProxy getPass( unsigned long i );
		CRenderPassProxy getPass( const char *name );

	private:
		NL3D::CDynMaterial *material;
	};


	/// Proxy class for Nel3D, so the material editor and Nel3D can interface
	class CNel3DInterface
	{
	public:
		CNel3DInterface();
		~CNel3DInterface();

		bool loadMaterial( const char *fname );
		bool saveMaterial( const char *fname );
		void newMaterial();

		CNelMaterialProxy getMaterial();

	private:
		NL3D::CDynMaterial *mat;
	};
}


#endif


