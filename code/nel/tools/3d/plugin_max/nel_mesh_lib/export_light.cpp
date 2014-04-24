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

#include "stdafx.h"
#include "export_nel.h"
#include "nel/3d/light.h"
#include "nel/misc/common.h"

using namespace NLMISC;
using namespace NL3D;

// ***************************************************************************

// Build a CLight
bool CExportNel::buildLight (GenLight &maxLight, NL3D::CLight& nelLight, INode& node, TimeValue time)
{
	// Eval the light state fot this time
	Interval valid=NEVER;
	LightState ls;
	if (maxLight.EvalLightState(time, valid, &ls)==REF_SUCCEED)
	{
		// Set the light mode
		switch (maxLight.Type())
		{
		case OMNI_LIGHT:
			nelLight.setMode (CLight::PointLight);
			break;
		case TSPOT_LIGHT:
		case FSPOT_LIGHT:
			nelLight.setMode (CLight::SpotLight);
			break;
		case DIR_LIGHT:
		case TDIR_LIGHT:
			nelLight.setMode (CLight::DirectionalLight);
			break;
		default:
			// Not initialized
			return false;
		}

		// *** Set the light color

		// Get the color
		CRGBA nelColor;
		Point3 maxColor=maxLight.GetRGBColor(time);

		// Mul by multiply
		CRGBAF nelFColor;
		nelFColor.R=maxColor.x;
		nelFColor.G=maxColor.y;
		nelFColor.B=maxColor.z;
		nelFColor.A=1.f;
		nelFColor*=maxLight.GetIntensity(time);
		nelColor=nelFColor;

		// Affect the ambiant color ?
		if (maxLight.GetAmbientOnly())
		{
			nelLight.setAmbiant (nelColor);
			nelLight.setDiffuse (CRGBA (0,0,0));
			nelLight.setSpecular (CRGBA (0,0,0));
		}
		else
		{
			nelLight.setAmbiant (CRGBA (0,0,0));

			// Affect the diffuse color ?
			if (maxLight.GetAffectDiffuse())
				nelLight.setDiffuse (nelColor);
			else
				nelLight.setDiffuse (CRGBA (0,0,0));

			// Affect the specular color ?
			if (maxLight.GetAffectSpecular())
				nelLight.setSpecular (nelColor);
			else
				nelLight.setSpecular (CRGBA (0,0,0));
		}

		// Set the light position
		Point3 pos=node.GetNodeTM(time).GetTrans ();
		CVector position;
		position.x=pos.x;
		position.y=pos.y;
		position.z=pos.z;

		// Set the position
		nelLight.setPosition (position);

		// Set the light direction
		CVector direction;
		INode* target=node.GetTarget ();
		if (target)
		{
			// Get the position of the target
			Point3 posTarget=target->GetNodeTM (time).GetTrans ();
			CVector positionTarget;
			positionTarget.x=posTarget.x;
			positionTarget.y=posTarget.y;
			positionTarget.z=posTarget.z;

			// Direction
			direction=positionTarget-position;
			direction.normalize ();
		}
		else	// No target
		{
			// Get orientation of the source as direction
			CMatrix nelMatrix;
			convertMatrix (nelMatrix, node.GetNodeTM(time));

			// Direction is -Z
			direction=-nelMatrix.getK();
			direction.normalize ();
		}

		// Set the direction
		nelLight.setDirection (direction);

		// Set spot light information
		nelLight.setCutoff ((float)(NLMISC::Pi*maxLight.GetFallsize(time)/180.f/2.f));

		// Compute the exponent value
		float angle=(float)(NLMISC::Pi*maxLight.GetHotspot(time)/(2.0*180.0));
		nelLight.setupSpotExponent (angle);

		// *** Set attenuation

		if (maxLight.GetUseAtten())
		{
			float nearAtten = maxLight.GetAtten (time, ATTEN_START);
			if (nearAtten == 0)
				nearAtten = 0.1f;
			nelLight.setupAttenuation (nearAtten, maxLight.GetAtten (time, ATTEN_END));
		}
		else
			nelLight.setNoAttenuation ();

		// Initialized
		return true;
	}

	// Not initialized
	return false;
}

// ***************************************************************************
// Get the ambient value
CRGBA CExportNel::getAmbientColor (TimeValue time)
{
	// validuty
	Interval valid=NEVER;

	// Get max color
	Point3 color=255.f*_Ip->GetAmbient (time, valid);
	clamp (color.x, 0.f, 255.f);
	clamp (color.y, 0.f, 255.f);
	clamp (color.z, 0.f, 255.f);

	// Return NeL color
	return CRGBA ((uint8)color.x, (uint8)color.y, (uint8)color.z);
}

// ***************************************************************************
// Get the ambient value
CRGBA CExportNel::getBackGroundColor (TimeValue time)
{
	// validuty
	Interval valid=NEVER;

	// Get max color
	Point3 color=255.f*_Ip->GetBackGround(time, valid);
	clamp (color.x, 0.f, 255.f);
	clamp (color.y, 0.f, 255.f);
	clamp (color.z, 0.f, 255.f);

	// Return NeL color
	return CRGBA ((uint8)color.x, (uint8)color.y, (uint8)color.z);
}

// ***************************************************************************

void CExportNel::getLights (std::vector<CLight>& vectLight, TimeValue time, INode* node)
{
	// Get the root node
	if (node==NULL)
		node=_Ip->GetRootNode();

	// Get a pointer on the object's node
    ObjectState os = node->EvalWorldState(time);
    Object *obj = os.obj;

	// Check if there is an object
	if (obj)
	{
		// Get a GenLight from the node
		if (obj->SuperClassID()==LIGHT_CLASS_ID)
		{
			/*GenLight *maxLight = (GenLight *) obj->ConvertToType(time, Class_ID(OMNI_LIGHT_CLASS_ID , 0));
			if (!maxLight)
				maxLight = (GenLight *) obj->ConvertToType(time, Class_ID(SPOT_LIGHT_CLASS_ID, 0));
			if (!maxLight)
				maxLight = (GenLight *) obj->ConvertToType(time, Class_ID(DIR_LIGHT_CLASS_ID, 0));
			if (!maxLight)
				maxLight = (GenLight *) obj->ConvertToType(time, Class_ID(FSPOT_LIGHT_CLASS_ID, 0));
			if (!maxLight)
				maxLight = (GenLight *) obj->ConvertToType(time, Class_ID(TDIR_LIGHT_CLASS_ID, 0));*/
			//if (maxLight)
			GenLight *maxLight = (GenLight *) obj;

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			bool deleteIt=false;
			if (obj != maxLight) 
				deleteIt = true;

			// Build a light
			CLight nelLight;

			// If build succesful
			if (buildLight (*maxLight, nelLight, *node, time))
				// Add the light in the list
				vectLight.push_back (nelLight);

			// Delete the GenLight if we should...
			if (deleteIt)
				maxLight->DeleteThis();
		}
	}

	// Recurse sub node
	for (int i=0; i<node->NumberOfChildren(); i++)
		getLights (vectLight, time, node->GetChildNode(i));
}

// ***************************************************************************


