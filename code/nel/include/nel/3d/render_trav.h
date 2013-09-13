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

#ifndef NL_RENDER_TRAV_H
#define NL_RENDER_TRAV_H

#include "nel/3d/trav_scene.h"
#include "nel/3d/ordering_table.h"
#include "nel/3d/layered_ordering_table.h"
#include "nel/misc/rgba.h"
#include "nel/3d/viewport.h"
#include "nel/3d/light_contribution.h"
#include "nel/3d/light.h"
#include "nel/3d/mesh_block_manager.h"
#include "nel/3d/shadow_map_manager.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/vertex_program.h"
#include <vector>


namespace	NL3D
{

using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;

class	IDriver;
class	CMaterial;

class	CTransform;
class	CLandscapeModel;

class	CVertexStreamManager;
class   CWaterModel;

// ***************************************************************************
/* Skin Manager setup
 * For the moment, the skin manager vertex buffer routes all its UV channel to UV0.
 * See void	CVertexStreamManager::init(IDriver *driver, uint vertexFormat, uint maxVertices)
 *	use 8 VBswap for minimal(=> no) lock() time. 2 is not enough (saw 7 ms lost because of this)
 *	=> size of the manager is 1280 Ko
 *	NB: 5000 vertices max for a model is a strong limitation, but it's OK for now...
 */
#define	NL3D_MESH_SKIN_MANAGER_VERTEXFORMAT		(CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag | CVertexBuffer::TexCoord0Flag)
#define	NL3D_MESH_SKIN_MANAGER_MAXVERTICES		5000
#define	NL3D_MESH_SKIN_MANAGER_NUMVB			8

/* Same for Shadow Generation.
 * NB: need much less Vertices because: 1/ foolish to do more. 2/ Only position=> no UV/Normal discontinuities.
 *	=> size of the manager is 1.3 Mo
 *	use 8 VBswap for minimal(=> no) lock() time. 2 is not enough (saw 7 ms lost because of this)
 *	=> size of the manager is 280 Ko
 */
#define	NL3D_SHADOW_MESH_SKIN_MANAGER_VERTEXFORMAT		(CVertexBuffer::PositionFlag)
#define	NL3D_SHADOW_MESH_SKIN_MANAGER_MAXVERTICES		3000
#define	NL3D_SHADOW_MESH_SKIN_MANAGER_NUMVB				8

/// Container for lighted vertex program.
class CVertexProgramLighted : CVertexProgram
{
public:
	static const uint MaxLight = 4;
	static const uint MaxPointLight = (MaxLight - 1);
	struct CIdxLighted
	{
		uint Ambient;
		uint Diffuse[MaxLight];
		uint Specular[MaxLight];
		uint DirOrPos[MaxLight]; // light 0, directional sun; light 1,2,3, omni point light
		uint EyePosition;
		uint DiffuseAlpha;
	};
	struct CFeaturesLighted
	{
		/// Number of point lights that this program is generated for, varies from 0 to 3.
		uint NumActivePointLights;
		bool SupportSpecular;
		bool Normalize;
		/// Start of constants to use for lighting with assembly shaders.
		uint CtStartNeLVP;
	};
	CVertexProgramLighted() { }
	virtual ~CVertexProgramLighted() { }
	virtual void buildInfo();
	const CIdxLighted &idxLighted() const { return m_IdxLighted; }
	const CFeaturesLighted &featuresLighted() const { return m_FeaturesLighted; }

private:
	CIdxLighted m_IdxLighted;
	CFeaturesLighted m_FeaturesLighted;

};


// ***************************************************************************
/**
 * The Render traversal.
 * The purpose of this traversal is to render a list of models. This traversals is tightly linked to the cliptraversal.
 * The clipTraversals insert directly the models with CRenderTrav::addRenderModel(m). The traverse() method should
 * render all the render models with IDriver.
 *
 * This traversal has no graph of models
 *
 * \b USER \b RULES: Before using traverse() on a render traversal, you should:
 *	- setFrustum() the camera shape (focale....)
 *	- setCamMatrix() for the camera transform
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CRenderTrav : public CTravCameraScene
{
public:

	/// Constructor
	CRenderTrav();

	/// \name ITravScene Implementation.
	//@{
	/** First traverse the root (if any), then render the render list.
	 * NB: no Driver clear buffers (color or ZBuffer) are done....
	 * \param renderPart : The part of the scene that must be rendered
	 * \param newRender true If scene render is beginning. Otherwise other parts of the scene have already been rendered.
	 */
	void				traverse(UScene::TRenderPart renderPart, bool newRender);
	//@}

	/// \name RenderList.
	//@{
	/// Clear the list of rendered models
	void			clearRenderList();
	/** Add a model to the list of rendered models. \b DOESN'T \b CHECK if already inserted.
	 */
	void			addRenderModel(CTransform *m)
	{
		// for possible removeRenderModel()
		m->_IndexLSBInRenderList= uint8(_CurrentNumVisibleModels&255);
		// add the model in the list
		RenderList[_CurrentNumVisibleModels]= m;
		_CurrentNumVisibleModels++;
	}
	// for createModel().
	void			reserveRenderList(uint numModels);

	/* This is for the rare case where objects are deleted during CScene::render(). called by deleteModel()
	 *	This method don't need to be called by ~CTransform
	 */
	void			removeRenderModel(CTransform *m);

	/// Special for water (list of water to render)
	void			clearWaterModelList();

	//@}


	// setup the Driver for render
	void			setDriver(IDriver *drv) {Driver= drv;}
	IDriver			*getDriver() {return Driver;}
	// Yoyo: temporary. May be used later if we decide to support a PBuffer to render texture for ShadowMap
	IDriver			*getAuxDriver() {return Driver;}
	void			setViewport (const CViewport& viewport)
	{
		_Viewport = viewport;
	}
	CViewport		getViewport () const
	{
		return _Viewport;
	}

	bool isCurrentPassOpaque() { return _CurrentPassOpaque; }

	/** Set the order or rendering for transparent objects.
	  * In real case, with direct order, we have:
	  * - Underwater is rendered.
	  * - Water is rendered.
	  * - Objects above water are rendered.
	  */
	void  setLayersRenderingOrder(bool directOrder = true) { _LayersRenderingOrder = directOrder; }
	bool  getLayersRenderingOrder() const { return _LayersRenderingOrder; }

	/** Setup transparency sorting
	  * \param maxPriority Defines the valid range for priority in the [0, n] interval. By default, there's no prioriy sorting (0 -> single priority, 255 -> 256 possible priorities)
	  *                    Objects with highers priority are displayed before any other object with lower priority,
	  *                    whatever their distance is.
	  * \param NbDistanceEntries Defines the granularity for distance sorting. A value of N with a view distance of D meters means
	  *                          that the sorting accuracy will be of D / N meters at worst (when visible objects occupy the whole distance range)
	  * NB : The memory allocated is a multiple of NumPriority * NbDistanceEntries * 2 (2 if because of water ordering)
	  */
	void setupTransparencySorting(uint8 maxPriority = 0, uint NbDistanceEntries = 1024);



	/// \name Render Lighting Setup.
	// @{

	// False by default. setuped by CScene
	bool						LightingSystemEnabled;

	// Global ambient. Default is (50,50,50).
	NLMISC::CRGBA				AmbientGlobal;
	// The Sun Setup.
	NLMISC::CRGBA				SunAmbient, SunDiffuse, SunSpecular;
	// set the direction of the sun. dir is normalized.
	void						setSunDirection(const CVector &dir);
	const CVector				getSunDirection() const {return _SunDirection;}

	// @}


	/** Set/Replace the MeshSkinManager. NULL by default => skinning is slower.
	 *	The ptr is handled but not deleted.
	 *	There should be one MeshSkinManager per driver.
	 */
	void						setMeshSkinManager(CVertexStreamManager *msm);

	/// get the MeshSkinManager
	CVertexStreamManager		*getMeshSkinManager() const {return _MeshSkinManager;}

	/// get the CShadowMapManager
	CShadowMapManager			&getShadowMapManager() {return _ShadowMapManager;}
	const CShadowMapManager		&getShadowMapManager() const {return _ShadowMapManager;}

	/// the MeshSkinManager for Shadow. Same Behaviour than std MeshSkinManager. NB: the Shadow MSM is inited with AuxDriver.
	void						setShadowMeshSkinManager(CVertexStreamManager *msm);
	CVertexStreamManager		*getShadowMeshSkinManager() const {return _ShadowMeshSkinManager;}


	// add a landscape. Special for CLandscapeModel::traverseRender();
	void			addRenderLandscape(CLandscapeModel *model);


	/// \name Temp Debug
	//@{
	// Test Memory of water model render list (because someone crash it...)
	// Yoyo: this crash seems to be fixed, but i leave the code, in case of.....
	void			debugWaterModelMemory(const char *tag, bool dumpList= false);
	//@}

// ******************
public:

	/// \name Render Lighting Setup. FOR MODEL TRAVERSING ONLY.
	// @{

	// Max VP Light setup Infos.
	enum	{MaxVPLight = CVertexProgramLighted::MaxLight};

	/** reset the lighting setup in the driver (all lights are disabled).
	 *	called at beginning of traverse(). Must be called by any model (before and after rendering)
	 *	that wish to use CDriver::setLight() instead of the standard behavior with changeLightSetup()
	 */
	void		resetLightSetup();

	/** setup the driver to the given lightContribution.
	 *	if lightContribution==NULL, then all currently enabled lights are disabled.
	 *	NB: lightContribution is cached, so if same than preceding, no-op.
	 *	cache cleared at each frame with resetLightSetup().
	 *	NB: models which are sticked or skinned on a skeleton have same lightContribution
	 *	because lightSetup is made on the skeleton only. Hence the interest of this cache.
	 *
	 *	\param useLocalAttenuation if true, use Hardware Attenuation, else use global one
	 *	(attenuation with AttFactor)
	 */
	void		changeLightSetup(CLightContribution	*lightContribution, bool useLocalAttenuation);


	/** setup the driver VP constants to get info from current LightSetup.
	 *	Only 0..3 Light + SunLights are supported. The VP do NOT support distance/Spot attenuation
	 *	Also it does not handle World Matrix with non uniform scale correctly since lighting is made in ObjectSpace
	 *
	 *	\param ctStart the program use ctes from ctStart to ctStart+NumCtes.
	 *	\param supportSpecular asitsounds. PointLights and dirLight are localViewer
	 *	\param invObjectWM the inverse of object matrix: lights are mul by this. Vp compute in object space.
	 */
	void		beginVPLightSetup(uint ctStart, bool supportSpecular, const CMatrix &invObjectWM);

	/** change the driver VP LightSetup constants which depends on material.
	 *  \param excludeStrongest This remove the strongest light from the setup. The typical use is to have it computed by using perpixel lighting.
	 */
	void		changeVPLightSetupMaterial(const CMaterial &mat, bool excludeStrongest);


	/** tool to get a VP fragment which compute lighting with following rules:
	 *	IN:
	 *		- R5  vertex in objectSpace (ie untransformed)
	 *		- R6  normal in objectSpace (ie untransformed)
	 *	OUT:
	 *		- R6  normal normalized
	 *		- o[COL0] and o[COL1] are setuped. NB: BF0 and BF1 not computed/setuped.
	 *	Scratch:
	 *		- R0, R1, R2, R3, R4
	 *
	 *	For information, constant mapping is (add ctStart):
     *
	 *  == Strongest light included ==
     *
	 *	if !supportSpecular:
	 *		- 0:		AmbientColor.
	 *		- 1..4:		DiffuseColor of 4 lights.
	 *		- 5:		- (directional Light direction) in objectSpace
	 *		- 6..8:		light position (3 pointLihgts) in objectSpace
	 *		- 9:		material Diffuse Alpha copied to output. cte is: {0,0, 1, alphaMat}
	 *		TOTAL: 10 constants used.
	 *	if supportSpecular:
	 *		- 0:		AmbientColor.
	 *		- 1..4:		DiffuseColor of 4 lights.
	 *		- 5..8:		SpecularColor of 4 lights. NB: SpecularColor[5].w get the specular exponent of the material
	 *		- 9:		- (directional Light direction) in objectSpace
	 *		- 10:		material Diffuse Alpha copied to output. cte is: {0,0, 1, alphaMat}
	 *		- 11:		eye position in objectSpace
	 *		- 12..14:	light position (3 pointLihgts) in objectSpace
	 *		TOTAL: 15 constants used.
	 *
	 *
	 *	 NB: the number of active light does not change the number of constantes used. But the VP code returned is
	 *	modified accordingly.
	 *
	 *  \param numActivePoinLights tells how many point light from 0 to 3 this VP must handle. NB: the Sun directionnal is not option
	 *		NB: nlassert(numActiveLights<=MaxVPLight-1).
	 */
	static	std::string		getLightVPFragmentNeLVP(uint numActivePointLights, uint ctStart, bool supportSpecular, bool normalize);
	// TODO_VP_GLSL

	/** This returns a reference to a driver light, by its index
	  * \see getStrongestLightIndex
	  */
	const CLight  &getDriverLight(sint index) const
	{
		nlassert(index >= 0 && index < NL3D_MAX_LIGHT_CONTRIBUTION+1);
		return _DriverLight[index];
	}

	/// return an index to the current strongest settuped light (or -1 if there's none)
	sint		getStrongestLightIndex() const;

	/** Get current color, diffuse and specular of the strongest light in the scene.
	  * These values are modulated by the current material color, so these values are valid only after
	  * changeVPLightSetupMaterial() has been called
	  */
	void		getStrongestLightColors(NLMISC::CRGBA &diffuse, NLMISC::CRGBA &specular);

	/** return the number of VP lights currently activated (sunlight included)
	 *	Value correct after beginVPLightSetup() only
	 */
	uint		getNumVPLights() const {return _VPNumLights;}

	// @}


	/// \name MeshBlock Manager. FOR MODEL TRAVERSING AND MESHS ONLY.
	// @{

	/// The manager of meshBlock. Used to add instances.
	CMeshBlockManager		MeshBlockManager;

	// @}

	// ReSetup the Driver Frustum/Camera. Called internally and by ShadowMapManager only.
	void			setupDriverCamera();

private:

	// A grow only list of models to be rendered.
	std::vector<CTransform*>	RenderList;
	uint32						_CurrentNumVisibleModels;

	// Ordering Table to sort transparent objects
	COrderingTable<CTransform>			OrderOpaqueList;
	std::vector<CLayeredOrderingTable<CTransform> >	_OrderTransparentListByPriority;
	uint8											_MaxTransparencyPriority;

	IDriver			*Driver;
	CViewport		_Viewport;

	// Temporary for the render
	bool			_CurrentPassOpaque;
	bool			_LayersRenderingOrder;


	/// \name Render Lighting Setup.
	// @{
	// The last setup.
	CLightContribution			*_CacheLightContribution;
	bool						_LastLocalAttenuation;
	// The number of light enabled
	uint						_NumLightEnabled;

	// More precise setup
	uint						_LastSunFactor;
	NLMISC::CRGBA				_LastFinalAmbient;
	CPointLight					*_LastPointLight[NL3D_MAX_LIGHT_CONTRIBUTION];
	uint8						_LastPointLightFactor[NL3D_MAX_LIGHT_CONTRIBUTION];
	bool						_LastPointLightLocalAttenuation[NL3D_MAX_LIGHT_CONTRIBUTION];

	CVector						_SunDirection;

	// driver Lights setuped in changeLightSetup()
	CLight						_DriverLight[NL3D_MAX_LIGHT_CONTRIBUTION+1];

	// index of the strongest light (when used)
	mutable uint				_StrongestLightIndex;
	mutable bool				_StrongestLightTouched;

	// Current ctStart setuped with beginVPLightSetup()
	uint						_VPCurrentCtStart;
	// Current num of VP lights enabled.
	uint						_VPNumLights;
	// Current support of specular
	bool						_VPSupportSpecular;
	// Sum of all ambiant of all lights + ambiantGlobal.
	NLMISC::CRGBAF				_VPFinalAmbient;
	// Diffuse/Spec comp of all light / 255.
	NLMISC::CRGBAF				_VPLightDiffuse[MaxVPLight];
	NLMISC::CRGBAF				_VPLightSpecular[MaxVPLight];

	NLMISC::CRGBA				_StrongestLightDiffuse;
	NLMISC::CRGBA				_StrongestLightSpecular;

	// Cache for changeVPLightSetupMaterial()
	bool						_VPMaterialCacheDirty;
	uint32						_VPMaterialCacheEmissive;
	uint32						_VPMaterialCacheAmbient;
	uint32						_VPMaterialCacheDiffuse;
	uint32						_VPMaterialCacheSpecular;
	float						_VPMaterialCacheShininess;

	// @}


	/// The manager of skin. NULL by default.
	CVertexStreamManager		*_MeshSkinManager;


	/// The ShadowMap Manager.
	CShadowMapManager			_ShadowMapManager;
	/// The SkinManager, but For Shadow rendering
	CVertexStreamManager		*_ShadowMeshSkinManager;


	/** \name Special Landscape RenderList.
	 *	The Landscape list is separated from std render List for optimisation purpose.
	 *	See Doc in traverse() method.
	 */
	//@{
	// clear the list
	void			clearRenderLandscapeList();
	// render the landscapes
	void			renderLandscapes();

	// A grow only list of landscapes to be rendered.
	std::vector<CLandscapeModel*>	_LandscapeRenderList;

	// @}

	/// \name Temp Debug
	//@{
	struct CWaterModelDump
	{
		void		*Address;
		void		*ClippedPolyBegin;
		void		*ClippedPolyEnd;
	};
	std::vector<CWaterModelDump>	_DebugWaterModelList;
	//@}

public:
	CWaterModel *_FirstWaterModel;
};


}


#endif // NL_RENDER_TRAV_H

/* End of render_trav.h */
