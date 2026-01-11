// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_LANDSCAPE_H
#define NL_LANDSCAPE_H


#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/triangle.h"
#include "nel/3d/zone.h"
#include "nel/3d/tile_bank.h"
#include "nel/3d/patch_rdr_pass.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/tile_far_bank.h"
#include "nel/3d/texture_near.h"
#include "nel/3d/quad_grid.h"
#include "nel/misc/block_memory.h"
#include "nel/3d/landscapevb_allocator.h"
#include "nel/3d/landscape_face_vector_manager.h"
#include "nel/3d/tess_face_priority_list.h"
#include "nel/3d/point_light_influence.h"
#include "nel/3d/shadow_poly_receiver.h"

#include <map>

#define NL_MAX_SIZE_OF_TEXTURE_EDGE_SHIFT (NL_MAX_TILES_BY_PATCH_EDGE_SHIFT+NL_NUM_PIXELS_ON_FAR_TILE_EDGE_SHIFT)
#define NL_MAX_SIZE_OF_TEXTURE_EDGE (1<<NL_MAX_SIZE_OF_TEXTURE_EDGE_SHIFT)		// Size max of a far texture edge in pixel


namespace NL3D
{


class	CHeightMap;
class	CTileNoiseMap;
class	CVegetableManager;
class	CVegetable;
class	CTileVegetableDesc;
class	CScene;
class	CTextureFar;
class	CTextureDLM;
class	CPatchDLMContextList;
class	CShadowMap;

using NLMISC::Exception;
using NLMISC::CTriangle;
using NLMISC::CBlockMemory;

// ***************************************************************************
// The maximum amount of different tiles in world.
const	sint	NbTilesMax= 65536;
// Size of a CTextureNear. 256 by default (works everywhere).
// Texures must be square, because of uvscalebias...
const	sint	TextureNearSize= 512;
const	sint	NbTileLightMapByLine= TextureNearSize/NL_TILE_LIGHTMAP_SIZE;
const	sint	NbTileLightMapByTexture= NbTileLightMapByLine*NbTileLightMapByLine;



// ***************************************************************************
/**
 * A landscape bind exception.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
struct EBadBind : public Exception
{
private:
	mutable	std::string		_Output;

public:
	struct	CBindError
	{
		CBindError(sint z, sint p) {ZoneId= z; PatchId= p;}
		sint	ZoneId;
		sint	PatchId;
	};


	// The result list of bind errors.
	std::list<CBindError>	BindErrors;

public:
	EBadBind() {}
	virtual ~EBadBind() NL_OVERRIDE {}
	virtual const char	*what() const throw() NL_OVERRIDE;

};


struct ULandscapeTileCallback;

// ***************************************************************************
/**
 * A landscape. Use CZone to build zone, and use landscape to dynamically add/remove them, for render.
 *
 *	Limits:
 *		- 65535	zones max in whole world (16 bits ZoneId ).
 *		- 65535 patchs max by zone.
 *		- patch order 2x2 minimum.
 *		- patch order 16x16 maximum.
 *		- connectivity on a edge: 1/1, 1/2, or 1/4.
 *		- connectivity on a edge of a zone: 1/1 only.
 *		- The value of Noise amplitude is global and cannot go over 1 meter (+- 1m).
 *			Sorry, this is a FIXED (for ever) value which should NEVER change (because of Gfx database).
 *
 *	If you use the tiles mapped on the patches, load the near bank file (.bank) and the far bank file (.farbank)
 *  by seralizing TileBank and TileFarBank with those files. Then call initTileBanks.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CLandscape : public NLMISC::CRefCount
{
public:
	// The bank of tiles information. Each time you change the bank, you should call CLandscape::initTileBanks();
	CTileBank		TileBank;
	CTileFarBank	TileFarBank;
	class CLandscapeModel	*OwnerModel;

public:

	/// Constructor
	CLandscape();
	/// Destructor. clear().
	virtual ~CLandscape();


	/// \name Init/Build.
	// @{
	/// init the landscape VBuffers, texture cache etc...
	void			init();

	/** Add a zone which should be builded (or loaded), but not compiled. CLandscape compile it.
	 * The contents of newZone are copied into the landscape.
	 * \param newZone the new zone.
	 * \return true if OK, false otherwise. As example, Fail if newZone is already connected.
	 */
	bool			addZone(const CZone	&newZone);

	/** remove a zone by its unique Id.
	 * The zone is release()-ed (disconnected), then deleted.
	 * \param zoneId the zone to be removed.
	 * \return true if OK, false otherwise. As example, Fail if zone is not connected.
	 */
	bool			removeZone(uint16 zoneId);

	/// Disconnect, and Delete all zones.
	void			clear();

	/// Verify the binding of patchs of all zones. throw EBadBind if error.
	void			checkBinds();
	/// Verify the binding of patchs of one zone. throw EBadBind if error. nop if zone not loaded.
	void			checkBinds(uint16 zoneId);

	/**
	  *  Build tileBank. Call this after loading the near and far tile banks.
	  *
	  *  \return true if ok, false else. If false, far texture will be desactived.
	  */
	bool			initTileBanks ();
	// @}


	/// \name Landscape Parameters.
	// @{
	/// Set tile near distance. Default 50.f. maximized to length of Far alpha transition).
	void			setTileNear (float tileNear);
	/// Get tile near distance.
	float			getTileNear () const	{return _TileDistNear;}
	/// Set threshold for subdivsion quality. The lower is threshold, the more the landscape is subdivided. Default: 0.001.
	void			setThreshold (float thre);
	/// Get threshold.
	float			getThreshold () const	{return _Threshold;}
	void			setRefineMode(bool enabled) {_RefineMode= enabled;}
	bool			getRefineMode() const {return _RefineMode;}
	/// Set Maximum Tile subdivision. Valid values must be in [0..4]  (assert). Default is 0 (for now :) ).
	void			setTileMaxSubdivision (uint tileDiv);
	/// Get Maximum Tile subdivision.
	uint 			getTileMaxSubdivision ();

	/// Enable the noise or not. NB: only new tesselation computed is modified, so you should call it only at init time.
	void			setNoiseMode(bool enabled);
	bool			getNoiseMode() const;

	// invalidate all the tiles (force the tiles callbakc to be called again)
	void invalidateAllTiles();

	// Store it by landscape, and not only globally in CLandscapeGlobals statics.
	// @}


	/// \name Render methods.
	// @{
	/**
	 *	A driver is needed for VB allocation. The VB will be allocated only the first time the landscape is clipped.
	 *	Because no VB are created for invisible patchs.
	 *	call setDriver() before any clip().
	 */
	void			setDriver(IDriver *drv);
	/** Clip the landscape according to frustum.
	 *	Planes must be normalized.
	 */
	void			clip(const CVector &refineCenter, const std::vector<CPlane>	&pyramid);
	/** Refine/Geomorph the tesselation of the landscape.
	 */
	void			refine(const CVector &refineCenter);
	/** Render the landscape.
	 *	A more precise clip is made on TessBlocks. pyramid should be the same as one passed to clip().
	 *	For optimisation, this pyramid should contains only the Left/Right and Top/Bottom clip planes, in this order.
	 *	\param refineCenter should be the position of the camera
	 *	\param frontVector should be the J vector of the camera
	 */
	void			render(const CVector &refineCenter, const CVector &frontVector, const CPlane pyramid[NL3D_TESSBLOCK_NUM_CLIP_PLANE], bool doTileAddPass=false);

	/// Refine/Geomorph ALL the tesselation of the landscape, from the view point refineCenter. Even if !RefineMode.
	void			refineAll(const CVector &refineCenter);
	/// This is especially for Pacs. exlude a patch to be refineAll()ed.
	void			excludePatchFromRefineAll(sint zoneId, uint patch, bool exclude);


	/** This is especially for Pacs. Each Vertex->EndPos which is not a corner of a patch
	 *	is set to the mean of its 2 shared Patchs.
	 *	NB: Works with special cases of rectangular patchs and binded patchs.
	 */
	void			averageTesselationVertices();

	// Profile at Current landscape state (nlinfos)
	void			profileRender();

	// Get the Last refine pos setuped at refine();
	const CVector	&getOldRefineCenter() const {return _OldRefineCenter;}

	// @}


	/// \name Collision methods.
	// @{
	/** Build the set of faces of landscape, which are IN a bbox. Useful for collisions.
	 * The faces are builded at Tile level (2m*2m).
	 * \param bbox the bbox where faces are searched.
	 * \param faces the result of the build.
	 * \param faceSplit if true, Only faces which are IN or partialy IN the bbox are returned. Else the clipping is done
	 * on patch level. Worst, but faster.
	 */
	void			buildCollideFaces(const CAABBoxExt &bbox, std::vector<CTriangle> &faces, bool faceSplit);
	/** Build the set of faces of landscape, from a certain patch. Useful for collisions. Triangles are built first in S
	 *  then T order. There is two triangles by tiles. So the number of triangles for a patch is 2*OrderS*OrderT.
	 */
	void			buildCollideFaces(sint zoneId, sint patch, std::vector<CTriangle> &faces);

	/** method relatively identical than buildCollideFaces(bbox....). Differences below:
	 * NB: this method use first a quadgrid to locate patchs of interest, then for each patch, it uses a
	 * convex hull subdivion to search in O(logn) what part of the patch to insert.
	 * \param bbox the bbox to test against. NB: you should modify your bbox according to heighfield.
	 * \param triangles array to be filled (array first cleared, then elements added).
	 * \param tileTessLevel 0,1 or 2  size of the triangles (2*2m, 1*1m or 0.5*0.5m). Level of subdivision of a tile.
	 */
	void			buildTrianglesInBBox(const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tileTessLevel);


	/** Same as buildTrianglesInBBox(), but fill blockId instead of raw triangles.
	 * NB: this method use first a quadgrid to locate patchs of interest, then for each patch, it uses a
	 * convex hull subdivion to search in O(logn) what part of the patch to insert.
	 * \param bbox the bbox to test against. NB: you should modify your bbox according to heighfield.
	 * \param paBlockIds array to be filled (no clear performed, elements added).
	 */
	void			buildPatchBlocksInBBox(const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds);
	/** Fill a CPatchQuadBlock, from its required PatchId.
	 * nlassert(PatchId size is less than NL_PATCH_BLOCK_MAX_QUAD)
	 */
	void			fillPatchQuadBlock(CPatchQuadBlock &quadBlock) const;


	/** From the current tesselation of a patch of landscape, and a UV in this patch, return tesselated position.
	 * NB: return Null if patch not found.
	 */
	CVector			getTesselatedPos(const CPatchIdent &patchId, const CUV &uv) const;


	/** From the current tesselation of landscape, build the list of leaves faces.
	 *	Warning: ptrs are only valid between 2 tesselation (2 calls of Scene::render() or 2 calls of refine*() etc...)
	 */
	void			getTessellationLeaves(std::vector<const CTessFace*>  &leaves) const;

	/** Get the camera 3rd person collision against the TileFaces (ie only under approx 50 m)
	 *	return a [0,1] value. 0 => collision at start. 1 => no collision.
	 *	\param radius is the radius of the 'cylinder'
	 *	\param cone if true, the object tested is a cone (radius goes to end)
	 */
	float			getCameraCollision(const CVector &start, const CVector &end, float radius, bool cone);

	/** Get the ray collision against the TileFaces (ie only under approx 50 m)
	 *	return a [0,1] value. 0 => collision at start. 1 => no collision.
	 */
	float			getRayCollision(const CVector &start, const CVector &end);

	// @}



	/// \name Accessors.
	// @{
	/** Get a zone pointer.
	 *
	 * \param zoneId the zone of the update.
	 * \return Return a zone pointer. NULL if the zone doesn't exist or isn't loaded.
	 */
	CZone*			getZone (sint zoneId);
	/** Get a zone pointer.
	 *
	 * \param zoneId the zone of the update.
	 * \return Return a zone pointer. NULL if the zone doesn't exist or isn't loaded.
	 */
	const CZone*	getZone (sint zoneId) const;
	/** Return list of zone loaded.
	 */
	void			getZoneList(std::vector<uint16>	&zoneIds) const;
	/// From an Id, return the name of the zone, in the form "150_EM", without extension.
	static void		buildZoneName(sint zoneId, std::string &zoneName);
	// @}


	/// \name Tile mgt.
	// @{
	/// Force a range of tiles to be loaded in the driver...
	void			flushTiles(IDriver *drv, uint32 tileStart, uint32 nbTiles);
	/** Force a range of tiles to be unloaded. You should call changePatchTextureAndColor() on all patch
	 *	or ensure that all zones are deleted.
	 */
	void			releaseTiles(uint32 tileStart, uint32 nbTiles);
	/// Delete All tiles. All zones must be deleted before (nlassert)
	void			releaseAllTiles();

	/// Return the texture for a tile Id. Useful for Tile edition.
	NLMISC::CSmartPtr<ITexture>		getTileTexture(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias);

	/// Return the tile element for a patch at specific UV. Useful for getting surface data. Return NULL if not found.
	CTileElement					*getTileElement(const CPatchIdent &patchId, const CUV &uv);
	// @}

	/// \name Lighting.
	// @{
	/**
	  *  Setup the light color use for static illumination.
	  *
	  *  \param diffuse is the color of the diffuse componante of the lighting.
	  *  \param ambiant is the color of the ambiante componante of the lighting.
	  *  \param multiply is the multiply factor. Final color is (diffuse*multiply*shading+ambiant*(1.0-shading))
	  */
	void setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply);
	/**
	  *  Get the light color by shading table.
	  *
	  *  \return a CRGBA[256] array. It give the static light color for a shading value.
	  */
	const CRGBA* getStaticLight () const
	{
		return _LightValue;
	}


	/**
	  *  Setup the equivalent material diffuse component used for both Static and Dynamic PointLights.
	  *	 Default is White.
	  */
	void	setPointLightDiffuseMaterial(CRGBA diffuse);
	/**
	  *	\see setPointLightDiffuseMaterial
	  */
	CRGBA	getPointLightDiffuseMaterial () const
	{
		return _PointLightDiffuseMaterial;
	}


	/**
	  *  Enable automatic near lightmap computing. use setupStaticLight().
	  *	 Default is disabled.
	  *	 NB: calling this method won't flush all texture near already computed.
	  */
	void	enableAutomaticLighting(bool enable);
	/**
	  *  For automatic near lightmap computing (if enabled): setup the lightdir
	  *
	  *  \param lightDir is the direction of the light vector used for the lighting. NB: the vector is normalized.
	  */
	void	setupAutomaticLightDir(const CVector &lightDir);
	/// return true if AutomaticLighting is enabled.
	bool	getAutomaticLighting() const {return _AutomaticLighting;}
	/// return the light direction setuped in enableAutomaticLighting().
	const CVector &getAutomaticLightDir() const {return _AutomaticLightDir;}


	/** This method remove all PointLights in all Zones, and hence reset TileLightInfluences.
	 */
	void	removeAllPointLights();


	/// update the Light factor for all pointLights in All zones, according to scene LightGroups and AnimatedLights.
	void	setPointLightFactor(const CScene &scene);


	// To init lightmap information
	void	initAnimatedLightIndex(const CScene &scene);


	// @}


	/// \name HeightField DeltaZ.
	// @{
	/// return the HeightField DeltaZ for the 2D position. (0,0,dZ) is returned.
	CVector		getHeightFieldDeltaZ(float x, float y) const;
	/** set the HeightField data. NB: take lot of place in memory.
	 * only one is possible. You should setup this heightfield around the zones which will be loaded.
	 * It is applied only when a zone is loaded, so you should setup it 2km around the user, each time you move too far
	 * from a previous place (eg 160m from last setup).
	 */
	void		setHeightField(const CHeightMap &hf);
	// @}


	/// Micro-Vegetation.
	// @{

	/** init the vegetable layer models in the CScene (see CVegetableManager).
	 *	NB: MOT stuff (called by CLandscapeModel), don't use it.
	 */
	void		createVegetableBlendLayersModels(CScene *scene);

	/** set the vegetable manager Time (in seconds)
	 *	NB: MOT stuff (called by CLandscapeModel), don't use it.
	 */
	void		setVegetableTime(double time);

	/** set the vegetable manager System Time (in seconds) for update lighting
	 *	NB: MOT stuff (called by CLandscapeModel), don't use it.
	 */
	void		setVegetableUpdateLightingTime(double time);


	/** enable the vegetable management in landscape. Valid only if VertexShader is OK.
	 *	if true, register TileBank vegetables Shape to manager.
	 */
	void		enableVegetable(bool enable);

	/** return if the vegetable management is actually activated:
	 *	actually return _VerexShaderOk && _VegetableEnabled.
	 */
	bool		isVegetableActive() const;

	/** load a texture for the vegetable, lookup in CPath
	 */
	void		loadVegetableTexture(const std::string &textureFileName);

	/**	setup lighting ambient and diffuse for vegetable.
	 */
	void		setupVegetableLighting(const CRGBA &ambient, const CRGBA &diffuse, const CVector &directionalLight);

	/** set the vegetable Wind for animation.
	 *	All thoses variables may be modified each frame without penalty.
	 *
	 *	\param windDir is the direction of the wind. NB: only XY direction is kept.
	 *	\param windFreq is the frequency for the animation (speed)
	 *	\param windPower is the power of the wind, and is a factor (0..1) of Bend
	 *	\param windBendMin is a value in (0..1) which indicate how much the vegetables are bended at minimum
	 *	(for very powerfull wind)
	 */
	void		setVegetableWind(const CVector &windDir, float windFreq, float windPower, float windBendMin);


	/** return the number of faces displayed in Driver with the vegetable manager.
	 *	Out  CPrimitiveProfile::NTriangles displayed by vegetable part is returned.
	 */
	uint		getNumVegetableFaceRendered() const;


	/** set the frequency of Vegetable lighting update. If freq==1, ALL lighted igs are updated each second.
	 *	e.g: if 1/20, then every 20 seconds, all Igs are updated.
	 *	If you set 0, no update will be done at all (this is the default setup!!).
	 */
	void		setVegetableUpdateLightingFrequency(float freq);


	/** Debug purpose only : setup the colors of the patch of all the currently loaded zones
	  * so that it shows which tiles have vegetable disabled, or are above, below water.
	  * User provides a table with 4 colors for each state :
	  * color 0 = above water
	  * color 1 = underwater
	  * color 2 = intersect water
	  * color 3 = vegetable disabled
	  */
	void		setupColorsFromTileFlags(const NLMISC::CRGBA colors[4]);

	/** Set a density ratio [0, 1] to possibly reduce the amount of micro vegetable drawn. Default to 1
	 */
	void		setVegetableDensity(float density);

	/** Get the density ratio [0, 1] of micro vegetable drawn. Default to 1
	 */
	float		getVegetableDensity() const;

	// @}


	/// \name Lightmap get interface.
	// @{

	/// Get the lumel under the position. return 255 if invalid patchId.
	uint8		getLumel(const CPatchIdent &patchId, const CUV &uv) const;

	/// Append lights under the position to pointLightList. Do nothing if invalid patchId.
	void		appendTileLightInfluences(const CPatchIdent &patchId, const CUV &uv,
		std::vector<CPointLightInfluence> &pointLightList) const;

	// @}


	/// \name Precision ZBuffer mgt.
	// @{

	/** Set the ModelPosition (for Precision ZBuffer purpose). NB: the model Pos may be floor-ed
	  *	(for greater float precision). Should be setup to the camera position each frame.
	  *
	  *	NB: if vertexProgram is used, it is as faster as before (because of geomorph done each frame,
	  *	and because of VP MAD instruction).
	  *	NB: if vertexProgram is not used, it is a little slower, because of the substraction.
	  */
	void			setPZBModelPosition(const CVector &pos);

	/** \see setPZBModelPosition()
	  */
	const CVector	&getPZBModelPosition() const {return _PZBModelPosition;}

	// @}


	/// \name UpdateLighting management
	// @{

	/** update the lighting of patch, within a certain amount of time.
	 *	called by CLandscapeModel
	 */
	void			updateLighting(double time);

	/** set the frequency of lighting update. If freq==1, ALL patchs are updated each second.
	 *	e.g: if 1/20, then every 20 seconds, all patchs are updated.
	 *	If you set 0, no update will be done at all (this is the default setup!!).
	 */
	void			setUpdateLightingFrequency(float freq);

	/** update the lighting of ALL patch (slow method). NB: work even if UpdateLightingFrequency==0
	 *	Additionaly, vegetables are also ALL updated.
	 */
	void			updateLightingAll();


	// @}


	/// \name Dynamic Lighting management
	// @{

	/** Compute dynamic lightmaps
	 *	\param pls list of pointLigths to influence landscape
	 */
	void			computeDynamicLighting(const std::vector<CPointLight*>	&pls);

	/** For bench only. return approximate ammount of memory taken by dynamic lightmap in RAM.
	 *	This is sizeof(global Texture in RAM) + all patchDLMContext.
	 *	NB: not so slow, but do not call in final release.
	 */
	uint			getDynamicLightingMemoryLoad() const;

	/** Set PointLight Max Attenuation End landscape support. Every pointLight AttEnd is clamped to this value.
	 *	Default is 30.f.
	 */
	void			setDynamicLightingMaxAttEnd(float maxAttEnd);

	/** see setDynamicLightingMaxAttEnd()
	 */
	float			getDynamicLightingMaxAttEnd() const {return _DLMMaxAttEnd;}

	/** For Vegetable Dynamic ligthing only: this is an approximate color of all vegetables.
	 *	Default is (180, 180, 180).
	 */
	void			setDLMGlobalVegetableColor(CRGBA gvc);

	/** see setDLMGlobalVegetableColor()
	 */
	CRGBA			getDLMGlobalVegetableColor() const {return _DLMGlobalVegetableColor;}

	// @}


	/// \name Dynamic ShadowMap
	// @{
	void			receiveShadowMap(IDriver *drv, CShadowMap *shadowMap, const CVector &casterPos, const CMaterial &shadowMat, const CVector &pzb);
	// @}


	/// \name Tile added/removed callback
	// @{
	void										 addTileCallback(ULandscapeTileCallback *cb);
	void										 removeTileCallback(ULandscapeTileCallback *cb);
	bool										 isTileCallback(ULandscapeTileCallback *cb) const;
	const std::vector<ULandscapeTileCallback *> &getTileCallbacks() const { return _TileCallbacks; }
	// @}

	// lockBuffers(), called by updateGlobalsAndLockBuffers().
	void			lockBuffers ();
	// unlockBuffers. This is the END call for updateGlobalsAndLockBuffers().
	void			unlockBuffers (bool force = false);
	// Check if buffers are locked
	bool			isLocked() const { return _LockCount != 0; }

	// for advanced use (decal rendering)
	CShadowPolyReceiver &getShadowPolyReceiver() { return _ShadowPolyReceiver; }

	// modify ZBuffer test of landscape material
	virtual	void					setZFunc(CMaterial::ZFunc val);


// ********************************
private:
	// Private part used by CTessFace / CPatch / CZone.
	friend class	CTessFace;
	friend class	CPatch;
	friend class	CZone;

	std::vector<ULandscapeTileCallback *> _TileCallbacks;

	/// \name Allocators.
	// @{

	CBlockMemory<CTessFace>			TessFaceAllocator;
	CBlockMemory<CTessVertex>		TessVertexAllocator;
	CBlockMemory<CTessNearVertex>	TessNearVertexAllocator;
	CBlockMemory<CTessFarVertex>	TessFarVertexAllocator;
	CBlockMemory<CTileMaterial>		TileMaterialAllocator;
	CBlockMemory<CTileFace>			TileFaceAllocator;

	CTessFace			*newTessFace();
	CTessVertex			*newTessVertex();
	CTessNearVertex		*newTessNearVertex();
	CTessFarVertex		*newTessFarVertex();
	CTileMaterial		*newTileMaterial();
	CTileFace			*newTileFace();

	void				deleteTessFace(CTessFace *f);
	void				deleteTessVertex(CTessVertex *v);
	void				deleteTessNearVertex(CTessNearVertex *v);
	void				deleteTessFarVertex(CTessFarVertex *v);
	void				deleteTileMaterial(CTileMaterial *tm);
	void				deleteTileFace(CTileFace *tf);

	// Allocator of FaceVector for allocation of packed triangles indices.
	CLandscapeFaceVectorManager		_FaceVectorManager;
	// This is not a valid TessBlock. But it is used as a Root for modification list.
	// Can't use a ptr, because of ~CTessBlock().
	CTessBlock						_TessBlockModificationRoot;
	// @}


	// Return the render pass for a far texture here.
	CPatchRdrPass	*getFarRenderPass(CPatch* pPatch, uint farIndex, float& far1UScale, float& far1VScale, float& far1UBias, float& far1VBias, bool& bRot);
	// Free the render pass for a far texture here.
	void			freeFarRenderPass (CPatch* pPatch, CPatchRdrPass* pass, uint farIndex);
	// Return the render pass for a tile Id, and a patch Lightmap.
	CPatchRdrPass	*getTileRenderPass(uint16 tileId, bool additiveRdrPass);
	// Return the UvScaleBias for a tile Id. uv.z has the scale info. uv.x has the BiasU, and uv.y has the BiasV.
	// if bitmap type is CTile::alpha, Return also the additionla rot for alpha (else 0).
	void			getTileUvScaleBiasRot(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias, uint8 &rotAlpha);

	// release Far render pass/reset Tile/Far render. Delete also VB, and FaceVectors
	void			resetRenderFarAndDeleteVBFV();
	/// For changing TileMaxSubdivision. force tesselation to be under tile.
	void			forceMergeAtTileLevel();

	// Update globals value to CTessFace, and lock Buffers if possible.
	void updateGlobalsAndLockBuffers (const CVector &refineCenter);
	// update TheFaceVector for which the faces may have been modified during refine(), refineAll() etc....
	void updateTessBlocksFaceVector();


	// System: update UL links, but don't call _TextureFars.erase()
	void clearFarRenderPass (CPatchRdrPass* pass);


private:
	TZoneMap		Zones;

	// Parameters.
	float			_TileDistNear;
	float			_Threshold;
	bool			_RefineMode;
	float			_FarTransition;
	uint			_TileMaxSubdivision;
	// For VertexProgram. true if change has occurred in threshold since the last render().
	float			_VPThresholdChange;

	/// \name VertexBuffer mgt.
	// @{
	CRefPtr<IDriver>			_Driver;
	CLandscapeVBAllocator		_Far0VB;
	CLandscapeVBAllocator		_Far1VB;
	CLandscapeVBAllocator		_TileVB;
	uint						_LockCount;	// Lock counter
	// True if we can compute Geomorph and Alpha with VertexShader.
	bool						_VertexShaderOk;
	// For CZone::refreshTesselationGeometry().
	bool						_RenderMustRefillVB;
	// @}


	// Tiles Types.
	//=============
	// Texture Map. Use a RefPtr because TileTextureMap must not reference the object, but the ptr.
	typedef	NLMISC::CRefPtr<ITexture>			RPTexture;
	typedef	std::map<std::string, RPTexture>	TTileTextureMap;
	typedef	TTileTextureMap::iterator			ItTileTextureMap;
	// RdrPass Set.
	typedef	std::set<CPatchRdrPass>				TTileRdrPassSet;
	typedef	TTileRdrPassSet::iterator			ItTileRdrPassSet;
	typedef NLMISC::CSmartPtr<CPatchRdrPass>	TSPRenderPass;

	// The additional realtime structure for a tile.
	struct	CTileInfo
	{
		// NB: CSmartPtr are not used for simplicity, and because of TTileRdrPassSet...
		// CPatchRdrPass::RefCount are excplictly incremented/decremented...
		// The rdrpass for diffuse+Alpha material.
		CPatchRdrPass	*DiffuseRdrPass;
		// The rdrpass for additive material (may be NULL if no additive part).
		CPatchRdrPass	*AdditiveRdrPass;
		// The scale/Bias to access those tiles in the big texture.
		// uv.z has the scale info. uv.x has the BiasU, and uv.y has the BiasV.
		// Manages the demi-texel on tile border too.
		CVector			DiffuseUvScaleBias;
		CVector			AlphaUvScaleBias;
		CVector			AdditiveUvScaleBias;
		// The additional rotation for this tile, in alpha.
		uint8			RotAlpha;
	};


	// Tiles Data.
	//=============
	// The map of tile texture loaded.
	TTileTextureMap				TileTextureMap;
	// The set of tile Rdr Pass.
	TTileRdrPassSet				TileRdrPassSet;
	// The parrallel array of tile of those existing in TileBank. size of NbTilesMax.
	std::vector<CTileInfo*>		TileInfos;
	// The Lightmap rdrpass for tiles.
	// must have a vector of pointer, because of vector reallocation.
	std::vector<TSPRenderPass>		_TextureNears;
	uint							_NFreeLightMaps;


	// The Tile material.
	CMaterial		TileMaterial;

	// The Far material.
	CMaterial		FarMaterial;


	// *** Far texture

	// ** Some types

	// The vector of far render pass
	// must have a vector of pointer, because of vector reallocation.
	typedef	std::vector<TSPRenderPass>::iterator	ItSPRenderPassVector;
	std::vector<TSPRenderPass>		_TextureFars;
	bool							_FarInitialized;

	// Used internaly by initTileBanks
	bool										eraseTileFarIfNotGood (uint tileNumber, uint sizeOrder0, uint sizeOrder1, uint sizeOrder2);

	// *** Lighting
	CRGBA			_LightValue[256];

	bool			_AutomaticLighting;
	CVector			_AutomaticLightDir;

private:
	// Internal only. Force load of the tile (with TileBank).
	void			loadTile(uint16 tileId);
	void			releaseTile(uint16 tileId);
	ITexture		*findTileTexture(const std::string &textName, bool clamp);
	CPatchRdrPass	*findTileRdrPass(const CPatchRdrPass &pass);

	// Tile LightMap mgt. NB: a lightmap is now a 2x2 tiles lightmap (10x10 pixels).
	// @{
	// Compute and get a lightmapId/lightmap renderpass.
	// lightmap returned is to be uses with getTileRenderPass(). The id returned must be stored.
	uint		getTileLightMap(CRGBA  map[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE], CPatchRdrPass *&lightmapRdrPass);
	// tileLightMapId must be the id returned  by getTileLightMap().
	void		getTileLightMapUvInfo(uint tileLightMapId, CVector &uvScaleBias);
	// tileLightMapId must be the id returned  by getTileLightMap().
	void		releaseTileLightMap(uint tileLightMapId);

	// refill a lightmap already computed. tileLightMapId must be the id returned  by getTileLightMap().
	void		refillTileLightMap(uint tileLightMapId, CRGBA  map[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE]);

	// @}


	// check a zone, adding error to exception.
	void			checkZoneBinds(CZone &curZone, EBadBind &bindError);


	/** A Bezier patch of One value only.
	 * NB: unlike CBezierPatch, layout is inverted on Y. (NB: same formulas...)
	 */
	struct	CBezierPatchZ
	{
		/// The vertices a,b,c,d of the quad patch.
		float		Vertices[4];
		/// The tangents ab, ba, bc, cb, cd, dc, da, ad. NB: tangents are points, not vectors.
		float		Tangents[8];
		/// The interiors, ia,ib,ic,id. NB: interiors are points, not vectors.
		float		Interiors[4];

		/// make default Interiors, according to Vertices and Tangents.
		void		makeInteriors();
		/// Evaluate.
		float		eval(float s, float t) const;			// s,t coordinates for quad.
	};


	// HeightFields.
	struct	CHeightField
	{
		std::vector<CBezierPatchZ>		ZPatchs;

		/// The origin of the bottom-left corner of this heightmap.
		float			OriginX, OriginY;
		/// The size of one Element ot this HeightMap (eg: 160x160 for a zone).
		float			SizeX, SizeY;
		float			OOSizeX, OOSizeY;
		/// The size of this array. Heights.size
		uint			Width, Height;
	};
	CHeightField	_HeightField;


	/// \name Visual Collision system.
	// @{

	// A CPatchIdent, with a ptr to the compiled CPatch
	struct	CPatchIdentEx : public CPatchIdent
	{
		const CPatch	*Patch;
	};

	/// The QuadGrid of patch.
	CQuadGrid<CPatchIdentEx>	_PatchQuadGrid;
	static	const uint			_PatchQuadGridSize;
	static	const float			_PatchQuadGridEltSize;

	/** This method search only on the given patch.
	 * NB: this method use a convex hull subdivion to search in O(logn) what part of the patch to insert.
	 * \param bbox the bbox to test against.
	 * \param triangles array to be filled (no clear performed, elements added).
	 * \param tileTessLevel 0,1 or 2  size of the triangles (2*2m, 1*1m or 0.5*0.5m). Level of subdivision of a tile.
	 */
	void			addTrianglesInBBox(const CPatchIdentEx &paIdEx, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tileTessLevel) const;

	/** private version of buildPatchBlocksInBBox, searching only for one patch.
	 */
	void			addPatchBlocksInBBox(const CPatchIdentEx &paIdEx, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds);
	// @}


	/// Noise Geometry.
	// @{
	// guess.
	bool			_NoiseEnabled;
	// @}


	/// Priority list.
	// @{
	/// The priority list of faces which may need to split
	CTessFacePriorityList		_SplitPriorityList;
	/// The priority list of faces which may need to merge
	CTessFacePriorityList		_MergePriorityList;
	/// OldRefineCenter setuped in prec refine()
	CVector						_OldRefineCenter;
	bool						_MustRefineAllAtNextRefine;
	/// newTessFace() append the face to _RootNewLeaves.
	CTessFacePListNode			_RootNewLeaves;
	// @}


	/// Micro-Vegetation.
	// @{
	/// The VegetableManager. (ptr only for include speed).
	CVegetableManager			*_VegetableManager;

	/// Tells if the Vegetable Managemnt is enabled.
	bool						_VegetableManagerEnabled;
	/// Tells if the current driver support vegetable.
	bool						_DriverOkForVegetable;

	/// List of VegetableBlock, to be tested for creation each frame.
	CTessList<CLandscapeVegetableBlock>		_VegetableBlockList;

	/** For a given tile Id, look into tileSet, and get the tile vegetable descriptor
	 *	\param tileId the tile [0..65535] to get the list of vegetable to create.
	 */
	const CTileVegetableDesc	&getTileVegetableDesc(uint16 tileId);

	// @}


	/// \name Precision ZBuffer mgt.
	// @{
	/** \see setPZBModelPosition()
	  */
	CVector						_PZBModelPosition;
	// @}


	/// \name UpdateLighting management
	// @{
	// Last update time.
	double						_ULPrecTime;
	bool						_ULPrecTimeInit;
	double						_ULTime;
	/// Frequency of update.
	float						_ULFrequency;


	/// Far UpdateLighting.
	sint						_ULTotalFarPixels;
	/// Current number of far pixels to update. If negative, I have some advance.
	float						_ULFarPixelsToUpdate;
	/// The current TextureFar rendered.
	CTextureFar					*_ULRootTextureFar;


	/// Near UpdateLighting.
	sint						_ULTotalNearPixels;
	/// Current number of near pixels to update. If negative, I have some advance.
	float						_ULNearPixelsToUpdate;
	/// The current patch rendered.
	CPatch						*_ULRootNearPatch;
	/// Current tessBlock id in the current patch processed
	uint						_ULNearCurrentTessBlockId;

	/// Used by Patch to link/unlink from _ULRootNearPatch
	void			linkPatchToNearUL(CPatch *patch);
	void			unlinkPatchFromNearUL(CPatch *patch);


	/// Update All Far texture, given a ratio along total lumels
	void			updateLightingTextureFar(float ratio);
	/// Update All Near texture, given a ratio along total lumels
	void			updateLightingTextureNear(float ratio);


	// @}


	/// \name DynamicLighting management
	// @{
	/// The dynamic lightmap Texture.
	NLMISC::CSmartPtr<ITexture>	_TextureDLM;
	/// for CPatch.
	CTextureDLM					*getTextureDLM() const {return (CTextureDLM*)(ITexture*)_TextureDLM;}
	/// List of DLM Context.
	CPatchDLMContextList		*_PatchDLMContextList;
	/// for CPatch.
	CPatchDLMContextList		*getPatchDLMContextList() const {return _PatchDLMContextList;}
	/// Max AttEnd
	float						_DLMMaxAttEnd;

	/// an approximate value used to simulate diffuse material of vegetables
	CRGBA						_DLMGlobalVegetableColor;

	/// The diffuse material of landscape, used for StaticPointLights and DynamicPointLights
	CRGBA						_PointLightDiffuseMaterial;

	// @}


	/// \name Dynamic ShadowMap
	// @{
	CShadowPolyReceiver			_ShadowPolyReceiver;
	void						appendToShadowPolyReceiver(CTessFace *face);
	void						removeFromShadowPolyReceiver(CTessFace *face);
	// @}

	/// \name Texture Profiling
	// @{
	NLMISC::CSmartPtr<ITexture::CTextureCategory>	_TextureTileCategory;
	NLMISC::CSmartPtr<ITexture::CTextureCategory>	_TextureFarCategory;
	NLMISC::CSmartPtr<ITexture::CTextureCategory>	_TextureNearCategory;
	// @}

};



} // NL3D


#endif // NL_LANDSCAPE_H

/* End of landscape.h */
