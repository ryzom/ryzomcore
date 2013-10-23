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

#ifndef NL_MESH_BASE_H
#define NL_MESH_BASE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/shape.h"
#include "nel/3d/driver.h"
#include "nel/3d/material.h"
#include "nel/3d/animated_material.h"
#include "nel/3d/animated_morph.h"
#include "nel/3d/ray_mesh.h"
#include <set>
#include <vector>



namespace NL3D {


class	CMeshBaseInstance;
class	CLodCharacterTexture;
class	CVisualCollisionMesh;


// ***************************************************************************
/**
 * A base mesh with just material manipulation. Used as an abstract base class for CMesh or CMeshMRM.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshBase : public IShape
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

public:
	/** If AutoCameraCol, meshs will build the cameracollision mesh only if the mesh is lightmapped
	 *	if NoCameraCol, the cameracollision won't be generated
	 *	if ForceCameraCol, the cameracollision will be generated whether lightmaped or not (as long as possible)
	 */
	enum	TCameraCollisionGenerate {AutoCameraCol= 0, NoCameraCol, ForceCameraCol};

public:

	/// \name Structures for building a mesh.
	//@{

	struct CMatStageV7
	{
		uint8 nMatNb, nStageNb;
		void	serial(NLMISC::IStream &f)
		{
			f.serial(nMatNb);
			f.serial(nStageNb);
		}
	};
	struct CLightInfoMapListV7 : std::list< CMatStageV7 >
	{
		void	serial(NLMISC::IStream &f)
		{
			f.serialCont((std::list< CMatStageV7 >&)*this);
		}
	};
	typedef std::map< std::string, CLightInfoMapListV7 >	TLightInfoMapV7;

	// Optinnal lightmap factor information
	class CLightMapInfoList
	{
	public:
		// Index of the material stage where the lightmap is used
		class CMatStage
		{
		public:
			// Material ID
			uint8	MatId;

			// Stage ID
			uint8	StageId;

			void	serial (NLMISC::IStream &f)
			{
				f.serialVersion (0);
				f.serial(MatId);
				f.serial(StageId);
			}
		};

		uint32					LightGroup;
		std::string				AnimatedLight;
		std::list<CMatStage>	StageList;
		void	serial(NLMISC::IStream &f)
		{
			f.serialVersion (0);
			f.serial (LightGroup);
			f.serial (AnimatedLight);
			f.serialCont(StageList);
		}
	};
	typedef std::vector< CLightMapInfoList >	TLightMapInfo;

	/// A mesh material information.
	struct	CMeshBaseBuild
	{
		bool					bCastShadows;
		bool					bRcvShadows;
		bool					UseLightingLocalAttenuation;

		// Default value for position of this mesh
		CVector					DefaultPos;
		CVector					DefaultPivot;
		CVector					DefaultRotEuler;
		CQuat					DefaultRotQuat;
		CVector					DefaultScale;

		// Material array
		std::vector<CMaterial>	Materials;

		// Map of light information
		TLightMapInfo			LightInfoMap;

		// Default position of the blend shapes factors
		std::vector<float>		DefaultBSFactors;
		std::vector<std::string>BSNames;

		// Indicate how this mesh should build the collision mesh (for camera ThirPerson collision)
		TCameraCollisionGenerate	CollisionMeshGeneration;

		CMeshBaseBuild();

		// Serialization is not used
		//void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};
	//@}


public:
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/// Constructor
	CMeshBase();
	virtual ~CMeshBase();

	/// \name animated material mgt. do it after CMesh::build(), or CMeshMRM::build()
	// @{
	/// setup a material as animated. Material must exist or else no-op.
	void			setAnimatedMaterial(uint id, const std::string &matName);
	/// return NULL if this material is NOT animated. (or if material do not exist)
	CMaterialBase	*getAnimatedMaterial(uint id);
	// @}

	/// \name access default tracks.
	// @{
	CTrackDefaultVector*	getDefaultPos ()		{return &_DefaultPos;}
	CTrackDefaultVector*	getDefaultPivot ()		{return &_DefaultPivot;}
	CTrackDefaultVector*	getDefaultRotEuler ()	{return &_DefaultRotEuler;}
	CTrackDefaultQuat*		getDefaultRotQuat ()	{return &_DefaultRotQuat;}
	CTrackDefaultVector*	getDefaultScale ()		{return &_DefaultScale;}
	CTrackDefaultRGBA*		getDefaultLMFactor ()	{return &_DefaultLMFactor;}
	// @}

	/// \name Material accessors
	// @{

	/// Get the number of materials in the mesh
	uint getNbMaterial() const
	{
		return (uint)_Materials.size();
	}

	/// Get a material
	CMaterial& getMaterial(uint id)
	{
		return _Materials[id];
	}
	/// Get a material, const version
	const CMaterial& getMaterial(uint id) const
	{
		return _Materials[id];
	}
	// @}

	/// serial the base Part of this mesh.
	void	serialMeshBase(NLMISC::IStream &f) throw(NLMISC::EStream);

	/// Flush textures
	void	flushTextures (IDriver &driver, uint selectedTexture);


	/// Is this mesh lightable? true if one of his materials is not a lightmap. NB: result stored in file.
	bool	isLightable() const {return _IsLightable;}

	/// tells if the shape wants LocalAttenuation for RealTime lighting.  Default is false
	virtual bool		useLightingLocalAttenuation () const;

	/// \name Automatic animation
	// @{
	/** Tells that the model instanciated from this shape should be automatically animated.
      * If so the scene will search for an automatic anim that has the same name than this shape and will bind it.
	  * This state is serialized.
	  * NB: This is a clue to scene::createInstance, a call to createInstance of this object won't make the animation binding.
	  */
	void						setAutoAnim(bool on = true) { _AutoAnim = on; }
	// Check whether automatic animation is enabled
	bool						getAutoAnim() const { return _AutoAnim; }
	// @}


	/// \name LodCharacterTexture
	// @{
	/// reset the lod texture info. see CSkeletonModel Lod system.
	void						resetLodCharacterTexture();
	/// setup the lod texture info. see CSkeletonModel Lod system
	void						setupLodCharacterTexture(CLodCharacterTexture &lodText);
	/// get the lod texture info. see CSkeletonModel Lod system. NULL per default. NB: serialised
	const CLodCharacterTexture	*getLodCharacterTexture() const {return _LodCharacterTexture;}
	// @}


	/// \name Visual Collision
	// @{
	/// return the visual collision mesh. NULL if not supported
	CVisualCollisionMesh		*getVisualCollisionMesh() const;
	/// return the System Geometry (for fast intersection computation)
	const CRayMesh				&getSystemGeometry() const {return _SystemGeometry;}
	// @}


	// Return the Default setup choosed for this shape (used for UInstance::setOpacity())
	bool			getDefaultOpacity() const {return _DefaultOpacity;}

	// Return the Default setup computed for this shape (used for UInstance::setTransparency())
	bool			getDefaultTransparency() const {return _DefaultTransparency;}


// ************************
protected:
	/// The Materials.
	std::vector<CMaterial>		_Materials;

public:
	// Map of light information ( LightName, list(MaterialNb, StageNb) )
	TLightMapInfo				_LightInfos;


protected:
	/// Animated Material mgt.
	typedef std::map<uint32, CMaterialBase>	TAnimatedMaterialMap;
	TAnimatedMaterialMap		_AnimatedMaterials;


	/// Transform default tracks. Those default tracks are instancied, ie, CInstanceMesh will have the same and can't specialize it.
	CTrackDefaultVector			_DefaultPos;
	CTrackDefaultVector			_DefaultPivot;
	CTrackDefaultVector			_DefaultRotEuler;
	CTrackDefaultQuat			_DefaultRotQuat;
	CTrackDefaultVector			_DefaultScale;
	CTrackDefaultRGBA			_DefaultLMFactor;

	std::vector<CMorphBase>		_AnimatedMorph;

	/// Is this mesh lightable??
	bool						_IsLightable;
	/// Is this mesh Use Lighting Local Attenuation ??
	bool						_UseLightingLocalAttenuation;

	bool						_AutoAnim;

	/// Default Opacity/Transparency State
	bool						_DefaultOpacity;
	bool						_DefaultTransparency;


	// Indicate how this mesh should build the collision mesh (for camera ThirPerson collision)
	TCameraCollisionGenerate	_CollisionMeshGeneration;
	// Built only by CMesh and CMeshMultiLod
	CVisualCollisionMesh		*_VisualCollisionMesh;

	/** (optional) SystemMem copy of the geometry, for ray intersection
	 *	We cannot use the meshgeom, since once the VBuffer/IndexBuffer are resident (maybe AGP),
	 *	they can't be locked without heavy performance penalty
	 *	This is built by CMesh/CMeshMRM/CMeshMultiLod::buildSystemGeometry()
	 *	And only if it has been flagged through CShapeBank::buildSystemGeometryForshape()
	 */
	CRayMesh		_SystemGeometry;

protected:
	/// Just copy information from a CMeshBaseBuild.
	void	buildMeshBase(CMeshBaseBuild &m);

	/// instanciate MeshBase part to an instance (a CMeshBaseInstance).
	void	instanciateMeshBase(CMeshBaseInstance *mi, CScene *ownerScene);

	/** delete any material not used, and build remap table (old to new).
	 *	_AnimatedMaterials are reseted
	 *	resetLodCharacterTexture(); is called
	 */
	void	applyMaterialUsageOptim(const std::vector<bool> &materialUsed, std::vector<sint> &remap);


private:

	/// The Texture Lod info for this mesh. NB: NULL by default.
	CLodCharacterTexture		*_LodCharacterTexture;

	/// compute _Lightable.
	void	computeIsLightable();

	/// Called at load or build time, all that is not serialized
	void	compileRunTime();
};


} // NL3D


#endif // NL_MESH_BASE_H

/* End of mesh_base.h */
