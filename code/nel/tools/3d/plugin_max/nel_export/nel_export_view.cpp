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

#include "std_afx.h"

#include "nel/3d/scene.h"
#include "nel/3d/register_3d.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/mesh_instance.h"
#include "nel/3d/light.h"
#include "nel/3d/water_pool_manager.h"
#include "nel/3d/instance_lighter.h"

#include "nel/pacs/retriever_bank.h"
#include "nel/pacs/global_retriever.h"

#include "../../object_viewer/object_viewer_interface.h"

// For lighting ig with pacs.
#include "../../ig_lighter_lib/ig_lighter_lib.h"


#include "../nel_mesh_lib/export_nel.h"
#include "../nel_patch_lib/rpo.h"
#include "../nel_mesh_lib/calc_lm.h"
#include "../nel_mesh_lib/export_appdata.h"

#include "nel/misc/time_nl.h"
#include "nel/misc/config_file.h"

#include "nel_export.h"
#include "progress.h"

using namespace NLMISC;
using namespace NL3D;
using namespace std;

extern CExportNelOptions theExportSceneStruct;

#define VIEW_WIDTH 800
#define VIEW_HEIGHT 600

typedef map<INode*, CExportNel::mapBoneBindPos > mapRootMapBoneBindPos;

// -----------------------------------------------------------------------------------------------

class CSkeletonDesc
{
public:
	CSkeletonDesc (uint skeletonInstance, const TInodePtrInt& mapId)
	{
		SkeletonInstance = skeletonInstance;
		MapId = mapId;
	}
	uint			SkeletonInstance;
	TInodePtrInt	MapId;
};


// -----------------------------------------------------------------------------------------------
class	CMaxInstanceLighter : public NL3D::CInstanceLighter
{
public:
	CProgressBar	ProgressBar;

	void	initMaxLighter(Interface& _Ip)
	{
		ProgressBar.initProgressBar (100, _Ip);
	}
	void	closeMaxLighter()
	{
		ProgressBar.uninitProgressBar ();
	}

	virtual void progress (const char *message, float progress)
	{
		ProgressBar.setLine(0, string(message));
		ProgressBar.update();
		ProgressBar.updateProgressBar ((uint32)(100 * progress));
	}
};

// -----------------------------------------------------------------------------------------------

void regsiterOVPath ()
{
// must test it first, because NL_DEBUG_FAST and NL_DEBUG are declared at same time.
//#ifdef NL_DEBUG_FAST
//	HMODULE hModule = GetModuleHandle("object_viewer_dll_df.dll");
#if defined (NL_DEBUG)
	HMODULE hModule = GetModuleHandle(_T("object_viewer_dll_d.dll"));
//#elif defined (NL_RELEASE_DEBUG)
//	HMODULE hModule = GetModuleHandle("object_viewer_dll_rd.dll");
#else
	HMODULE hModule = GetModuleHandle(_T("object_viewer_dll_r.dll"));
#endif
	if (!hModule) { ::MessageBox(NULL, _T("'hModule' failed at '") __FUNCTION__ _T("' in file '") __FILE__ _T(" on line ") NL_MACRO_TO_STR(__LINE__), _T("NeL Export"), MB_OK | MB_ICONERROR); return; }
	TCHAR sModulePath[256];
	int res = GetModuleFileName(hModule, sModulePath, 256);
	if (!res) { ::MessageBox(NULL, _T("'res' failed at '") __FUNCTION__ _T("' in file '") __FILE__ _T(" on line ") NL_MACRO_TO_STR(__LINE__), _T("NeL Export"), MB_OK | MB_ICONERROR); return; }

	std::string modulePath = NLMISC::CFile::getPath(MCharStrToUtf8(sModulePath)) + "object_viewer.cfg";

	// Load the config file
	CConfigFile cf;
	cf.load (modulePath);

	try
	{
		// Add search pathes
		CConfigFile::CVar &search_pathes = cf.getVar ("search_pathes");
		for (uint i=0; i<(uint)search_pathes.size(); i++)
			CPath::addSearchPath (search_pathes.asString(i));
	}
	catch(const EUnknownVar &)
	{}

	try
	{
		// Add recusrive search pathes
		CConfigFile::CVar &recursive_search_pathes = cf.getVar ("recursive_search_pathes");
		for (uint i=0; i<(uint)recursive_search_pathes.size(); i++)
			CPath::addSearchPath (recursive_search_pathes.asString(i), true, false);
	}
	catch(const EUnknownVar &)
	{}

	// Add extension remapping
	try
	{
		CConfigFile::CVar &extensions_remapping = cf.getVar ("extensions_remapping");
		if (extensions_remapping.size()%2 != 0)
		{
			nlwarning ("extensions_remapping must have a multiple of 2 entries (ex: extensions_remapping={\"dds\",\"tga\"};)");
		}
		else
		{
			for (uint i=0; i<(uint)extensions_remapping.size(); i+=2)
				CPath::remapExtension(extensions_remapping.asString(i), extensions_remapping.asString(i+1), true);
		}
	}
	catch (const EUnknownVar &)
	{
	}
}

void CNelExport::viewMesh (TimeValue time)
{
	// Register classes
	// done in dllentry registerSerial3d ();

	CScene::registerBasics ();
	
	// Register CPath in our module
	regsiterOVPath ();

	// Create an object viewer
	IObjectViewer* view = IObjectViewer::getInterface();

	// Check wether there's not an instance currently running
	if (view->isInstanceRunning())
	{
		::MessageBox(NULL, _T("An instance of the viewer is currently running, please close it :)"), _T("NeL Export"), MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	// set the water pool manager
	view->setWaterPoolManager(NL3D::GetWaterPoolManager());
	
	// Build a skeleton map
	mapRootMapBoneBindPos				skeletonMap;

	std::map<INode*, CSkeletonDesc>	mapSkeletonShape;

	if (view)
	{
		// Init it
		if (!view->initUI())
		{
			::MessageBox(NULL, _T("Failed to initialize object viewer ui, this may be a driver init issue, check your log.log files"), _T("NeL Export"), MB_OK|MB_ICONEXCLAMATION);
			IObjectViewer::releaseInterface(view);
			return;
		}

		// Get node count
		int nNumSelNode=_Ip->GetSelNodeCount();
		int nNbMesh=0;
		// Create an animation for the models
		CAnimation *autoAnim=new CAnimation;

		// *******************
		// * First build skeleton bind pos information and animations
		// *******************

		int nNode;
		for (nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=_Ip->GetSelNode (nNode);

			// It is a zone ?
			if (RPO::isZone (*pNode, time))
			{
			}
			// Try to export a mesh
			else if (CExportNel::isMesh (*pNode, time))
			{
				// Build skined ?
				bool skined=false;
				
				// Skinning ?
				if (CExportNel::isSkin (*pNode))
				{
					// Get root of the skeleton
					INode *skeletonRoot=CExportNel::getSkeletonRootBone (*pNode);

					// HULUD TEST
					//skeletonRoot=NULL;
					
					// Root exist ?
					if (skeletonRoot)
					{
						// Ok, look for the set in the map of desc
						mapRootMapBoneBindPos::iterator iteSkeleton=skeletonMap.find (skeletonRoot);

						// Not found ?
						if (iteSkeleton==skeletonMap.end())
						{
							// Insert one
							skeletonMap.insert (mapRootMapBoneBindPos::value_type (skeletonRoot, CExportNel::mapBoneBindPos()));
							iteSkeleton=skeletonMap.find (skeletonRoot);
						}
						
						// Add the bind pos for the skin
						CExportNel::addSkeletonBindPos (*pNode, iteSkeleton->second);
					}
				}
			}
		}

		// *******************
		// * Then, build skeleton shape
		// *******************

		for (nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=_Ip->GetSelNode (nNode);

			// It is a zone ?
			if (RPO::isZone (*pNode, time))
			{
			}
			// Try to export a mesh
			else if (CExportNel::isMesh (*pNode, time))
			{
				// Build skined ?
				bool skined=false;
				++nNbMesh;
				// Skinning ?
				if (CExportNel::isSkin (*pNode))
				{
					// Get root of the skeleton
					INode *skeletonRoot=CExportNel::getSkeletonRootBone (*pNode);
					
					// HULUD TEST
					//skeletonRoot=NULL;
					
					// Root exist ?
					if (skeletonRoot)
					{
						// Ok, look for the set in the map of desc
						mapRootMapBoneBindPos::iterator iteSkeleton = skeletonMap.find (skeletonRoot);
						std::map<INode*, CSkeletonDesc>::iterator skelBindPod = mapSkeletonShape.find (skeletonRoot);

						// Not found ?
						if (skelBindPod==mapSkeletonShape.end())
						{
							// Insert it
							CSkeletonShape *skelShape=new CSkeletonShape();

							// A matrix id map
							TInodePtrInt mapId;

							// Build the skeleton based on the bind pos information
							_ExportNel->buildSkeletonShape (*skelShape, *skeletonRoot, &(iteSkeleton->second), mapId, time);

							// Add the shape in the view
							uint instance = view->addSkel (skelShape, MCharStrToUtf8(skeletonRoot->GetName()));

							// Add tracks
							CAnimation *anim=new CAnimation;
							_ExportNel->addAnimation (*anim, *skeletonRoot, "", true);

							// Set the single animation							
							view->setSingleAnimation (anim, "3dsmax current animation", instance);

							// Insert in the map
							mapSkeletonShape.insert (std::map<INode*, CSkeletonDesc>::value_type ( skeletonRoot, CSkeletonDesc (instance, mapId)));
						}
					}
				}
			}
		}

		CAnimationSet *animationSet = new CAnimationSet;
		for (nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=_Ip->GetSelNode (nNode);

			// Is it a automatic light ? if yes add tracks from nel_light (color controller)
			int bAnimated = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_LM_ANIMATED, 0);
			if (bAnimated)
				_ExportNel->addAnimation( *autoAnim, *pNode, "", true);

				
		} 
		animationSet->addAnimation ("Automatic", autoAnim);
		animationSet->build();
		view->setAutoAnimation (animationSet);

		// *******************
		// * Then, build Mesh shapes
		// *******************

		// Prepare ig export.
		std::vector<INode*>						igVectNode;
		std::map<std::string, NL3D::IShape *>	igShapeMap;
		// SunDirection.
		NLMISC::CVector							igSunDirection(0, 1, -1);
		// SunColor.
		NLMISC::CRGBA							igSunColor(255, 255, 255);
		SLightBuild								sgLightBuild;


		// Build Mesh Shapes.
		CProgressBar ProgBar;
		ProgBar.initProgressBar (nNbMesh, *_Ip);
		theExportSceneStruct.FeedBack = &ProgBar;
		nNbMesh = 0;

		// Map for IG animations
		typedef std::map<INode *, CAnimation *> TIGAnimation;
		TIGAnimation igAnim;

		// View all selected objects
		for (nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=_Ip->GetSelNode (nNode);

			string sTmp = "Object Name: ";
			sTmp += MCharStrToUtf8(pNode->GetName());
			ProgBar.setLine (0, sTmp);
			sTmp.clear();
			for (uint32 i = 1; i < 10; ++i) 
				ProgBar.setLine (i, sTmp);
			sTmp = "Last Error";
			ProgBar.setLine (10, sTmp);
			ProgBar.update();
			
			// It is a zone ?
			if (RPO::isZone (*pNode, time))
			{
			}
			// Try to export a mesh
			else if (CExportNel::isMesh (*pNode, time))
			{
				// Build skined ?
				bool skined=false;
				++nNbMesh;
				// Skinning ?
				if (CExportNel::isSkin (*pNode))
				{
					// Create a skeleton
					INode *skeletonRoot=CExportNel::getSkeletonRootBone (*pNode);

					// HULUD TEST
					//skeletonRoot=NULL;
					
					// Skeleton exist ?
					if (skeletonRoot)
					{
						// Look for bind pos info for this skeleton
						mapRootMapBoneBindPos::iterator iteSkel=skeletonMap.find (skeletonRoot);
						std::map<INode*, CSkeletonDesc>::iterator iteSkelShape=mapSkeletonShape.find (skeletonRoot);
						nlassert (iteSkel!=skeletonMap.end());
						nlassert (iteSkelShape!=mapSkeletonShape.end());

						// Export the shape
						IShape *pShape;
						pShape=_ExportNel->buildShape (*pNode, time, &iteSkelShape->second.MapId, true);

						// Build succesful ?
						if (pShape)
						{
							// Add the shape in the view
							uint instance = view->addMesh (pShape, MCharStrToUtf8(pNode->GetName()), iteSkelShape->second.SkeletonInstance);

							// Add tracks
							CAnimation *anim=new CAnimation;
							_ExportNel->addAnimation (*anim, *pNode, "", true);

							// Set the single animation
							view->setSingleAnimation (anim, "3dsmax current animation", instance);

							// ok
							skined=true;
						}
					}
				}
				// Build skined ?
				if (!skined)
				{
					// Export the shape
					IShape *pShape = NULL;
					pShape=_ExportNel->buildShape (*pNode, time, NULL, true);

					// Export successful ?
					if (pShape)
					{
						// get the nelObjectName, as used in building of the instanceGroup.
						std::string	nelObjectName= CExportNel::getNelObjectName(*pNode);

						// ugly way to verify the shape is really added to the sahepBank: use a refPtr :)
						NLMISC::CRefPtr<IShape>		prefShape= pShape;

	
						std::string nelObjectNameNoExt;
						// Add to the view, but don't create the instance (created in ig).						
						if (!(nelObjectName.find(".shape") != std::string::npos || nelObjectName.find(".ps") != std::string::npos))
						{
							nelObjectNameNoExt = nelObjectName;
							nelObjectName += ".shape";							
						}
						else
						{
							std::string::size_type pos = nelObjectName.find(".");
							nlassert(pos != std::string::npos);
							nelObjectNameNoExt = std::string(nelObjectName, 0, pos);
						}
						
						// Add to the view, but don't create the instance (created in ig).
						// Since IG use strlwr version of the name, must strlwr it here.
						std::string	nelObjectNameLwr= nelObjectName;
						strlwr(nelObjectNameLwr);
						view->addMesh (pShape, nelObjectNameLwr.c_str(), 0xffffffff, NULL, false);


						// If the shape is not destroyed in addMesh() (with smarPtr), then add it to the shape map.
						if(prefShape)
						{
							igShapeMap.insert( std::make_pair(nelObjectNameNoExt, pShape) );
						}
						
						// Add to list of node for IgExport.
						igVectNode.push_back(pNode);
					}

					// Add tracks
					igAnim.insert (TIGAnimation::value_type (pNode, new CAnimation));
					_ExportNel->addAnimation (*igAnim[pNode], *pNode, "", true);
				}
			}

			ProgBar.updateProgressBar (nNbMesh);
			if( ProgBar.isCanceledProgressBar() )
				break;
		}

		// if ExportLighting, Export all lights in scene (not only selected ones).
		if(theExportSceneStruct.bExportLighting)
		{
			// List all nodes in scene.
			vector<INode*>	nodeList;
			_ExportNel->getObjectNodes(nodeList, time);
			
			// For all of them.
			for(uint i=0;i<nodeList.size();i++)
			{
				INode	*pNode= nodeList[i];

				if( sgLightBuild.canConvertFromMaxLight(pNode, time) )
				{
					// Convert it.
					sgLightBuild.convertFromMaxLight(pNode, time);

					// PointLight/SpotLight/AmbientLight ??
					if(sgLightBuild.Type != SLightBuild::LightDir)
					{
						// Add to list of node for IgExport.
						igVectNode.push_back(pNode);
					}
					// "SunLight" ??
					else if( sgLightBuild.Type == SLightBuild::LightDir )
					{
						// if this light is checked to export as Sun Light
						int		nExportSun= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED);
						if(nExportSun== BST_CHECKED)
						{
							// Add to list of node for IgExport (enabling SunLight in the ig)
							igVectNode.push_back(pNode);

							// Replace sun Direciton.
							igSunDirection= sgLightBuild.Direction;
							// Replace sun Color.
							igSunColor= sgLightBuild.Diffuse;
						}
					}
				}
			}
		}


		ProgBar.uninitProgressBar();
		theExportSceneStruct.FeedBack = NULL;
	

		// *******************
		// * Export instance Group.
		// *******************

		// Info for lighting: retrieverBank and globalRetriever.
		CIgLighterLib::CSurfaceLightingInfo		slInfo;
		slInfo.RetrieverBank= NULL;
		slInfo.GlobalRetriever= NULL;

		// Result instance group
		vector<INode*> resultInstanceNode;

		// Build the ig (with pointLights)
		NL3D::CInstanceGroup	*ig= _ExportNel->buildInstanceGroup(igVectNode, resultInstanceNode, time);
		if(ig)
		{
			// If ExportLighting
			if( theExportSceneStruct.bExportLighting )
			{
				// Light the ig.
				NL3D::CInstanceGroup	*igOut= new NL3D::CInstanceGroup;
				// Init the lighter.
				CMaxInstanceLighter		maxInstanceLighter;
				maxInstanceLighter.initMaxLighter(*_Ip);

				// Setup LightDesc Ig.
				CInstanceLighter::CLightDesc	lightDesc;
				// Copy map to get info on shapes.
				lightDesc.UserShapeMap= igShapeMap;
				// Setup Shadow and overSampling.
				lightDesc.Shadow= theExportSceneStruct.bShadow;
				lightDesc.OverSampling= NLMISC::raiseToNextPowerOf2(theExportSceneStruct.nOverSampling);
				clamp(lightDesc.OverSampling, 0U, 32U);
				if(lightDesc.OverSampling==1)
					lightDesc.OverSampling= 0;
				// Setup LightDirection.
				lightDesc.LightDirection= igSunDirection.normed();
				// For interiors ig, disable Sun contrib according to ig.
				lightDesc.DisableSunContribution= !ig->getRealTimeSunContribution();


				// If View SurfaceLighting enabled
				if(theExportSceneStruct.bTestSurfaceLighting)
				{
					// Setup a CSurfaceLightingInfo
					slInfo.CellSurfaceLightSize= theExportSceneStruct.SurfaceLightingCellSize;
					NLMISC::clamp(slInfo.CellSurfaceLightSize, 0.001f, 1000000.f);
					slInfo.CellRaytraceDeltaZ= theExportSceneStruct.SurfaceLightingDeltaZ;
					// no more add any prefix to the colision identifier.
					slInfo.ColIdentifierPrefix.clear();
					slInfo.ColIdentifierSuffix.clear();
					// Build RetrieverBank and GlobalRetriever from collisions in scene
					_ExportNel->computeCollisionRetrieverFromScene(time, slInfo.RetrieverBank, slInfo.GlobalRetriever, 
						slInfo.ColIdentifierPrefix.c_str(), slInfo.ColIdentifierSuffix.c_str(), slInfo.IgFileName);
				}


				// Light Ig.
				CIgLighterLib::lightIg(maxInstanceLighter, *ig, *igOut, lightDesc, slInfo, "");

				// Close the lighter.
				maxInstanceLighter.closeMaxLighter();

				// Swap pointer and release unlighted one.
				swap(ig, igOut);
				delete igOut;
			}

			// Setup the ig in Viewer.
			uint firstInstance = view->addInstanceGroup(ig);

			// Setup animations
			uint i;
			for (i=0; i<ig->getNumInstance(); i++)
			{
				// Set the single animation
				view->setSingleAnimation (igAnim[resultInstanceNode[i]], "3dsmax current animation", firstInstance + i);

				// Remove the animation
				igAnim.erase (resultInstanceNode[i]);
			}
		}


		// Add cameras
		for (nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=_Ip->GetSelNode (nNode);

			// Is a camera ?
			if (CExportNel::isCamera (*pNode, time))
			{
				// Export the shape
				CCameraInfo cameraInfo;
				_ExportNel->buildCamera (cameraInfo, *pNode, time);

				// Camera name
				std::string name = CExportNel::getNelObjectName(*pNode);
				strlwr (name);

				uint instance = view->addCamera (cameraInfo, name.c_str ());

				// Add tracks
				CAnimation *anim=new CAnimation;
				_ExportNel->addAnimation (*anim, *pNode, "", true);

				// Set the single animation
				view->setSingleAnimation (anim, "3dsmax current animation", instance);
				instance++;
			}
		}

		// Erase unused animations
		TIGAnimation::iterator ite = igAnim.begin();
		while (ite != igAnim.end())
		{
			// Delete it
			delete ite->second;

			// Next
			ite++;
		}

		// *******************
		// * Launch
		// *******************


		// Setup background color
		if (theExportSceneStruct.bExportBgColor)
			view->setBackGroundColor(_ExportNel->getBackGroundColor(time));

		// ExportLighting?
		if ( theExportSceneStruct.bExportLighting )
		{
			// Take the ambient of the scene as the ambient of the sun.
			CRGBA	sunAmb= _ExportNel->getAmbientColor (time);

			// Disable Global ambient light
			view->setAmbientColor (CRGBA::Black);

			// setup lighting and sun, if any light added. Else use std OpenGL front lighting
			view->setupSceneLightingSystem(true, igSunDirection, sunAmb, igSunColor, igSunColor);
			// If GlobalRetriever for DynamicObjectLightingTest is present, use it.
			if(slInfo.GlobalRetriever && ig)
				view->enableDynamicObjectLightingTest(slInfo.GlobalRetriever, ig);
		}
		else
		{
			/*// Setup ambient light
			view->setAmbientColor (_ExportNel->getAmbientColor (time));

			// Build light vector
			std::vector<CLight> vectLight;
			_ExportNel->getLights (vectLight, time);

			// Light in the scene ?
			if (!vectLight.empty())
			{
				// Use old Driver Light mgt.
				view->setupSceneLightingSystem(false, igSunDirection, CRGBA::Black, igSunColor, igSunColor);

				// Insert each lights
				for (uint light=0; light<vectLight.size(); light++)
					view->setLight (light, vectLight[light]);
			}*/
		}

		// Reset the camera
		view->resetCamera ();

		// Go
		view->go ();

		// Release object viewer
		view->releaseUI ();

		// Delete the pointer
		IObjectViewer::releaseInterface (view);

		// Collisions information are no more used.
		delete slInfo.RetrieverBank;
		delete slInfo.GlobalRetriever;
	}
}


