#include "stdafx.h"
#include "paint_light.h"
#include "../nel_mesh_lib/export_nel.h"
#include "../nel_mesh_lib/export_appdata.h"
#include "nel/3d/scene.h"
#include "nel/3d/landscape.h"
#include "nel/3d/point_light_model.h"

using namespace NL3D;
using namespace NLMISC;

/*-------------------------------------------------------------------*/

void CPaintLight::setup (NL3D::CLandscape &landscape, NL3D::CScene &scene)
{
	// Setup the landscape
	landscape.setDynamicLightingMaxAttEnd (1000);

	// Setup the scene
	scene.enableLightingSystem (true);

	// For each lights
	for (uint light=0; light<_Lights.size (); light++)
	{
		// Create a model
		CTransform *model= scene.createModel(PointLightModelId);
		
		// If not found, return NULL.
		if(model==NULL)
			return;

		// Cast to point light model
		CPointLightModel	*pointLightModel = safe_cast<CPointLightModel*>(model);

		// Direct matrix
		pointLightModel->setTransformMode (ITransformable::DirectMatrix);

		// Position
		CMatrix mt = CMatrix::Identity;
		mt.setPos (_Lights[light].Position);
		pointLightModel->setMatrix (mt);
		
		// Attenuation
		pointLightModel->PointLight.setupAttenuation (_Lights[light].rRadiusMin, _Lights[light].rRadiusMax);

		// Colors
		// Ensure A=255 for localAmbient to work.
		NLMISC::CRGBA	ambient= _Lights[light].Ambient;
		ambient.A= 255;

		// Set the colors
		pointLightModel->PointLight.setAmbient (ambient);
		pointLightModel->PointLight.setDiffuse (_Lights[light].Diffuse);
		pointLightModel->PointLight.setSpecular (_Lights[light].Specular);

		// Which light type??
		if(_Lights[light].bAmbientOnly || _Lights[light].Type== SLightBuild::LightAmbient)
			pointLightModel->PointLight.setType (CPointLight::AmbientLight);
		else if(_Lights[light].Type== SLightBuild::LightPoint)
			pointLightModel->PointLight.setType (CPointLight::PointLight);
		else if(_Lights[light].Type== SLightBuild::LightSpot)
		{
			pointLightModel->PointLight.setType (CPointLight::SpotLight);

			// Export Spot infos.
			pointLightModel->lookAt (_Lights[light].Position, _Lights[light].Position + _Lights[light].Direction);
			pointLightModel->PointLight.setupSpotAngle (_Lights[light].rHotspot, _Lights[light].rFallof);
		}
		else
		{
			// What???
			nlstop;
		}
	}
}

/*-------------------------------------------------------------------*/

void CPaintLight::build (Interface &ip, INode *node)
{
	SLightBuild		sLightBuild;

	// First ? Clear array of lights
	if (node == NULL)
	{
		_Lights.clear ();
		node = ip.GetRootNode ();
	}

	// Sun light (not used for the moment)
	bool sunLightEnabled = false;

	// If it is a Max Light.
	if ( sLightBuild.canConvertFromMaxLight(node, 0) )
	{
		// And if this light is checked to realtime export
		int		nRTExport= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, BST_CHECKED);
		if(nRTExport == BST_CHECKED)
		{
			// get Max Light info.
			sLightBuild.convertFromMaxLight(node, 0);

			// Skip if LightDir
			if(sLightBuild.Type != SLightBuild::LightDir)
			{
				// Add to the array
				_Lights.push_back (sLightBuild);
			}
		}

		// if this light is a directionnal and checked to export as Sun Light
		int		nExportSun= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED);
		if(nExportSun== BST_CHECKED)
		{
			// get Max Light info.
			sLightBuild.convertFromMaxLight(node, 0);

			// Skip if not dirLight.
			if(sLightBuild.Type == SLightBuild::LightDir)
				sunLightEnabled= true;
		}
	}

	// First pointer on the root scene tree
	for (uint i = 0; i < (uint)node->NumberOfChildren(); i++)
	{
		// Build child
		build (ip, node->GetChildNode (i));
	}
}

/*-------------------------------------------------------------------*/
