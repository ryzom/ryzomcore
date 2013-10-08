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
#include "nel/3d/usr_shader_manager.h"
#include "nel/3d/usr_shader_program.h"
#include "nel/3d/usr_shader_loader.h"
#include "nel/3d/usr_shader_saver.h"
#include "nel/3d/driver_user.h"
#include "nel/3d/scene_user.h"
#include "nel/3d/u_camera.h"
#include "nel/3d/u_instance.h"
#include "nel/3d/u_light.h"
#include "nel/3d/u_3d_mouse_listener.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

namespace MaterialEditor
{

	NL3D::UInstance currentShape;

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

	const NLMISC::CVariant::EVarType evartypes[] =
	{
		NLMISC::CVariant::Vector4,
		NLMISC::CVariant::Vector4,
		NLMISC::CVariant::Float,
		NLMISC::CVariant::Double,
		NLMISC::CVariant::Int,
		NLMISC::CVariant::UInt,
		NLMISC::CVariant::Matrix4,
		NLMISC::CVariant::String
	};

	NLMISC::CVariant::EVarType toEVarType( uint8 t )
	{
		NLMISC::CVariant::EVarType rt = NLMISC::CVariant::Float;

		unsigned long s = sizeof( evartypes ) / sizeof( NLMISC::CVariant::EVarType );

		if( t >= s )
			return rt;
		else
			rt = evartypes[ t ];

		return rt;

	}

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
			p.value.valueAsString( prop.value );

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
			p.value.fromString( itr->value, toEVarType( p.type ) );

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

	void CRenderPassProxy::getShaderRef( std::string &s )
	{
		pass->getShaderRef( s );
	}

	void CRenderPassProxy::setShaderRef( const std::string &s )
	{
		pass->setShaderRef( s );
	}

	bool CRenderPassProxy::getProperty( const std::string &name, SMatProp &p )
	{
		uint32 count = pass->count();
		uint32 i = 0;
		
		for( i = 0; i < count; i++ )
		{
			if( pass->getProperty( i )->prop == name )
				break;
		}

		if( i == count )
			return false;
		
		const NL3D::SDynMaterialProp *prop = pass->getProperty( i );
		p.id    = prop->prop;
		p.label = prop->label;
		p.type  = prop->type;
		prop->value.valueAsString( p.value );

		return true;
	}

	bool CRenderPassProxy::changeProperty( const SMatProp &p )
	{
		NL3D::SDynMaterialProp prop;
		prop.prop  = p.id;
		prop.label = p.label;
		prop.type  = p.type;
		prop.value.fromString( p.value, toEVarType( prop.type ) );

		return pass->changeProperty( prop.prop, prop );
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
		if( i >= material->getPassCount()  )
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
		shaderManager = new NL3D::CUsrShaderManager();
		driver = NULL;
		scene = NULL;
		mouseListener = NULL;
		subMatId = 0;
	}

	CNel3DInterface::~CNel3DInterface()
	{
		delete shaderManager;
		shaderManager = NULL;
		killViewPort();
	}

	bool CNel3DInterface::loadMaterial( const char *fname )
	{
		if( currentShape.empty() )
			return false;

		NLMISC::CIFile file;
		if( !file.open( fname, true ) )
			return false;

		NLMISC::CIXml xml;		
		if( !xml.init( file ) )
			return false;

		NL3D::CDynMaterial *mat = currentShape.getMaterial( subMatId ).getObjectPtr()->getDynMat();
		if( mat != NULL )
			mat->clear();
		else
		{
			currentShape.getMaterial( subMatId ).getObjectPtr()->createCleanDynMat();
			mat = currentShape.getMaterial( subMatId ).getObjectPtr()->getDynMat();
		}

		mat->serial( xml );
		file.close();

		return true;
	}

	bool CNel3DInterface::saveMaterial( const char *fname )
	{
		if( currentShape.empty() )
			return false;

		NLMISC::COFile file;
		if( !file.open( fname, false, true ) )
			return false;
		
		NLMISC::COXml xml;
		if( !xml.init( &file ) )
			return false;

		currentShape.getMaterial( subMatId ).getObjectPtr()->getDynMat()->serial( xml );

		xml.flush();
		file.close();

		return true;
	}

	void CNel3DInterface::genMaterials()
	{
		int c = currentShape.getNumMaterials();
		for( int i = 0; i < c; i++ )
			currentShape.getMaterial( i ).getObjectPtr()->createDynMat();
	}

	void CNel3DInterface::newMaterial()
	{
		if( currentShape.empty() )
			return;

		for( int i = 0; i < currentShape.getNumMaterials(); i++ )
			currentShape.getMaterial( i ).getObjectPtr()->createCleanDynMat();
	}

	bool CNel3DInterface::selectSubMaterial( int id )
	{
		if( currentShape.empty() )
			return false;

		if( currentShape.getNumMaterials() < id )
			return false;

		subMatId = id;

		return true;
	}

	CNelMaterialProxy CNel3DInterface::getMaterial()
	{
		NL3D::CDynMaterial *mat = NULL;
		if( !currentShape.empty() && ( currentShape.getNumMaterials() > 0 ) )
			mat = currentShape.getMaterial( subMatId ).getObjectPtr()->getDynMat();

		return CNelMaterialProxy( mat );
	}

	void CNel3DInterface::getShaderList( std::vector< std::string > &v )
	{
		shaderManager->getShaderList( v );
	}

	bool CNel3DInterface::getShaderInfo( const std::string &name, SShaderInfo &info )
	{
		NL3D::CUsrShaderProgram program;
		bool ok = shaderManager->getShader( name, &program );
		if( !ok )
			return false;

		std::string s;
		info.name = name;

		program.getDescription( s );
		info.description = s;

		program.getVP( s );
		info.vp = s;

		program.getFP( s );
		info.fp = s;

		return true;
	}

	bool CNel3DInterface::updateShaderInfo( const SShaderInfo &info )
	{
		NL3D::CUsrShaderProgram program;
		program.setName( info.name );
		program.setDescription( info.description );
		program.setVP( info.vp );
		program.setFP( info.fp );

		return shaderManager->changeShader( info.name, &program );
	}

	bool CNel3DInterface::addShader( const SShaderInfo &info )
	{
		NL3D::CUsrShaderProgram *program = new NL3D::CUsrShaderProgram();

		program->setName( info.name );
		program->setDescription( info.description );
		program->setVP( info.vp );
		program->setFP( info.fp );

		bool ok = shaderManager->addShader( program );
		if( !ok )
		{
			delete program;
			return false;
		}

		return true;
	}

	bool CNel3DInterface::removeShader( const std::string &name )
	{
		return shaderManager->removeShader( name );
	}

	void CNel3DInterface::loadShaders()
	{
		NL3D::CUsrShaderLoader loader;
		loader.setManager( shaderManager );
		loader.loadShaders( "./shaders" );
	}

	void CNel3DInterface::saveShader( const std::string &name )
	{
		NL3D::CUsrShaderSaver saver;
		saver.setManager( shaderManager );
		saver.saveShader( "./shaders", name );
		NL3D::CDriverUser *d = dynamic_cast< NL3D::CDriverUser* >( driver );
		if( d != NULL )
			d->getDriver()->reloadUserShaders();
	}

	void CNel3DInterface::deleteShader( const std::string &name )
	{
		NLMISC::CFile::deleteFile( "./shaders/"  + name + ".nlshdr" );
	}

	void CNel3DInterface::initViewPort( unsigned long wnd, unsigned long w, unsigned long h )
	{
		//driver = NL3D::UDriver::createDriver( 0, false, 0 );
		driver = NL3D::UDriver::createDriver( 0, NL3D::UDriver::OpenGl3, 0 );
		nlassert( driver != NULL );
		driver->setDisplay( (nlWindow)wnd, NL3D::UDriver::CMode( w, h, 32 ) );

		scene = driver->createScene( true );


		driver->enableFog( true );
		driver->setupFog( 1.0f, 5.0f, NLMISC::CRGBA::White );

		NL3D::ULight *ld;
		
		/*
		// Let's add a directional light!
		ld = NL3D::ULight::createLight();
		ld->setMode( NL3D::ULight::DirectionalLight );
		ld->setDirection( NLMISC::CVector( -100.0f, 100.0f, 100.0f ) );
		ld->setAmbiant( NLMISC::CRGBA::CRGBA( 0.1f, 0.1f, 0.1f, 0.1f ) );
		ld->setSpecular( NLMISC::CRGBA::White );
		ld->setDiffuse( NLMISC::CRGBA::White );
		driver->setLight( 0, *ld );
		driver->enableLight( 0, true );
		delete ld;
		*/
		
		// Let's add a point light as well!
		ld = NL3D::ULight::createLight();
		ld->setMode( NL3D::ULight::PointLight );
		ld->setPosition( NLMISC::CVector( 0.0f, 0.0f, 0.0f ) );
		ld->setAmbiant( NLMISC::CRGBA::CRGBA( 0.1f, 0.1f, 0.1f, 0.1f ) );
		ld->setSpecular( NLMISC::CRGBA::White );
		ld->setDiffuse( NLMISC::CRGBA::White );
		ld->setConstantAttenuation( 1.0f );
		ld->setLinearAttenuation( 0.0f );
		ld->setQuadraticAttenuation( 0.0f );
		driver->setLight( 1, *ld );
		driver->enableLight( 1, true );
		delete ld;
		ld = NULL;

		scene->enableLightingSystem( false );


		mouseListener = driver->create3dMouseListener();
		mouseListener->setMouseMode( NL3D::U3dMouseListener::nelStyle );
	}

	void CNel3DInterface::killViewPort()
	{
		driver->deleteScene( scene );
		scene = NULL;
		driver->delete3dMouseListener( mouseListener );
		mouseListener = NULL;
		delete driver;
		driver = NULL;
	}

	void CNel3DInterface::resizeViewPort( unsigned long w, unsigned long h )
	{
		if( scene != NULL )
		{
			scene->getCam().setPerspective( 90.0f, w / (float)h, 0.1f, 1000.0f );
		}
	}

	bool CNel3DInterface::addCube()
	{
		return loadShape( "primitives/cube.shape" );
	}

	bool CNel3DInterface::addSphere()
	{
		return loadShape( "primitives/sphere.shape" );
	}

	bool CNel3DInterface::addCylinder()
	{
		return loadShape( "primitives/cylinder.shape" );
	}

	bool CNel3DInterface::addTeaPot()
	{
		return loadShape( "primitives/teapot.shape" );
	}

	bool CNel3DInterface::loadShape( const std::string &fileName )
	{
		NLMISC::CPath::addSearchPath( NLMISC::CFile::getPath( fileName ), false, false );
		NL3D::UInstance instance = scene->createInstance( fileName );
		if( instance.empty() )
			return false;

		clearScene();
		currentShape = instance;

		subMatId = 0;

		setupCamera();
		
		return true;
	}

	void CNel3DInterface::clearScene()
	{
		if( scene != NULL )
		{
			if( currentShape.empty() )
				return;
			scene->deleteInstance( currentShape );
			currentShape = NL3D::UInstance();
			subMatId = 0;
		}

		if( driver == NULL )
			return;

		driver->clearBuffers();
		driver->swapBuffers();
	}

	void CNel3DInterface::updateInput()
	{
		driver->EventServer.pump();
	}

	void CNel3DInterface::renderScene()
	{
		if( scene != NULL )
		{
			scene->getCam().setTransformMode( NL3D::UTransformable::DirectMatrix );
			scene->getCam().setMatrix( mouseListener->getViewMatrix() );

			driver->clearBuffers();
			scene->render();
			driver->swapBuffers();
		}
	}

	unsigned long CNel3DInterface::getShapeMatCount() const
	{
		if( currentShape.empty() )
			return 0;

		return currentShape.getNumMaterials();
	}

	void CNel3DInterface::setupCamera()
	{
		NLMISC::CAABBox bbox;
		currentShape.getShapeAABBox( bbox );
		
		NLMISC::CVector center = bbox.getCenter();
		NLMISC::CQuat q( 0.0f, 0.0f, 0.0f, 0.0f );
		currentShape.getDefaultRotQuat( q );

		NLMISC::CVector max_radius = bbox.getHalfSize();
		float radius = std::max( max_radius.x, std::max( max_radius.y, max_radius.z ) );
		float distance = radius / tan( 45.0f );

		NLMISC::CVector axis = q.getAxis();
		if( axis.isNull() || ( axis == NLMISC::CVector::I ) )
			axis = NLMISC::CVector::J;
		else
		if( axis == -NLMISC::CVector::K )
			axis = -NLMISC::CVector::J;

		NLMISC::CVector eye = center - axis * ( radius + distance );		
		scene->getCam().lookAt( eye, center );

		mouseListener->setHotSpot( center );
		mouseListener->setMatrix( scene->getCam().getMatrix() );
	}

}

