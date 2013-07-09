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
	class CShaderManager;
	class UDriver;
	class UScene;
	class U3dMouseListener;
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
			Texture,
			EType_count
		};

		static std::string typeIdToString( unsigned char type );
		static unsigned char typeStringToId( const std::string &s );

		std::string id;
		std::string label;
		unsigned char type;
		std::string value;

	private:
		static const char *idToString[];
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
		void getShaderRef( std::string &s );
		void setShaderRef( const std::string &s );

		bool getProperty( const std::string &name, SMatProp &p );
		bool changeProperty( const SMatProp &p );

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


	struct SShaderInfo
	{
		std::string name;
		std::string description;
		std::string vp;
		std::string fp;
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


		void getShaderList( std::vector< std::string > &v );
		bool getShaderInfo( const std::string &name, SShaderInfo &info );
		bool updateShaderInfo( const SShaderInfo &info );
		bool addShader( const SShaderInfo &info );
		bool removeShader( const std::string &name );

		void loadShaders();
		void saveShader( const std::string &name );
		void deleteShader( const std::string &name );

		void initViewPort( unsigned long wnd, unsigned long w, unsigned long h );
		void killViewPort();
		void resizeViewPort( unsigned long w, unsigned long h );
		NL3D::UDriver* getDriver(){ return driver; }

		bool loadShape( const std::string &fileName );
		void clearScene();
		void updateInput();
		void renderScene();

	private:
		void setupCamera();

		NL3D::CDynMaterial *mat;
		NL3D::CShaderManager *shaderManager;
		NL3D::UDriver *driver;
		NL3D::UScene *scene;
		NL3D::U3dMouseListener *mouseListener;
	};
}


#endif


