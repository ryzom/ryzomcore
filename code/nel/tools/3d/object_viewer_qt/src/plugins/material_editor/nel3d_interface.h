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
	class CUsrShaderManager;
	class UDriver;
	class UScene;
	class U3dMouseListener;
}

namespace MaterialEditor
{
	/// Material Property, holds the user shader parameters as string ( for the editor )
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

		/// Creates a string representation of the type id
		static std::string typeIdToString( unsigned char type );

		/// Turns the type id string back to Id
		static unsigned char typeStringToId( const std::string &s );

		std::string id;
		std::string label;
		unsigned char type;
		std::string value;

	private:
		static const char *idToString[];
	};


	/// Proxy class for the rendering pass
	class CRenderPassProxy
	{
	public:
		CRenderPassProxy( NL3D::SRenderPass *p )
		{
			pass = p;
		}

		~CRenderPassProxy(){}

		/// Retrieves the rendering properties as a vector
		void getProperties( std::vector< SMatProp > &v );

		/// Clears the properties and then copies the ones from the vector specified
		void setProperties( std::vector< SMatProp > &v );

		/// Retrieves the name of the pass
		void getName( std::string &name );

		/// Sets the name of the pass
		void setName( const std::string &name );

		/// Returns the reference ( just a string ) to the user shader associated
		void getShaderRef( std::string &s );

		/// Sets the reference ( just a string ) to the user shader associated
		void setShaderRef( const std::string &s );

		/// Retrieves a single rendering property
		bool getProperty( const std::string &name, SMatProp &p );

		/// Changes a single rendering property
		bool changeProperty( const SMatProp &p );

	private:
		NL3D::SRenderPass *pass;
	};

	/// Proxy class for the dynamic material
	class CNelMaterialProxy
	{
	public:
		CNelMaterialProxy( NL3D::CDynMaterial *mat )
		{
			material = mat;
		}

		~CNelMaterialProxy(){}

		/// Retrieves the list of rendering passes
		void getPassList( std::vector< std::string > &l );

		/// Adds a new pass
		void addPass( const char *name );

		/// Removes the specified pass, if exists
		void removePass( const char *name );

		/// Moves the pass up by one position
		void movePassUp( const char *name );

		/// Moves the pass down by one position
		void movePassDown( const char *name );

		/// Renames the specified pass
		void renamePass( const char *from, const char *to );

		/// Retrieves the specified pass, by position
		CRenderPassProxy getPass( unsigned long i );

		/// Retrieves the specified pass, by name
		CRenderPassProxy getPass( const char *name );

		bool isEmpty() const{
			if( material == NULL )
				return true;
			else
				return false;
		}

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

	struct SFogSettings
	{
		bool enable;
		float start;
		float end;
		unsigned char color[ 4 ];

		SFogSettings()
		{
			enable = false;
			start = 0.0f;
			end = 0.0f;
			color[ 0 ] = 0.0f;
			color[ 1 ] = 0.0f;
			color[ 2 ] = 0.0f;
			color[ 3 ] = 0.0f;
		}

	};

	struct SLightInfo
	{
		enum LightType
		{
			Directional,
			Point,
			Spot
		};

		bool enabled;
		unsigned char type;
		float posOrDir[ 3 ];
		float ambColor[ 3 ];
		float diffColor[ 3 ];
		float specColor[ 3 ];
		float constAttn;
		float linAttn;
		float quadAttn;

		SLightInfo()
		{
			enabled = true;
			type = Directional;
			posOrDir[ 0 ] = posOrDir[ 1 ] = posOrDir[ 2 ] = 0.0f;
			ambColor[ 0 ] = ambColor[ 1 ] = ambColor[ 2 ] = 255;
			diffColor[ 0 ] = diffColor[ 1 ] = diffColor[ 2 ] = 255;
			specColor[ 0 ] = specColor[ 1 ] = specColor[ 2 ] = 255;
			constAttn = 1.0f;
			linAttn = quadAttn = 0.0f;
		}

	};

	/// Proxy class for Nel3D, so the material editor and Nel3D can interface
	class CNel3DInterface
	{
	public:
		CNel3DInterface();
		~CNel3DInterface();

		/// Load a material for the current (sub)object
		bool loadMaterial( const char *fname );

		/// Save the current (sub)object's material
		bool saveMaterial( const char *fname );

		/// Generate materials from the current (sub)object(s) "old" material(s)
		void genMaterials();

		/// Creates new material(s) for the current (sub)object(s)
		void newMaterial();

		/// Makes the specified sub-material current
		bool selectSubMaterial( int id );

		/// Returns a proxy object to the current sub-material
		CNelMaterialProxy getMaterial();

		/// Retrieves a list of user shaders loaded
		void getShaderList( std::vector< std::string > &v );

		/// Retrieves the specified user shader if exists
		bool getShaderInfo( const std::string &name, SShaderInfo &info );

		/// Updates a user shader
		bool updateShaderInfo( const SShaderInfo &info );

		/// Adds a new user shader
		bool addShader( const SShaderInfo &info );

		/// Removes a user shader
		bool removeShader( const std::string &name );

		/// Loads the user shaders
		void loadShaders();

		/// Saves the specified user shader
		void saveShader( const std::string &name );

		/// Deletes the specified user shader
		void deleteShader( const std::string &name );

		/// Sets up the viewport widget
		void initViewPort( unsigned long wnd, unsigned long w, unsigned long h );

		/// Shuts down the viewport widget
		void killViewPort();

		/// Resizes the viewport widget
		void resizeViewPort( unsigned long w, unsigned long h );
		NL3D::UDriver* getDriver(){ return driver; }

		/// Clears the scene then adds a cube
		bool addCube();

		/// Clears the scene then adds a sphere
		bool addSphere();

		/// Clears the scene then add a cylinder
		bool addCylinder();

		/// Clears the scene the adds a teapot
		bool addTeaPot();


		/// Clears the scene then loads a shape
		bool loadShape( const std::string &fileName );

		/// Clears the scene, as the name suggests
		void clearScene();

		/// Sends the input events to Nel3D
		void updateInput();

		/// Renders the scene
		void renderScene();

		unsigned long getShapeMatCount() const;

		void getFogSettings( SFogSettings &s );
		void setFogSettings( const SFogSettings &s );

		unsigned char getMaxLights() const;
		void getLightInfo( unsigned char light, SLightInfo &info );
		void setLightInfo( unsigned char light, const SLightInfo &info );

		void setBGColor( unsigned char R, unsigned char G, unsigned char B, unsigned char A ){
			bgColor[ 0 ] = R;
			bgColor[ 1 ] = G;
			bgColor[ 2 ] = B;
			bgColor[ 3 ] = A;
		}

	private:
		void setupCamera();

		unsigned long subMatId;

		NL3D::CUsrShaderManager *shaderManager;
		NL3D::UDriver *driver;
		NL3D::UScene *scene;
		NL3D::U3dMouseListener *mouseListener;
		unsigned char bgColor[ 4 ];
	};
}


#endif


