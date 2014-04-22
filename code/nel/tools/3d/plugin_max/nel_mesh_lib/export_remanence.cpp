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


#include "StdAfx.h"
#include "export_nel.h"
#include "nel/3d/seg_remanence_shape.h"
#include "nel/misc/common.h"
#include "export_appdata.h"

#include <memory>

using namespace NL3D;
using namespace NLMISC;



//=============================================================================================
static void buildRemanenceError(CExportNel *en, INode &node, const char *mess)
{
	char message[512];
	smprintf (message, 512, mess, node.GetName());
	en->outputErrorMessage (message);
}

//=============================================================================================
NL3D::IShape *CExportNel::buildRemanence(INode& node, TimeValue time)
{
	std::auto_ptr<CSegRemanenceShape> srs(new CSegRemanenceShape);
	uint  numSlices = getScriptAppData (&node, NEL3D_APPDATA_REMANENCE_SLICE_NUMBER, 2);
	float samplingPeriod = getScriptAppData (&node, NEL3D_APPDATA_REMANENCE_SAMPLING_PERIOD, 0.02f);
	float rollupRatio    = getScriptAppData (&node, NEL3D_APPDATA_REMANENCE_ROLLUP_RATIO, 1.f);

	if (samplingPeriod <= 0.f) samplingPeriod = 0.02f;
	if (numSlices <= 2) numSlices = 2;
	if (rollupRatio <= 0) rollupRatio = 1.f;
	
	srs->setNumSlices((uint32) numSlices);
	srs->setSliceTime(samplingPeriod);
	srs->setRollupRatio(rollupRatio);

	// get material from this node
	std::vector<NL3D::CMaterial> materials;
	CMaxMeshBaseBuild mmbb;
	buildMaterials(materials, mmbb, node, time);
	if (materials.size() != 1)
	{	
		buildRemanenceError(this, node, "The remanent segment %s should have a single material");
		return NULL;
	}
	srs->setMaterial(materials[0]);
	// 
	// get geometry

	ObjectState os = node.EvalWorldState(_Ip->GetTime());		
	Object *obj = node.EvalWorldState(time).obj;	
	if (obj->SuperClassID() != SHAPE_CLASS_ID)
	{
		buildRemanenceError(this, node, "Can't get curves from %s");
		return NULL;
	}
	
	ShapeObject *so = (ShapeObject *) obj;	
	if (so->NumberOfCurves() != 1 || so->NumberOfPieces(time, 0) == 0)
	{
		buildRemanenceError(this, node, "Remanence export : %s should only contain one curve with at least one segment!");		
		return NULL;
	}

	int numPieces = so->NumberOfPieces(time, 0);
	srs->setNumCorners(numPieces + 1);

	// build offset matrix
	Matrix3 invNodeTM = node.GetNodeTM(time);
	invNodeTM.Invert();

	// Get the object matrix
	Matrix3 objectTM = node.GetObjectTM(time);

	// Compute the local to world matrix
	Matrix3 objectToLocal = objectTM*invNodeTM;	


	for(uint k = 0; k <= (uint) numPieces; ++k)
	{
		Point3 pos;		
		pos = (k == 0) ? so->InterpPiece3D(time, 0, 0, 0.f)
					   : so->InterpPiece3D(time, 0, k - 1, 1.f);		
		NLMISC::CVector nelPos;		
		convertVector(nelPos, objectToLocal * pos);
		srs->setCorner(k, nelPos);
	}

	
	srs->setTextureShifting(CExportNel::getScriptAppData (&node, NEL3D_APPDATA_REMANENCE_SHIFTING_TEXTURE, 0) != 0);	


	if (CExportNel::getScriptAppData (&node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, 0) != 0)
	{
		srs->setAnimatedMaterial(mmbb.MaterialInfo[0].MaterialName);
	}					
	

	// ********************************
	// *** Export default transformation
	// ********************************

	// Get the node matrix
	Matrix3 localTM;
	getLocalMatrix (localTM, node, time);

	// Get the translation, rotation, scale of the node
	CVector pos, scale;
	CQuat rot;
	decompMatrix (scale, rot, pos, localTM);

	// Set the default values
	srs->getDefaultPos()->setDefaultValue(pos);					
	srs->getDefaultScale()->setDefaultValue(scale);					
	srs->getDefaultRotQuat()->setDefaultValue(rot);


	return srs.release();

	/*ObjectState os = node.EvalWorldState(_Ip->GetTime());		
	Object *obj = node.EvalWorldState(time).obj;	
	TriObject *tri = (TriObject *) obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));	
	bool deleteIt=false;
	if (obj != tri) 
		deleteIt = true;
	Mesh *pMesh = &tri->mesh;
	if (pMesh && pMesh->numVerts > 1) 
	{
	  	Point3 v = pMesh->getVert(0);
		NLMISC::CVector minV, maxV;
		convertVector(minV, v);
		maxV = minV;
		for(uint k = 1; k < (uint) pMesh->numVerts; ++k)
		{
			NLMISC::CVector nv;
			v = pMesh->getVert(k);
			convertVector(nv, v);
			maxV.maxof(maxV, nv);
			minV.minof(minV, nv);
		}
		srs->setSeg(0, minV);
		srs->setSeg(1, maxV);	
	}
	else
	{
		buildRemanenceError(this, node, "Can't get mesh from %s or empty mesh");
	}*/

	//	
}



