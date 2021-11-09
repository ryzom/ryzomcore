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

#include "std3d.h"

#include "nel/3d/mesh_base.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/lod_character_texture.h"
#include "nel/3d/visual_collision_mesh.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{



// ***************************************************************************
CMeshBase::CMeshBase()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	_UseLightingLocalAttenuation= false;

	// To have same functionnality than previous version, init to identity.
	_DefaultPos.setDefaultValue(CVector(0,0,0));
	_DefaultPivot.setDefaultValue(CVector(0,0,0));
	_DefaultRotEuler.setDefaultValue(CVector(0,0,0));
	_DefaultRotQuat.setDefaultValue(CQuat::Identity);
	_DefaultScale.setDefaultValue(CVector(1,1,1));
	_DefaultLMFactor.setDefaultValue(CRGBA(255,255,255,255));

	_AutoAnim = false;

	_LodCharacterTexture= NULL;

	_CollisionMeshGeneration= AutoCameraCol;

	_VisualCollisionMesh= NULL;

	_DefaultOpacity= false;
	_DefaultTransparency= false;
}


// ***************************************************************************
CMeshBase::~CMeshBase()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// free if exist
	resetLodCharacterTexture();
	// delete the Col mesh if created
	if(_VisualCollisionMesh)
	{
		delete _VisualCollisionMesh;
		_VisualCollisionMesh= NULL;
	}
}


// ***************************************************************************
// ***************************************************************************
// Animated material.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CMeshBase::setAnimatedMaterial(uint id, const std::string &matName)
{
	nlassert(!matName.empty());
	if(id<_Materials.size())
	{
		// add / replace animated material.
		_AnimatedMaterials[id].Name= matName;
		// copy Material default.
		_AnimatedMaterials[id].copyFromMaterial(&_Materials[id]);
	}
}

// ***************************************************************************
CMaterialBase	*CMeshBase::getAnimatedMaterial(uint id)
{
	TAnimatedMaterialMap::iterator	it;
	it= _AnimatedMaterials.find(id);
	if(it!=_AnimatedMaterials.end())
		return &it->second;
	else
		return NULL;
}


// ***************************************************************************
// ***************************************************************************
// Serial - buildBase.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMeshBase::CMeshBaseBuild::CMeshBaseBuild()
{
	DefaultPos.set(0,0,0);
	DefaultPivot.set(0,0,0);
	DefaultRotEuler.set(0,0,0);
	DefaultScale.set(1,1,1);

	bCastShadows= false;
	bRcvShadows= false;
	UseLightingLocalAttenuation= false;
	CollisionMeshGeneration= CMeshBase::AutoCameraCol;
}

// ***************************************************************************
#if 0
void	CMeshBase::CMeshBaseBuild::serial(NLMISC::IStream &f)
{
	/*
	Version 1:
		- Cut in version because of badly coded ITexture* serialisation. throw an exception if
			find a version < 1.
	Version 0:
		- 1st version.
	*/
	sint	ver= f.serialVersion(1);

	if(ver<1)
		throw NLMISC::EStream(f, "MeshBuild in Stream is too old (MeshBaseBuild version < 1)");

	f.serial( DefaultPos );
	f.serial( DefaultPivot );
	f.serial( DefaultRotEuler );
	f.serial( DefaultRotQuat );
	f.serial( DefaultScale );

	f.serialCont( Materials );
}
#endif

// ***************************************************************************
void	CMeshBase::serialMeshBase(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/*
	Version 9:
		- _CollisionMeshGeneration
	Version 8:
		- new format for CLightMapInfoList
	Version 7:
		- _LodCharacterTexture
	Version 6:
		- _DistMax
	Version 5:
		- _AutoAnim
	Version 4:
		- _UseLightingLocalAttenuation
	Version 3:
		- _IsLightable
	Version 2:
		- Added Blend Shapes factors
	Version 1:
		- Cut in version because of badly coded ITexture* serialisation. throw an exception if
			find a version < 1.
	Version 0:
		- 1st version.
	*/
	sint ver = f.serialVersion(9);

	if (ver >= 2)
	{
		f.serialCont (_AnimatedMorph);
	}

	if(ver<1)
		throw NLMISC::EStream(f, "Mesh in Stream is too old (MeshBase version < 1)");

	f.serial (_DefaultPos);
	f.serial (_DefaultPivot);
	f.serial (_DefaultRotEuler);
	f.serial (_DefaultRotQuat);
	f.serial (_DefaultScale);

	f.serialCont(_Materials);
	f.serialCont(_AnimatedMaterials);

	if(ver >= 8)
		f.serialCont(_LightInfos);
	else
	{
		TLightInfoMapV7 temp;
		f.serialCont(temp);
	}

	if(ver>=3)
		// read/write _IsLightable flag.
		f.serial(_IsLightable);
	else if( f.isReading() )
		// update _IsLightable flag.
		computeIsLightable();

	if(ver>=4)
		f.serial(_UseLightingLocalAttenuation);
	else if( f.isReading() )
		_UseLightingLocalAttenuation= false;

	if (ver >= 5)
	{
		f.serial(_AutoAnim);
	}

	if(ver >= 6)
		f.serial(_DistMax);

	if(ver >= 7)
		f.serialPtr(_LodCharacterTexture);

	if(ver >= 9)
		f.serialEnum(_CollisionMeshGeneration);
	else
		_CollisionMeshGeneration= AutoCameraCol;

	// Some runtime not serialized compilation
	if(f.isReading())
		compileRunTime();
}


// ***************************************************************************
void	CMeshBase::buildMeshBase(CMeshBaseBuild &m)
{
	// Copy light information
	_LightInfos = m.LightInfoMap;

	// copy the materials.
	_Materials= m.Materials;

	// clear the animated materials.
	_AnimatedMaterials.clear();

	/// Copy default position values
	_DefaultPos.setDefaultValue (m.DefaultPos);
	_DefaultPivot.setDefaultValue (m.DefaultPivot);
	_DefaultRotEuler.setDefaultValue (m.DefaultRotEuler);
	_DefaultRotQuat.setDefaultValue (m.DefaultRotQuat);
	_DefaultScale.setDefaultValue (m.DefaultScale);

	_AnimatedMorph	.resize(m.DefaultBSFactors.size());
	for (uint32 i = 0; i < m.DefaultBSFactors.size(); ++i)
	{
		_AnimatedMorph[i].DefaultFactor.setDefaultValue (m.DefaultBSFactors[i]);
		_AnimatedMorph[i].Name = m.BSNames[i];
	}

	// update _IsLightable flag.
	computeIsLightable();
	// copy _UseLightingLocalAttenuation
	_UseLightingLocalAttenuation= m.UseLightingLocalAttenuation;

	// copy CollisionMeshGeneration
	_CollisionMeshGeneration= m.CollisionMeshGeneration;

	// Some runtime not serialized compilation
	compileRunTime();
}




// ***************************************************************************
void	CMeshBase::instanciateMeshBase(CMeshBaseInstance *mi, CScene *ownerScene)
{
	uint32 i;


	// setup animated blendShapes
	//===========================
	mi->_AnimatedMorphFactor.reserve(_AnimatedMorph.size());
	for(i = 0; i < _AnimatedMorph.size(); ++i)
	{
		CAnimatedMorph am(&_AnimatedMorph[i]);
		mi->_AnimatedMorphFactor.push_back (am);
	}

	// setup materials.
	//=================
	// Copy material. Textures are referenced only
	mi->Materials= _Materials;

	// Instanciate selectable textures (use default set)
	mi->selectTextureSet(0);

	// prepare possible AsyncTextures
	mi->AsyncTextures.resize(_Materials.size());

	// setup animated materials.
	//==========================
	TAnimatedMaterialMap::iterator	it;
	mi->_AnimatedMaterials.reserve(_AnimatedMaterials.size());
	for(it= _AnimatedMaterials.begin(); it!= _AnimatedMaterials.end(); it++)
	{
		CAnimatedMaterial	aniMat(&it->second);

		// set the target instance material.
		nlassert(it->first < mi->Materials.size());
		aniMat.setMaterial(&mi->Materials[it->first]);

		// Must set the Animatable father of the animated material (the mesh_base_instance!).
		aniMat.setFather(mi, CMeshBaseInstance::OwnerBit);

		// Append this animated material.
		mi->_AnimatedMaterials.push_back(aniMat);
	}

	// Misc
	//==========================

	// Setup position with the default value
	mi->ITransformable::setPos( _DefaultPos.getDefaultValue() );
	mi->ITransformable::setRotQuat( _DefaultRotQuat.getDefaultValue() );
	mi->ITransformable::setScale( _DefaultScale.getDefaultValue() );
	mi->ITransformable::setPivot( _DefaultPivot.getDefaultValue() );

	// Setup default opcaity / transparency state
	mi->setOpacity( this->getDefaultOpacity() );
	mi->setTransparency( this->getDefaultTransparency() );

	// if the mesh is lightable, then the instance is
	mi->setIsLightable(this->isLightable());

	// a mesh is considered big for lightable if it uses localAttenuation
	mi->setIsBigLightable(this->useLightingLocalAttenuation());

	// The mesh support fast intersect, if the system geometry has been built
	mi->enableFastIntersectSupport(!_SystemGeometry.empty());
}


// ***************************************************************************
void	CMeshBase::applyMaterialUsageOptim(const std::vector<bool> &materialUsed, std::vector<sint> &remap)
{
	nlassert(_Materials.size()==materialUsed.size());

	// security reset
	resetLodCharacterTexture();
	_AnimatedMaterials.clear();

	// init all ids to "Not Used"
	remap.clear();
	remap.resize(_Materials.size(), -1);

	// remove unused materials and build remap
	vector<CMaterial>::iterator		itMat= _Materials.begin();
	uint							dstIdx= 0;
	uint i;
	for(i=0;i<materialUsed.size();i++)
	{
		// if used, still use it, and remap.
		if(materialUsed[i])
		{
			remap[i]= dstIdx;
			itMat++;
			dstIdx++;
		}
		// remove from the array
		else
		{
			itMat= _Materials.erase(itMat);
		}
	}

	// apply the remap to LightMaps infos
	const uint count = (uint)_LightInfos.size ();
	for (i=0; i<count; i++)
	{
		CLightMapInfoList &mapInfoList = _LightInfos[i];
		std::list<CMeshBase::CLightMapInfoList::CMatStage>::iterator ite = mapInfoList.StageList.begin ();
		while (ite != mapInfoList.StageList.end ())
		{
			sint	newId= remap[ite->MatId];
			// If material used
			if(newId>=0)
			{
				// apply remap on the material id
				ite->MatId= newId;
				ite++;
			}
			else
			{
				// remove it from list of light infos
				ite= mapInfoList.StageList.erase(ite);
			}
		}
	}
}


// ***************************************************************************
void	CMeshBase::flushTextures(IDriver &driver, uint selectedTexture)
{
	// Mat count
	uint matCount=(uint)_Materials.size();

	// Flush each material textures
	for (uint mat=0; mat<matCount; mat++)
	{
		/// Flush material textures
		_Materials[mat].flushTextures (driver, selectedTexture);
	}
}


// ***************************************************************************
void	CMeshBase::computeIsLightable()
{
	// by default the mesh is not lightable
	_IsLightable= false;

	// Mat count
	uint matCount=(uint)_Materials.size();

	// for each material
	for (uint mat=0; mat<matCount; mat++)
	{
		// if this one is not a lightmap, then OK, the mesh is lightable
		if( _Materials[mat].getShader()!=CMaterial::LightMap )
		{
			_IsLightable= true;
			break;
		}
	}
}


// ***************************************************************************
bool	CMeshBase::useLightingLocalAttenuation () const
{
	return _UseLightingLocalAttenuation;
}


// ***************************************************************************
void	CMeshBase::resetLodCharacterTexture()
{
	if(_LodCharacterTexture)
	{
		delete _LodCharacterTexture;
		_LodCharacterTexture= NULL;
	}
}

// ***************************************************************************
void	CMeshBase::setupLodCharacterTexture(CLodCharacterTexture &lodText)
{
	// delete old
	resetLodCharacterTexture();
	// seutp new
	_LodCharacterTexture= new CLodCharacterTexture;
	*_LodCharacterTexture= lodText;
}

// ***************************************************************************
CVisualCollisionMesh		*CMeshBase::getVisualCollisionMesh() const
{
	return _VisualCollisionMesh;
}

// ***************************************************************************
void	CMeshBase::compileRunTime()
{
	_DefaultTransparency= false;
	_DefaultOpacity= false;
	for( uint i = 0; i < _Materials.size(); ++i )
		if( _Materials[i].getBlend() )
			_DefaultTransparency= true;
		else
			_DefaultOpacity= true;
}


} // NL3D
