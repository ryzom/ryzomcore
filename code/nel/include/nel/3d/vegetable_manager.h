// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_VEGETABLE_MANAGER_H
#define NL_VEGETABLE_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/rgba.h"
#include "nel/misc/block_memory.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/vegetable_clip_block.h"
#include "nel/3d/vegetable_sort_block.h"
#include "nel/3d/vegetable_instance_group.h"
#include "nel/3d/vegetable_shape.h"
#include "nel/3d/vegetablevb_allocator.h"
#include "nel/3d/material.h"
#include "nel/3d/driver.h"
#include "nel/3d/vegetable_uv8.h"


namespace NL3D
{


class	CVegetableBlendLayerModel;
class	CScene;
class	CVegetableLightEx;


// ***************************************************************************
// By default there is 20 layers.
#define	NL3D_VEGETABLE_DEFAULT_NUM_BLEND_LAYER	20
// default distance is 60 meters.
#define	NL3D_VEGETABLE_DEFAULT_DIST_MAX			60.f

class CVertexProgramVeget;

// ***************************************************************************
/**
 * Manager of vegetable. Instance Factory and rendering.
 *	A VegetableManager should be put into a CScene model which is Opaque (ie rendered in Opaque pass), and call
 *	vegetableManager::render() at this time. a good example is CLandscape.
 *
 *	Because during render(), it uses and setup special "Vegetable Blend Layer models" to render transparents
 *	alpha blended vegetables. Toses models are transparent so they are drawn during the transparent pass of
 *	the renderTrav's CScene (so after the Opaque pass).
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableManager
{
public:
	/// Micro vegetation position against Water. Above water is the default.
	enum TVegetableWater { AboveWater = 0, UnderWater, IntersectWater, VegetInfoLast };

public:

	/**
	 * \param maxVertexVbHardUnlit maximum VertexCount in VBHard for Unlit (or precomputed lighted) vegetables
	 * \param maxVertexVbHardLighted maximum VertexCount in VBHard for Lighted vegetables
	 * \param nbBlendLayers for ZSort/AlphaBlend rdrPass: number of layers of vegetables rendered independently.
	 * \param blendLayerDistMax for ZSort/AlphaBlend rdrPass: distance of the farest layer.
	 */
	CVegetableManager(uint maxVertexVbHardUnlit, uint maxVertexVbHardLighted,
		uint nbBlendLayers= NL3D_VEGETABLE_DEFAULT_NUM_BLEND_LAYER,
		float blendLayerDistMax= NL3D_VEGETABLE_DEFAULT_DIST_MAX);
	~CVegetableManager();

	/** Before any render(), you must call this method (else nlassert). It creates the necessary models in the scene,
	 *	to manage AlphaBlending correctly. Those models are deleted in the object dtor.
	 */
	void						createVegetableBlendLayersModels(CScene *scene);


	/// \name Shape management
	// @{

	/// Load a shape if necessary, and return a shapeId for this shape.
	CVegetableShape				*getVegetableShape(const std::string &shape);

	// @}


	/// \name instance management
	// @{

	/// Create a clipBlock where SortBlock will be created.
	CVegetableClipBlock			*createClipBlock();
	/// delete such a clipBlock. all sortBlocks and so all ig must be deleted before.
	void						deleteClipBlock(CVegetableClipBlock *clipBlock);

	/** Create a SortBlock in a clipBlock where instance group (ig) will be created.
	 *	All AlphaBlend instances created in a SortBlock should have the same vegetWaterState: AboveWater or UnderWater.
	 *	Each time an instance is added to the sortBlock it changes the _UnderWater state of the sortBlock.
	 *	\param center you must give an approximate center for the sortBlock (for sorting)
	 *	\param radius you must give an approximate radius for the sortBlock (for the system to know when you are IN
	 *	the sortBlock, and then to sort in a better way)
	 */
	CVegetableSortBlock			*createSortBlock(CVegetableClipBlock *clipBlock, const CVector &center, float radius);
	/// delete such a SortBlock. all ig must be deleted before.
	void						deleteSortBlock(CVegetableSortBlock *sortBlock);

	/** create an instance group in a sortBlock, where instances will be created.
	 *	Instances will be frustum-clipped by the clipBlock, and sorted (for the ZSort rdrPass only) by sortBlock.
	 */
	CVegetableInstanceGroup		*createIg(CVegetableSortBlock *sortBlock);
	/** delete such an ig.
	 *	After doing this, you must call igSortBlockOwner->updateSortBlock()
	 *	If the sortBlock has many Igs, you can do it after deleting all your igs.
	 */
	void						deleteIg(CVegetableInstanceGroup *ig);

	// @}


	/// \name Adding Instances to an Ig.
	/**
	 *	Adding instances in an Ig is a 2 step process:
	 *		- reserve Ig Space
	 *		- add instances to the ig
	 *
	 *	More precisely:
	 *		- For all vegetables shapes which will be created in an instance group
	 *			- reserveIgAddInstances(shape, waterState, number of instances, vegetIgReserve)
	 *		- call reserveIgCompile(ig, vegetIgReserve)
	 *		- For all vegetables instances
	 *			- addInstance(ig, ...)
	 *
	 *	After this setup, you CANNOT add anymore instances to the Ig (this is a requirement for fast allocation).
	 *
	 *	After adding all your instances to an Ig, you must call sortBlockOwnerOfTheIg->updateSortBlock()
	 *	If the sortBlock has many Igs, you can do it after filling all your igs.
	 */
	// @{

	/**	reserve some instance space in an Ig.
	 *	nothing is really done here, after doing this for all shapes of your ig, you must call
	 *	reserveIgCompile()
	 *	\param vegetIgReserve the object where space required for the ig is added
	 */
	void			reserveIgAddInstances(CVegetableInstanceGroupReserve &vegetIgReserve, CVegetableShape *shape, TVegetableWater vegetWaterState, uint numInstances);
	/** reserve the space in the ig.
	 *	nlassert() if the ig is not empty.
	 *  \see reserveIgAddInstances()
	 */
	void			reserveIgCompile(CVegetableInstanceGroup *ig, const CVegetableInstanceGroupReserve &vegetIgReserve);


	/** add an instance to an ig, enlarging the associated clipBlock bbox.
	 *	If the shape is not lighted, then only diffuseColor is used, to setup color per vertex.
	 *	Warning! Use OptFastFloor()! So call must be enclosed with a OptFastFloorBegin()/OptFastFloorEnd().
	 *
	 *	Also, buffer must be locked.
	 *
	 *	ambientColor and diffuseColor should be in [0..1] (no clamp), else uint8 will wrap...
	 *
	 *	nlassert() if no sufficient space reserved in reserveIgCompile().
	 *
	 *	\param dlmUV is the dynamic lightmap UV for this vegetable.
	 *	\see reserveIgAddInstances() reserveIgCompile()
	 */
	void						addInstance(CVegetableInstanceGroup *ig,
		CVegetableShape	*shape, const NLMISC::CMatrix &mat,
		const NLMISC::CRGBAF &ambientColor, const NLMISC::CRGBAF &diffuseColor,
		float	bendFactor, float bendPhase, float bendFreqFactor, float blendDistMax,
		TVegetableWater vegetWaterState, CVegetableUV8 dlmUV);

	/** Setup a density multiplicator [0,1], for performance reason for instance
	 */
	void			setGlobalDensity(float density);

	/** Get density multiplicator [0,1], for performance reason for instance
	 */
	float			getGlobalDensity() const {return _GlobalDensity;}

	// @}


	/// \name render
	// @{

	/// must give a driver to the vegetableManager, before any addInstance().
	void			updateDriver(IDriver *driver);

	/// load a global texture used for all vegetables (lookup into CPath).
	void			loadTexture(const std::string &texName);
	/// setup a global texture used for all vegetables (smartPtr-ized).
	void			loadTexture(ITexture *itex);
	/// setup the directional light
	void			setDirectionalLight(const CRGBA &ambient, const CRGBA &diffuse, const CVector &light);

	/** lock any AGP vertex buffers. Do it wisely (just one time before refine as example).
	 *	You MUST enclose calls to addInstance() (and so CVegetable::generateInstance())
	 *	with lockBuffers() / unlockBuffers().
	 */
	void			lockBuffers();
	/// unlock any AGP vertex buffers
	void			unlockBuffers();

	/** render the manager into a driver, with current viewMatrix/frustum/fog  setuped
	 *	Buffers should be unlocked.
	 *	\param	textureDLM is the dynamic lightmap to use. can be NULL if don't want DLM
	 */
	void			render(const CVector &viewCenter, const CVector &frontVector, const std::vector<CPlane> &pyramid,
		ITexture *textureDLM, IDriver *driver);

	// @}


	/// \name Wind animation
	// @{

	/** set the Wind for animation.
	 *	All thoses variables may be modified each frame without penalty.
	 *
	 *	\param windDir is the direction of the wind. NB: only XY direction is kept.
	 *	\param windFreq is the frequency for the animation (speed)
	 *	\param windPower is the power of the wind, and is a factor (0..1) of Bend
	 *	\param windBendMin is a value in (0..1) which indicate how much the vegetables are bended at minimum
	 *	(for very powerfull wind)
	 */
	void		setWind(const CVector &windDir, float windFreq, float windPower, float windBendMin);

	/** set the current Time (in seconds). For Wind animation
	 */
	void		setTime(double time);

	// @}


	/// \name UpdateLighting management
	// @{

	/** set the vegetable manager System Time (in seconds)
	 *	This time is used for lighting update, and is independent of setTime()
	 */
	void		setUpdateLightingTime(double time);

	/** update the lighting of Igs, within a certain amount of time.
	 *	You MUST enclose calls to updateLighting() with lockBuffers() / unlockBuffers().
	 */
	void		updateLighting();

	/** set the frequency of lighting update. If freq==1, ALL lighted igs are updated each second.
	 *	e.g: if 1/20, then every 20 seconds, all Igs are updated.
	 *	If you set 0, no update will be done at all (this is the default setup!!).
	 */
	void		setUpdateLightingFrequency(float freq);

	/** like updateLighting(), but update ALL vegetable
	 *	You MUST enclose calls to updateLighting() with lockBuffers() / unlockBuffers().
	 */
	void		updateLightingAll();

	// @}


	/// \name Profile
	// @{

	/// set to 0 the number of faces rendered
	void		resetNumVegetableFaceRendered();
	/// get the number of faces rendered by the vegetable manager
	uint		getNumVegetableFaceRendered() const;

	// @}

// *********************
private:
	friend class	CVegetableBlendLayerModel;

	NLMISC::CBlockMemory<CVegetableClipBlock>		_ClipBlockMemory;
	NLMISC::CBlockMemory<CVegetableSortBlock>		_SortBlockMemory;
	NLMISC::CBlockMemory<CVegetableInstanceGroup>	_InstanceGroupMemory;

	// List of ClipBlock not empty. tested for clipping
	CTessList<CVegetableClipBlock>					_ClipBlockList;
	// List of ClipBlock created, with no Ig, so not tested for clipping
	CTessList<CVegetableClipBlock>					_EmptyClipBlockList;


	// Vegetable Shape map.
	typedef	std::map<std::string, CVegetableShape>	TShapeMap;
	typedef	TShapeMap::iterator						ItShapeMap;
	TShapeMap										_ShapeMap;


	// Vertex Buffers for display. One allocator for Lighted and Unlit mode.
	CVegetableVBAllocator							_VBHardAllocator[CVegetableVBAllocator::VBTypeCount];
	// The same, but no VBHard.
	CVegetableVBAllocator							_VBSoftAllocator[CVegetableVBAllocator::VBTypeCount];
	// Vertex Program. One VertexProgram for each rdrPass (with / without fog)
	CSmartPtr<CVertexProgramVeget>					_VertexProgram[NL3D_VEGETABLE_NRDRPASS][2];
	CRefPtr<CVertexProgramVeget>					_ActiveVertexProgram;


	// Material. Useful for texture and alphaTest
	CMaterial										_VegetableMaterial;
	// Norm
	CVector											_DirectionalLight;
	NLMISC::CRGBA									_GlobalAmbient;
	NLMISC::CRGBA									_GlobalDiffuse;

	/// Global Vegetable Density
	float											_GlobalDensity;

	/// profile
	uint											_NumVegetableFaceRendered;


	// return true if the ith rdrPass is 2Sided.
	static	bool	doubleSidedRdrPass(uint rdrPass);


	/// get the rdrPass and other info for a given shape.
	uint			getRdrPassInfoForShape(CVegetableShape *shape, TVegetableWater vegetWaterState,
		bool &instanceLighted, bool &instanceDoubleSided, bool &instanceZSort,
		bool &destLighted, bool &precomputeLighting);


	/// Get the good allocator for the appropriate rdr pass.
	CVegetableVBAllocator	&getVBAllocatorForRdrPassAndVBHardMode(uint rdrPass, uint vbHardMode);


	/// init the ith vertexProgram.
	void					initVertexProgram(uint vpType, bool fogEnabled);


	/// setup the vertexProgram constants.
	void					setupVertexProgramConstants(IDriver *driver, bool fogEnabled);


	/** swap the RdrPass type (hard or soft) of the rdrPass of an instance group.
	 *	vertices are allocated in other VBallocator, copied and freed in the old VBallocator.
	 */
	void					swapIgRdrPassHardMode(CVegetableInstanceGroup *, uint rdrPass);


	/// \name Wind animation
	// @{
	CVector											_WindDirection;
	float											_WindFrequency;
	float											_WindPower;
	float											_WindBendMin;
	// nb: used for wind animation
	double											_Time;
	double											_WindPrecRenderTime;
	// updated at each render().
	double											_WindAnimTime;

	// Constant LUT.
	float											_CosTable[NL3D_VEGETABLE_VP_LUT_SIZE];
	// computed at each render().
	NLMISC::CVector2f								_WindTable[NL3D_VEGETABLE_VP_LUT_SIZE];
	NLMISC::CVector2f								_WindDeltaTable[NL3D_VEGETABLE_VP_LUT_SIZE];


	// @}


	/// \name Misc data to setup renderState (computed at each render())
	// @{

	CVector					_AngleAxis;
	CVector					_ViewCenter;
	bool					_BkupFog;
	// NB: the manager matrix may not be Identity, for ZBuffer precision reason.
	NLMISC::CMatrix			_ManagerMatrix;

	// @}


	/// \name CVegetableBlendLayerModel mgt.
	// @{


	/// For Alpha Blend rdrPass, ordering into layers.
	uint					_NumZSortBlendLayers;
	float					_ZSortLayerDistMax;
	CScene					*_ZSortScene;
	std::vector<CVegetableBlendLayerModel*>		_ZSortModelLayers;
	// The same but under water
	std::vector<CVegetableBlendLayerModel*>		_ZSortModelLayersUW;


	/// called by CVegetableBlendLayerModel.
	void					setupRenderStateForBlendLayerModel(IDriver *driver);
	void					exitRenderStateForBlendLayerModel(IDriver *driver);

	// @}


	/// \name UpdateLighting management
	/**
	 *	NB: we update at the precision of a shape (a dozen of vertices).
	 */
	// @{

	// Last update time.
	double					_ULPrecTime;
	bool					_ULPrecTimeInit;
	double					_ULTime;

	/// Frequency of update.
	float					_ULFrequency;
	/// Current number of vertices to update. If negative, I have some advance.
	float					_ULNVerticesToUpdate;
	/// Sum of all ig vertices to update.
	uint					_ULNTotalVertices;
	/// the priority list of ig to update
	CVegetableInstanceGroup	*_ULRootIg;
	/// Current instance to render in the first ig to update: rdrpass/instanceId.
	uint					_ULCurrentIgRdrPass;
	uint					_ULCurrentIgInstance;

	/// update lighting according to _ULNVerticesToUpdate
	void		doUpdateLighting();

	/** update part of the RootIg, according to _ULNVerticesToUpdate (while > 0)
	 *	if all Ig is updated, return true and _ULCurrentIgRdrPass and _ULCurrentIgInstance is updated.
	 */
	bool		updateLightingIGPart();


	/** update part of an ig. Do not use/modify _UL*
	 *	return number of vertices processed (nb vertices of the shape)
	 */
	uint		updateInstanceLighting(CVegetableInstanceGroup *ig, uint rdrPassId, uint instanceId);


	// @}

	// last driver setupped with a call to update driver
	NLMISC::CRefPtr<IDriver> _LastDriver;


};


} // NL3D


#endif // NL_VEGETABLE_MANAGER_H

/* End of vegetable_manager.h */
