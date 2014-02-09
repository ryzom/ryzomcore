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

#include <vector>

#include "export_nel.h"
#include "export_appdata.h"
#include "../nel_patch_lib/rpo.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/triangle.h"
#include "nel/misc/bsphere.h"
#include "nel/misc/path.h"
#include "nel/3d/quad_tree.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/light.h"
#include "nel/3d/bsp_tree.h"
#include "nel/3d/quad_grid.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;


#include "calc_lm.h"
#include "calc_lm_plane.h"
#include "calc_lm_rt.h"



// TOOLS
// *****

// Substract to all to get more precision
CVector vGlobalPos;

// ***********************************************************************************************
// SLightBuild
// ***********************************************************************************************

// -----------------------------------------------------------------------------------------------
SLightBuild::SLightBuild()
{
	Type = LightPoint;
	Position = CVector(0.0, 0.0, 0.0);
	Direction = CVector(1.0, 0.0, 0.0);
	rRadiusMin = 1.0f;
	rRadiusMax = 2.0f;
	Ambient = CRGBA(0, 0, 0, 0);
	Diffuse = CRGBA(0, 0, 0, 0);
	Specular = CRGBA(0, 0, 0, 0);
	bCastShadow = false;
	rMult = 1.0f;
	LightGroup = 0;
	rDirRadius = 0.0f;
}

// -----------------------------------------------------------------------------------------------
bool SLightBuild::canConvertFromMaxLight (INode *node, TimeValue tvTime)
{
	// Get a pointer on the object's node
	ObjectState os = node->EvalWorldState(tvTime);
    Object *obj = os.obj;

	// Check if there is an object
	if (!obj)
		return false;

	// Get a GenLight from the node
	if (!(obj->SuperClassID()==LIGHT_CLASS_ID))
		return false;

	GenLight *maxLight = (GenLight *) obj;
	bool deleteIt=false;
	if (obj != maxLight) 
		deleteIt = true;

	Interval valid=NEVER;
	LightState ls;
	if (maxLight->EvalLightState(tvTime, valid, &ls)!=REF_SUCCEED)
		return false;

	if( deleteIt )
		maxLight->DeleteThis();

	return true;
}

// -----------------------------------------------------------------------------------------------
void SLightBuild::convertFromMaxLight (INode *node,TimeValue tvTime)
{
	// Get a pointer on the object's node
	ObjectState os = node->EvalWorldState(tvTime);
    Object *obj = os.obj;

	// Check if there is an object
	if (!obj) return;

	// Get a GenLight from the node
	if (!(obj->SuperClassID()==LIGHT_CLASS_ID))
		return ;

	GenLight *maxLight = (GenLight *) obj;
	bool deleteIt = false;
	if (obj != maxLight) 
		deleteIt = true;

	Interval valid=NEVER;
	LightState ls;
	if (maxLight->EvalLightState(tvTime, valid, &ls)!=REF_SUCCEED)
		return;

	this->Name = node->GetName();

	// Retrieve the correct light Group Name
	this->AnimatedLight = CExportNel::getAnimatedLight (node);
	this->LightGroup = CExportNel::getLightGroup (node);

	// Eval the light state fot this tvTime
	// Set the light mode
	switch (maxLight->Type())
	{
		case OMNI_LIGHT:
			this->Type = SLightBuild::LightPoint;
		break;
		case TSPOT_LIGHT:
		case FSPOT_LIGHT:
			this->Type = SLightBuild::LightSpot;
		break;
		case DIR_LIGHT:
		case TDIR_LIGHT:
			this->Type = SLightBuild::LightDir;
		break;
		default:
			// Not initialized
		break;
	}

	// *** Set the light color

	// Get the color
	CRGBA nelColor;
	Point3 maxColor = maxLight->GetRGBColor(tvTime);

	// Mul by multiply
	CRGBAF nelFColor;
	nelFColor.R = maxColor.x;
	nelFColor.G = maxColor.y;
	nelFColor.B = maxColor.z;
	nelFColor.A = 1.f;
	// nelFColor   *= maxLight->GetIntensity(tvTime);
	nelColor = nelFColor;

	// Affect the ambiant color ?
	this->Ambient = CRGBA (0,0,0);
	this->Diffuse = CRGBA (0,0,0);
	this->Specular = CRGBA (0,0,0);

	
	if (maxLight->GetAmbientOnly())
	{
		this->bAmbientOnly= true;
		this->Ambient = nelColor;
	}
	else
	{
		this->bAmbientOnly= false;
		// Affect the diffuse color ?
		if( maxLight->GetAffectDiffuse() )
			this->Diffuse = nelColor;
		// Affect the specular color ?
		if (maxLight->GetAffectSpecular())
			this->Specular = nelColor;
	}

	// Set the light position
	Point3 pos = node->GetNodeTM(tvTime).GetTrans ();
	CVector position;
	position.x=pos.x;
	position.y=pos.y;
	position.z=pos.z;

	// Set the position
	this->Position = position - vGlobalPos;

	// Set the light direction
	CVector direction;
	INode* target = node->GetTarget ();
	if (target)
	{
		// Get the position of the target
		Point3 posTarget=target->GetNodeTM (tvTime).GetTrans ();
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
		CExportNel::convertMatrix (nelMatrix, node->GetNodeTM(tvTime));

		// Direction is -Z
		direction=-nelMatrix.getK();
		direction.normalize ();
	}

	// Set the direction
	this->Direction = direction;

	this->rHotspot = (float)(Pi * maxLight->GetHotspot(tvTime) /(2.0*180.0));
	this->rFallof =  (float)(Pi * maxLight->GetFallsize(tvTime)/(2.0*180.0));

	if (maxLight->GetUseAtten())
	{
		this->rRadiusMin = maxLight->GetAtten (tvTime, ATTEN_START);
		this->rRadiusMax = maxLight->GetAtten (tvTime, ATTEN_END);
	}
	else
	{	// Limit
		this->rRadiusMin = 10.0;
		this->rRadiusMax = 10.0;
	}

	this->bCastShadow = ( maxLight->GetShadow() != 0 );
	this->rMult = maxLight->GetIntensity (tvTime);

	// Construct the bitmap projector if there is a projector
	if (maxLight->GetProjector() != 0)
	{
		Texmap* tm = maxLight->GetProjMap();
		CExportNel::convertMatrix (this->mProj, node->GetNodeTM(tvTime));
		if (CExportNel::isClassIdCompatible( *tm, Class_ID (BMTEX_CLASS_ID,0)))
		{
			BitmapTex* bmt = (BitmapTex*)tm;
			Bitmap *pProjMap = bmt->GetBitmap(tvTime);


			// Construct the projector bitmap if some
			if( pProjMap != NULL )
			{
				ProjBitmap.resize (pProjMap->Width(), pProjMap->Height(), CBitmap::RGBA);
				// Copy the bitmap
				CObjectVector<uint8> &rBitmap = ProjBitmap.getPixels();
				BMM_Color_64 OnePixel;
				for( uint32 k = 0; k < ProjBitmap.getHeight(); ++k )
				for( uint32 j = 0; j < ProjBitmap.getWidth(); ++j )
				{
					pProjMap->GetPixels( j, k, 1, &OnePixel );
					rBitmap[(j+k*ProjBitmap.getWidth())*4+0] = OnePixel.r>>8;
					rBitmap[(j+k*ProjBitmap.getWidth())*4+1] = OnePixel.g>>8;
					rBitmap[(j+k*ProjBitmap.getWidth())*4+2] = OnePixel.b>>8;
					rBitmap[(j+k*ProjBitmap.getWidth())*4+3] = OnePixel.a>>8;
				}
				ProjBitmap.buildMipMaps();
			}
		}
	}

#if (MAX_RELEASE < 4000)
	// Convert exclusion list
	NameTab& ntExclu = maxLight->GetExclusionList();
	for (sint i = 0; i < ntExclu.Count(); ++i)
	{
		string tmp = *ntExclu.Addr(i);
		this->setExclusion.insert(tmp);
	}
#else // (MAX_RELEASE < 4000)
	ExclList& exclusionList = maxLight->GetExclusionList();
	for (sint i = 0; i < exclusionList.Count(); ++i)
	{
		INode *exclNode = exclusionList[i];
		string tmp = exclNode->GetName();
		this->setExclusion.insert(tmp);
	}
#endif // (MAX_RELEASE < 4000)

	// Get Soft Shadow information
	string sTmp = CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_RADIUS, toString(NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT));
	this->rSoftShadowRadius = (float)atof(sTmp.c_str());
	sTmp = CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH, toString(NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT));
	this->rSoftShadowConeLength = (float)atof(sTmp.c_str());

	if( deleteIt )
		maxLight->DeleteThis();
}

// ***********************************************************************************************
// SGradient
// ***********************************************************************************************
struct SGradient
{
	// Vertex gradient
	double InitPx, InitPy, InitPz;
	double GraduPx, GradvPx;
	double GraduPy, GradvPy;
	double GraduPz, GradvPz;
	// Normal gradient
	double InitNx, InitNy, InitNz;
	double GraduNx, GradvNx;
	double GraduNy, GradvNy;
	double GraduNz, GradvNz;
	// Color gradient
	double InitR, InitG, InitB;
	double GraduR, GradvR;
	double GraduG, GradvG;
	double GraduB, GradvB;
	// Initial u,v
	double InitU, InitV;

	// -----------------------------------------------------------------------------------------------
	void init( CMesh::CFace *pF, vector<CVector>& vVertices, CVector &n1, CVector &n2, CVector &n3 )
	{
		double	u1 = pF->Corner[0].Uvws[1].U,
				v1 = pF->Corner[0].Uvws[1].V,
				u2 = pF->Corner[1].Uvws[1].U,
				v2 = pF->Corner[1].Uvws[1].V,
				u3 = pF->Corner[2].Uvws[1].U,
				v3 = pF->Corner[2].Uvws[1].V;
		CVector p1 = vVertices[pF->Corner[0].Vertex],
				p2 = vVertices[pF->Corner[1].Vertex],
				p3 = vVertices[pF->Corner[2].Vertex];

		CRGBA	c1 = pF->Corner[0].Color,
				c2 = pF->Corner[1].Color,
				c3 = pF->Corner[2].Color;

		double GradDen = ( (u3-u1)*(v2-v1) - (u2-u1)*(v3-v1) );

		if ( fabs (GradDen) > 0.000001 )
			GradDen = 1.0 / GradDen;

		this->InitU = u1;
		this->InitV = v1;

		this->InitPx = p1.x;
		this->InitPy = p1.y;
		this->InitPz = p1.z;

		this->InitNx = n1.x;
		this->InitNy = n1.y;
		this->InitNz = n1.z;

		this->InitR = c1.R;
		this->InitG = c1.G;
		this->InitB = c1.B;

		// Gradients for the vertex

		this->GraduPx = ( (p3.x-p1.x)*(v2-v1)-(p2.x-p1.x)*(v3-v1) ) * GradDen;
		this->GradvPx = ( (p2.x-p1.x)*(u3-u1)-(p3.x-p1.x)*(u2-u1) ) * GradDen;

		this->GraduPy = ( (p3.y-p1.y)*(v2-v1)-(p2.y-p1.y)*(v3-v1) ) * GradDen;
		this->GradvPy = ( (p2.y-p1.y)*(u3-u1)-(p3.y-p1.y)*(u2-u1) ) * GradDen;

		this->GraduPz = ( (p3.z-p1.z)*(v2-v1)-(p2.z-p1.z)*(v3-v1) ) * GradDen;
		this->GradvPz = ( (p2.z-p1.z)*(u3-u1)-(p3.z-p1.z)*(u2-u1) ) * GradDen;

		// The same for the normal
						
		this->GraduNx = ( (n3.x-n1.x)*(v2-v1)-(n2.x-n1.x)*(v3-v1) ) * GradDen;
		this->GradvNx = ( (n2.x-n1.x)*(u3-u1)-(n3.x-n1.x)*(u2-u1) ) * GradDen;

		this->GraduNy = ( (n3.y-n1.y)*(v2-v1)-(n2.y-n1.y)*(v3-v1) ) * GradDen;
		this->GradvNy = ( (n2.y-n1.y)*(u3-u1)-(n3.y-n1.y)*(u2-u1) ) * GradDen;

		this->GraduNz = ( (n3.z-n1.z)*(v2-v1)-(n2.z-n1.z)*(v3-v1) ) * GradDen;
		this->GradvNz = ( (n2.z-n1.z)*(u3-u1)-(n3.z-n1.z)*(u2-u1) ) * GradDen;

		// The same for the color

		this->GraduR = ( (c3.R-c1.R)*(v2-v1)-(c2.R-c1.R)*(v3-v1) ) * GradDen;
		this->GradvR = ( (c2.R-c1.R)*(u3-u1)-(c3.R-c1.R)*(u2-u1) ) * GradDen;

		this->GraduG = ( (c3.G-c1.G)*(v2-v1)-(c2.G-c1.G)*(v3-v1) ) * GradDen;
		this->GradvG = ( (c2.G-c1.G)*(u3-u1)-(c3.G-c1.G)*(u2-u1) ) * GradDen;

		this->GraduB = ( (c3.B-c1.B)*(v2-v1)-(c2.B-c1.B)*(v3-v1) ) * GradDen;
		this->GradvB = ( (c2.B-c1.B)*(u3-u1)-(c3.B-c1.B)*(u2-u1) ) * GradDen;
		
	}

	// -----------------------------------------------------------------------------------------------
	CVector getInterpolatedVertex( double u, double v )
	{
		CVector vRet;
		vRet.x = (float)(this->GraduPx*(u-this->InitU) + this->GradvPx*(v-this->InitV) + this->InitPx);
		vRet.y = (float)(this->GraduPy*(u-this->InitU) + this->GradvPy*(v-this->InitV) + this->InitPy);
		vRet.z = (float)(this->GraduPz*(u-this->InitU) + this->GradvPz*(v-this->InitV) + this->InitPz);
		return vRet;
	}

	// -----------------------------------------------------------------------------------------------
	CVector getInterpolatedNormal( double u, double v )
	{
		CVector vRet;
		vRet.x = (float)(this->GraduNx*(u-this->InitU) + this->GradvNx*(v-this->InitV) + this->InitNx);
		vRet.y = (float)(this->GraduNy*(u-this->InitU) + this->GradvNy*(v-this->InitV) + this->InitNy);
		vRet.z = (float)(this->GraduNz*(u-this->InitU) + this->GradvNz*(v-this->InitV) + this->InitNz);
		vRet.normalize();
		return vRet;
	}

	// -----------------------------------------------------------------------------------------------
	CRGBAF getInterpolatedColor( double u, double v )
	{
		CRGBAF vRet;
		vRet.R = (float)(this->GraduR*(u-this->InitU) + this->GradvR*(v-this->InitV) + this->InitR);
		vRet.G = (float)(this->GraduG*(u-this->InitU) + this->GradvG*(v-this->InitV) + this->InitG);
		vRet.B = (float)(this->GraduB*(u-this->InitU) + this->GradvB*(v-this->InitV) + this->InitB);

		if (vRet.R < 0.0f)
			vRet.R = 0.0f;
		if (vRet.G < 0.0f)
			vRet.G = 0.0f;
		if (vRet.B < 0.0f)
			vRet.B = 0.0f;

		if (vRet.R > 255.0f)
			vRet.R = 255.0f;
		if (vRet.G > 255.0f)
			vRet.G = 255.0f;
		if (vRet.B > 255.0f)
			vRet.B = 255.0f;

		return vRet;
	}

	// -----------------------------------------------------------------------------------------------
	// Uin and Vin are out of the face pF so calculate a U,V in face
	CVector getInterpolatedVertexInFace( double Uin, double Vin, CMesh::CFace *pF )
	{
		double Uout, Vout;
		double Utmp, Vtmp;
		double u1 = pF->Corner[0].Uvws[1].U, v1 = pF->Corner[0].Uvws[1].V;
		double u2 = pF->Corner[1].Uvws[1].U, v2 = pF->Corner[1].Uvws[1].V;
		double u3 = pF->Corner[2].Uvws[1].U, v3 = pF->Corner[2].Uvws[1].V;
		double rDist = 10000000.0f, rDistTmp, factor;
		// Get the nearest point from (Uin,Vin) to the face pF
		rDistTmp = sqrt( (Uin-u1)*(Uin-u1) + (Vin-v1)*(Vin-v1) );
		if( rDistTmp < rDist )
		{
			rDist = rDistTmp;
			Uout = u1;	Vout = v1;
		}
		rDistTmp = sqrt( (Uin-u2)*(Uin-u2) + (Vin-v2)*(Vin-v2) );
		if( rDistTmp < rDist )
		{
			rDist = rDistTmp;
			Uout = u2;	Vout = v2;
		}
		rDistTmp = sqrt( (Uin-u3)*(Uin-u3) + (Vin-v3)*(Vin-v3) );
		if( rDistTmp < rDist )
		{
			rDist = rDistTmp;
			Uout = u3;	Vout = v3;
		}

		factor = ( (Uin-u1)*(u2-u1) + (Vin-v1)*(v2-v1) ) / ( (u2-u1)*(u2-u1) + (v2-v1)*(v2-v1) );
		if( ( factor >= 0.0 ) && ( factor <= 1.0 ) )
		{
			Utmp = u1+(u2-u1)*factor; Vtmp = v1+(v2-v1)*factor;
			rDistTmp = sqrt( (Uin-Utmp)*(Uin-Utmp) + (Vin-Vtmp)*(Vin-Vtmp) );
			if( rDistTmp < rDist )
			{ rDist = rDistTmp; Uout = Utmp; Vout = Vtmp; }
		}

		factor = ( (Uin-u2)*(u3-u2) + (Vin-v2)*(v3-v2) ) / ( (u3-u2)*(u3-u2) + (v3-v2)*(v3-v2) );
		if( ( factor >= 0.0 ) && ( factor <= 1.0 ) )
		{
			Utmp = u2+(u3-u2)*factor; Vtmp = v2+(v3-v2)*factor;
			rDistTmp = sqrt( (Uin-Utmp)*(Uin-Utmp) + (Vin-Vtmp)*(Vin-Vtmp) );
			if( rDistTmp < rDist )
			{ rDist = rDistTmp; Uout = Utmp; Vout = Vtmp; }
		}

		factor = ( (Uin-u3)*(u1-u3) + (Vin-v3)*(v1-v3) ) / ( (u1-u3)*(u1-u3) + (v1-v3)*(v1-v3) );
		if( ( factor >= 0.0 ) && ( factor <= 1.0 ) )
		{
			Utmp = u3+(u1-u3)*factor; Vtmp = v3+(v1-v3)*factor;
			rDistTmp = sqrt( (Uin-Utmp)*(Uin-Utmp) + (Vin-Vtmp)*(Vin-Vtmp) );
			if( rDistTmp < rDist )
			{ rDist = rDistTmp; Uout = Utmp; Vout = Vtmp; }
		}

		// Calculate the 3d point
		return getInterpolatedVertex( Uout, Vout );
	}

};

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

CExportNelOptions gOptions;

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
/*
void SortFaceByTextureName(vector<CMesh::CFace*> &AllFaces, CMesh::CMeshBuild *pMB, CMeshBase::CMeshBaseBuild *pMBB)
{
	int i, j;
	int nNbFace = AllFaces.size();

	for( i = 0; i < nNbFace-1; ++i )
	for( j = i+1; j < nNbFace; ++j )
	{
		ITexture *pT = pMBB->Materials[AllFaces[i]->MaterialId].getTexture(0);
		CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
		string namei = "Default";
		if( pTF != NULL )
			namei = pTF->getFileName();
		pT = pMBB->Materials[AllFaces[j]->MaterialId].getTexture(0);
		pTF = dynamic_cast<CTextureFile *>(pT);
		string namej = "Default";
		if( pTF != NULL )
			namej = pTF->getFileName();
		if( namei < namej )
		{
			CMesh::CFace *pFaceTemp = AllFaces[i];
			AllFaces[i] = AllFaces[j];
			AllFaces[j] = pFaceTemp;
		}
	}
}

// -----------------------------------------------------------------------------------------------
// TextureNames is an array which indicates the number of faces that follows which have the same texture name
void ComputeAreaOfTextureName(vector<sint32> &TextureNames, vector<CMesh::CFace*> &AllFaces, CMesh::CMeshBuild *pMB, 
							  CMeshBase::CMeshBaseBuild *pMBB)
{
	int i, nNbFace = AllFaces.size();
	TextureNames.resize(nNbFace);
	ITexture *pT = pMBB->Materials[AllFaces[0]->MaterialId].getTexture(0);
	CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
	string CurrentName = "Default";
	sint32 lastface = 0, nNbTexName = 0;
	if( pTF != NULL )
		CurrentName = pTF->getFileName();
	for( i = 0; i < nNbFace; ++i )
	{
		ITexture *pT = pMBB->Materials[AllFaces[i]->MaterialId].getTexture(0);
		CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
		string namei = "Default";
		if( pTF != NULL )
			namei = pTF->getFileName();
		if( ( namei != CurrentName ) || ( i == (nNbFace-1) ) )
		{
			CurrentName = namei;
			TextureNames[nNbTexName] = i-lastface;
			nNbTexName++;
			lastface = i;
		}
	}
	TextureNames[nNbTexName-1] += 1;
	TextureNames.resize( nNbTexName );
}
*/
// -----------------------------------------------------------------------------------------------
void ClearFaceWithNoLM( CMesh::CMeshBuild *pMB, CMeshBase::CMeshBaseBuild *pMBB, vector<CMesh::CFace*> &ZeFaces )
{
	sint32 i;
	vector<CMesh::CFace*>::iterator ItParseI = ZeFaces.begin();
	sint32 nNbFace = ZeFaces.size();
	
	for( i = 0; i < nNbFace; ++i )
	{
		CMesh::CFace *pF = *ItParseI;
		if( pMBB->Materials[pF->MaterialId].getShader() != CMaterial::LightMap )
		{
			ItParseI = ZeFaces.erase( ItParseI );
			nNbFace--;
			i--;
		}
		else
		{
			++ItParseI;
		}
	}
}


// -----------------------------------------------------------------------------------------------
void SortFaceByMaterialId(  vector<sint32> &FaceGroup, vector<CMesh::CFace*>::iterator ItFaces, sint32 nNbFace )
{
	int i, j;
	sint32 nMatID;

	// Bubble sort face

	vector<CMesh::CFace*>::iterator ItParseI = ItFaces;
	for( i = 0; i < nNbFace-1; ++i )
	{
		vector<CMesh::CFace*>::iterator ItParseJ = ItParseI;
		++ItParseJ;
		for( j = i+1; j < nNbFace; ++j )
		{
			if( (*ItParseI)->MaterialId < (*ItParseJ)->MaterialId )
			{
				CMesh::CFace *pFaceTemp = *ItParseI;
				*ItParseI = *ItParseJ;
				*ItParseJ = pFaceTemp;
			}
			++ItParseJ;
		}
		++ItParseI;
	}

	// Indicates the groups
	
	FaceGroup.resize( nNbFace );
	ItParseI = ItFaces;
	j = 0; nMatID = (*ItParseI)->MaterialId; ++ItParseI;
	FaceGroup[j] = 1;
	for( i = 1; i < nNbFace; ++i )
	{
		if( (*ItParseI)->MaterialId != nMatID )
		{
			nMatID = (*ItParseI)->MaterialId; ++j;
			FaceGroup[j] = 1;
		}
		else
		{
			FaceGroup[j] ++;
		}
		++ItParseI;
	}
	FaceGroup.resize( j+1 );
}

// -----------------------------------------------------------------------------------------------
// Test if the 2 faces are continuous (same vertex, same normal (if wanted), same uv (if wanted))
bool FaceContinuous( CMesh::CFace *pF1, CMesh::CFace *pF2, bool bTestUV = true, bool bTestNormal = true )
{
	sint8 i, j, inext, jnext;
	sint8 F1c[2] = { -1, -1 };
	sint8 F2c[2] = { -1, -1 };

	// Is there a vertices continuity
	for( j = 0; j < 3; ++j )
	for( i = 0; i < 3; ++i )
	{
		inext = (i == 2) ? 0 : (i+1);
		jnext = (j == 2) ? 0 : (j+1);
		if( (pF1->Corner[j].Vertex == pF2->Corner[i].Vertex) && 
			(pF1->Corner[jnext].Vertex == pF2->Corner[inext].Vertex) )
		{
			F1c[0] = j; F1c[1] = jnext;
			F2c[0] = i; F2c[1] = inext;
			break;
		}
		if( (pF1->Corner[j].Vertex == pF2->Corner[inext].Vertex) && 
			(pF1->Corner[jnext].Vertex == pF2->Corner[i].Vertex) )
		{
			F1c[0] = jnext;	F1c[1] = j;
			F2c[0] = i;		F2c[1] = inext;
			break;
		}
	}
	// No -> out
	if( F1c[0] == -1 ) 
		return false;
	// Here we get the vertex continuity between F1c[0] and F2c[0], and, F1c[1] and F2c[1]
	// Is there a normal continuity
	if( bTestNormal )
	for( i = 0; i < 2; ++i )
	{
		CVector n1 = pF1->Corner[F1c[i]].Normal;
		CVector n2 = pF2->Corner[F2c[i]].Normal;
		// is n1 equal to n2 ?
		double epsilon = 1.0 - (n1*n2); // theorically n1*n2 equal to 1.0 but epsilon error
		if( epsilon > 0.001 )
			return false;
	}
	// Is there a mapping continuity
	if( bTestUV )
	for( i = 0; i < 2; ++i )
	{
		if((fabs( pF1->Corner[F1c[i]].Uvws[1].U - pF2->Corner[F2c[i]].Uvws[1].U) > 0.001) ||
		   (fabs( pF1->Corner[F1c[i]].Uvws[1].V - pF2->Corner[F2c[i]].Uvws[1].V) > 0.001) )
		   return false;
	}
	return true;
}

struct FaceNext
{
	CMesh::CFace *face;
	vector<sint32> cont;
	uint8 nNbCont;
	bool added;
};

// -----------------------------------------------------------------------------------------------
void SortFaceBySMoothGroup( vector<sint32> &FaceGroup, vector<CMesh::CFace*>::iterator ItFaces, uint32 nNbFace )
{
	uint32 i, j, k, nCurSG_NbFace, nCurSG_NbFaceLeft, nGroupNb;
	vector<FaceNext> FacesNext;
	vector<FaceNext> FacesFinal;

	if (gOptions.FeedBack != NULL)
	{
		string sTmp = "Grouping (1/2)";
		gOptions.FeedBack->setLine (4, sTmp);
		gOptions.FeedBack->update ();
	}


	vector<CMesh::CFace*>::iterator ItParseI = ItFaces;
	FacesNext.resize (nNbFace);
	FacesFinal.resize (nNbFace);
	for (i = 0; i < nNbFace; ++i, ++ItParseI)
	{
		FacesNext[i].face = *ItParseI;
		FacesNext[i].cont.reserve(3);
		FacesNext[i].added = false;
	}

	vector<CMesh::CFace*>::iterator ItParseJ = ItFaces;

	for (j = 0; j < nNbFace; ++j, ++ItParseJ)
	{
		ItParseI = ItParseJ;
		for (i = j; i < nNbFace; ++i, ++ItParseI)
		if (i != j)
		{
			if (FaceContinuous (*ItParseI, *ItParseJ, false))
			{
				FacesNext[j].cont.resize (FacesNext[j].cont.size()+1);
				FacesNext[j].cont[FacesNext[j].cont.size()-1] = i;

				FacesNext[i].cont.resize (FacesNext[i].cont.size()+1);
				FacesNext[i].cont[FacesNext[i].cont.size()-1] = j;
			}
		}
	}

	if (gOptions.FeedBack != NULL)
	{
		string sTmp = "Grouping (2/2)";
		gOptions.FeedBack->setLine (4, sTmp);
		gOptions.FeedBack->update ();
	}

	FaceGroup.resize (nNbFace);
	for( j = 0; j < nNbFace; ++j )
		FaceGroup[j] = 1;
	nGroupNb = 0;

	i = 0;
	for (j = 0; j < nNbFace; ++j)
	{
		if (FacesNext[j].added == false)
		{
			FacesFinal[i] = FacesNext[j];
			FacesNext[j].added = true;
			nCurSG_NbFace = 1;
			for (nCurSG_NbFaceLeft = 1; nCurSG_NbFaceLeft > 0; --nCurSG_NbFaceLeft)
			{
				for (k = 0; k < FacesFinal[i].cont.size(); ++k)
					if (FacesNext[FacesFinal[i].cont[k]].added == false)
					{
						FacesNext[FacesFinal[i].cont[k]].added = true;
						FacesFinal[i+nCurSG_NbFaceLeft] = FacesNext[FacesFinal[i].cont[k]];
						++nCurSG_NbFace;
						++nCurSG_NbFaceLeft;
					}
				++i;
			}
			FaceGroup[nGroupNb] = nCurSG_NbFace;
			++nGroupNb;
		}
	}
	FaceGroup.resize(nGroupNb);

	ItParseI = ItFaces;
	for (i = 0; i < nNbFace; ++i, ++ItParseI)
		*ItParseI = FacesFinal[i].face;
}


// -----------------------------------------------------------------------------------------------
void SortFaceByTextureSurface( int offset, int nNbFace, vector<CMesh::CFace*> &AllFaces )
{
	int i, j;

	for( i = 0; i < nNbFace-1; ++i )
	for( j = i+1; j < nNbFace; ++j )
	if( AllFaces[i]->MaterialId == AllFaces[j]->MaterialId )
	{
		// Texture surface of the i face = .5*|(u1-u0)*(v2-v0)-(v1-v0)*(u2-u0)|
		// in fact this is lightmap mapping surface
		double surfacei = 0.5*fabs(
		(AllFaces[i]->Corner[1].Uvws[1].U - AllFaces[i]->Corner[0].Uvws[1].U)*
		(AllFaces[i]->Corner[2].Uvws[1].V - AllFaces[i]->Corner[0].Uvws[1].V)-
		(AllFaces[i]->Corner[1].Uvws[1].V - AllFaces[i]->Corner[0].Uvws[1].V)*
		(AllFaces[i]->Corner[2].Uvws[1].U - AllFaces[i]->Corner[0].Uvws[1].U) );
		double surfacej = 0.5*fabs(
		(AllFaces[j]->Corner[1].Uvws[1].U - AllFaces[j]->Corner[0].Uvws[1].U)*
		(AllFaces[j]->Corner[2].Uvws[1].V - AllFaces[j]->Corner[0].Uvws[1].V)-
		(AllFaces[j]->Corner[1].Uvws[1].V - AllFaces[j]->Corner[0].Uvws[1].V)*
		(AllFaces[j]->Corner[2].Uvws[1].U - AllFaces[j]->Corner[0].Uvws[1].U) );
		if( surfacei < surfacej )
		{
			CMesh::CFace *pFaceTemp = AllFaces[i];
			AllFaces[i] = AllFaces[j];
			AllFaces[j] = pFaceTemp;
		}
	}
}

// -----------------------------------------------------------------------------------------------
bool isInTriangleOrEdge(double x, double y, double xt1, double yt1, double xt2, double yt2, double xt3, double yt3)
{
	// Test vector T1X and T1T2
	double sign1 = ((xt2-xt1)*(y-yt1) - (yt2-yt1)*(x-xt1));
	// Test vector T2X and T2T3
	double sign2 = ((xt3-xt2)*(y-yt2) - (yt3-yt2)*(x-xt2));
	// Test vector T3X and T3T1
	double sign3 = ((xt1-xt3)*(y-yt3) - (yt1-yt3)*(x-xt3));
	if( (sign1 <= 0.0)&&(sign2 <= 0.0)&&(sign3 <= 0.0) )
		return true;
	if( (sign1 >= 0.0)&&(sign2 >= 0.0)&&(sign3 >= 0.0) )
		return true;
	return false;
}

// -----------------------------------------------------------------------------------------------
bool isInTriangle(double x, double y, double xt1, double yt1, double xt2, double yt2, double xt3, double yt3)
{
	// Test vector T1X and T1T2
	double sign1 = ((xt2-xt1)*(y-yt1) - (yt2-yt1)*(x-xt1));
	// Test vector T2X and T2T3
	double sign2 = ((xt3-xt2)*(y-yt2) - (yt3-yt2)*(x-xt2));
	// Test vector T3X and T3T1
	double sign3 = ((xt1-xt3)*(y-yt3) - (yt1-yt3)*(x-xt3));
	if( (sign1 < 0.0)&&(sign2 < 0.0)&&(sign3 < 0.0) )
		return true;
	if( (sign1 > 0.0)&&(sign2 > 0.0)&&(sign3 > 0.0) )
		return true;
	return false;
}

// Segment line intersection P1P2 and P3P4
// -----------------------------------------------------------------------------------------------
bool segmentIntersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	double denominator = (y4-y3)*(x2-x1) - (x4-x3)*(y2-y1);
	if( denominator == 0.0 )
		return false; // The segment are colinear
	double k = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3) ) / denominator;
	if( (k<=0.0) || (k>=1.0) ) return false;
	k = ( (x2-x1)*(y1-y3) - (y2-y1)*(x1-x3) ) / denominator;
	if( (k<=0.0) || (k>=1.0) ) return false;
	return true;
}

// -----------------------------------------------------------------------------------------------
bool vertexInSquare(double xs, double ys, double ws, double hs, double xv, double yv)
{
	if ((fabs(xv - xs) < ws) && (fabs(yv - ys) < hs))
		return true;
	else
		return false;
}

// -----------------------------------------------------------------------------------------------
// Automatic mapping of a face (dont remove)
void MapFace( CMesh::CFace *pFace, vector<CVector> &Vertices, float rRatio )
{
	CVector V01 = Vertices[pFace->Corner[1].Vertex] - Vertices[pFace->Corner[0].Vertex];
	CVector V02 = Vertices[pFace->Corner[2].Vertex] - Vertices[pFace->Corner[0].Vertex];
	CVector n = V01 ^ V02;
	n.normalize();
	
	// Quantize the normal

	// Table of unitary vector with relevant direction to map The I vector represent the plane normal
	// and the J,K vector the U,V vector
	CMatrix QuantizationTbl[3]; 
	QuantizationTbl[0].identity();
	QuantizationTbl[1].identity(); QuantizationTbl[1].rotateZ((float)(Pi/2.0));
	QuantizationTbl[2].identity(); QuantizationTbl[2].rotateY((float)(Pi/2.0));

	float fMax = 0.0f;
	int pos = 0;
	for( int i = 0; i < 3; ++i )
	{
		if( fMax < fabsf(QuantizationTbl[i].getI()*n) )
		{
			fMax = fabsf(QuantizationTbl[i].getI()*n);
			pos = i;
		}
	}

	// Map with the i_th vector from the quantization table
	// Projection of the 3 vertices of the triangle on the plane 
	// defined by the quantized vector (as the plane normal) and the origin (as a point in the plane)

	// This is equivalent to a base changement with annulation of the I vector
	CMatrix invMat = QuantizationTbl[pos].inverted();
	CVector newPtinUVBasis = invMat.mulPoint(Vertices[pFace->Corner[0].Vertex]);
	pFace->Corner[0].Uvws[1].U = newPtinUVBasis.y / rRatio;
	pFace->Corner[0].Uvws[1].V = newPtinUVBasis.z / rRatio;
	newPtinUVBasis = invMat.mulPoint(Vertices[pFace->Corner[1].Vertex]);
	pFace->Corner[1].Uvws[1].U = newPtinUVBasis.y / rRatio;
	pFace->Corner[1].Uvws[1].V = newPtinUVBasis.z / rRatio;
	newPtinUVBasis = invMat.mulPoint(Vertices[pFace->Corner[2].Vertex]);
	pFace->Corner[2].Uvws[1].U = newPtinUVBasis.y / rRatio;
	pFace->Corner[2].Uvws[1].V = newPtinUVBasis.z / rRatio;	
}

// -----------------------------------------------------------------------------------------------
CMatrix getObjectToWorldMatrix( CMesh::CMeshBuild *pMB, CMeshBase::CMeshBaseBuild *pMBB )
{
	CMatrix m1;

	m1.identity();
	// T*P
	m1.translate (pMBB->DefaultPos + pMBB->DefaultPivot);
	// R*S*P-1.
	m1.rotate (pMBB->DefaultRotQuat);
	m1.scale (pMBB->DefaultScale);
	m1.translate (-pMBB->DefaultPivot);

	return m1;
}

// -----------------------------------------------------------------------------------------------
float getUVDist( CUV& UV1, CUV& UV2 )
{
	return sqrtf( (UV2.U - UV1.U)*(UV2.U - UV1.U) + (UV2.V - UV1.V)*(UV2.V - UV1.V) );
}

// -----------------------------------------------------------------------------------------------
void getLightNodeList (std::vector<INode*>& vectLightNode, TimeValue tvTime, Interface& ip, bool visibleOnly, INode*node=NULL )
{
	if( node == NULL )
		node = ip.GetRootNode();

	SLightBuild nelLight;

	if (nelLight.canConvertFromMaxLight(node, tvTime))
	{
		// visible, or add hidden too?
		if(!visibleOnly || !node->IsHidden())
		{
			// Yoyo: if this light is checked to lightMap export
			int		nLMExport= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT, BST_CHECKED);
			if(nLMExport == BST_CHECKED)
				vectLightNode.push_back (node);
		}
	}

	// Recurse sub node
	for (int i=0; i<node->NumberOfChildren(); i++)
		getLightNodeList (vectLightNode, tvTime, ip, visibleOnly, node->GetChildNode(i));
}

// -----------------------------------------------------------------------------------------------
void getLightmapLightBuilds( vector<SLightBuild> &lights, TimeValue tvTime, Interface& ip, bool visibleOnly )
{
	vector<INode*> nodeLights;

	getLightNodeList (nodeLights, tvTime, ip, visibleOnly);

	lights.resize(nodeLights.size());
	for(uint32 i = 0; i < nodeLights.size(); ++i)
	{
		lights[i].convertFromMaxLight (nodeLights[i], tvTime);
	}
}

// -----------------------------------------------------------------------------------------------
double calculateTriangleSurface( CVector &p1, CVector &p2, CVector &p3 )
{
	CVector n = ((p2-p1)^(p3-p1));
	return 0.5 * n.norm(); // Half of the norm
}

// -----------------------------------------------------------------------------------------------
void MoveFaceUV1( vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace, double rOffsU, double rOffsV )
{
	sint32 i,j ;
	for( i = 0; i < nNbFace; ++i )
	{
		CMesh::CFace *pF = (*ItFace);
		for( j = 0; j < 3; ++j )
		{
			pF->Corner[j].Uvws[1].U += (float)rOffsU;
			pF->Corner[j].Uvws[1].V += (float)rOffsV;
		}
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
void MultiplyFaceUV1( vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace, double rFactor )
{
	sint32 i,j ;
	for( i = 0; i < nNbFace; ++i )
	{
		CMesh::CFace *pF = (*ItFace);
		for( j = 0; j < 3; ++j )
		{
			pF->Corner[j].Uvws[1].U *= (float)rFactor;
			pF->Corner[j].Uvws[1].V *= (float)rFactor;
		}
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
bool PutFaceUV1InLumelCoord( double rRatioLightMap, vector<CVector> &Vertices, 
						    vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace )
{
	sint32 i, j;
	double SpaceSurf = 0.0, TextureSurf = 0.0;
	vector<CMesh::CFace*>::iterator ItParseI = ItFace;
	for( i = 0; i < nNbFace; ++i )
	{
		CVector p1, p2, p3;
		CMesh::CFace* pF = (*ItParseI);
		p1 = Vertices[pF->Corner[0].Vertex];
		p2 = Vertices[pF->Corner[1].Vertex];
		p3 = Vertices[pF->Corner[2].Vertex];
		SpaceSurf += calculateTriangleSurface( p1, p2, p3 );
		p1.x = pF->Corner[0].Uvws[1].U; p1.y = pF->Corner[0].Uvws[1].V; p1.z = 0.0f;
		p2.x = pF->Corner[1].Uvws[1].U; p2.y = pF->Corner[1].Uvws[1].V; p2.z = 0.0f;
		p3.x = pF->Corner[2].Uvws[1].U; p3.y = pF->Corner[2].Uvws[1].V; p3.z = 0.0f;
		TextureSurf += calculateTriangleSurface( p1, p2, p3 );
		// Next face
		++ItParseI;
	}
	if( TextureSurf < 0.00001 )
		return false;
	double LMTextRatio = sqrt(SpaceSurf / TextureSurf) * (1.0/rRatioLightMap);

	ItParseI = ItFace;
	for( i = 0; i < nNbFace; ++i )
	{
		CMesh::CFace* pF = (*ItParseI);
		for( j = 0; j < 3; ++j ) // Express the UVs in lumel for each corner
		{
			pF->Corner[j].Uvws[1].U *= (float)LMTextRatio;
			pF->Corner[j].Uvws[1].V *= (float)LMTextRatio;
		}
		++ItParseI;
	}
	return true;
}

// -----------------------------------------------------------------------------------------------
void PutFaceUV1InTextureCoord( sint32 TextureSizeX, sint32 TextureSizeY, 
								vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace )
{
	sint32 i,j;
	for( i = 0; i < nNbFace; ++i )
	{
		for( j = 0; j < 3; ++j )
		{
			CMesh::CFace *pF = *ItFace;
			pF->Corner[j].Uvws[1].U /= (float)TextureSizeX;
			pF->Corner[j].Uvws[1].V /= (float)TextureSizeY;
		}
		// Next face
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
bool IsFaceCoverFace( CMesh::CFace *pF1, CMesh::CFace *pF2 )
{
	sint32 i, j;

	for( j = 0; j < 3; ++j )
	for( i = 0; i < 3; ++i )
		if( segmentIntersection(pF1->Corner[i].Uvws[1].U, pF1->Corner[i].Uvws[1].V, 
								pF1->Corner[(i+1)%3].Uvws[1].U, pF1->Corner[(i+1)%3].Uvws[1].V, 
								pF2->Corner[j].Uvws[1].U, pF2->Corner[j].Uvws[1].V, 
								pF2->Corner[(j+1)%3].Uvws[1].U, pF2->Corner[(j+1)%3].Uvws[1].V ) )
			return true;

	for( i = 0; i < 3; ++i )
		if( isInTriangle(	pF1->Corner[i].Uvws[1].U, pF1->Corner[i].Uvws[1].V, 
							pF2->Corner[0].Uvws[1].U, pF2->Corner[0].Uvws[1].V, 
							pF2->Corner[1].Uvws[1].U, pF2->Corner[1].Uvws[1].V, 
							pF2->Corner[2].Uvws[1].U, pF2->Corner[2].Uvws[1].V ) )
			return true;

	for( i = 0; i < 3; ++i )
		if( isInTriangle(	pF2->Corner[i].Uvws[1].U, pF2->Corner[i].Uvws[1].V, 
							pF1->Corner[0].Uvws[1].U, pF1->Corner[0].Uvws[1].V, 
							pF1->Corner[1].Uvws[1].U, pF1->Corner[1].Uvws[1].V, 
							pF1->Corner[2].Uvws[1].U, pF1->Corner[2].Uvws[1].V ) )
			return true;

	return false;
}

// -----------------------------------------------------------------------------------------------
void SortFaceByPlane( vector<sint32> &FaceGroup, vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace )
{
	sint32 j, k, nGroupNb = 0;
	FaceGroup.resize( nNbFace );
	for( j = 0; j < nNbFace; ++j )
		FaceGroup[j] = 1;

	vector<CMesh::CFace*>::iterator CurGrpBeg = ItFace;
	vector<CMesh::CFace*>::iterator CurGrpEnd = ItFace;
	sint32 nGroupOffset = 1;
	list<CMesh::CFace*> lifo;
	lifo.clear();
	for( nGroupOffset = 1; nGroupOffset <= nNbFace; )
	{
		lifo.push_front( *CurGrpBeg );
		// Do a complete plane : Graph traversal in width
		while( ! lifo.empty() )
		{
			CMesh::CFace *pFace = lifo.back();
			lifo.pop_back();
			
			vector<CMesh::CFace*>::iterator ItParseJ = CurGrpEnd;
			++ItParseJ;
			for( j = nGroupOffset; j < nNbFace; ++j )		
			{
				if( FaceContinuous( pFace, *ItParseJ ) )
				{
					// Is this face cover other face present in the current group ?
					vector<CMesh::CFace*>::iterator ItParseK = CurGrpBeg;
					bool bFaceCovering = false;
					for( k = 0; k < FaceGroup[nGroupNb]; ++k )
					{
						if( IsFaceCoverFace( *ItParseK, *ItParseJ ) )
						{
							bFaceCovering = true;
							break;
						}
						++ItParseK;
					}

					// The face do not cover other face -> add it to current group
					if( !bFaceCovering )
					{
						lifo.push_front( *ItParseJ );
						++CurGrpEnd;
						CMesh::CFace *pFaceTemp = *CurGrpEnd;
						*CurGrpEnd = *ItParseJ;
						*ItParseJ = pFaceTemp;
						nGroupOffset += 1;
						FaceGroup[nGroupNb] += 1;
					}

				}
				++ItParseJ;
			}
		}
		++CurGrpEnd;
		CurGrpBeg = CurGrpEnd;
		++nGroupNb;
		nGroupOffset += 1;
	}
	FaceGroup.resize( nGroupNb );
}

// -----------------------------------------------------------------------------------------------
void SortPlanesBySurface( vector<SLMPlane*> &planes )
{
	uint32 i, j;

	for( i = 0; i < planes.size()-1; ++i )
	for( j = i+1; j < planes.size(); ++j )
	{
		if( (planes[i]->w *planes[i]->h) < (planes[j]->w *planes[j]->h) )
		{
			SLMPlane *tmp = planes[i];
			planes[i] = planes[j];
			planes[j] = tmp;
		}
	}
}

// -----------------------------------------------------------------------------------------------
void ModifyLMPlaneWithOverSampling( SLMPlane *pPlane, double rOverSampling, bool bCreateMask )
{
	uint32 i, j;
	vector<CMesh::CFace*>::iterator ItFace = pPlane->faces.begin();
	uint32 nNbFace = pPlane->faces.size();

	pPlane->stretch( rOverSampling );

	MultiplyFaceUV1( ItFace, nNbFace, rOverSampling );

	if( bCreateMask )
	{
		// Reset the mask
		for( j = 0; j < pPlane->w*pPlane->h; ++j )
			pPlane->msk[j] = 0;

		ItFace = pPlane->faces.begin();
		// Recreate the form
		for( i = 0; i < nNbFace; ++i )
		{
			CMesh::CFace *pF = *ItFace;

			TTicks ttTemp3 = CTime::getPerformanceTime();
			pPlane->createFromFace (pF);

			++ItFace;
		}
	}
}

// -----------------------------------------------------------------------------------------------
void PlaceLMPlaneInLMPLane( SLMPlane &Dst, SLMPlane &Src )
{
	TTicks ttTemp = CTime::getPerformanceTime();
	while( true )
	{
		if( ! Src.tryAllPosToPutIn( Dst ) )
		{
			if( ( Dst.w < MAXLIGHTMAPSIZE ) || ( Dst.h < MAXLIGHTMAPSIZE ) )
			{
				if( Dst.w < Dst.h )
					Dst.resize( Dst.w*2, Dst.h );
				else
					Dst.resize( Dst.w, Dst.h*2 );
			}
			else
			{
				// ERROR: we reached the maximum texture size
				break;
			}
		}
		else
		{
			// We found a position
			Src.putIn( Dst );
			break;
		}
	}
}

// -----------------------------------------------------------------------------------------------
CRGBAF LightAVertex( uint8 &rtVal, CVector &pRT, CVector &p, CVector &n, 
					vector<sint32> &vLights, vector<SLightBuild> &AllLights,
					CRTWorld &wrt, bool bDoubleSided, bool bRcvShadows )
{
	CRGBAF rgbafRet;
					
	rgbafRet.R = rgbafRet.G = rgbafRet.B = rgbafRet.A = 0.0;
	// Color calculation
	for( uint32 nLight = 0; nLight < vLights.size(); ++nLight )
	{
		SLightBuild &rLight = AllLights[vLights[nLight]];
		CRGBAF lightAmbiCol = CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
		CRGBAF lightDiffCol = CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
		CRGBAF lightSpecCol = CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
		CRGBAF RTFactor = CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
		float light_intensity = 0.0;

		switch( rLight.Type )
		{
			case SLightBuild::LightAmbient:
				lightAmbiCol.R = rLight.Ambient.R / 255.0f;
				lightAmbiCol.G = rLight.Ambient.G / 255.0f;
				lightAmbiCol.B = rLight.Ambient.B / 255.0f;
				lightAmbiCol.A = rLight.Ambient.A / 255.0f;
				light_intensity = 1.0;
			break;
			case SLightBuild::LightPoint:
			{
				CVector p_light = rLight.Position - p;
				float p_light_distance = p_light.norm();
				if( p_light_distance < rLight.rRadiusMin )
					light_intensity = 1.0f;
				else
				if( p_light_distance > rLight.rRadiusMax )
					light_intensity = 0.0f;
				else
					light_intensity = 1.0f - (p_light_distance-rLight.rRadiusMin)/(rLight.rRadiusMax-rLight.rRadiusMin);
				p_light.normalize();

				if( bDoubleSided && (n*p_light < 0.0f) )
				{
					p_light = -p_light;
				}
				light_intensity *= rLight.rMult;
				lightAmbiCol.R = light_intensity * rLight.Ambient.R / 255.0f;
				lightAmbiCol.G = light_intensity * rLight.Ambient.G / 255.0f;
				lightAmbiCol.B = light_intensity * rLight.Ambient.B / 255.0f;
				lightAmbiCol.A = light_intensity * rLight.Ambient.A / 255.0f;
				light_intensity *= max(0.0f, n*p_light);
				lightDiffCol.R = light_intensity * rLight.Diffuse.R / 255.0f;
				lightDiffCol.G = light_intensity * rLight.Diffuse.G / 255.0f;
				lightDiffCol.B = light_intensity * rLight.Diffuse.B / 255.0f;
				lightDiffCol.A = light_intensity * rLight.Diffuse.A / 255.0f;
			}
			break;
			case SLightBuild::LightDir:
			{	
				CVector p_light = - rLight.Direction;
				p_light.normalize();
				if( bDoubleSided && (n*p_light < 0.0f) )
				{
					p_light = -p_light;
				}
				light_intensity = rLight.rMult;
				lightAmbiCol.R = light_intensity * rLight.Ambient.R / 255.0f;
				lightAmbiCol.G = light_intensity * rLight.Ambient.G / 255.0f;
				lightAmbiCol.B = light_intensity * rLight.Ambient.B / 255.0f;
				lightAmbiCol.A = light_intensity * rLight.Ambient.A / 255.0f;
				light_intensity *= max(0.0f, n*p_light);
				lightDiffCol.R = light_intensity * rLight.Diffuse.R / 255.0f;
				lightDiffCol.G = light_intensity * rLight.Diffuse.G / 255.0f;
				lightDiffCol.B = light_intensity * rLight.Diffuse.B / 255.0f;
				lightDiffCol.A = light_intensity * rLight.Diffuse.A / 255.0f;

			}
			break;
			case SLightBuild::LightSpot:
			{
				CVector p_light = rLight.Position - p;
				float p_light_distance = p_light.norm();
				if( p_light_distance < rLight.rRadiusMin )
					light_intensity = 1.0f;
				else
				if( p_light_distance > rLight.rRadiusMax )
					light_intensity = 0.0f;
				else
					light_intensity = 1.0f - (p_light_distance-rLight.rRadiusMin)/(rLight.rRadiusMax-rLight.rRadiusMin);
				p_light.normalize();

				float ang = acosf( p_light * (-rLight.Direction) );
				if( ang > rLight.rFallof )
					light_intensity = 0.0f;
				else
				if( ang > rLight.rHotspot )
					light_intensity *= 1.0f - (ang-rLight.rHotspot)/(rLight.rFallof-rLight.rHotspot);
				light_intensity *= rLight.rMult;

				if( bDoubleSided && (n*p_light < 0.0f) )
				{
					p_light = -p_light;
				}
				lightAmbiCol.R = light_intensity * rLight.Ambient.R / 255.0f;
				lightAmbiCol.G = light_intensity * rLight.Ambient.G / 255.0f;
				lightAmbiCol.B = light_intensity * rLight.Ambient.B / 255.0f;
				lightAmbiCol.A = light_intensity * rLight.Ambient.A / 255.0f;
				light_intensity *= max(0.0f, n*p_light);
				lightDiffCol.R = light_intensity * rLight.Diffuse.R / 255.0f;
				lightDiffCol.G = light_intensity * rLight.Diffuse.G / 255.0f;
				lightDiffCol.B = light_intensity * rLight.Diffuse.B / 255.0f;
				lightDiffCol.A = light_intensity * rLight.Diffuse.A / 255.0f;

				// Apply projected image if some
				if ((rLight.ProjBitmap.getHeight() != 0) && (light_intensity > 0.0f))
				{
					// Make the plane where the texture is
					CPlane plane; // Projection plane
					CVector ori = rLight.Position + rLight.mProj.getK();
					CVector norm = rLight.mProj.getK();
					plane.make( norm, ori );
					CVector inter = plane.intersect( rLight.Position, p );
					// Intersection conversion in i,j coordinate system with ori as origin
					//float dotSize = gOptions.rLumelSize * (rLight.Position-inter).norm() / (rLight.Position-p).norm();
					float x = -(inter-ori)*rLight.mProj.getI();
					float y = (inter-ori)*rLight.mProj.getJ();
					// Normalization x [-tan(fallof),tan(fallof)] -> [0,1]
					x = ((x / tanf( rLight.rFallof ))+1.0f)/2.0f;
					y = ((y / tanf( rLight.rFallof ))+1.0f)/2.0f;
					
					CRGBAF col = rLight.ProjBitmap.getColor(x, y);
					lightDiffCol.R *= col.R;
					lightDiffCol.G *= col.G;
					lightDiffCol.B *= col.B;
					lightDiffCol.A *= col.A;
				}
			}
			break;
			default:
			break;
		}
		if( light_intensity > 0.0f )
		{
			if( bRcvShadows && rLight.bCastShadow && gOptions.bShadow )
				RTFactor = wrt.raytrace (pRT, vLights[nLight], rtVal, gOptions.nExportLighting==1);
			else
				RTFactor = CRGBAF(1.0f, 1.0f, 1.0f, 1.0f);
		}
		
		rgbafRet.R += lightAmbiCol.R + lightDiffCol.R * RTFactor.R;
		if( rgbafRet.R > 2.0f ) rgbafRet.R = 2.0;
		rgbafRet.G += lightAmbiCol.G + lightDiffCol.G * RTFactor.G;
		if( rgbafRet.G > 2.0f ) rgbafRet.G = 2.0;
		rgbafRet.B += lightAmbiCol.B + lightDiffCol.B * RTFactor.B;
		if( rgbafRet.B > 2.0f ) rgbafRet.B = 2.0;
		rgbafRet.A += lightAmbiCol.A + lightDiffCol.A * RTFactor.A;
		if( rgbafRet.A > 2.0f ) rgbafRet.A = 2.0;
	}
	return rgbafRet;
}

// -----------------------------------------------------------------------------------------------
bool segmentIntersectBSphere( CVector &p1, CVector &p2, CBSphere &bs )
{
	// Is one point is in the sphere ?
	CVector r = bs.Center - p1;
	float f;
	if( r.norm() <= bs.Radius )
		return true;
	r = bs.Center - p2;
	if( r.norm() <= bs.Radius )
		return true;
	// Is the orthogonal projection of the center on the segment is in the sphere ?
	r = p2 - p1;
	f = r.norm();
	f = ( r * (bs.Center - p1) ) / ( f * f );
	if( ( f >= 0.0 ) && ( f <= 1.0 ) )
	{
		r = bs.Center - (p1 + r*f);
		if( r.norm() <= bs.Radius )
			return true;
	}
	return false;
}

// -----------------------------------------------------------------------------------------------
void FirstLight( CMesh::CMeshBuild* pMB, CMeshBase::CMeshBaseBuild *pMBB, SLMPlane &Plane, vector<CVector> &vVertices, 
				CMatrix& ToWorldMat, vector<sint32> &vLights, vector<SLightBuild> &AllLights,
				uint32 nLayerNb, CRTWorld &wrt )
{
	// Fill interiors
	vector<CMesh::CFace*>::iterator ItFace = Plane.faces.begin();
	uint32 nNbFace = Plane.faces.size();
	uint32 i;
	sint32 j, k;
	double rMinU = 1000000.0, rMaxU = -1000000.0, rMinV = 1000000.0, rMaxV = -1000000.0;
	sint32 nPosMinU, nPosMaxU, nPosMinV, nPosMaxV;
	CMesh::CFace *pF;
	SGradient g;
	
	for( i = 0; i < Plane.w*Plane.h; ++i )
		if( Plane.msk[i] != 0 )
			Plane.msk[i] = 1;

	for( i = 0; i < nNbFace; ++i )
	{
		pF = *ItFace;

		bool doubleSided = pMBB->Materials[pF->MaterialId].getDoubleSided();
		CRGBA matDiff = pMBB->Materials[pF->MaterialId].getDiffuse();
		
		// Select bounding square of the triangle
		for( j = 0; j < 3; ++j )
		{
			if( rMinU > pF->Corner[j].Uvws[1].U ) rMinU = pF->Corner[j].Uvws[1].U;
			if( rMaxU < pF->Corner[j].Uvws[1].U ) rMaxU = pF->Corner[j].Uvws[1].U;
			if( rMinV > pF->Corner[j].Uvws[1].V ) rMinV = pF->Corner[j].Uvws[1].V;
			if( rMaxV < pF->Corner[j].Uvws[1].V ) rMaxV = pF->Corner[j].Uvws[1].V;
		}
		nPosMaxU = ((sint32)floor( rMaxU + 0.5 ));
		nPosMaxV = ((sint32)floor( rMaxV + 0.5 ));
		nPosMinU = ((sint32)floor( rMinU - 0.5 ));
		nPosMinV = ((sint32)floor( rMinV - 0.5 ));

		CVector n1 = ToWorldMat.mulVector( pF->Corner[0].Normal );
		CVector n2 = ToWorldMat.mulVector( pF->Corner[1].Normal );
		CVector n3 = ToWorldMat.mulVector( pF->Corner[2].Normal );
	
		g.init( pF, vVertices, n1, n2, n3 );

		// Process all the interior
		for( k = nPosMinV; k <= nPosMaxV; ++k )
		for( j = nPosMinU; j <= nPosMaxU; ++j )
		if( Plane.msk[j-Plane.x + (k-Plane.y)*Plane.w] == 1 )
		{
			if( isInTriangleOrEdge( j+0.5, k+0.5,
									pF->Corner[0].Uvws[1].U, pF->Corner[0].Uvws[1].V,
									pF->Corner[1].Uvws[1].U, pF->Corner[1].Uvws[1].V,
									pF->Corner[2].Uvws[1].U, pF->Corner[2].Uvws[1].V ) )
			{
				CVector p = g.getInterpolatedVertex( j+0.5, k+0.5);
				CVector n = g.getInterpolatedNormal( j+0.5, k+0.5);
				CRGBAF vl = g.getInterpolatedColor( j+0.5, k+0.5);
				uint8 rtVal = Plane.ray[j-Plane.x + (k-Plane.y)*Plane.w];
				CRGBAF col = LightAVertex( rtVal, p, p, n, vLights, AllLights, wrt, doubleSided, pMBB->bRcvShadows );
				Plane.ray[j-Plane.x + (k-Plane.y)*Plane.w] = rtVal;
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w+Plane.w*Plane.h*nLayerNb].R = col.R*(vl.R/255.0f)*(matDiff.R/255.0f);
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w+Plane.w*Plane.h*nLayerNb].G = col.G*(vl.G/255.0f)*(matDiff.G/255.0f);
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w+Plane.w*Plane.h*nLayerNb].B = col.B*(vl.B/255.0f)*(matDiff.B/255.0f);
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w+Plane.w*Plane.h*nLayerNb].A = 1.0f;
				// Darken the plane to indicate pixel is calculated
				Plane.msk[j-Plane.x + (k-Plane.y)*Plane.w] = 2;
			}
		}
		// Next Face
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
void SecondLight( CMesh::CMeshBuild *pMB, CMeshBase::CMeshBaseBuild *pMBB, 
				 vector<SLMPlane*>::iterator ItPlanes, uint32 nNbPlanes,
					vector<CVector> &vVertices, CMatrix& ToWorldMat, 
					vector<sint32> &vLights, vector<SLightBuild> &AllLights,
					uint32 nLayerNb, CRTWorld &wrt)
{
	// Fill interiors
	uint32 nPlanes1;

	vector<SLMPlane*>::iterator ItPlanes1 = ItPlanes;
	for( nPlanes1 = 0; nPlanes1 < nNbPlanes; ++nPlanes1 )
	{
		uint32 i;
		sint32 j, k;
		sint32 nPosMinU, nPosMaxU, nPosMinV, nPosMaxV;
		SGradient g;
		
		SLMPlane *pP1 = *ItPlanes1;
		vector<CMesh::CFace*>::iterator ItParseI = pP1->faces.begin();
		uint32 nNbFace1 = pP1->faces.size();
		for( i = 0; i < nNbFace1; ++i )
		{
			CMesh::CFace *pF1 = *ItParseI;
			double rMinU = 1000000.0, rMaxU = -1000000.0, rMinV = 1000000.0, rMaxV = -1000000.0;
			bool doubleSided = pMBB->Materials[pF1->MaterialId].getDoubleSided();
			CRGBA matDiff = pMBB->Materials[pF1->MaterialId].getDiffuse();

			// Select bounding square of the triangle
			for( j = 0; j < 3; ++j )
			{
				if( rMinU > pF1->Corner[j].Uvws[1].U ) rMinU = pF1->Corner[j].Uvws[1].U;
				if( rMaxU < pF1->Corner[j].Uvws[1].U ) rMaxU = pF1->Corner[j].Uvws[1].U;
				if( rMinV > pF1->Corner[j].Uvws[1].V ) rMinV = pF1->Corner[j].Uvws[1].V;
				if( rMaxV < pF1->Corner[j].Uvws[1].V ) rMaxV = pF1->Corner[j].Uvws[1].V;
			}
			nPosMaxU = ((sint32)floor( rMaxU + 0.5 ));
			nPosMaxV = ((sint32)floor( rMaxV + 0.5 ));
			nPosMinU = ((sint32)floor( rMinU - 0.5 ));
			nPosMinV = ((sint32)floor( rMinV - 0.5 ));

			CVector n1 = ToWorldMat.mulVector( pF1->Corner[0].Normal );
			CVector n2 = ToWorldMat.mulVector( pF1->Corner[1].Normal );
			CVector n3 = ToWorldMat.mulVector( pF1->Corner[2].Normal );
	
			g.init( pF1, vVertices, n1, n2, n3 );

			double	lumx1 = pF1->Corner[0].Uvws[1].U, lumy1 = pF1->Corner[0].Uvws[1].V, 
					lumx2 = pF1->Corner[1].Uvws[1].U, lumy2 = pF1->Corner[1].Uvws[1].V, 
					lumx3 = pF1->Corner[2].Uvws[1].U, lumy3 = pF1->Corner[2].Uvws[1].V;

			// Process all the exterior and try to link with other planes
			for( k = nPosMinV; k < nPosMaxV; ++k )
			for( j = nPosMinU; j < nPosMaxU; ++j )
			if( ( pP1->msk[j-pP1->x   + (k-pP1->y)*pP1->w]   == 1 ) ||
				( pP1->msk[1+j-pP1->x + (k-pP1->y)*pP1->w]   == 1 ) ||
				( pP1->msk[1+j-pP1->x + (1+k-pP1->y)*pP1->w] == 1 ) ||
				( pP1->msk[j-pP1->x   + (1+k-pP1->y)*pP1->w] == 1 ) )
			if( segmentIntersection(j+0.5, k+0.5, j+1.5, k+0.5, lumx1, lumy1, lumx2, lumy2) ||
				segmentIntersection(j+0.5, k+0.5, j+1.5, k+0.5, lumx2, lumy2, lumx3, lumy3) ||
				segmentIntersection(j+0.5, k+0.5, j+1.5, k+0.5, lumx3, lumy3, lumx1, lumy1) ||
	
				segmentIntersection(j+0.5, k+0.5, j+0.5, k+1.5, lumx1, lumy1, lumx2, lumy2) ||
				segmentIntersection(j+0.5, k+0.5, j+0.5, k+1.5, lumx2, lumy2, lumx3, lumy3) ||
				segmentIntersection(j+0.5, k+0.5, j+0.5, k+1.5, lumx3, lumy3, lumx1, lumy1) ||
	
				segmentIntersection(j+1.5, k+1.5, j+1.5, k+0.5, lumx1, lumy1, lumx2, lumy2) ||
				segmentIntersection(j+1.5, k+1.5, j+1.5, k+0.5, lumx2, lumy2, lumx3, lumy3) ||
				segmentIntersection(j+1.5, k+1.5, j+1.5, k+0.5, lumx3, lumy3, lumx1, lumy1) ||
	
				segmentIntersection(j+1.5, k+1.5, j+0.5, k+1.5, lumx1, lumy1, lumx2, lumy2) ||
				segmentIntersection(j+1.5, k+1.5, j+0.5, k+1.5, lumx2, lumy2, lumx3, lumy3) ||
				segmentIntersection(j+1.5, k+1.5, j+0.5, k+1.5, lumx3, lumy3, lumx1, lumy1) ||

				vertexInSquare(j+0.5, k+0.5, 1.0, 1.0, lumx1, lumy1) || 
				vertexInSquare(j+0.5, k+0.5, 1.0, 1.0, lumx2, lumy2) || 
				vertexInSquare(j+0.5, k+0.5, 1.0, 1.0, lumx3, lumy3)							)

			{
				// If all segment of the current face are linked with a face in this plane, no need to continue
				vector<CMesh::CFace*>::iterator ItParseM = pP1->faces.begin();
				uint32 nNbSeg = 0;
				uint32 m, n;
				for( m = 0; m < nNbFace1; ++m )
				{
					CMesh::CFace *pF2 = *ItParseM;
					if( m != i )
						if( FaceContinuous( pF1, pF2 ) )
							++nNbSeg;
					++ItParseM;
				}
				if( nNbSeg >= 3 )
					continue;
				// Get the face on the other plane with a common segment
				vector<SLMPlane*>::iterator ItParsePlanes = ItPlanes;
				for( m = 0; m < nNbPlanes; ++m )
				{
					SLMPlane *pP2 = *ItParsePlanes;
					if( pP2 != pP1 )
					for( n = 0; n < pP2->faces.size(); ++n )
					{
						CMesh::CFace *pF2 = pP2->faces[n];
						if( FaceContinuous( pF1, pF2 ) )
						{
							for( uint32 o = 0; o < 4; ++o )
							{
								sint32 nAbsX = j + (o/2), nAbsY = k + (o%2);
								// Is it a pixel to treat and pixel in the 2nd plane
								if( ( pP1->msk[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w] == 1 ) &&
									(nAbsX >= pP2->x) && (nAbsX < (pP2->x+(sint32)pP2->w) ) &&
									(nAbsY >= pP2->y) && (nAbsY < (pP2->y+(sint32)pP2->h) ) )
								{
									// Is it an interior calculated pixel ?
									if( pP2->msk[nAbsX-pP2->x + (nAbsY-pP2->y)*pP2->w] == 2 )
									{ // Yes -> ok so get it
										pP1->col[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w+pP1->w*pP1->h*nLayerNb] = 
											pP2->col[nAbsX-pP2->x + (nAbsY-pP2->y)*pP2->w+pP2->w*pP2->h*nLayerNb];
										pP1->msk[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w] = 3;
										pP1->ray[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w] = 
											pP2->ray[nAbsX-pP2->x + (nAbsY-pP2->y)*pP2->w];
									}
									else
									if( pP2->msk[nAbsX-pP2->x + (nAbsY-pP2->y)*pP2->w] == 1 )
									{ // No -> Add extrapolated value
										CVector iv = g.getInterpolatedVertex( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
										CVector in = g.getInterpolatedNormal( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
										CRGBAF vl = g.getInterpolatedColor( j+0.5, k+0.5);
										CVector rv = g.getInterpolatedVertexInFace( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5, pF1 );
										uint8 rtVal = pP2->ray[nAbsX-pP2->x + (nAbsY-pP2->y)*pP2->w];
										CRGBAF col = LightAVertex( rtVal, rv, iv, in, vLights, AllLights, wrt, doubleSided, pMBB->bRcvShadows );
										pP2->ray[nAbsX-pP2->x + (nAbsY-pP2->y)*pP2->w] = rtVal;
										//float f = 1.0f;
										pP2->col[nAbsX-pP2->x + (nAbsY-pP2->y)*pP2->w+pP2->w*pP2->h*nLayerNb].R += col.R*(vl.R/255.0f)*(matDiff.R/255.0f);
										pP2->col[nAbsX-pP2->x + (nAbsY-pP2->y)*pP2->w+pP2->w*pP2->h*nLayerNb].G += col.G*(vl.G/255.0f)*(matDiff.G/255.0f);
										pP2->col[nAbsX-pP2->x + (nAbsY-pP2->y)*pP2->w+pP2->w*pP2->h*nLayerNb].B += col.B*(vl.B/255.0f)*(matDiff.B/255.0f);
										pP2->col[nAbsX-pP2->x + (nAbsY-pP2->y)*pP2->w+pP2->w*pP2->h*nLayerNb].A += 1.0f;
									}
								}
							}
						}
					}
					++ItParsePlanes;
				}

				for( sint32 o = 0; o < 4; ++o )
				{
					sint32 nAbsX = j + (o/2), nAbsY = k + (o%2);
					if( pP1->msk[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w] == 1 )
					{
						CVector iv = g.getInterpolatedVertex( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
						CVector in = g.getInterpolatedNormal( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
						CRGBAF vl = g.getInterpolatedColor( j+0.5, k+0.5);
						CVector rv = g.getInterpolatedVertexInFace( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5, pF1 );
						uint8 rtVal = pP1->ray[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w];
						CRGBAF col = LightAVertex( rtVal, rv, iv, in, vLights, AllLights, wrt, doubleSided, pMBB->bRcvShadows );
						pP1->ray[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w] = rtVal;
						//float f = 1.0f;
						pP1->col[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w+pP1->w*pP1->h*nLayerNb].R += col.R*(vl.R/255.0f)*(matDiff.R/255.0f);
						pP1->col[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w+pP1->w*pP1->h*nLayerNb].G += col.G*(vl.G/255.0f)*(matDiff.G/255.0f);
						pP1->col[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w+pP1->w*pP1->h*nLayerNb].B += col.B*(vl.B/255.0f)*(matDiff.B/255.0f);
						pP1->col[nAbsX-pP1->x + (nAbsY-pP1->y)*pP1->w+pP1->w*pP1->h*nLayerNb].A += 1.0f;
					}
				}
			}
			// Next Face
			++ItParseI;
		}
		++ItPlanes1;
	}
	// All planes are done so now we have to average the value of lumels grouping severals normals
	ItPlanes1 = ItPlanes;
	for( nPlanes1 = 0; nPlanes1 < nNbPlanes; ++nPlanes1 )
	{
		uint32 j, k;
		SLMPlane *pP1 = *ItPlanes1;
		
		for( k = 0; k < pP1->h; ++k )
		for( j = 0; j < pP1->w; ++j )
		{
			if( pP1->msk[j+k*pP1->w] == 1 )
			{
				sint32 nNbNormals = (sint32)pP1->col[j + k*pP1->w+pP1->w*pP1->h*nLayerNb].A;

				// Avoid divid by zero
				if (nNbNormals)
				{
					pP1->col[j + k*pP1->w+pP1->w*pP1->h*nLayerNb].R /= nNbNormals;
					pP1->col[j + k*pP1->w+pP1->w*pP1->h*nLayerNb].G /= nNbNormals;
					pP1->col[j + k*pP1->w+pP1->w*pP1->h*nLayerNb].B /= nNbNormals;
				}
				pP1->col[j + k*pP1->w+pP1->w*pP1->h*nLayerNb].A = 1.0f;
				pP1->msk[j + k*pP1->w] = 4;
			}
		}
		++ItPlanes1;
	}
}

// -----------------------------------------------------------------------------------------------
bool isAllFaceMapped( vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFaces )
{
	sint32 i, j;

	vector<CMesh::CFace*>::iterator ItParseI = ItFace;
	for( i = 0; i < nNbFaces; ++i )
	{
		CMesh::CFace *pF = *ItParseI;
		for( j = 0; j < 3; ++j )
		{
			if( (fabsf(pF->Corner[j].Uvws[1].U) > 64.0) || 
				(fabsf(pF->Corner[j].Uvws[1].V) > 64.0) )
				return false;
		}
		++ItParseI;
	}
	double TextureSurf = 0.0f;
	ItParseI = ItFace;
	for( i = 0; i < nNbFaces; ++i )
	{
		CMesh::CFace *pF = *ItParseI;
		CVector p1, p2, p3;
		p1.x = pF->Corner[0].Uvws[1].U; p1.y = pF->Corner[0].Uvws[1].V; p1.z = 0.0f;
		p2.x = pF->Corner[1].Uvws[1].U; p2.y = pF->Corner[1].Uvws[1].V; p2.z = 0.0f;
		p3.x = pF->Corner[2].Uvws[1].U; p3.y = pF->Corner[2].Uvws[1].V; p3.z = 0.0f;
		TextureSurf += calculateTriangleSurface( p1, p2, p3 );
		++ItParseI;
	}
	if( fabs(TextureSurf) < 0.000001 )
		return false;
	return true;
}

// -----------------------------------------------------------------------------------------------
CAABBox getMeshBBox (CMesh::CMeshBuild& rMB, CMeshBase::CMeshBaseBuild &rMBB, bool bNeedToTransform)
{
	CAABBox meshBox;
	if( bNeedToTransform )
	{
		CMatrix MBMatrix = getObjectToWorldMatrix (&rMB, &rMBB);
		MBMatrix.movePos (-vGlobalPos);

		for( uint32 j = 0; j < rMB.Vertices.size(); ++j )
			if( j == 0 )
				meshBox.setCenter( MBMatrix * rMB.Vertices[j] );
			else
				meshBox.extend( MBMatrix * rMB.Vertices[j] );
	}
	else
	{
		for( uint32 j = 0; j < rMB.Vertices.size(); ++j )
			if( j == 0 )
				meshBox.setCenter( rMB.Vertices[j] );
			else
				meshBox.extend( rMB.Vertices[j] );
	}
	return meshBox;
}

// -----------------------------------------------------------------------------------------------
CAABBox getLightBBox( CVector &vPos, float rRadius )
{
	CAABBox lightBox;
	lightBox.setCenter( vPos );
	lightBox.extend( vPos - CVector(rRadius,0,0) );
	lightBox.extend( vPos + CVector(rRadius,0,0) );
	lightBox.extend( vPos - CVector(0,rRadius,0) );
	lightBox.extend( vPos + CVector(0,rRadius,0) );
	lightBox.extend( vPos - CVector(0,0,rRadius) );
	lightBox.extend( vPos + CVector(0,0,rRadius) );
	return lightBox;
}

// -----------------------------------------------------------------------------------------------
bool isLightCanCastShadowOnBox( SLightBuild &rSLB, CAABBox &b )
{
	switch( rSLB.Type )
	{
		case SLightBuild::LightAmbient: // No need an ambient light...
			// No ambient handled for the moment
		break;
		case SLightBuild::LightSpot: // For the moment spot like point
		case SLightBuild::LightPoint:
		{
			CAABBox lightBox = getLightBBox( rSLB.Position, rSLB.rRadiusMax );
			if( lightBox.intersect( b ) )
				return true;

			if( b.include( lightBox.getMin() ) )
				return true;

			if( lightBox.include( b.getMin() ) )
				return true;
		}
		break;
		case SLightBuild::LightDir:
			// Attenuation not handled (ask cyril later)
		break;
	}
	return false;
}

// -----------------------------------------------------------------------------------------------
bool isInteractionLightMesh( SLightBuild &rSLB, CAABBox &meshBox)
{
	if (rSLB.Type == SLightBuild::LightAmbient)
		return true;

	if (rSLB.Type == SLightBuild::LightDir)
	{
		// Construct acceleration for a dir light to not select all nodes in scene
		rSLB.rDirRadius = meshBox.getHalfSize().norm();
		rSLB.Position = meshBox.getCenter();
		return true;
	}
	return isLightCanCastShadowOnBox( rSLB, meshBox );
}

// -----------------------------------------------------------------------------------------------
bool isInteractionLightMesh( SLightBuild &rSLB, CMesh::CMeshBuild &rMB, CMeshBase::CMeshBaseBuild &rMBB )
{
	CAABBox		meshBox;
	
	meshBox = getMeshBBox( rMB, rMBB, true );

	return isInteractionLightMesh(rSLB, meshBox);
}


// -----------------------------------------------------------------------------------------------
// Get all lights that can cast shadows on the current mesh
void getLightInteract( CMesh::CMeshBuild* pMB, CMeshBase::CMeshBaseBuild *pMBB, vector<SLightBuild> &AllLights, vector< vector<sint32> >&vvLights )
{
	uint32 nNbGroup = 0;
	vector<sint32> vlbTmp;
	uint32 i, j;

	for( i = 0; i < AllLights.size(); ++i )
	{
		if( isInteractionLightMesh( AllLights[i], *pMB, *pMBB ) )
		{
			// Is the light name already exist
			for( j = 0; j < nNbGroup; ++j )
				if ( ( AllLights[vvLights[j].operator[](0)].AnimatedLight == AllLights[i].AnimatedLight ) &&
					 ( AllLights[vvLights[j].operator[](0)].LightGroup == AllLights[i].LightGroup ) )
					break;

			// The light name does not exist create a new group
			if ( j == nNbGroup )
			{
				vvLights.push_back( vlbTmp ); // Static lighting
				vvLights[nNbGroup].push_back( i );
				++nNbGroup;
			}
			else
			{
				vvLights[j].push_back( i );
			}
		}
	}
}

// -----------------------------------------------------------------------------------------------
void convertToWorldCoordinate (CMesh::CMeshBuild *pMB, CMeshBase::CMeshBaseBuild *pMBB, CVector &translation)
{
	uint32 j, k;
	CMatrix MBMatrix = getObjectToWorldMatrix (pMB, pMBB);
	MBMatrix.movePos (translation);
	// Update vertices
	for( j = 0; j < pMB->Vertices.size(); ++j )
		pMB->Vertices[j] = MBMatrix * pMB->Vertices[j];
	// Update normals
	MBMatrix.invert();
	MBMatrix.transpose();
	for( j = 0; j < pMB->Faces.size(); ++j )
		for( k = 0; k < 3 ; ++k )
			pMB->Faces[j].Corner[k].Normal = 
								MBMatrix.mulVector( pMB->Faces[j].Corner[k].Normal );
}

// -----------------------------------------------------------------------------------------------
// Is the box b1 can cast shadow on the box b2 with the light l ?
bool isBoxCanCastShadowOnBoxWithLight( CAABBox &b1, CAABBox &b2, SLightBuild &l )
{
	// if the light is included in the box b2
	return isLightCanCastShadowOnBox( l, b1 );
}

// -----------------------------------------------------------------------------------------------
void supprLightNoInteractOne( vector<SLightBuild> &vLights, CMesh::CMeshBuild* pMB, CMeshBase::CMeshBaseBuild *pMBB, INode &node )
{
	uint32 i;

	// temp result
	vector<SLightBuild> result;
	result.reserve(vLights.size());

	// build the bb of the shape only one time
	CAABBox		meshBox;
	meshBox = getMeshBBox( *pMB, *pMBB, true );

	// for all lights
	for( i = 0; i < vLights.size(); ++i )
	{
		bool bInteract = false;

		if( vLights[i].setExclusion.find( node.GetName() ) != vLights[i].setExclusion.end() )
		{
			bInteract = false;
		}
		else
		{
			if( isInteractionLightMesh( vLights[i], meshBox) )
			{
				bInteract = true;			
			}
		}
		if( bInteract )
		{
			result.push_back(vLights[i]);
		}
	}

	// copy result
	vLights= result;
}

// -----------------------------------------------------------------------------------------------
// Add information for ont mesh to reference all the lights that interact with him
void addLightInfo( CMesh::CMeshBuild *pMB, CMeshBase::CMeshBaseBuild *pMBB, string &animatedLight, uint lightGroup, uint8 nMatNb, uint8 nStageNb )
{
	/* Search in the light mesh info the good light group. Add the material stage if it exists. 
	 * Else, add a new entry in the light group. */

	const uint count = pMBB->LightInfoMap.size ();
	uint i;
	for (i=0; i<count; i++)
	{
		CMeshBase::CLightMapInfoList &info = pMBB->LightInfoMap[i];
		if ( (info.AnimatedLight == animatedLight) && (info.LightGroup == lightGroup) )
		{
			// This one, append the material stage
			CMeshBase::CLightMapInfoList::CMatStage temp;
			temp.MatId = nMatNb;
			temp.StageId = nStageNb;
			info.StageList.push_back (temp);
			break;
		}
	}
	
	// Not found ?
	if (i == count)
	{
		// Add a new entry

		CMeshBase::CLightMapInfoList info;
		info.LightGroup = lightGroup;
		info.AnimatedLight = animatedLight;
		CMeshBase::CLightMapInfoList::CMatStage temp;
		temp.MatId = nMatNb;
		temp.StageId = nStageNb;
		info.StageList.push_back (temp);
		pMBB->LightInfoMap.push_back (info);
	}
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CExportNel::deleteLM(INode& ZeNode)
{
	sint32 i;

	// Suppress all lightmap files
	for( i = 0; i < 8; ++i )
	{		
		string sSaveName;
		sSaveName = _Options.sExportLighting;
		if( sSaveName[sSaveName.size()-1] != '\\' ) sSaveName += "\\";
		sSaveName += ZeNode.GetName();
		char tmp[32];
		sprintf( tmp, "%d", i );
		sSaveName += tmp;
		sSaveName += ".tga";
		if (CFile::fileExists(sSaveName))
		{
			if (!CFile::deleteFile(sSaveName))
			{
				nlwarning("Failed to delete file %s.", sSaveName.c_str());
			}
		}
	}
}

// In : set of faces
// Out : planes
/*
void computeSetOfFaces (vector<SLMPlane*>&planes, )
*/

// Object that touch the reference object
struct SSurObj
{
	CMesh::CMeshBuild *pMB;
	CMeshBase::CMeshBaseBuild *pMBB;
	vector<CMesh::CFace*> faces;
};

// Get the objects next to one
/*
void sans_majuscule_au_debut_LinkToObjectAround (CMesh::CMeshBuild *pMB, CMeshBase::CMeshBaseBuild *pMBB, 
												 vector<CVector>& vertices,
												 SWorldRT& wrt)
{
	CAABBox ref = getMeshBBox (*pMB, *pMBB, true);
	vector<SSurObj> objs;
	uint32 i, k, l, m;

	for( i = 0; wrt.vMB.size(); ++i)
	{
		if (ref.intersect (getMeshBBox (*wrt.vMB[i], *wrt.vMBB[i], false)))
		{
			SSurObj oTmp;
			vector<sint32> ivert;
			oTmp.pMB = wrt.vMB[i];
			oTmp.pMBB = wrt.vMBB[i];
			// check a vertex continuity (precision is 1 centimeter)
			for (k = 0; k < vertices.size(); ++k)
			for (l = 0; l < wrt.vMB[i]->Vertices.size(); ++l)
			{
				float dist = (vertices[k] - wrt.vMB[i]->Vertices[l]).norm();
				if (dist < 0.01)
				{
					ivert.push_back (l);
				}
			}
			
			if (ivert.size() > 0)
			{
				// Get all faces that contains at least one shared vertex
				for (k = 0; k < wrt.vMB[i]->Faces.size(); ++k)
				for (l = 0; l < ivert.size(); ++l)
				{
					if ((wrt.vMB[i]->Faces[k].Corner[0].Vertex == ivert[l]) ||
						(wrt.vMB[i]->Faces[k].Corner[1].Vertex == ivert[l]) ||
						(wrt.vMB[i]->Faces[k].Corner[2].Vertex == ivert[l]))
					{
						bool bToAdd = true;
						// Check if the face is not already added
						for (m = 0; m < oTmp.faces.size(); ++m)
						if (oTmp.faces[m] == &wrt.vMB[i]->Faces[k])
						{
							bToAdd = false;
							break;
						}
						if (bToAdd)
							oTmp.faces.push_back (&wrt.vMB[i]->Faces[k]);
					}
				}
				// Add object
				objs.push_back (oTmp);
			}
		}
	}

	// Here objs contains the objects of the worldRT structure that touch the reference mesh
	// And it contains the pointer to the face that must be rendered to give continuity to our object
}
*/

void createLightmapLog (COFile &outputLog, const char *outputDirectory, const char *projectName, const char *objectName)
{
	string outputFilename = CPath::standardizePath (outputDirectory, true);
	outputFilename += projectName;
	outputFilename += "_";
	outputFilename += objectName;
	outputFilename += ".txt";
	if (!outputLog.open (outputFilename, false, true, false))
	{
		nlwarning ("Can't open the file %s for writing.", outputFilename.c_str());
	}
}

void appendLightmapLog (COFile &outputLog, const char *lightmapName, const vector<sint32> lightIndexes, const vector<SLightBuild> &lights)
{
	try
	{
		string text;
		text = lightmapName;
		text += " :\n";

		// Dump light name used on this lightmap
		uint i;
		for (i=0; i<lightIndexes.size(); i++)
		{
			text += "\t";
			text += lights[lightIndexes[i]].Name + toString (" (group %d, animation \"%s\")", lights[lightIndexes[i]].LightGroup, lights[lightIndexes[i]].AnimatedLight.c_str());
			text += "\n";
		}
		
		outputLog.serialBuffer ((uint8*)text.c_str(), text.size());
	}
	catch (exception &e)
	{
		nlwarning ("Error writing the file : %s", e.what());
	}
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Caclulate the lightmap of the specified mesh build
// absolutePath tell this code to put the name of the lightmap in absolute or relative path
// this is very usefull for viewer inside MAX
bool CExportNel::calculateLM( CMesh::CMeshBuild *pZeMeshBuild, CMeshBase::CMeshBaseBuild *pZeMeshBaseBuild, INode& ZeNode, 
							TimeValue tvTime, uint firstMaterial, bool outputLightmapLog)
{
	DWORD t = timeGetTime();

	uint32 i, j;

	// **** Retrieve Shape Node properties
	string sLumelSizeMul = CExportNel::getScriptAppData (&ZeNode, NEL3D_APPDATA_LUMELSIZEMUL, "1.0");
	float rLumelSizeMul = (float)atof(sLumelSizeMul.c_str());
	// 8Bits LightMap Compression
	bool	lmcEnabled= CExportNel::getScriptAppData (&ZeNode, NEL3D_APPDATA_EXPORT_LMC_ENABLED, BST_UNCHECKED)==BST_CHECKED;
	enum	{NumLightGroup= 3};
	CRGBA	lmcAmbient[NumLightGroup];
	CRGBA	lmcDiffuse[NumLightGroup];
	for(i=0;i<NumLightGroup;i++)
	{
		lmcAmbient[i]= CExportNel::getScriptAppData (&ZeNode, NEL3D_APPDATA_EXPORT_LMC_AMBIENT_START+i, CRGBA::Black);
		lmcDiffuse[i]= CExportNel::getScriptAppData (&ZeNode, NEL3D_APPDATA_EXPORT_LMC_DIFFUSE_START+i, CRGBA::White);
	}
	
	
	// **** Read options
	gOptions = _Options;

	if (rLumelSizeMul > 0.0f)
		gOptions.rLumelSize *= rLumelSizeMul;

	//if (gOptions.nExportLighting == 1)
	//	return calculateLMRad (pZeMeshBuild, pZeMeshBaseBuild, ZeNode, ip, tvTime, absolutePath, gOptions);

	if(gOptions.FeedBack != NULL)
	{
		string sTmp = "LumelSize = " + toString(gOptions.rLumelSize);
		if (gOptions.bShadow)
			sTmp += ",Shadow On";
		else
			sTmp += ",Shadow Off";
		sTmp += ",OverSampling = " +toString(gOptions.nOverSampling*gOptions.nOverSampling);
		gOptions.FeedBack->setLine (1, sTmp);
	}


	CRTWorld WorldRT (_ErrorInDialog, _View, _AbsolutePath, _Ip, _ErrorTitle, this); // The static world for raytrace
	vector<SLightBuild> AllLights;

	CMatrix mtmp = getObjectToWorldMatrix (pZeMeshBuild, pZeMeshBaseBuild);
	vGlobalPos = mtmp.getPos();
	vGlobalPos.x = (float)((int)vGlobalPos.x);
	vGlobalPos.y = (float)((int)vGlobalPos.y);
	vGlobalPos.z = (float)((int)vGlobalPos.z);

	if (gOptions.FeedBack != NULL)
	{
		string sTmp = "Initializing";
		gOptions.FeedBack->setLine (2, sTmp);
		gOptions.FeedBack->update ();
	}
	// Select meshes to test for raytrace
	// Get all lights from MAX
	getLightmapLightBuilds( AllLights, tvTime, *_Ip );
	// Get all lights L that have influence over the mesh selected
	supprLightNoInteractOne( AllLights, pZeMeshBuild, pZeMeshBaseBuild, ZeNode );

	// Get all the lod child nodes and the node for which this node is a lod
	std::set<INode*>	lodListToExclude;
	addChildLodNode (lodListToExclude);
	addParentLodNode (ZeNode, lodListToExclude);

	// Remove from exclude the node
	lodListToExclude.erase (&ZeNode);

	// List to include
	std::set<INode*>	lodListToInclude;
	lodListToInclude.insert (&ZeNode);

	// Get all meshes that are influenced by the lights L
	WorldRT.build (AllLights, -vGlobalPos, gOptions.bExcludeNonSelected, lodListToExclude, lodListToInclude);

	//for( nNode=0; nNode < nNbMesh; ++nNode )
	{
		// First order face by Material and by texture surface
		CMesh::CMeshBuild *pMB = pZeMeshBuild;
		CMeshBase::CMeshBaseBuild *pMBB = pZeMeshBaseBuild;
		vector<CMesh::CFace*> AllFaces;
		CMatrix MBMatrix = getObjectToWorldMatrix (pMB, pMBB);
		MBMatrix.movePos (-vGlobalPos);
		vector<CVector> AllVertices; // All vertices in world space
		vector<sint32> FaceGroupByMat; // Number of faces with the same properties
		uint32 nNbFace = pMB->Faces.size(), nNbVertex = pMB->Vertices.size();
		sint32 offsetMat, offsetSmooth, offsetPlane;
		vector<SLMPlane*> AllPlanes;
		sint32 AllPlanesPrevSize;
		vector< vector<sint32> > vvLights;

		// Select Lights interacting with the node
		getLightInteract( pMB, pMBB, AllLights, vvLights );

		AllPlanes.clear();
		// Make Geometry like we want			
		// Make a vector of pointer to all the faces of the MeshBuild
		AllFaces.resize( nNbFace );
		for( i = 0; i < nNbFace; ++i )
			AllFaces[i] = &pMB->Faces[i];
		// Make All vertices of the mesh in the world basis
		AllVertices.resize(nNbVertex);
		for( i = 0; i < nNbVertex; ++i )
			AllVertices[i] = MBMatrix * pMB->Vertices[i];
		// Invert and transpose for use of futur normal
		MBMatrix.invert();
		MBMatrix.transpose();

		// Bubble sort pointer to the faces (Material sorting)
		ClearFaceWithNoLM( pMB, pMBB, AllFaces );
		if( AllFaces.size() == 0 )
		{
			if (InfoLog)
				InfoLog->display("CalculateLM : %d ms\n", timeGetTime()-t);
			return false;
		}

		if (gOptions.FeedBack != NULL)
		{
			string sTmp = "Calculating";
			gOptions.FeedBack->setLine (2, sTmp);
			gOptions.FeedBack->update ();
		}
		SortFaceByMaterialId (FaceGroupByMat, AllFaces.begin(), AllFaces.size());
		if( ! isAllFaceMapped (AllFaces.begin(), AllFaces.size() ) )
		{
			string thetext;
			thetext = "Warning ";
			thetext += ZeNode.GetName();
			thetext = "have all faces NOT mapped (UV2)";
			if (gOptions.FeedBack != NULL)
			{
				gOptions.FeedBack->setLine (11, thetext);
				gOptions.FeedBack->update ();
			}
			//MessageBox( NULL, thetext.c_str(), "LightMap ERROR", MB_OK|MB_ICONERROR );
			if (InfoLog)
				InfoLog->display("CalculateLM : %d ms\n", timeGetTime()-t);
			return false;
		}

		// PATCH
		FaceGroupByMat.resize(1);
		FaceGroupByMat[0] = AllFaces.size();

		offsetMat = 0;
		for( uint32 nMat = 0; nMat < FaceGroupByMat.size(); ++nMat )
		{
			vector<sint32> FaceGroupBySmooth;

			if (gOptions.FeedBack != NULL)
			{
				string sTmp = "Material (" + toString(1+nMat) + "/" + toString(FaceGroupByMat.size()) + ")";
				gOptions.FeedBack->setLine (3, sTmp);
				gOptions.FeedBack->update ();
			}
			// Sort faces by smoothing group
			SortFaceBySMoothGroup( FaceGroupBySmooth, AllFaces.begin()+offsetMat, FaceGroupByMat[nMat] );

			offsetSmooth = offsetMat;
			for( uint32 nSmoothNb = 0; nSmoothNb < FaceGroupBySmooth.size(); ++nSmoothNb )
			{
				sint32 nPlaneNb, nLight;
				vector<sint32> FaceGroupByPlane;
				
				if (gOptions.FeedBack != NULL)
				{
					string sTmp = "SmoothGroup (" + toString(1+nSmoothNb) + "/" + toString(FaceGroupBySmooth.size()) + ")";
					gOptions.FeedBack->setLine (4, sTmp);
					gOptions.FeedBack->update ();
				}

				if( ! PutFaceUV1InLumelCoord( gOptions.rLumelSize, AllVertices, 
										AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] ) )
				{
					// Make an error message
					string sTmp = "Warning : ";
					sTmp += ZeNode.GetName();
					sTmp += " has mapping problem";

					// Script trace
					mprintf ((sTmp+"\n").c_str());

					// Feedback is here ?
					if (gOptions.FeedBack != NULL)
					{
						gOptions.FeedBack->setLine (11, sTmp);

						sTmp = "Mat(" + toString(1+nMat) + ") at least one group with texture";						
						gOptions.FeedBack->setLine (12, sTmp);
						sTmp = "surface equal to zero";
						gOptions.FeedBack->setLine (13, sTmp);
						gOptions.FeedBack->update ();
					}
					continue;
				}

				SortFaceByPlane( FaceGroupByPlane, AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] );
				//AllPlanes.resize( FaceGroupByPlane.size() );

				//SortPlanesBySurface( FaceGroupByPlane, AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] );

				AllPlanesPrevSize = AllPlanes.size();
				AllPlanes.resize( AllPlanesPrevSize + FaceGroupByPlane.size() );

				offsetPlane = offsetSmooth;
				for( nPlaneNb = 0; nPlaneNb < (sint)FaceGroupByPlane.size(); ++nPlaneNb )
				{
					AllPlanes[AllPlanesPrevSize+nPlaneNb] = new SLMPlane;
					for( nLight = 0; nLight < ((sint32)vvLights.size()-1); ++nLight )
						AllPlanes[AllPlanesPrevSize+nPlaneNb]->newLayer();
					// Fill planes (part of lightmap)
					AllPlanes[AllPlanesPrevSize+nPlaneNb]->createFromFaceGroup( AllFaces.begin()+offsetPlane, 
																				FaceGroupByPlane[nPlaneNb] );
					// Next group of face with the same plane in the same smooth group of the same material
					offsetPlane += FaceGroupByPlane[nPlaneNb];
				}
				// Make join between all planes (all planes must be created)
				for( nLight = 0; nLight < (sint)vvLights.size(); ++nLight )
				{
					vector<SLMPlane*> TempPlanes;
					TempPlanes.resize(FaceGroupByPlane.size());
					for( nPlaneNb = 0; nPlaneNb < (sint)FaceGroupByPlane.size(); ++nPlaneNb )
					{
						TempPlanes[nPlaneNb] = new SLMPlane;
						TempPlanes[nPlaneNb]->createFromPlane (*AllPlanes[AllPlanesPrevSize+nPlaneNb]);
					}
					for( nPlaneNb = 0; nPlaneNb < (sint)FaceGroupByPlane.size(); ++nPlaneNb )
					{					
						// Light the LightMap for the plane (interior only)
						FirstLight( pMB, pMBB, *TempPlanes[nPlaneNb], 
									AllVertices, MBMatrix, vvLights[nLight], AllLights,
									0, WorldRT );
					}
					// Make extoriors
					SecondLight( pMB, pMBB, TempPlanes.begin(), FaceGroupByPlane.size(),
								AllVertices, MBMatrix, vvLights[nLight], AllLights,
								0, WorldRT );

					// Oversampling optimization
					if( gOptions.nOverSampling > 1 )
					{
						for( nPlaneNb = 0; nPlaneNb < (sint)FaceGroupByPlane.size(); ++nPlaneNb )
						{
							// Detect which pixels need to be& oversampled						
							TempPlanes[nPlaneNb]->contourDetect();
							// Enlarge image
							ModifyLMPlaneWithOverSampling( TempPlanes[nPlaneNb], gOptions.nOverSampling, true );
							// And the contour detection and the mask
							TempPlanes[nPlaneNb]->andRayWidthMask();
						}

						for( nPlaneNb = 0; nPlaneNb < (sint)FaceGroupByPlane.size(); ++nPlaneNb )
							FirstLight( pMB, pMBB, *TempPlanes[nPlaneNb], 
										AllVertices, MBMatrix, vvLights[nLight], AllLights,
										0, WorldRT );
						SecondLight( pMB, pMBB, TempPlanes.begin(), FaceGroupByPlane.size(),
									AllVertices, MBMatrix, vvLights[nLight],  AllLights,
									0, WorldRT );

						for( nPlaneNb = 0; nPlaneNb < (sint)FaceGroupByPlane.size(); ++nPlaneNb )
							ModifyLMPlaneWithOverSampling( TempPlanes[nPlaneNb],
															1.0/((double)gOptions.nOverSampling), false );
					}


					for( nPlaneNb = 0; nPlaneNb < (sint)FaceGroupByPlane.size(); ++nPlaneNb )
						TempPlanes[nPlaneNb]->copyFirstLayerTo(*AllPlanes[AllPlanesPrevSize+nPlaneNb],(uint8)nLight);

					for( nPlaneNb = 0; nPlaneNb < (sint)FaceGroupByPlane.size(); ++nPlaneNb )
					{
						delete TempPlanes[nPlaneNb];
						TempPlanes[nPlaneNb] = NULL;
					}
				}

				// Next group of face with the same smooth group and the same material
				offsetSmooth += FaceGroupBySmooth[nSmoothNb];
			}
			// Next group of face with the same material
			offsetMat += FaceGroupByMat[nMat];
		}
		
		// Create the lightmap
		if (gOptions.FeedBack != NULL)
		{
			string sTmp = "Placement";
			gOptions.FeedBack->setLine (3, sTmp);
			sTmp = "";
			for(i=4;i<10;++i)
				gOptions.FeedBack->setLine (i, sTmp);
			gOptions.FeedBack->update ();
		}

		// Optimize the planes if we can (if a plane contains the same color)
		for (i = 0; i < AllPlanes.size(); ++i)
		{
			SLMPlane *pPlane = AllPlanes[i];
			bool bIsGoodForOptimization = true;
			for (j = 0; j < pPlane->nNbLayerUsed; ++j)
			{
				CRGBAF rAverage = pPlane->getAverageColor ((uint8)j);
				if (!pPlane->isSameColorAs((uint8)j, rAverage, 1.0f/256.0f))
				{
					bIsGoodForOptimization = false;
					break;
				}
			}
			if (bIsGoodForOptimization)
			{
				vector<CRGBAF> allColors;
				uint32 k;

				for (j = 0; j < pPlane->nNbLayerUsed; ++j)
					allColors.push_back(pPlane->getAverageColor ((uint8)j));

				for (j = 0; j < pPlane->faces.size(); ++j)
					for (k = 0; k < 3; ++k)
					{
						float u, v;
						u = pPlane->faces[j]->Corner[k].Uvws[1].U - (float)pPlane->x - 0.5f;
						u = u / (float)pPlane->w;
						
						v = pPlane->faces[j]->Corner[k].Uvws[1].V - (float)pPlane->y - 0.5f;
						v = v / (float)pPlane->h;
						
						if (u < 0.0f)
							u = 0.0f;
						if (u > 1.0f)
							u = 1.0f;

						if (v < 0.0f)
							v = 0.0f;
						if (v > 1.0f)
							v = 1.0f;

						pPlane->faces[j]->Corner[k].Uvws[1].U = u + (float)pPlane->x + 0.5f;
						pPlane->faces[j]->Corner[k].Uvws[1].V = v + (float)pPlane->y + 0.5f;
					}
				pPlane->resize (2,2);
				for (j = 0; j < pPlane->nNbLayerUsed; ++j)
					for (k = 0; k < 4; ++k)
					pPlane->col[k+j*4] = allColors[j];

				for (k = 0; k < 4; ++k)
					pPlane->msk[k] = 1; // Just set the mask to a non zero value
			}
		}

		// Place the planes in the lightmap
		SLMPlane LightMap;
		SortPlanesBySurface( AllPlanes );
		for( i = 0; i < AllPlanes.size(); ++i )
		{
			if (gOptions.FeedBack != NULL)
			{
				string sTmp = "Plane (" + toString(i) + "/" + toString(AllPlanes.size()) + ")";
				gOptions.FeedBack->setLine (4, sTmp);
				gOptions.FeedBack->update ();
			}
			// Put in the basis of the plane
			MoveFaceUV1( AllPlanes[i]->faces.begin(), AllPlanes[i]->faces.size(), 
						-AllPlanes[i]->x, -AllPlanes[i]->y );
			PlaceLMPlaneInLMPLane( LightMap, *AllPlanes[i] );
			LightMap.nNbLayerUsed = AllPlanes[i]->nNbLayerUsed;
			// Put in the new basis
			MoveFaceUV1( AllPlanes[i]->faces.begin(), AllPlanes[i]->faces.size(), 
						AllPlanes[i]->x, AllPlanes[i]->y );
			delete AllPlanes[i];
			AllPlanes[i] = NULL;
		}
		
		// Save the lightmap				
		// Assign the name of the lightmap and get the complete save name
			
		// Get the name of the max project
		char projectName[512];
		_splitpath (_Ip->GetCurFileName(), NULL, NULL, projectName, NULL);

		// Add lightmap information in the lightmap log
		COFile outputLog;
		if (outputLightmapLog)
			createLightmapLog (outputLog, gOptions.sExportLighting.c_str(), projectName, ZeNode.GetName());

		// Update UV coords to Texture space
		PutFaceUV1InTextureCoord( LightMap.w, LightMap.h, AllFaces.begin(), AllFaces.size() );
		uint32 nLightMapNb = 0;
		for (j = 0; j < LightMap.nNbLayerUsed; ++j)
		{
			CRGBA	lmcLayerAmbient= CRGBA::Black;
			CRGBA	lmcLayerDiffuse= CRGBA::White;
			if (lmcEnabled)
			{
				// get the compress color term according to light group
				if( !vvLights.empty() && !vvLights[j].empty() )
				{
					// Take the first light that is not an ambiant light
					SLightBuild &rTmpLB = AllLights[vvLights[j][0]];
					if(rTmpLB.LightGroup<NumLightGroup)
					{
						lmcLayerAmbient= lmcAmbient[rTmpLB.LightGroup];
						lmcLayerDiffuse= lmcDiffuse[rTmpLB.LightGroup];
					}
				}
			}

			// if lightmap compression enabled, must not skip this layer if the ambient term is not black!
			if (!LightMap.isAllBlack((uint8)j) || lmcLayerAmbient!=CRGBA::Black)
			{
				CTextureFile *pLightMap = new CTextureFile();
				//string sSaveName = AllMeshBuilds[nNode].second->GetName();
				string sSaveName = ZeNode.GetName();
				char tmp[32];
				sSaveName += "_";
				sprintf( tmp, "%d", nLightMapNb );
				sSaveName += tmp;
				sSaveName += ".tga";

				// Concat name of the project with name of the file
				sSaveName = (const char*)projectName + std::string ("_") + sSaveName;
				sSaveName = strlwr (sSaveName);

				// Remove spaces
				uint i;
				for (i=0; i<sSaveName.length(); i++)
				{
					if (sSaveName[i] == ' ')
						sSaveName[i] = '_';
				}

				pLightMap->setFileName (sSaveName);
				sSaveName = gOptions.sExportLighting;
				if (sSaveName[sSaveName.size()-1] != '\\') sSaveName += "\\";
				sSaveName += pLightMap->getFileName();
				if (_AbsolutePath)
					pLightMap->setFileName (sSaveName);
				// if first time Remove all lightmaps of the same name over all layers
				if (j == 0)
				{
					string sBaseName = sSaveName.substr(0, 1+sSaveName.rfind('_'));
					for (i = 0; i < 256; ++i)
					{
						string sLMName = sBaseName + NLMISC::toString(i) + ".tga";
						if (CFile::fileExists(sLMName))
						{
							nlinfo("DELETE %s",  sLMName.c_str());
							if (!CFile::deleteFile(sLMName))
							{
								nlwarning("Failed to delete file %s", sLMName.c_str());
							}
						}
					}
				}

				// Convert and write
				if (lmcEnabled)
				{
					// compress to 8 bits
					LightMap.copyColToBitmap8x2 (pLightMap, j, lmcLayerAmbient, lmcLayerDiffuse);
				}
				else
					// cust copy to 32 bits
					LightMap.copyColToBitmap32 (pLightMap, j);

				COFile f( sSaveName );
				try
				{
					nlinfo("SAVE %s", sSaveName.c_str());
					if (lmcEnabled)
					{
						// In fact the output is 32 bits because we need the alpha channel
						// to indicate where the lightmap parts are.
						pLightMap->loadGrayscaleAsAlpha(false);
						pLightMap->writeTGA (f, 32);
					}
					else
					{
						pLightMap->writeTGA (f, 32);
					}
				}
				catch(Exception &e)
				{
					if (gOptions.FeedBack != NULL)
					{
						char message[512];
						sprintf (message, "Can't write the file %s : %s", sSaveName, e.what());
						mprintf (message);
					}
				}

				// Add lightmap information in the lightmap log
				if (outputLightmapLog)
					appendLightmapLog (outputLog, sSaveName.c_str(), vvLights[j], AllLights);

				if (lmcEnabled)
					pLightMap->setUploadFormat (ITexture::Luminance);
				else
					pLightMap->setUploadFormat (ITexture::RGB565);

				pLightMap->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
				pLightMap->setAllowDegradation (false);
				if (gOptions.bShowLumel)
					pLightMap->setFilterMode (ITexture::Nearest, ITexture::NearestMipMapOff);

				// Setup all material of the object
				for( i = firstMaterial; i < pMBB->Materials.size(); ++i )
				if( pMBB->Materials[i].getShader() == CMaterial::LightMap )
				{
					pMBB->Materials[i].setLightMap( nLightMapNb, pLightMap );
					
					// If some light for this layer
					if( !vvLights.empty() && !vvLights[j].empty() )
					{
						// Set the light factor if it is a 8 bits lightmap
						if (lmcEnabled)
						{
							// Divide by 2 the ambient color in the material. see compute
							CRGBA	mA;
							mA.modulateFromui(lmcLayerAmbient, 127);
							pMBB->Materials[i].setLMCColors ( nLightMapNb, mA, lmcLayerDiffuse );
							pMBB->Materials[i].setLightMapMulx2 (true);
						}

						addLightInfo( pMB, pMBB, AllLights[vvLights[j].operator[](0)].AnimatedLight, 
										AllLights[vvLights[j].operator[](0)].LightGroup, 
										(uint8)i, (uint8)nLightMapNb );
					}
				}
				++nLightMapNb;
			}
		}
		// Next mesh
	}


	// End of the lighting process for this node we have to export the data
	CMesh::CMeshBuild *pMB = pZeMeshBuild;
	CMeshBase::CMeshBaseBuild *pMBB = pZeMeshBaseBuild;
	pMB->VertexFlags |= CVertexBuffer::TexCoord1Flag;
	// Build the mesh with the build interface
	for( i = firstMaterial; i < pMBB->Materials.size(); ++i )
	if( pMBB->Materials[i].getShader() == CMaterial::LightMap )
	{
		pMBB->Materials[i].setLighting( false );
		pMBB->Materials[i].setColor( CRGBA(255,255,255,255) );
	}

	if (InfoLog)
		InfoLog->display("CalculateLM : %d ms\n", timeGetTime()-t);
	return true;	
}


