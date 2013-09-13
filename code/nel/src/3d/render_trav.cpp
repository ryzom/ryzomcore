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

#include "nel/3d/render_trav.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/light_trav.h"
#include "nel/3d/driver.h"
#include "nel/3d/light.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/scene.h"
#include "nel/3d/coarse_mesh_manager.h"
#include "nel/3d/lod_character_manager.h"
#include "nel/3d/water_model.h"
#include "nel/3d/water_shape.h"
#include "nel/misc/hierarchical_timer.h"

#include "nel/3d/transform.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/vertex_stream_manager.h"
#include "nel/3d/landscape_model.h"
#include "nel/3d/shape_bank.h"

using namespace std;
using namespace NLMISC;

namespace	NL3D
{

// default is undefined, allows to see which CTransformShape are displayed in a scene, useful for debugging
//#define NL_DEBUG_RENDER_TRAV






// ***************************************************************************
// ***************************************************************************
// CRenderTrav
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CRenderTrav::CRenderTrav()
{
	RenderList.resize(1024);
	_CurrentNumVisibleModels= 0;
	_MaxTransparencyPriority = 0;
	OrderOpaqueList.init(1024);
	setupTransparencySorting();
	Driver = NULL;
	_CurrentPassOpaque = true;

	_CacheLightContribution= NULL;

	// Default light Setup.
	LightingSystemEnabled= false;
	AmbientGlobal= CRGBA(50, 50, 50);
	SunAmbient= CRGBA::Black;
	SunDiffuse= SunSpecular= CRGBA::White;
	_SunDirection.set(0, 0.5, -0.5);
	_SunDirection.normalize();

	_StrongestLightTouched = true;

	_MeshSkinManager= NULL;
	_ShadowMeshSkinManager= NULL;

	_LayersRenderingOrder= true;
	_FirstWaterModel = NULL;
}





// ***************************************************************************
void		CRenderTrav::traverse(UScene::TRenderPart renderPart, bool newRender)
{
	#ifdef NL_DEBUG_RENDER_TRAV
		nlwarning("Render trave begin");
	#endif
	H_AUTO( NL3D_TravRender );
	if (getDriver()->isLost()) return; // device is lost so no need to render anything
	CTravCameraScene::update();
	// Bind to Driver.
	setupDriverCamera();
	getDriver()->setupViewport(_Viewport);

	// reset the light setup, and set global ambient.
	resetLightSetup();
	if (newRender)
	{

		// reset the Skin manager, if needed
		if(_MeshSkinManager)
		{
			if(Driver!=_MeshSkinManager->getDriver())
			{
				_MeshSkinManager->release();
				_MeshSkinManager->init(Driver,
					NL3D_MESH_SKIN_MANAGER_VERTEXFORMAT,
					NL3D_MESH_SKIN_MANAGER_MAXVERTICES,
					NL3D_MESH_SKIN_MANAGER_NUMVB,
					"MRMSkinVB", true);
			}
		}

		// Same For Shadow ones. NB: use AuxDriver here!!!
		if(_ShadowMeshSkinManager)
		{
			if(getAuxDriver()!=_ShadowMeshSkinManager->getDriver())
			{
				_ShadowMeshSkinManager->release();
				_ShadowMeshSkinManager->init(getAuxDriver(),
					NL3D_SHADOW_MESH_SKIN_MANAGER_VERTEXFORMAT,
					NL3D_SHADOW_MESH_SKIN_MANAGER_MAXVERTICES,
					NL3D_SHADOW_MESH_SKIN_MANAGER_NUMVB,
					"ShadowSkinVB", true);
			}
		}


		// Fill OT with models, for both Opaque and transparent pass
		// =============================

		// Sort the models by distance from camera
		// This is done here and not in the addRenderModel because of the LoadBalancing traversal which can modify
		// the transparency flag (multi lod for instance)

		// clear the OTs, and prepare to allocate max element space
		OrderOpaqueList.reset(_CurrentNumVisibleModels);
		for(uint k = 0; k <= (uint) _MaxTransparencyPriority; ++k)
		{
			_OrderTransparentListByPriority[k].reset(_CurrentNumVisibleModels);	// all table share the same allocator (CLayeredOrderingTable::shareAllocator has been called)
																			// and an object can be only inserted in one table, so we only need to init the main allocator
		}

		// fill the OTs.
		CTransform			**itRdrModel= NULL;
		uint32				nNbModels = _CurrentNumVisibleModels;
		if(nNbModels)
			itRdrModel= &RenderList[0];
		float	rPseudoZ, rPseudoZ2;

		// Some precalc
		float	OOFar= 1.0f / this->Far;
		uint32	opaqueOtSize= OrderOpaqueList.getSize();
		uint32	opaqueOtMax= OrderOpaqueList.getSize()-1;
		uint32	transparentOtSize= _OrderTransparentListByPriority[0].getSize(); // there is at least one list, and all list have the same number of entries
		uint32	transparentOtMax= _OrderTransparentListByPriority[0].getSize()-1;
		uint32	otId;
		// fast floor
		NLMISC::OptFastFloorBegin();
		// For all rdr models
		for( ; nNbModels>0; itRdrModel++, nNbModels-- )
		{
			CTransform			*pTransform = *itRdrModel;

			// if this entry was killed by removeRenderModel(), skip!
			if(!pTransform)
				continue;

			// Yoyo: skins are rendered through skeletons, so models WorldMatrix are all good here (even sticked objects)
			rPseudoZ = (pTransform->getWorldMatrix().getPos() - CamPos).norm();

			// rPseudoZ from 0.0 -> 1.0
			rPseudoZ =  sqrtf( rPseudoZ * OOFar );

			if( pTransform->isOpaque() )
			{
				// since norm, we are sure that rPseudoZ>=0
				rPseudoZ2 = rPseudoZ * opaqueOtSize;
				otId= NLMISC::OptFastFloor(rPseudoZ2);
				otId= min(otId, opaqueOtMax);
				OrderOpaqueList.insert( otId, pTransform );
			}
			if( pTransform->isTransparent() )
			{
				// since norm, we are sure that rPseudoZ>=0
				rPseudoZ2 = rPseudoZ * transparentOtSize;
				otId= NLMISC::OptFastFloor(rPseudoZ2);
				otId= min(otId, transparentOtMax);
				// must invert id, because transparent, sort from back to front
				_OrderTransparentListByPriority[std::min(pTransform->getTransparencyPriority(), _MaxTransparencyPriority)].insert( pTransform->getOrderingLayer(), pTransform, transparentOtMax-otId );
			}

		}
		// fast floor
		NLMISC::OptFastFloorEnd();
	}

	if (renderPart & UScene::RenderOpaque)
	{
		// Render Opaque stuff.
		// =============================

		// TestYoyo
		//OrderOpaqueList.reset(0);
		//OrderTransparentList.reset(0);

		// Clear any landscape
		clearRenderLandscapeList();

		// Start LodCharacter Manager render.
		CLodCharacterManager	*clodMngr= Scene->getLodCharacterManager();
		if(clodMngr)
			clodMngr->beginRender(getDriver(), CamPos);

		// Render the opaque materials
		_CurrentPassOpaque = true;
		OrderOpaqueList.begin();
		while( OrderOpaqueList.get() != NULL )
		{
			CTransform	*tr= OrderOpaqueList.get();
			#ifdef NL_DEBUG_RENDER_TRAV
				CTransformShape *trShape = dynamic_cast<CTransformShape *>(tr);
				if (trShape)
				{
					const std::string *shapeName = Scene->getShapeBank()->getShapeNameFromShapePtr(trShape->Shape);
					if (shapeName)
					{
						nlwarning("Displaying %s", shapeName->c_str());
					}
				}
			#endif
			tr->traverseRender();
			OrderOpaqueList.next();
		}

		/* Render MeshBlock Manager.
			Some Meshs may be render per block. Interesting to remove VertexBuffer and Material setup overhead.
			Faster if rendered before lods, for ZBuffer optimisation: render first near objects then far.
			Lods are usually far objects.
		*/
		MeshBlockManager.flush(Driver, Scene, this);


		// End LodCharacter Manager render.
		if(clodMngr)
			clodMngr->endRender();


		/* Render Scene CoarseMeshManager.
			Important to render them at end of Opaque rendering, because coarses instances are created/removed during
			this model opaque rendering pass.
		*/
		if( Scene->getCoarseMeshManager() )
			Scene->getCoarseMeshManager()->flushRender(Driver);

		/* Render ShadowMaps.
			Important to render them at end of Opaque rendering, because alphaBlended objects must blend with opaque
			objects shadowed.
			Therefore, transparent objects neither can't cast or receive shadows...

			NB: Split in 2 calls and interleave Landscape Rendering between the 2. WHY???
			Because it is far more efficient for VBLock (but not for ZBuffer optim...) because in renderGenerate()
			the ShadowMeshSkinManager do lot of VBLocks that really stall (because only 2 VBHard with swap scheme).

			Therefore the first Lock that stall will wait not only for the first MeshSkin to finish but also for the
			preceding landscape render to finish too! => big STALL.
		*/

		// Generate ShadowMaps
		_ShadowMapManager.renderGenerate(Scene);

		// Render the Landscape
		renderLandscapes();

		// Project ShadowMaps.
		if(Scene->getLandscapePolyDrawingCallback() != NULL)
		{
			Scene->getLandscapePolyDrawingCallback()->beginPolyDrawing();
		}
		_ShadowMapManager.renderProject(Scene);
		if(Scene->getLandscapePolyDrawingCallback())
		{
			Scene->getLandscapePolyDrawingCallback()->endPolyDrawing();
		}

		// Profile this frame?
		if(Scene->isNextRenderProfile())
		{
			OrderOpaqueList.begin();
			while( OrderOpaqueList.get() != NULL )
			{
				OrderOpaqueList.get()->profileRender();
				OrderOpaqueList.next();
			}
		}
	}


	if (renderPart & UScene::RenderTransparent)
	{
		if (_FirstWaterModel) // avoid a lock if no water is to be rendered
		{
			// setup water models
			CWaterModel *curr = _FirstWaterModel;
			uint numWantedVertices = 0;
			while (curr)
			{
				numWantedVertices += curr->getNumWantedVertices();
				curr = curr->_Next;
			}
			if (numWantedVertices != 0)
			{
				CWaterModel::setupVertexBuffer(Scene->getWaterVB(), numWantedVertices, getDriver());
				//
				{
					CVertexBufferReadWrite vbrw;
					Scene->getWaterVB().lock(vbrw);
					CWaterModel *curr = _FirstWaterModel;
					void *datas = vbrw.getVertexCoordPointer(0);
					//
					uint tri = 0;
					while (curr)
					{
						tri = curr->fillVB(datas, tri, *getDriver());
						nlassert(tri <= numWantedVertices);
						curr = curr->_Next;
					}
					nlassert(tri * 3 == numWantedVertices);
				}
			}
			// Unlink all water model
			clearWaterModelList();
		}
	}

	if ((renderPart & UScene::RenderTransparent) &&
		(renderPart & UScene::RenderFlare)
	   )
	{
		// Render all transparent stuffs including flares.
		// =============================
		 // Render transparent materials (draw higher priority last, because their appear in front)
		_CurrentPassOpaque = false;
		for(std::vector<CLayeredOrderingTable<CTransform> >::iterator it = _OrderTransparentListByPriority.begin(); it != _OrderTransparentListByPriority.end(); ++it)
		{
			it->begin(_LayersRenderingOrder);
			while( it->get() != NULL )
			{
				#ifdef NL_DEBUG_RENDER_TRAV
					CTransformShape *trShape = dynamic_cast<CTransformShape *>(it->get());
					if (trShape)
					{
						const std::string *shapeName = Scene->getShapeBank()->getShapeNameFromShapePtr(trShape->Shape);
						if (shapeName)
						{
							nlwarning("Displaying %s", shapeName->c_str());
						}
					}
				#endif
				it->get()->traverseRender();
				it->next();
			}
		}

		// Profile this frame?
		if(Scene->isNextRenderProfile())
		{
			for(std::vector<CLayeredOrderingTable<CTransform> >::iterator it = _OrderTransparentListByPriority.begin(); it != _OrderTransparentListByPriority.end(); ++it)
			{
				it->begin();
				while( it->get() != NULL )
				{
					it->get()->profileRender();
					it->next();
				}
			}
		}
	}
	else if (renderPart & UScene::RenderTransparent)
	{
		// Render all transparent stuffs, don't render flares
		// =============================
		_CurrentPassOpaque = false;
		for(std::vector<CLayeredOrderingTable<CTransform> >::iterator it = _OrderTransparentListByPriority.begin(); it != _OrderTransparentListByPriority.end(); ++it)
		{
			it->begin(_LayersRenderingOrder);
			while( it->get() != NULL )
			{
				if (!it->get()->isFlare())
				{
					#ifdef NL_DEBUG_RENDER_TRAV
						CTransformShape *trShape = dynamic_cast<CTransformShape *>(it->get());
						if (trShape)
						{
							const std::string *shapeName = Scene->getShapeBank()->getShapeNameFromShapePtr(trShape->Shape);
							if (shapeName)
							{
								nlwarning("Displaying %s", shapeName->c_str());
							}
						}
					#endif
					it->get()->traverseRender();
				}
				it->next();
			}
		}

		// Profile this frame?
		if(Scene->isNextRenderProfile())
		{
			for(std::vector<CLayeredOrderingTable<CTransform> >::iterator it = _OrderTransparentListByPriority.begin(); it != _OrderTransparentListByPriority.end(); ++it)
			{
				it->begin();
				while( it->get() != NULL )
				{
					if (!it->get()->isFlare())
					{
						it->get()->profileRender();
					}
					it->next();
				}
			}
		}
	}
	else if (renderPart & UScene::RenderFlare)
	{
		// Render flares only
		// =============================
		_CurrentPassOpaque = false;
		for(std::vector<CLayeredOrderingTable<CTransform> >::iterator it = _OrderTransparentListByPriority.begin(); it != _OrderTransparentListByPriority.end(); ++it)
		{
			it->begin(_LayersRenderingOrder);
			while( it->get() != NULL )
			{
				if (it->get()->isFlare())
				{
					#ifdef NL_DEBUG_RENDER_TRAV
						CTransformShape *trShape = dynamic_cast<CTransformShape *>(it->get());
						if (trShape)
						{
							const std::string *shapeName = Scene->getShapeBank()->getShapeNameFromShapePtr(trShape->Shape);
							if (shapeName)
							{
								nlwarning("Displaying %s", shapeName->c_str());
							}
						}
					#endif
					it->get()->traverseRender();
				}
				it->next();
			}
		}

		// Profile this frame?
		if(Scene->isNextRenderProfile())
		{
			for(std::vector<CLayeredOrderingTable<CTransform> >::iterator it = _OrderTransparentListByPriority.begin(); it != _OrderTransparentListByPriority.end(); ++it)
			{
				it->begin();
				while( it->get() != NULL )
				{
					if (it->get()->isFlare())
					{
						it->get()->profileRender();
					}
					it->next();
				}
			}
		}
	}

	// END!
	// =============================

	// clean: reset the light setup
	resetLightSetup();

}

// ***************************************************************************
void		CRenderTrav::setupDriverCamera()
{
	getDriver()->setFrustum(Left, Right, Bottom, Top, Near, Far, Perspective);
	// Use setupViewMatrixEx() for ZBuffer precision.
	getDriver()->setupViewMatrixEx(ViewMatrix, CamPos);
}

// ***************************************************************************
void		CRenderTrav::clearRenderList()
{
	_CurrentNumVisibleModels= 0;
}


// ***************************************************************************
void		CRenderTrav::setSunDirection(const CVector &dir)
{
	_SunDirection= dir;
	_SunDirection.normalize();
}


// ***************************************************************************
void		CRenderTrav::setMeshSkinManager(CVertexStreamManager *msm)
{
	_MeshSkinManager= msm;
}

// ***************************************************************************
void		CRenderTrav::setShadowMeshSkinManager(CVertexStreamManager *msm)
{
	_ShadowMeshSkinManager= msm;
}

// ***************************************************************************
void		CRenderTrav::reserveRenderList(uint numModels)
{
	// enlarge only.
	if(numModels>RenderList.size())
		RenderList.resize(numModels);
}

// ***************************************************************************
void		CRenderTrav::removeRenderModel(CTransform *m)
{
	// NB: storing a 8 bit in CTransform, instead of a 32 bits, is just to save space.
	uint	lsb= m->_IndexLSBInRenderList;

	// this method is rarely called, so don't bother the slow down
	// btw, we parse the entire list / 256!!! which is surely fast!!
	for(uint i=lsb;i<_CurrentNumVisibleModels;i+=256)
	{
		// if i am really this entry, then set NULL
		if(RenderList[i]==m)
		{
			RenderList[i]= NULL;
			break;
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// LightSetup
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CRenderTrav::resetLightSetup()
{
	// If lighting System disabled, skip
	if(!LightingSystemEnabled)
	{
		// Dont modify Driver lights, but setup default lighting For VertexProgram Lighting.
		_NumLightEnabled= 1;
		// Setup A default directionnal.
		CVector		defDir(-0.5f, 0.0, -0.85f);
		defDir.normalize();
		CRGBA		aday= CRGBA(130,  105,  119);
		CRGBA		dday= CRGBA(238, 225, 204);
		_DriverLight[0].setupDirectional(aday, dday, dday, defDir);

		return;
	}
	else
	{
		uint i;

		// Disable all lights.
		for(i=0; i<Driver->getMaxLight(); ++i)
		{
			Driver->enableLight(uint8(i), false);
		}


		// setup the precise cache, and setup lights according to this cache?
		// setup blackSun (factor==0).
		_LastSunFactor= 0;
		_LastFinalAmbient.set(0,0,0,255);
		_DriverLight[0].setupDirectional(CRGBA::Black, CRGBA::Black, CRGBA::Black, _SunDirection);
		Driver->setLight(0, _DriverLight[0]);
		// setup NULL point lights (=> cache will fail), so no need to setup other lights in Driver.
		for(i=0; i<NL3D_MAX_LIGHT_CONTRIBUTION; i++)
		{
			_LastPointLight[i]= NULL;
		}


		// Set the global ambientColor
		Driver->setAmbientColor(AmbientGlobal);


		// clear the cache.
		_CacheLightContribution= NULL;
		_NumLightEnabled= 0;

		_StrongestLightTouched = true;
	}
}


// ***************************************************************************
void		CRenderTrav::changeLightSetup(CLightContribution	*lightContribution, bool useLocalAttenuation)
{
	// If lighting System disabled, skip
	if(!LightingSystemEnabled)
		return;

	uint		i;

	// if same lightContribution, no-op.
	if (_CacheLightContribution == lightContribution && (lightContribution == NULL || _LastLocalAttenuation == useLocalAttenuation))
		return;
	// else, must setup the lights into driver.
	else
	{
		_StrongestLightTouched = true;
		// if the setup is !NULL
		if(lightContribution)
		{
			// Compute SunAmbient / LocalAmbient
			//-----------
			// Take the current model ambient
			CRGBA	finalAmbient= lightContribution->computeCurrentAmbient(SunAmbient);
			// If use the mergedPointLight, add it to final Ambient
			if(lightContribution->UseMergedPointLight)
				finalAmbient.addRGBOnly(finalAmbient, lightContribution->MergedPointLight);
			// Force Alpha to 255 for good cache test.
			finalAmbient.A= 255;


			// Setup the directionnal Sunlight.
			//-----------
			// expand 0..255 to 0..256, to avoid loss of precision.
			uint	ufactor= lightContribution->SunContribution;
			//	different SunLight as in cache ??
			//	NB: sunSetup can't change during renderPass, so need only to test factor.
			if(ufactor != _LastSunFactor || finalAmbient != _LastFinalAmbient)
			{
				// cache (before expanding!!)
				_LastSunFactor= ufactor;
				// Cache final ambient light
				_LastFinalAmbient= finalAmbient;

				// expand to 0..256.
				ufactor+= ufactor>>7;	// add 0 or 1.
				// modulate color with factor of the lightContribution.
				CRGBA	sunDiffuse, sunSpecular;
				sunDiffuse.modulateFromuiRGBOnly(SunDiffuse, ufactor);
				sunSpecular.modulateFromuiRGBOnly(SunSpecular, ufactor);
				// setup driver light
				_DriverLight[0].setupDirectional(finalAmbient, sunDiffuse, sunSpecular, _SunDirection);
				Driver->setLight(0, _DriverLight[0]);
			}


			// Setup other point lights
			//-----------
			uint	plId=0;
			// for the list of light.
			while(lightContribution->PointLight[plId]!=NULL)
			{
				CPointLight		*pl= lightContribution->PointLight[plId];
				uint			inf;
				if(useLocalAttenuation)
					inf= lightContribution->Factor[plId];
				else
					inf= lightContribution->AttFactor[plId];

				// different PointLight setup than in cache??
				// NB: pointLight setup can't change during renderPass, so need only to test pointer,
				// attenuation mode and factor.
				if( pl!=_LastPointLight[plId] ||
					inf!=_LastPointLightFactor[plId] ||
					useLocalAttenuation!=_LastPointLightLocalAttenuation[plId] )
				{
					// need to resetup the light. Cache it.
					_LastPointLight[plId]= pl;
					_LastPointLightFactor[plId]= uint8(inf);
					_LastPointLightLocalAttenuation[plId]= useLocalAttenuation;

					// compute the driver light
					if(useLocalAttenuation)
						pl->setupDriverLight(_DriverLight[plId+1], uint8(inf));
					else
						// Compute it with user Attenuation
						pl->setupDriverLightUserAttenuation(_DriverLight[plId+1], uint8(inf));

					// setup driver. decal+1 because of sun.
					Driver->setLight(uint8(plId+1), _DriverLight[plId+1]);
				}

				// next light?
				plId++;
				if(plId>=NL3D_MAX_LIGHT_CONTRIBUTION)
					break;
			}


			// Disable olds, enable news, and cache.
			//-----------
			// count new number of light enabled.
			uint	newNumLightEnabled;
			// number of pointLight + the sun
			newNumLightEnabled= plId + 1;

			// enable lights which are used now and were not before.
			for(i=_NumLightEnabled; i<newNumLightEnabled; i++)
			{
				Driver->enableLight(uint8(i), true);
			}

			// disable lights which are no more used.
			for(i=newNumLightEnabled; i<_NumLightEnabled; i++)
			{
				Driver->enableLight(uint8(i), false);
			}

			// cache the setup.
			_CacheLightContribution = lightContribution;
			_NumLightEnabled= newNumLightEnabled;
			_LastLocalAttenuation= useLocalAttenuation;
		}
		else
		{
			// Disable old lights, and cache.
			//-----------
			// disable lights which are no more used.
			for(i=0; i<_NumLightEnabled; i++)
			{
				Driver->enableLight(uint8(i), false);
			}

			// cache the setup.
			_CacheLightContribution = NULL;
			_NumLightEnabled= 0;
		}


	}
}

// ***************************************************************************
// ***************************************************************************
// VertexProgram LightSetup
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CRenderTrav::beginVPLightSetup(CVertexProgramLighted *program, const CMatrix &invObjectWM)
{
	uint	i;
	nlassert(MaxVPLight==4);
	_VPNumLights= min(_NumLightEnabled, (uint)MaxVPLight);
	// _VPCurrentCtStart= ctStart;
	// _VPSupportSpecular= supportSpecular;
	_VPCurrent = program;
	bool supportSpecular = program->featuresLighted().SupportSpecular;

	// Prepare Colors (to be multiplied by material)
	//================
	// Ambient. _VPCurrentCtStart+0
	_VPFinalAmbient= AmbientGlobal;
	for(i=0; i<_VPNumLights; i++)
	{
		_VPFinalAmbient+= _DriverLight[i].getAmbiant();
	}
	// Diffuse. _VPCurrentCtStart+1 to 4
	for(i=0; i<_VPNumLights; i++)
	{
		_VPLightDiffuse[i]= _DriverLight[i].getDiffuse();
	}
	// reset other to 0.
	for(; i<MaxVPLight; i++)
	{
		_VPLightDiffuse[i] = CRGBA::Black;
		if (program->idxLighted().Diffuse[i] != ~0)
		{
			Driver->setUniform4f(IDriver::VertexProgram, program->idxLighted().Diffuse[i], 0.f, 0.f, 0.f, 0.f);
		}
	}
	// Specular. _VPCurrentCtStart+5 to 8 (only if supportSpecular)
	if(supportSpecular)
	{
		for(i=0; i<_VPNumLights; i++)
		{
			_VPLightSpecular[i]= _DriverLight[i].getSpecular();
		}
		// reset other to 0.
		for(; i<MaxVPLight; i++)
		{
			_VPLightSpecular[i]= CRGBA::Black;
			if (program->idxLighted().Specular[i] != ~0)
			{
				Driver->setUniform4f(IDriver::VertexProgram, program->idxLighted().Specular[i], 0.f, 0.f, 0.f, 0.f);
			}
		}
	}


	// Compute Eye position in Object space.
	CVector		eye= invObjectWM * CamPos;


	// Setup Sun Directionnal light.
	//================
	CVector		lightDir;
	// in objectSpace.
	lightDir= invObjectWM.mulVector(_DriverLight[0].getDirection());
	lightDir.normalize();
	lightDir= -lightDir;
	Driver->setUniform3f(IDriver::VertexProgram, program->idxLighted().DirOrPos[0], lightDir); // The sun is the same for every instance.


	// Setup PointLights
	//================
	uint		startPLPos;
	if (supportSpecular)
	{
		// Setup eye in objectSpace for localViewer
		Driver->setUniform3f(IDriver::VertexProgram, program->idxLighted().EyePosition, eye);
	}
	// For all pointLight enabled (other are black: don't matter)
	for(i=1; i<_VPNumLights; i++)
	{
		// Setup position of light.
		CVector		lightPos;
		lightPos = invObjectWM * _DriverLight[i].getPosition();
		Driver->setUniform3f(IDriver::VertexProgram, program->idxLighted().DirOrPos[i], lightPos);
	}


	// Must force real light setup at least the first time, in changeVPLightSetupMaterial()
	_VPMaterialCacheDirty= true;
}

// ***************************************************************************
void		CRenderTrav::changeVPLightSetupMaterial(const CMaterial &mat, bool excludeStrongest)
{
	CVertexProgramLighted *program = _VPCurrent;
	nlassert(program);

	// Must test if at least done one time.
	if(!_VPMaterialCacheDirty)
	{
		// Must test if same as in cache
		if( _VPMaterialCacheEmissive == mat.getEmissive().getPacked() &&
			_VPMaterialCacheAmbient == mat.getAmbient().getPacked() &&
			_VPMaterialCacheDiffuse == mat.getDiffuse().getPacked() )
		{
			// Same Diffuse part, test if same specular if necessary
			if( !program->featuresLighted().SupportSpecular ||
				( _VPMaterialCacheSpecular == mat.getSpecular().getPacked() &&
				  _VPMaterialCacheShininess == mat.getShininess() )  )
			{
				// Then ok, skip.
				return;
			}
		}
	}

	// If not skiped, cache now. cache all for simplification
	_VPMaterialCacheDirty= false;
	_VPMaterialCacheEmissive= mat.getEmissive().getPacked();
	_VPMaterialCacheAmbient= mat.getDiffuse().getPacked();
	_VPMaterialCacheDiffuse= mat.getDiffuse().getPacked();
	_VPMaterialCacheSpecular= mat.getSpecular().getPacked();
	_VPMaterialCacheShininess= mat.getShininess();

	// Setup constants
	CRGBAF	color;
	uint	i;
	CRGBAF	matDiff= mat.getDiffuse();
	CRGBAF	matSpec= mat.getSpecular();
	float	specExp= mat.getShininess();

	uint strongestLightIndex = excludeStrongest ? getStrongestLightIndex() : _VPNumLights;

	// setup Ambient + Emissive
	color= _VPFinalAmbient * mat.getAmbient();
	color+= mat.getEmissive();
	Driver->setUniform4f(IDriver::VertexProgram, program->idxLighted().Ambient, color);


	// is the strongest light is not excluded, its index should have been setup to _VPNumLights

	// setup Diffuse.
	for(i = 0; i < strongestLightIndex; ++i)
	{
		color= _VPLightDiffuse[i] * matDiff;
		Driver->setUniform4f(IDriver::VertexProgram, program->idxLighted().Diffuse[i], color);
	}


	if (i != _VPNumLights)
	{
		color= _VPLightDiffuse[i] * matDiff;
		_StrongestLightDiffuse.set((uint8) (255.f * color.R), (uint8) (255.f * color.G), (uint8) (255.f * color.B), (uint8) (255.f * color.A));
		// setup strongest light to black for the gouraud part
		Driver->setUniform4f(IDriver::VertexProgram, program->idxLighted().Diffuse[i], 0.f, 0.f, 0.f, 0.f);
		++i;
		// setup other lights
		for(; i < _VPNumLights; i++)
		{
			color= _VPLightDiffuse[i] * matDiff;
			Driver->setUniform4f(IDriver::VertexProgram, program->idxLighted().Diffuse[i], color);
		}
	}

	// setup Specular
	if (program->featuresLighted().SupportSpecular)
	{
		for(i = 0; i < strongestLightIndex; ++i)
		{
			color= _VPLightSpecular[i] * matSpec;
			color.A= specExp;
			Driver->setUniform4f(IDriver::VertexProgram, program->idxLighted().Specular[i], color);
		}

		if (i != _VPNumLights)
		{
			color= _VPLightSpecular[i] * matSpec;
			_StrongestLightSpecular.set((uint8) (255.f * color.R), (uint8) (255.f * color.G), (uint8) (255.f * color.B), (uint8) (255.f * color.A));

			// setup strongest light to black (for gouraud part)
			Driver->setUniform4f(IDriver::VertexProgram, program->idxLighted().Specular[i], 0.f, 0.f, 0.f, 0.f);
			++i;
			// setup other lights
			for(; i < _VPNumLights; i++)
			{
				color= _VPLightSpecular[i] * matSpec;
				color.A= specExp;
				Driver->setUniform4f(IDriver::VertexProgram, program->idxLighted().Specular[i], color);
			}
		}
	}

	// setup alpha.
	static	float	alphaCte[4]= {0,0,1,0};
	alphaCte[3]= matDiff.A;
	// setup at good place
	Driver->setUniform4fv(IDriver::VertexProgram, program->idxLighted().DiffuseAlpha, 1, alphaCte);
}

// ***************************************************************************
sint CRenderTrav::getStrongestLightIndex() const
{
	if (!_StrongestLightTouched) return -1;
	uint vpNumLights = min(_NumLightEnabled, (uint)MaxVPLight);
	// If there is only a directionnal light, use it
	// If there is any point light, use the nearest, or the directionnal light if it is brighter
	if (vpNumLights == 0) return -1;
	if (vpNumLights == 1) return 0;
	// First point light is brightest ?
	float lumDir = _VPLightDiffuse[0].R + _VPLightDiffuse[0].G + _VPLightDiffuse[0].B + _VPLightDiffuse[0].A
				   + _VPLightSpecular[0].R + _VPLightSpecular[0].G + _VPLightSpecular[0].B + _VPLightSpecular[0].A;
	float lumOmni = _VPLightDiffuse[1].R + _VPLightDiffuse[1].G + _VPLightDiffuse[1].B + _VPLightDiffuse[1].A
				   + _VPLightSpecular[1].R + _VPLightSpecular[1].G + _VPLightSpecular[1].B + _VPLightSpecular[1].A;
	return lumDir > lumOmni ? 0 : 1;
}

// ***************************************************************************
void	CRenderTrav::getStrongestLightColors(NLMISC::CRGBA &diffuse, NLMISC::CRGBA &specular)
{
	sint strongestLightIndex = getStrongestLightIndex();
	if (strongestLightIndex == -1)
	{
		diffuse = specular = NLMISC::CRGBA::Black;
	}
	else
	{
		diffuse = _StrongestLightDiffuse;
		specular = _StrongestLightSpecular;
	}
}


// ***************************************************************************
static const char*	LightingVPFragmentNormalize=
"	# normalize normal																	\n\
	DP3	R6.w, R6, R6;																	\n\
	RSQ	R6.w, R6.w;																		\n\
	MUL	R6, R6, R6.w;																	\n\
";


// ***************************************************************************
// NB: all CTS+x are replaced with good cte index.
static const char*	LightingVPFragmentNoSpecular_Begin=
"																						\n\
	# Global Ambient.																	\n\
	MOV	R2, c[CTS+0];																	\n\
																						\n\
	# Diffuse Sun																		\n\
	DP3	R0.x, R6, c[CTS+5];			# R0.x= normal*-lightDir							\n\
	LIT	R0.y, R0.xxxx;				# R0.y= R0.x clamped								\n\
	MAD	R2, R0.y, c[CTS+1], R2;		# R2= summed vertex color.							\n\
";

// The 3 point Light code.
static const char*	LightingVPFragmentNoSpecular_PL[]=
{
"	# Diffuse PointLight 0.																\n\
	ADD	R0, c[CTS+6], -R5;			# R0= lightPos-vertex								\n\
	DP3	R0.w, R0, R0;				# normalize R0.										\n\
	RSQ	R0.w, R0.w;																		\n\
	MUL	R0, R0, R0.w;																	\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	LIT	R0.y, R0.xxxx;				# R0.y= R0.x clamped								\n\
	MAD	R2, R0.y, c[CTS+2], R2;		# R2= summed vertex color.							\n\
",
"	# Diffuse PointLight 1.																\n\
	ADD	R0, c[CTS+7], -R5;			# R0= lightPos-vertex								\n\
	DP3	R0.w, R0, R0;				# normalize R0.										\n\
	RSQ	R0.w, R0.w;																		\n\
	MUL	R0, R0, R0.w;																	\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	LIT	R0.y, R0;					# R0.y= R0.x clamped								\n\
	MAD	R2, R0.y, c[CTS+3], R2;		# R2= summed vertex color.							\n\
",
"	# Diffuse PointLight 2.																\n\
	ADD	R0, c[CTS+8], -R5;			# R0= lightPos-vertex								\n\
	DP3	R0.w, R0, R0;				# normalize R0.										\n\
	RSQ	R0.w, R0.w;																		\n\
	MUL	R0, R0, R0.w;																	\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	LIT	R0.y, R0;					# R0.y= R0.x clamped								\n\
	MAD	R2, R0.y, c[CTS+4], R2;		# R2= summed vertex color.							\n\
"
};

// The End code.
static const char*	LightingVPFragmentNoSpecular_End=
"	# output to o[COL0] only, replacing alpha with material alpha.						\n\
	MAD	o[COL0], R2, c[CTS+9].zzzx, c[CTS+9].xxxw;										\n\
";


// ***************************************************************************
// NB: all CTS+x are replaced with good cte index.
static const char*	LightingVPFragmentSpecular_Begin=
"																						\n\
	# Global Ambient.																	\n\
	MOV	R2, c[CTS+0];																	\n\
																						\n\
	# Always keep Specular exponent in R0.w												\n\
	MOV	R0.w, c[CTS+5].w;																\n\
																						\n\
	# Compute vertex-to-eye vector normed.												\n\
	ADD	R4, c[CTS+11], -R5;																\n\
	DP3	R4.w, R4, R4;																	\n\
	RSQ	R4.w, R4.w;																		\n\
	MUL R4, R4, R4.w;																	\n\
																						\n\
	# Diffuse-Specular Sun																\n\
	# Compute R1= halfAngleVector= (lightDir+R4).normed().								\n\
	ADD	R1.xyz, c[CTS+9], R4;		# R1= halfAngleVector								\n\
	DP3	R1.w, R1, R1;				# normalize R1.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R1.xyz, R1, R1.w;																\n\
	# Compute Factors and colors.														\n\
	DP3	R0.x, R6, c[CTS+9];			# R0.x= normal*-lightDir							\n\
	DP3	R0.yz, R6, R1;				# R0.yz= normal*halfAngleVector						\n\
	LIT	R0.yz, R0;					# R0.y= R0.x clamped, R0.z= pow(spec, R0.w) clamp	\n\
	MAD	R2, R0.y, c[CTS+1], R2;		# R2= summed vertex color.							\n\
	MUL	R3, R0.z, c[CTS+5];			# R3= specular color.								\n\
";

// The 3 point Light code.
static const char*	LightingVPFragmentSpecular_PL[]=
{
"	# Diffuse-Specular PointLight 0.													\n\
	# Compute R0= (lightPos-vertex).normed().											\n\
	ADD	R0.xyz, c[CTS+12], -R5;		# R0= lightPos-vertex								\n\
	DP3	R1.w, R0, R0;				# normalize R0.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R0.xyz, R0, R1.w;																\n\
	# Compute R1= halfAngleVector= (R0+R4).normed().									\n\
	ADD	R1.xyz, R0, R4;				# R1= halfAngleVector								\n\
	DP3	R1.w, R1, R1;				# normalize R1.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R1.xyz, R1, R1.w;																\n\
	# Compute Factors and colors.														\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	DP3	R0.yz, R6, R1;				# R0.yz= normal*halfAngleVector						\n\
	LIT	R0.yz, R0;					# R0.y= R0.x clamped, R0.z= pow(spec, R0.w) clamp	\n\
	MAD	R2, R0.y, c[CTS+2], R2;		# R2= summed vertex color.							\n\
	MAD	R3, R0.z, c[CTS+6], R3;		# R3= summed specular color.						\n\
",
"	# Diffuse-Specular PointLight 1.													\n\
	# Compute R0= (lightPos-vertex).normed().											\n\
	ADD	R0.xyz, c[CTS+13], -R5;		# R0= lightPos-vertex								\n\
	DP3	R1.w, R0, R0;				# normalize R0.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R0.xyz, R0, R1.w;																\n\
	# Compute R1= halfAngleVector= (R0+R4).normed().									\n\
	ADD	R1.xyz, R0, R4;				# R1= halfAngleVector								\n\
	DP3	R1.w, R1, R1;				# normalize R1.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R1.xyz, R1, R1.w;																\n\
	# Compute Factors and colors.														\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	DP3	R0.yz, R6, R1;				# R0.yz= normal*halfAngleVector						\n\
	LIT	R0.yz, R0;					# R0.y= R0.x clamped, R0.z= pow(spec, R0.w) clamp	\n\
	MAD	R2, R0.y, c[CTS+3], R2;		# R2= summed vertex color.							\n\
	MAD	R3, R0.z, c[CTS+7], R3;		# R3= summed specular color.						\n\
",
"	# Diffuse-Specular PointLight 2.													\n\
	# Compute R0= (lightPos-vertex).normed().											\n\
	ADD	R0.xyz, c[CTS+14], -R5;		# R0= lightPos-vertex								\n\
	DP3	R1.w, R0, R0;				# normalize R0.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R0.xyz, R0, R1.w;																\n\
	# Compute R1= halfAngleVector= (R0+R4).normed().									\n\
	ADD	R1.xyz, R0, R4;				# R1= halfAngleVector								\n\
	DP3	R1.w, R1, R1;				# normalize R1.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R1.xyz, R1, R1.w;																\n\
	# Compute Factors and colors.														\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	DP3	R0.yz, R6, R1;				# R0.yz= normal*halfAngleVector						\n\
	LIT	R0.yz, R0;					# R0.y= R0.x clamped, R0.z= pow(spec, R0.w) clamp	\n\
	MAD	R2, R0.y, c[CTS+4], R2;		# R2= summed vertex color.							\n\
"
};


// The End code.
static const char*	LightingVPFragmentSpecular_End=
"	# output directly to secondary color.												\n\
	MAD	o[COL1], R0.z, c[CTS+8], R3;	# final summed specular color.					\n\
																						\n\
	# output diffuse to o[COL0], replacing alpha with material alpha.					\n\
	MAD	o[COL0], R2, c[CTS+10].zzzx, c[CTS+10].xxxw;									\n\
";

// ***************************************************************************
static	void	strReplaceAll(string &strInOut, const string &tokenSrc, const string &tokenDst)
{
	string::size_type pos;
	string::difference_type srcLen= tokenSrc.size();
	while( (pos=strInOut.find(tokenSrc)) != string::npos)
	{
		strInOut.replace(pos, srcLen, tokenDst);
	}
}

void CVertexProgramLighted::buildInfo()
{
	CVertexProgram::buildInfo();
	if (profile() == nelvp)
	{
		// Fixed uniform locations
		m_IdxLighted.Ambient = m_FeaturesLighted.CtStartNeLVP + 0;
		for (uint i = 0; i < MaxLight; ++i)
		{
			m_IdxLighted.Diffuse[i] = m_FeaturesLighted.CtStartNeLVP + 1 + i;
		}
		if (m_FeaturesLighted.SupportSpecular)
		{
			for (uint i = 0; i < MaxLight; ++i)
			{
				m_IdxLighted.Specular[i] = m_FeaturesLighted.CtStartNeLVP + 5 + i;
			}
			m_IdxLighted.DirOrPos[0] = 9;
			for (uint i = 1; i < MaxLight; ++i)
			{
				m_IdxLighted.DirOrPos[i] = m_FeaturesLighted.CtStartNeLVP + (12 - 1) + i;
			}
			m_IdxLighted.DiffuseAlpha = m_FeaturesLighted.CtStartNeLVP + 10;
			m_IdxLighted.EyePosition = m_FeaturesLighted.CtStartNeLVP + 11;
		}
		else
		{
			for (uint i = 0; i < MaxLight; ++i)
			{
				m_IdxLighted.Specular[i] = ~0;
			}
			for (uint i = 0; i < MaxLight; ++i)
			{
				m_IdxLighted.DirOrPos[i] = m_FeaturesLighted.CtStartNeLVP + 5 + i;
			}
			m_IdxLighted.DiffuseAlpha = m_FeaturesLighted.CtStartNeLVP + 9;
			m_IdxLighted.EyePosition = ~0;
		}
	}
	else
	{
		// Named uniform locations
		// TODO_VP_GLSL
		// m_IdxLighted.Ambient = getUniformIndex("ambient");
		// etc
	}

	nlassert(m_IdxLighted.Diffuse[0] != ~0);
	if (m_FeaturesLighted.SupportSpecular)
	{
		nlassert(m_IdxLighted.Specular[0] != ~0);
		nlassert(m_IdxLighted.EyePosition != ~0);
	}
	nlassert(m_IdxLighted.DirOrPos[0] != ~0);
	nlassert(m_IdxLighted.DiffuseAlpha != ~0);
}

// generates the lighting part of a vertex program, nelvp profile
// ***************************************************************************
std::string		CRenderTrav::getLightVPFragmentNeLVP(uint numActivePointLights, uint ctStart, bool supportSpecular, bool normalize)
{
	string	ret;

	// Code frag written for 4 light max.
	nlassert(MaxVPLight==4);
	nlassert(numActivePointLights<=MaxVPLight-1);

	// Add LightingVPFragmentNormalize fragment?
	if(normalize)
		ret+= LightingVPFragmentNormalize;

	// Which fragment to use...
	if(supportSpecular)
	{
		// Add start of VP.
		ret+= LightingVPFragmentSpecular_Begin;

		// Add needed pointLights.
		for(uint i=0;i<numActivePointLights;i++)
		{
			ret+= LightingVPFragmentSpecular_PL[i];
		}

		// Add end of VP.
		ret+= LightingVPFragmentSpecular_End;
	}
	else
	{
		// Add start of VP.
		ret+= LightingVPFragmentNoSpecular_Begin;

		// Add needed pointLights.
		for(uint i=0;i<numActivePointLights;i++)
		{
			ret+= LightingVPFragmentNoSpecular_PL[i];
		}

		// Add end of VP.
		ret+= LightingVPFragmentNoSpecular_End;
	}

	// Replace all CTS+x with good index. do it for 15 possible indices: 0 to 14 if specular.
	// run from 14 to 0 so CTS+14 will not be taken for a CTS+1 !!
	for(sint i=14; i>=0; i--)
	{
		char	tokenSrc[256];
		sprintf(tokenSrc, "CTS+%d", i);
		char	tokenDst[256];
		sprintf(tokenDst, "%d", ctStart+i);
		// replace all in the string
		strReplaceAll(ret, tokenSrc, tokenDst);
	}

	// verify no CTS+ leaved... (not all ctes parsed!!!)
	nlassert( ret.find("CTS+")==string::npos );

	return ret;
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void			CRenderTrav::clearRenderLandscapeList()
{
	_LandscapeRenderList.clear();
}

// ***************************************************************************
void			CRenderTrav::addRenderLandscape(CLandscapeModel *model)
{
	_LandscapeRenderList.push_back(model);
}

// ***************************************************************************
void			CRenderTrav::renderLandscapes()
{
	// Render Each Landscapes.
	for(uint i=0;i<_LandscapeRenderList.size();i++)
	{
		_LandscapeRenderList[i]->clipAndRenderLandscape();
	}
}

// ***************************************************************************
void CRenderTrav::setupTransparencySorting(uint8 maxPriority /*=0*/,uint NbDistanceEntries /*=1024*/)
{
	NLMISC::contReset(_OrderTransparentListByPriority); // avoid useless object copy when vector is resized (every element is reseted anyway)
	_OrderTransparentListByPriority.resize((uint) maxPriority + 1);
	for(uint k = 0; k < _OrderTransparentListByPriority.size(); ++k)
	{
		_OrderTransparentListByPriority[k].init(NbDistanceEntries);
		if (k != 0) _OrderTransparentListByPriority[k].shareAllocator(_OrderTransparentListByPriority[0]); // node allocator is shared between all layers
	}
	_MaxTransparencyPriority = maxPriority;
}

// ***************************************************************************
void CRenderTrav::clearWaterModelList()
{
	while (_FirstWaterModel)
	{
		_FirstWaterModel->unlink();
	}
}

// ***************************************************************************
void CRenderTrav::debugWaterModelMemory(const char *tag, bool dumpList)
{
	// Test Memory of water model render list (because someone crash it...)
	// Yoyo: this crash seems to be fixed, but i leave the code, in case of.....

	if(dumpList)
		_DebugWaterModelList.clear();

	CWaterModel		*curr= _FirstWaterModel;
	while(curr)
	{
		// the model in the list Must have not empty clipped poly
		CWaterModelDump		dmp;
		dmp.Address= (void*)curr;
		curr->debugDumpMem(dmp.ClippedPolyBegin, dmp.ClippedPolyEnd);
		// if same ptr (begin==end), error!!
		if(dmp.ClippedPolyBegin==dmp.ClippedPolyEnd)
		{
			// Before crash, do some log
			nlwarning("******* WaterModelList crash after %s", tag);
			nlwarning("Current: Ptr:%p. List:%p/%p", dmp.Address, dmp.ClippedPolyBegin, dmp.ClippedPolyEnd);
			// Log also the list bkuped (to do comparisons)
			for(uint i=0;i<_DebugWaterModelList.size();i++)
			{
				CWaterModelDump		&bkup= _DebugWaterModelList[i];
				nlwarning("List%02d: Ptr:%p. Array:%p/%p", i, bkup.Address, bkup.ClippedPolyBegin, bkup.ClippedPolyEnd);
			}

			// crash (assert not stop for clearness)
			nlassert(dmp.ClippedPolyBegin!=dmp.ClippedPolyEnd);
		}

		// bkup infos for future log
		if(dumpList)
			_DebugWaterModelList.push_back(dmp);

		// next
		curr= curr->_Next;
	}
}

}












































