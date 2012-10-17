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


#include "stdafx.h"			// Precompiled header
#include "export_nel.h"
#include "export_appdata.h"
#include "calc_lm_rt.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;


// ***********************************************************************************************
// CRTWorld
// ***********************************************************************************************

// -----------------------------------------------------------------------------------------------
CRTWorld::CRTWorld (bool errorInDialog, bool view, bool absolutePath, Interface *ip, std::string errorTitle, CExportNel *exp)
{
	_Ip = ip;
	_AbsolutePath = absolutePath;
	_View = view;
	_ErrorInDialog = errorInDialog;
	_ErrorTitle = errorTitle;
	_Export = exp;
}

// -----------------------------------------------------------------------------------------------
CRTWorld::~CRTWorld()
{
	uint32 i;

	for (i = 0; i < vMB.size(); ++i)
	{
		delete vMB[i];
		delete vMBB[i];
	}

	for (i = 0; i < vLightAccel.size(); ++i)
		if (vLightAccel[i] != NULL)
			delete vLightAccel[i];
}

// -----------------------------------------------------------------------------------------------
void CRTWorld::build (vector<SLightBuild> &AllLights, CVector &trans, bool bExcludeNonSelected, const set<INode*> &excludeNode, const set<INode*> &includeNode)
{
	uint32 i, j, k;
	TTicks ttTemp = CTime::getPerformanceTime();

	DWORD t = timeGetTime();

	GlobalTrans = trans;
	// Get all the nodes in the scene
	if( bExcludeNonSelected )
		getAllSelectedNode (vMB, vMBB, vINode, AllLights, excludeNode, includeNode);
	else
		getAllNodeInScene (vMB, vMBB, vINode, AllLights, excludeNode);

	// Transform the meshbuilds vertices and normals to have world coordinates
	for( i = 0; i < vMB.size(); ++i )
	{
		convertToWorldCoordinate (vMB[i], vMBB[i], GlobalTrans);
	}

	if (InfoLog)
		InfoLog->display("Object Construction : %d ms\n", timeGetTime()-t);

	// Construct all cube grids from all lights
	//wrt.cgAccel.resize( AllLights.size() );
	//wrt.dirAccel.resize( AllLights.size() );

	vLight.resize (AllLights.size());
	vLightAccel.resize (AllLights.size());

	for (i = 0; i < AllLights.size(); ++i)
	{
		SLightBuild &rLight = AllLights[i];
		vLight[i].Position = rLight.Position;
		vLight[i].Direction = rLight.Direction;
		vLight[i].rSoftShadowRadius = rLight.rSoftShadowRadius;
		vLight[i].rSoftShadowConeLength = rLight.rSoftShadowConeLength;
		vLightAccel[i] = NULL;

		switch (rLight.Type)
		{
			// ------------------------
			case SLightBuild::LightAmbient:
				vLight[i].Type = CRTLight::RTLightAmbient;
			break;

			// --------------------------
			case SLightBuild::LightSpot: // For the moment spot like point
				vLight[i].Type = CRTLight::RTLightSpot;
			case SLightBuild::LightPoint:
			{
				vLight[i].Type = CRTLight::RTLightPoint;
				CRTLightAccelPoint *pLAP = new CRTLightAccelPoint;
				pLAP->create( 64 ); // width of each grid in number of square
				for( j = 0; j < vMB.size(); ++j )
				{
					if (rLight.setExclusion.find (vINode[j]->GetName()) != rLight.setExclusion.end()) 
						continue;
					
					for (k = 0; k < vMB[j]->Faces.size(); ++k)
					{
						SGridCell cell;
						cell.pF = &(vMB[j]->Faces[k]);
						cell.pMB = vMB[j];
						cell.pMBB = vMBB[j];
						CTriangle tri = CTriangle( 
							cell.pMB->Vertices[cell.pF->Corner[0].Vertex] - rLight.Position,
							cell.pMB->Vertices[cell.pF->Corner[1].Vertex] - rLight.Position,
							cell.pMB->Vertices[cell.pF->Corner[2].Vertex] - rLight.Position );
						if (intersectionTriangleSphere (tri, CBSphere(CVector(0,0,0), rLight.rRadiusMax)))
							pLAP->insert( tri, cell );
					}
				}
				vLightAccel[i] = pLAP;
			}
			break;

			// ------------------------
			case SLightBuild::LightDir:
			{
				vLight[i].Type = CRTLight::RTLightDir;
				CRTLightAccelDir *pLAD = new CRTLightAccelDir;
				pLAD->create (64, rLight.rDirRadius/64.0f, rLight.Direction);
				for( j = 0; j < vMB.size(); ++j )
				{
					if (rLight.setExclusion.find (vINode[j]->GetName()) != rLight.setExclusion.end())
						continue;
					
					for (k = 0; k < vMB[j]->Faces.size(); ++k)
					{
						SGridCell cell;
						cell.pF = &(vMB[j]->Faces[k]);
						cell.pMB = vMB[j];
						cell.pMBB = vMBB[j];
						CTriangle tri = CTriangle( 
							cell.pMB->Vertices[cell.pF->Corner[0].Vertex],
							cell.pMB->Vertices[cell.pF->Corner[1].Vertex],
							cell.pMB->Vertices[cell.pF->Corner[2].Vertex] );
						// Convert the triangle into a sphere
						CBSphere s;
						s.Center = (tri.V0 + tri.V1 + tri.V2)/3.0f;
						s.Radius = (tri.V0-s.Center).norm();
						float tmp = (tri.V1-s.Center).norm();
						if (tmp > s.Radius) s.Radius = tmp;
						tmp = (tri.V2-s.Center).norm();
						if (tmp > s.Radius) s.Radius = tmp;

						if (intersectionSphereCylinder (s, rLight.Position, rLight.Direction, rLight.rDirRadius))
						{
							float t = ((s.Center - rLight.Position)*rLight.Direction) / (rLight.Direction*rLight.Direction);

							if ((t-s.Radius) < pLAD->rMin)
								pLAD->rMin = t - s.Radius;

							if ((t+s.Radius) > pLAD->rMax)
								pLAD->rMax = t + s.Radius;

							pLAD->insert (tri, cell);
						}
					}
				}
				vLightAccel[i] = pLAD;
			}
			break;
		}
	}
}

// -----------------------------------------------------------------------------------------------
CRGBAF CRTWorld::raytrace (NLMISC::CVector &vVertex, sint32 nLightNb, uint8& rtVal, bool bSoftShadow)
{
	CRGBAF Factor = CRGBAF(1.0f, 1.0f, 1.0f, 1.0f);
	CVector vVertexPos, vLightPos, vSelect;

	CRTLight &rLight = vLight[nLightNb];
	switch (rLight.Type)
	{
		case SLightBuild::LightAmbient:
			Factor = CRGBAF(1.0f, 1.0f, 1.0f, 1.0f);
		break;
		case SLightBuild::LightSpot:
		case SLightBuild::LightPoint:
		{
			CVector light_p = vVertex - rLight.Position;
			float light_p_distance = light_p.norm();
			light_p_distance = light_p_distance - (0.01f+(0.05f*light_p_distance/100.0f)); // Substract n centimeter
			light_p.normalize();
			light_p *= light_p_distance;
			vLightPos = rLight.Position;
			vVertexPos = rLight.Position + light_p;
			vSelect = vVertexPos - vLightPos;
		}
		break;
		case SLightBuild::LightDir:
		{
			float lightdist = ((vVertex - rLight.Position)*rLight.Direction) / (rLight.Direction.norm());
			CRTLightAccelDir *pLAD = (CRTLightAccelDir*)(vLightAccel[nLightNb]);
			lightdist = lightdist - pLAD->rMin;
			vLightPos = vVertex - rLight.Direction*lightdist;
			vVertexPos = vVertex - (0.01f+(0.05f*lightdist/100.0f))*rLight.Direction;
			vSelect = vVertexPos;
		}
		break;
		default:
		break;
	}

	float rSoftShadowRadius = rLight.rSoftShadowRadius;

	// Work with the directionnal light only for now
	if ((bSoftShadow) && (rLight.Type == SLightBuild::LightDir))
	{
		CRTRay RayOfLight;
		float rAreaInit;

		RayOfLight.initDirectionnal(8, vVertex, rLight.Direction, rSoftShadowRadius, rLight.rSoftShadowConeLength);
		rAreaInit = RayOfLight.getArea();
		vLightAccel[nLightNb]->select (vSelect, rSoftShadowRadius);
		while (!vLightAccel[nLightNb]->isEndSel())
		{
			// Get selected element
			SGridCell cell = vLightAccel[nLightNb]->getSel();

			CTriangle t(cell.pMB->Vertices[cell.pF->Corner[0].Vertex],
						cell.pMB->Vertices[cell.pF->Corner[1].Vertex],
						cell.pMB->Vertices[cell.pF->Corner[2].Vertex] );

			RayOfLight.clip (t);

			if (RayOfLight.Shapes.size() == 0)
				return CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
			
			// Next selected element
			vLightAccel[nLightNb]->nextSel();
		}

		float rVal = RayOfLight.getArea() / rAreaInit; // rVal = AreaNotShadowed / AreaInit
		
		return CRGBAF(rVal, rVal, rVal, 255.0f);
	}
	else // Normal shadowing
	{
		vLightAccel[nLightNb]->select (vSelect);
		while (!vLightAccel[nLightNb]->isEndSel())
		{
			// Get selected element
			SGridCell cell = vLightAccel[nLightNb]->getSel();

			testCell (Factor, cell, vLightPos, vVertexPos, rtVal);
			
			if ((Factor.R == 0.0f) &&
				(Factor.G == 0.0f) &&
				(Factor.B == 0.0f))
				return Factor;
			
			// Next selected element
			vLightAccel[nLightNb]->nextSel();
		}
	}

	return Factor;
}

// -----------------------------------------------------------------------------------------------
void CRTWorld::testCell (CRGBAF &retValue, SGridCell &cell, CVector &vLightPos, CVector &vVertexPos, uint8& rtVal)
{
	CVector hit;

	CTriangle t(cell.pMB->Vertices[cell.pF->Corner[0].Vertex],
				cell.pMB->Vertices[cell.pF->Corner[1].Vertex],
				cell.pMB->Vertices[cell.pF->Corner[2].Vertex] );
	CPlane plane;
	plane.make( t.V0, t.V1, t.V2 );

	if( t.intersect( vLightPos, vVertexPos, hit, plane ) )
	{
		if( cell.pMBB->Materials[cell.pF->MaterialId].getBlend() ||
			cell.pMBB->Materials[cell.pF->MaterialId].getAlphaTest() )
		{ // This is a transparent face we have to look in the texture
			ITexture *pT = cell.pMBB->Materials[cell.pF->MaterialId].getTexture(0);
			CRGBAF cPixMap;
			if( pT == NULL )
			{
				retValue *= 1.0f - (cell.pMBB->Materials[cell.pF->MaterialId].getOpacity()/255.0f);
				cPixMap = CRGBAF(1.0f, 1.0f, 1.0f, 0.0f);
			}
			else
			{
				CVector gradU, gradV;
				t.computeGradient(	cell.pF->Corner[0].Uvws[0].U,
									cell.pF->Corner[1].Uvws[0].U,
									cell.pF->Corner[2].Uvws[0].U, gradU );
				t.computeGradient(	cell.pF->Corner[0].Uvws[0].V,
									cell.pF->Corner[1].Uvws[0].V,
									cell.pF->Corner[2].Uvws[0].V, gradV );
				float u = cell.pF->Corner[0].Uvws[0].U+gradU*(hit-t.V0);
				float v = cell.pF->Corner[0].Uvws[0].V+gradV*(hit-t.V0);
				u = fmodf( u, 1.0f ); if( u < 0.0f ) u += 1.0f;
				v = fmodf( v, 1.0f ); if( v < 0.0f ) v += 1.0f;

				if( pT->getWidth() == 0 )
					((CTextureFile*)pT)->generate();
				cPixMap = pT->getColor( u,v );
			}
			cPixMap.A *= cell.pMBB->Materials[cell.pF->MaterialId].getOpacity()/255.0f;
			cPixMap.R *= cell.pMBB->Materials[cell.pF->MaterialId].getDiffuse().R/255.0f;
			cPixMap.G *= cell.pMBB->Materials[cell.pF->MaterialId].getDiffuse().G/255.0f;
			cPixMap.B *= cell.pMBB->Materials[cell.pF->MaterialId].getDiffuse().B/255.0f;
			if (cell.pMBB->Materials[cell.pF->MaterialId].getStainedGlassWindow())
			{
				retValue = (1.0f - cPixMap.A)*(	retValue*(1.0f-cPixMap.A) + 
												retValue*cPixMap*cPixMap.A );
			}
			else
			{
				retValue *= (1.0f - cPixMap.A);
			}
			rtVal = 255;
		}
		else
		{ // This is not a transparent face so if we intersect we get shadow
			if( rtVal < 255 ) rtVal += 1;
			retValue = CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
}

// -----------------------------------------------------------------------------------------------
void CRTWorld::addNode (INode *pNode, vector< CMesh::CMeshBuild* > &Meshes,  vector< CMeshBase::CMeshBaseBuild* > &MeshesBase,
						vector< INode* > &INodes, vector<SLightBuild> &AllLights, const set<INode*> &excludeNode, TimeValue tvTime)
{
	if (! RPO::isZone (*pNode, tvTime) )
	if (CExportNel::isMesh (*pNode, tvTime))
	{
		// Nel export
		CExportNel exportNel (_ErrorInDialog, _View, true, _Ip, _ErrorTitle, NULL);
		CAABBox aabbox;
		exportNel.buildMeshAABBox(*pNode, aabbox, tvTime);
		aabbox.setCenter(aabbox.getCenter()+GlobalTrans);

		// Not an excluded node ?
		bool bInteract = false;
		if (excludeNode.find (pNode) == excludeNode.end())
		{
			for( uint32 i = 0; i < AllLights.size(); ++i )
			{
				if( isInteractionWithLight (AllLights[i], aabbox))
				{
					bInteract = true;
					break;
				}
			}
		}


		if( bInteract )
		{
			int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);
			if ((nAccelType&3) == 0) // If not an accelerator
			{
				CMesh::CMeshBuild *pMB;
				CMeshBase::CMeshBaseBuild *pMBB;
				pMB = exportNel.createMeshBuild ( *pNode, tvTime, pMBB);
				if( pMBB->bCastShadows )
				{
					Meshes.push_back( pMB );
					MeshesBase.push_back( pMBB );
					INodes.push_back( pNode );
				}
				else
				{
					delete pMB; // No interaction so delete the mesh
					delete pMBB; // No interaction so delete the mesh
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------------------------
void CRTWorld::getAllSelectedNode	(vector< CMesh::CMeshBuild* > &Meshes,  
									vector< CMeshBase::CMeshBaseBuild* > &MeshesBase,
									vector< INode* > &INodes,
									vector<SLightBuild> &AllLights, const set<INode*> &excludeNode, const set<INode*> &includeNode)
{
	// Get time
	TimeValue tvTime = _Ip->GetTime();

	// Get node count
	int nNumSelNode = _Ip->GetSelNodeCount();
	// Save all selected objects
	for (int nNode=0; nNode<nNumSelNode; nNode++)
	{
		// Get the node 
		INode* pNode = _Ip->GetSelNode (nNode);

		// Already in the include list ?
		if (includeNode.find (pNode) == includeNode.end())
		{
			// Add the node
			addNode (pNode, Meshes,  MeshesBase, INodes, AllLights, excludeNode, tvTime);
		}
	}

	// Add the included 
	set<INode*>::const_iterator ite = includeNode.begin();
	while (ite != includeNode.end())
	{
		// Add the included object
		addNode (*ite, Meshes, MeshesBase, INodes, AllLights, excludeNode, tvTime);

		// Next node
		ite++;
	}
}

// -----------------------------------------------------------------------------------------------
void CRTWorld::getAllNodeInScene	(vector< CMesh::CMeshBuild* > &Meshes, 
									vector< CMeshBase::CMeshBaseBuild* > &BaseMeshes,
									vector< INode* > &INodes,
									vector<SLightBuild> &AllLights, const set<INode*> &excludeNode,
									INode* pNode)
{
	if( pNode == NULL )
		pNode = _Ip->GetRootNode();

	// Get a pointer on the object's node
	TimeValue tvTime = _Ip->GetTime();

	// Add the node
	addNode (pNode, Meshes,  BaseMeshes, INodes, AllLights, excludeNode, tvTime);

	for( sint32 i = 0; i < pNode->NumberOfChildren(); ++i )
		getAllNodeInScene( Meshes, BaseMeshes, INodes, AllLights, excludeNode, pNode->GetChildNode(i) );
}

// -----------------------------------------------------------------------------------------------
bool CRTWorld::intersectionTriangleSphere( CTriangle &t, CBSphere &s )
{
	// if a vertex of the triangle is in the sphere
	CVector v = t.V0 - s.Center;
	float f = v.norm();
	if( f < s.Radius )
		return true;
	v = t.V1 - s.Center;
	f = v.norm();
	if( f < s.Radius )
		return true;
	v = t.V2 - s.Center;
	f = v.norm();
	if( f < s.Radius )
		return true;
	// Ok sonow project the center of the triangle on the plane
	CPlane p;
	p.make( t.V0, t.V1, t.V2 );
	p.normalize();
	
	CVector newCenter = p.project( s.Center );
	v = newCenter - s.Center;
	float newRadius = v.norm() / s.Radius;
	if( newRadius > 1.0 )
		newRadius = 1.0;
	newRadius = cosf( newRadius * PI / 2.0f );

	CVector n = p.getNormal();
	CPlane p2;
	p2.make( t.V0, t.V1, t.V0 + n ); p2.normalize();
	f = p2*newCenter;
	p2.make( t.V1, t.V2, t.V1 + n ); p2.normalize();
	float f2 = p2*newCenter;
	p2.make( t.V2, t.V0, t.V2 + n ); p2.normalize();
	float f3 = p2*newCenter;

	// Is the newcenter insied the triangle ?
	if( ( f <= 0.0 ) && ( f2 <= 0.0 ) && ( f3 <= 0.0 ) )
		return true;
	if( ( f >= 0.0 ) && ( f2 >= 0.0 ) && ( f3 >= 0.0 ) )
		return true;

	// Is the newCenter at a distance < newradius from one of the triangle edge ?
	if( ( fabs(f) < newRadius ) || ( fabs(f2) < newRadius ) || ( fabs(f3) < newRadius ) )
		return true;
	return false;
}

// -----------------------------------------------------------------------------------------------
bool CRTWorld::intersectionSphereCylinder (CBSphere &s, CVector &cyCenter, CVector &cyDir, float cyRadius)
{
	float t = ((s.Center - cyCenter)*cyDir) / (cyDir*cyDir);
	CVector Xp = cyCenter + t*cyDir;
	float d = (s.Center-Xp).norm();
	if (d <= (cyRadius+s.Radius))
		return true;
	else
		return false;
}

// -----------------------------------------------------------------------------------------------
bool CRTWorld::isInteractionWithLight (SLightBuild &rSLB, CAABBox &meshBox)
{
	switch( rSLB.Type )
	{
		case SLightBuild::LightAmbient: // No need an ambient light...
			return false;
		break;
		case SLightBuild::LightSpot: // For the moment spot like point
		case SLightBuild::LightPoint:
		{
			// Get light bbox
			CAABBox lightBox;
			lightBox.setCenter( rSLB.Position );
			lightBox.extend( rSLB.Position - CVector(rSLB.rRadiusMax,0,0) );
			lightBox.extend( rSLB.Position + CVector(rSLB.rRadiusMax,0,0) );
			lightBox.extend( rSLB.Position - CVector(0,rSLB.rRadiusMax,0) );
			lightBox.extend( rSLB.Position + CVector(0,rSLB.rRadiusMax,0) );
			lightBox.extend( rSLB.Position - CVector(0,0,rSLB.rRadiusMax) );
			lightBox.extend( rSLB.Position + CVector(0,0,rSLB.rRadiusMax) );

			if( lightBox.intersect( meshBox ) )
				return true;

			if( meshBox.include( lightBox.getMin() ) )
				return true;

			if( lightBox.include( meshBox.getMin() ) )
				return true;
		}
		break;
		case SLightBuild::LightDir:
		{
			// Use acceleration for dir light to not select all nodes in scene
			CBSphere s;
			s.Radius = meshBox.getHalfSize().norm();
			s.Center = meshBox.getCenter();
			// Test against the cylinder
			if (intersectionSphereCylinder (s, rSLB.Position, rSLB.Direction, rSLB.rDirRadius))
				return true;
		}
		break;
	}
	return false;
}

// -----------------------------------------------------------------------------------------------
bool CRTWorld::isInteractionLightMesh (SLightBuild &rSLB, NL3D::CMesh::CMeshBuild &rMB, NL3D::CMeshBase::CMeshBaseBuild &rMBB)
{
	if (rSLB.Type == SLightBuild::LightAmbient)
		return false;

	// Get the mesh bbox. Warning the vertices need to be transformed.
	CAABBox meshBox;
	CMatrix MBMatrix = getObjectToWorldMatrix (&rMB, &rMBB);
	MBMatrix.movePos (GlobalTrans);

	for( uint32 j = 0; j < rMB.Vertices.size(); ++j )
		if( j == 0 )
			meshBox.setCenter( MBMatrix * rMB.Vertices[j] );
		else
			meshBox.extend( MBMatrix * rMB.Vertices[j] );

	return isInteractionWithLight(rSLB, meshBox);
}


// ***********************************************************************************************
// Ray representation for soft shadow
// ***********************************************************************************************


// -----------------------------------------------------------------------------------------------
void CRTRay::initDirectionnal	(uint32 nNbSide, const NLMISC::CVector &vVertex, const NLMISC::CVector &lightDir, float rRadius, float rDistCyl)
{
	// Construct the matrix with K= vertex to light direction
	CVector vDirection = -lightDir;
	vDirection.normalize();
	CVector	I = (fabs(vDirection*CVector(1.f,0,0))>0.99)?CVector(0.f,1.f,0.f):CVector(1.f,0.f,0.f);
	CVector	K = vDirection;
	CVector	J = K^I;
	J.normalize();
	I=J^K;
	I.normalize();

	// Setup the Inv Vertex Basis
	_InvVertexMat.identity();
	_InvVertexMat.setRot (I, J, K, true);
	_InvVertexMat.setPos (vVertex);
	_InvVertexMat.invert();
	
	// Build the clipping pyramids, in vertex space
	CVector		lb(-rRadius, -rRadius, rDistCyl);
	CVector		lt(-rRadius, rRadius, rDistCyl);
	CVector		rb(rRadius, -rRadius, rDistCyl);
	CVector		rt(rRadius, rRadius, rDistCyl);
	// Cone pyramid: perspective and capped at near (for precision and z division) and far (where start the cylinder)
	_ConePyramid[0].make(-CVector::K, CVector(0,0,0.01f));		// near
	_ConePyramid[1].make( CVector::K, CVector(0,0,rDistCyl));	// far
	// NB: don't need the full clip (faster)
	/*_ConePyramid[2].make(CVector::Null, lt, lb);	// left
	_ConePyramid[3].make(CVector::Null, rt, lt);	// top
	_ConePyramid[4].make(CVector::Null, rb, rt);	// right
	_ConePyramid[5].make(CVector::Null, lb, rb);	// bottom*/
	// Cylinder pyramid: orthogonal and capped at near (ie rDistCyl, where start the cylinder) only
	_CylinderPyramid[0].make(-CVector::K, CVector(0,0,rDistCyl));	// near
	// NB: don't need the full clip (faster)
	/*_CylinderPyramid[1].make(-CVector::I, lb);	// left
	_CylinderPyramid[2].make( CVector::I, rb);	// right
	_CylinderPyramid[3].make(-CVector::J, lb);	// bottom
	_CylinderPyramid[4].make( CVector::J, lt);	// top*/
	

	// Build the initial shape
	Shapes.resize (1);
	SConvexShape &rShp = Shapes[0];
	rShp.Vertices.resize (nNbSide);
	for (uint32 i = 0; i < rShp.Vertices.size(); ++i)
	{
		rShp.Vertices[i].U = rRadius * cosf(i*2.0f*(float)Pi / rShp.Vertices.size());
		rShp.Vertices[i].V = rRadius * sinf(i*2.0f*(float)Pi / rShp.Vertices.size());
	}
	_DistCyl = rDistCyl;
}

// -----------------------------------------------------------------------------------------------
void CRTRay::clip (const CTriangle& t)
{
	sint	i;

	CVector	tv0= _InvVertexMat.mulPoint (t.V0);
	CVector	tv1= _InvVertexMat.mulPoint (t.V1);
	CVector	tv2= _InvVertexMat.mulPoint (t.V2);


	// **** Contribution in the cone part
	nlctassert(NumConePlanes>NumCylinderPlanes);
	CVector		apolyIn[3+NumConePlanes];
	CVector		apolyOut[3+NumConePlanes];
	CVector		*polyIn= apolyIn;
	CVector		*polyOut= apolyOut;
	sint		numVerts;
	polyIn[0]= tv0;
	polyIn[1]= tv1;
	polyIn[2]= tv2;
	numVerts= 3;
	for(i=0;i<NumConePlanes;i++)
	{
		numVerts= _ConePyramid[i].clipPolygonBack(polyIn, polyOut, numVerts);
		swap(polyIn, polyOut);
		// entirely out
		if(!numVerts)
			break;
	}
	// for all result triangles
	for(i=0;i<numVerts-2;i++)
	{
		CVector	v0= polyIn[0];
		CVector	v1= polyIn[i+1];
		CVector	v2= polyIn[i+2];
		// cone: project at z=distCyl
		v0= v0 * _DistCyl / v0.z;
		v1= v1 * _DistCyl / v1.z;
		v2= v2 * _DistCyl / v2.z;
		
		clipProjected(v0,v1,v2);
	}
	

	// **** Contribution in the cylinder part
	polyIn[0]= tv0;
	polyIn[1]= tv1;
	polyIn[2]= tv2;
	numVerts= 3;
	for(i=0;i<NumCylinderPlanes;i++)
	{
		numVerts= _CylinderPyramid[i].clipPolygonBack(polyIn, polyOut, numVerts);
		swap(polyIn, polyOut);
		// entirely out
		if(!numVerts)
			break;
	}
	// for all result triangles
	for(i=0;i<numVerts-2;i++)
	{
		CVector	v0= polyIn[0];
		CVector	v1= polyIn[i+1];
		CVector	v2= polyIn[i+2];
		// cylinder: no projection
		
		clipProjected(v0,v1,v2);
	}
	
}

// -----------------------------------------------------------------------------------------------
void	CRTRay::clipProjected(const NLMISC::CVector &v0, const NLMISC::CVector &v1, const NLMISC::CVector &v2)
{
	CUV	tri[3];

	tri[0].U = v0.x;	tri[0].V = v0.y;
	tri[1].U = v2.x;	tri[1].V = v2.y;
	tri[2].U = v1.x;	tri[2].V = v1.y;
	
	// Backface swapping
	if (((tri[1].U-tri[0].U) * (tri[2].V-tri[0].V) - (tri[1].V-tri[0].V) * (tri[2].U-tri[0].U)) < 0.0f)
	{
		swap(tri[0],tri[1]);
	}
	
	
	// **** Clip the convex shapes
	std::vector<SConvexShape> shapesOut;
	for (uint32 i = 0; i < Shapes.size(); ++i)
	{
		SConvexShape &rShp = Shapes[i];
		
		// Clip each shapes with the triangle
		if (isShapeMustBeClippedByTriangle (rShp, tri))
		{
			std::vector<SConvexShape> shapesTmp;
			shapesTmp.resize (0);
			clipShape (rShp, tri, shapesTmp);
			for (uint32 j = 0; j < shapesTmp.size(); ++j)
				shapesOut.push_back (shapesTmp[j]);
		}
		else // The shape is not clipped by the triangle so must be add it directly
		{
			shapesOut.push_back (rShp);
		}
	}
	
	// We are now like shapesOut
	Shapes = shapesOut;
}

// -----------------------------------------------------------------------------------------------
float CRTRay::getArea ()
{
	float rSize = 0.0f;

	// For all shapes
	for (uint32 i = 0; i < Shapes.size(); ++i)
	{
		SConvexShape &rShp = Shapes[i];

		// Get all triangles from the shapes	
		for (uint32 j = 0; j < rShp.Vertices.size()-2; ++j)
		{
			CUV v1 = rShp.Vertices[j+1] - rShp.Vertices[0];
			CUV v2 = rShp.Vertices[j+2] - rShp.Vertices[0];
			
			rSize += 0.5f * fabsf(v1.U*v2.V - v1.V*v2.U);
		}
	}

	return rSize;
}

// -----------------------------------------------------------------------------------------------
bool CRTRay::isShapeMustBeClippedByTriangle (CRTRay::SConvexShape& scs, CUV tri[3])
{
	uint32 i,j;
	bool bAllVertExt;
	// Is the convex shape vertices are all on a side of the triangle
	for (i = 0; i < 3; ++i)
	{
		CUV &p0 = tri[i];
		CUV &p1 = tri[(i+1)%3];
		bAllVertExt = true;
		for (j = 0; j < scs.Vertices.size(); ++j)
			// Does the vertex j is interior ?
			if (((p1.U-p0.U) * (scs.Vertices[j].V-p0.V) - (p1.V-p0.V) * (scs.Vertices[j].U-p0.U)) > 0.0f)
			{
				bAllVertExt = false; // Yes so all vertex are NOT exterior
				break;
			}
		if (bAllVertExt)
			return false; // Clip not needed because all vertices exterior
	}
	// Is all vertices of the triangle on a side of the convex shape side
	for (i = 0; i < scs.Vertices.size(); ++i)
	{
		CUV &p0 = scs.Vertices[i];
		CUV &p1 = scs.Vertices[(i+1)%scs.Vertices.size()];
		bAllVertExt = true;
		for (j = 0; j < 3; ++j)
			// Does the vertex j is interior ?
			if ((p1.U-p0.U) * (tri[j].V-p0.V) - (p1.V-p0.V) * (tri[j].U-p0.U) > 0.0f)
			{
				bAllVertExt = false; // Yes so all vertex are NOT exterior
				break;
			}
		if (bAllVertExt)
			return false; // Clip not needed because all vertices exterior
	}
	return true;
}

// -----------------------------------------------------------------------------------------------
void CRTRay::clipShape (CRTRay::SConvexShape& ShapeIn, CUV Tri[3], vector<CRTRay::SConvexShape> &ShapesOut)
{
	uint32 i, j, lastj;
	SConvexShape Front, Back, BackInput;
	BackInput = ShapeIn;

	// For each side of the triangle
	for (i = 0; i < 3; ++i)
	{
		CUV &sideV0 = Tri[i];
		CUV &sideV1 = Tri[(i+1)%3];

		// Front is the polygon exterior to the side (which does not contain the triangle)
		Front.Vertices.resize (0);
		Back.Vertices.resize (0);

		lastj = BackInput.Vertices.size()-1;
		bool previousExt = ((sideV1.U-sideV0.U) * (BackInput.Vertices[lastj].V-sideV0.V) 
							- (sideV1.V-sideV0.V) * (BackInput.Vertices[lastj].U-sideV0.U) < 0.0f);
		// BackInput -> Back, Front   split from the current edge
		for (j = 0; j < BackInput.Vertices.size(); ++j)
		{
			bool currentExt = ((sideV1.U-sideV0.U) * (BackInput.Vertices[j].V-sideV0.V) 
								- (sideV1.V-sideV0.V) * (BackInput.Vertices[j].U-sideV0.U) < 0.0f);
			if (currentExt)
			{
				// Vertex j exterior
				if (!previousExt) // Previous vertex interior ?
				{
					// Ok, intersect
					CUV tmp = getLineIntersection (BackInput.Vertices[lastj], BackInput.Vertices[j], sideV0, sideV1);
					Front.Vertices.push_back (tmp);
					Back.Vertices.push_back (tmp);
				}
				// Store new vertex front
				Front.Vertices.push_back (BackInput.Vertices[j]);
			}
			else
			{
				// Vertex j is interior
				if (previousExt) // Previous vertex exterior ?
				{
					// Ok, intersect
					CUV tmp = getLineIntersection (BackInput.Vertices[lastj], BackInput.Vertices[j], sideV0, sideV1);
					Front.Vertices.push_back (tmp);
					Back.Vertices.push_back (tmp);
				}
				// Store new vertex back
				Back.Vertices.push_back (BackInput.Vertices[j]);
			}
			previousExt = currentExt;
			lastj = j;
		}

		// Run through the front and back to weld vertices
		weldVertices (Front);
		weldVertices (Back);

		if(Front.Vertices.size() > 2)
			// Front is a poly that we will out
			ShapesOut.push_back (Front);

		// Get the back poly to the next iteration
		BackInput = Back;
		
		if (BackInput.Vertices.size() < 3)
			return;
	}
}

// -----------------------------------------------------------------------------------------------
void CRTRay::weldVertices (CRTRay::SConvexShape& Shp)
{
	uint32 curVertex = 0;
	for (uint32 j = 0; j < Shp.Vertices.size(); ++j)
	{
		CUV &p0 = Shp.Vertices[j];
		CUV &p1 = Shp.Vertices[(j+1)%Shp.Vertices.size()];
		if (((p1.U-p0.U)*(p1.U-p0.U)+(p1.V-p0.V)*(p1.V-p0.V)) > 0.0001f)
		{
			Shp.Vertices[curVertex] = p0;
			curVertex++;
		}
	}
	Shp.Vertices.resize (curVertex);
}


// -----------------------------------------------------------------------------------------------
CUV CRTRay::getLineIntersection (const CUV &l1p1, const CUV &l1p2, const CUV &l2p1, const CUV &l2p2)
{
	float denominator = (l1p2.U-l1p1.U)*(l2p2.V-l2p1.V) - (l1p2.V-l1p1.V)*(l2p2.U-l2p1.U);
	if (denominator == 0.0)
		return l1p1; // The segment are colinear
	float k = ((l2p1.U-l1p1.U)*(l2p2.V-l2p1.V) - (l2p1.V-l1p1.V)*(l2p2.U-l2p1.U) ) / denominator;
	if (k <= 0.0f)
		return l1p1;
	if (k >= 1.0f)
		return l1p2;

	return l1p1 + (l1p2-l1p1) * k;
}

// ***********************************************************************************************
// Light accelerator for directionnal lights
// ***********************************************************************************************

// -----------------------------------------------------------------------------------------------
CRTLightAccelDir::CRTLightAccelDir()
{
	rMin = 100000.0f;
	rMax = -100000.0f;
}

// -----------------------------------------------------------------------------------------------
CRTLightAccelDir::~CRTLightAccelDir()
{
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelDir::create (int nSize, float rRadius, CVector &vDirection)
{
	grid.create	( nSize, 2*rRadius );

	CMatrix	tmp;
	CVector	I = (fabs(vDirection*CVector(1.f,0,0))>0.99)?CVector(0.f,1.f,0.f):CVector(1.f,0.f,0.f);
	CVector	K = vDirection;
	CVector	J = K^I;
	J.normalize();
	I=J^K;
	I.normalize();
	
	tmp.identity();
	tmp.setRot (I, J, K, true);
	invMat = tmp;		
	invMat.invert ();

	tmp.identity();
	grid.changeBase (tmp);
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelDir::insert (CTriangle &tri, SGridCell &cell)
{
	CVector vMin;
	CVector vMax;
	CTriangle t = tri;

	t.V0 = invMat.mulPoint (t.V0);
	t.V1 = invMat.mulPoint (t.V1);
	t.V2 = invMat.mulPoint (t.V2);

	vMin = t.V0; vMax = t.V0;
	
	if (vMin.x > t.V1.x) vMin.x = t.V1.x;
	if (vMin.y > t.V1.y) vMin.y = t.V1.y;
	if (vMin.z > t.V1.z) vMin.z = t.V1.z;
	if (vMin.x > t.V2.x) vMin.x = t.V2.x;
	if (vMin.y > t.V2.y) vMin.y = t.V2.y;
	if (vMin.z > t.V2.z) vMin.z = t.V2.z;

	if (vMax.x < t.V1.x) vMax.x = t.V1.x;
	if (vMax.y < t.V1.y) vMax.y = t.V1.y;
	if (vMax.z < t.V1.z) vMax.z = t.V1.z;
	if (vMax.x < t.V2.x) vMax.x = t.V2.x;
	if (vMax.y < t.V2.y) vMax.y = t.V2.y;
	if (vMax.z < t.V2.z) vMax.z = t.V2.z;

	grid.insert (vMin, vMax, cell);
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelDir::select (CVector &v)
{
	CVector vSel = v;
	vSel = invMat.mulPoint (vSel);
	grid.select (vSel, vSel);
	itSel = grid.begin();
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelDir::select (CVector &v, float rRadius)
{
	CVector vSelMin = v, vSelMax;
	vSelMin = invMat.mulPoint (vSelMin);
	vSelMax = vSelMin;

	vSelMin.x -= sqrtf(2.0f)*rRadius;
	vSelMin.y -= sqrtf(2.0f)*rRadius;
	vSelMax.x += sqrtf(2.0f)*rRadius;
	vSelMax.y += sqrtf(2.0f)*rRadius;

	grid.select (vSelMin, vSelMax);
	itSel = grid.begin();
}

// -----------------------------------------------------------------------------------------------
SGridCell CRTLightAccelDir::getSel ()
{
	return *itSel;
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelDir::nextSel ()
{
	++itSel;
}

// -----------------------------------------------------------------------------------------------
bool CRTLightAccelDir::isEndSel ()
{
	return (itSel == grid.end());
}

// ***********************************************************************************************
// Light accelerator for point and spot lights
// ***********************************************************************************************

// -----------------------------------------------------------------------------------------------
CRTLightAccelPoint::CRTLightAccelPoint ()
{
	CMatrix	tmp;
	CVector	I, J, K;

	// grids[kUp].changeBase(  );
	I = CVector(  1,  0,  0 );
	J = CVector(  0, -1,  0 );
	K = CVector(  0,  0, -1 );
	tmp.identity(); tmp.setRot( I, J, K, true );
	_Grids[kDown].changeBase( tmp );

	I = CVector(  0,  0,  1 );
	J = CVector(  0,  1,  0 );
	K = CVector( -1,  0,  0 );
	tmp.identity(); tmp.setRot( I, J, K, true);
	_Grids[kLeft].changeBase( tmp );

	I = CVector(  0,  0, -1 );
	J = CVector(  0,  1,  0 );
	K = CVector(  1,  0,  0 );
	tmp.identity(); tmp.setRot( I, J, K, true);
	_Grids[kRight].changeBase( tmp );

	I = CVector(  1,  0,  0 );
	J = CVector(  0,  0,  1 );
	K = CVector(  0, -1,  0 );
	tmp.identity(); tmp.setRot( I, J, K, true);
	_Grids[kFront].changeBase( tmp );

	I = CVector(  1,  0,  0 );
	J = CVector(  0,  0, -1 );
	K = CVector(  0,  1,  0 );
	tmp.identity(); tmp.setRot( I, J, K, true);
	_Grids[kBack].changeBase( tmp );
}

// -----------------------------------------------------------------------------------------------
CRTLightAccelPoint::~CRTLightAccelPoint ()
{
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelPoint::create (int nSize)
{
	_Grids[kUp].create		( nSize, 1.0f / ((float)nSize) );
	_Grids[kDown].create	( nSize, 1.0f / ((float)nSize) );
	_Grids[kLeft].create	( nSize, 1.0f / ((float)nSize) );
	_Grids[kRight].create	( nSize, 1.0f / ((float)nSize) );
	_Grids[kFront].create	( nSize, 1.0f / ((float)nSize) );
	_Grids[kBack].create	( nSize, 1.0f / ((float)nSize) );
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelPoint::insert (CTriangle &tri, SGridCell &cell)
{
	CPlane p[4], gp;
	// Construct clip pyramid for grid : UP
	p[0].make( CVector(0,0,0), CVector( -1,-1,+1 ), CVector( +1,-1,+1 ) );
	p[1].make( CVector(0,0,0), CVector( +1,-1,+1 ), CVector( +1,+1,+1 ) );
	p[2].make( CVector(0,0,0), CVector( +1,+1,+1 ), CVector( -1,+1,+1 ) );
	p[3].make( CVector(0,0,0), CVector( -1,+1,+1 ), CVector( -1,-1,+1 ) );
	gp.make( CVector(0,0,1), CVector(0,0,0.5) );
	project( tri, p, gp, kUp, cell );
	// Construct clip pyramid for grid : DOWN
	p[0].make( CVector(0,0,0), CVector( +1,-1,-1 ), CVector( -1,-1,-1 ) );
	p[1].make( CVector(0,0,0), CVector( -1,-1,-1 ), CVector( -1,+1,-1 ) );
	p[2].make( CVector(0,0,0), CVector( -1,+1,-1 ), CVector( +1,+1,-1 ) );
	p[3].make( CVector(0,0,0), CVector( +1,+1,-1 ), CVector( +1,-1,-1 ) );
	gp.make( CVector(0,0,-1), CVector(0,0,-0.5) );
	project( tri, p, gp, kDown, cell );
	// Construct clip pyramid for grid : LEFT
	p[0].make( CVector(0,0,0), CVector( -1,-1,-1 ), CVector( -1,-1,+1 ) );
	p[1].make( CVector(0,0,0), CVector( -1,-1,+1 ), CVector( -1,+1,+1 ) );
	p[2].make( CVector(0,0,0), CVector( -1,+1,+1 ), CVector( -1,+1,-1 ) );
	p[3].make( CVector(0,0,0), CVector( -1,+1,-1 ), CVector( -1,-1,-1 ) );
	gp.make( CVector(-1,0,0), CVector(-0.5,0,0) );
	project( tri, p, gp, kLeft, cell );
	// Construct clip pyramid for grid : RIGHT
	p[0].make( CVector(0,0,0), CVector( +1,-1,+1 ), CVector( +1,-1,-1 ) );
	p[1].make( CVector(0,0,0), CVector( +1,-1,-1 ), CVector( +1,+1,-1 ) );
	p[2].make( CVector(0,0,0), CVector( +1,+1,-1 ), CVector( +1,+1,+1 ) );
	p[3].make( CVector(0,0,0), CVector( +1,+1,+1 ), CVector( +1,-1,+1 ) );
	gp.make( CVector(1,0,0), CVector(0.5,0,0) );
	project( tri, p, gp, kRight, cell );
	// Construct clip pyramid for grid : FRONT
	p[0].make( CVector(0,0,0), CVector( -1,-1,-1 ), CVector( +1,-1,-1 ) );
	p[1].make( CVector(0,0,0), CVector( +1,-1,-1 ), CVector( +1,-1,+1 ) );
	p[2].make( CVector(0,0,0), CVector( +1,-1,+1 ), CVector( -1,-1,+1 ) );
	p[3].make( CVector(0,0,0), CVector( -1,-1,+1 ), CVector( -1,-1,-1 ) );
	gp.make( CVector(0,-1,0), CVector(0,-0.5,0) );
	project( tri, p, gp, kFront, cell );
	// Construct clip pyramid for grid : BACK
	p[0].make( CVector(0,0,0), CVector( +1,+1,+1 ), CVector( +1,+1,-1 ) );
	p[1].make( CVector(0,0,0), CVector( +1,+1,-1 ), CVector( -1,+1,-1 ) );
	p[2].make( CVector(0,0,0), CVector( -1,+1,-1 ), CVector( -1,+1,+1 ) );
	p[3].make( CVector(0,0,0), CVector( -1,+1,+1 ), CVector( +1,+1,+1 ) );
	gp.make( CVector(0,1,0), CVector(0,0.5,0) );
	project( tri, p, gp, kBack, cell );
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelPoint::select (CVector &v)
{
	SSelector sel;
	CPlane gp;
	// Get the plane
	if( ( -v.z <= v.x ) && ( v.x <= v.z ) &&
		( -v.z <= v.y ) && ( v.y <= v.z ) &&
		( 0.0f <= v.z ) )
	{
		sel.nSelGrid = kUp;
		gp.make( CVector(0,0,1), CVector(0,0,0.5) );
	}
	if( ( v.z <= v.x ) && ( v.x <= -v.z ) &&
		( v.z <= v.y ) && ( v.y <= -v.z ) &&
		( v.z <= 0.0f ) )
	{
		sel.nSelGrid = kDown;
		gp.make( CVector(0,0,-1), CVector(0,0,-0.5) );
	}
	if( ( v.x <= 0.0f ) &&
		( v.x <= v.y ) && ( v.y <= -v.x ) &&
		( v.x <= v.z ) && ( v.z <= -v.x ) )
	{
		sel.nSelGrid = kLeft;
		gp.make( CVector(-1,0,0), CVector(-0.5,0,0) );
	}
	if( ( 0.0f <= v.x ) &&
		( -v.x <= v.y ) && ( v.y <= v.x ) &&
		( -v.x <= v.z ) && ( v.z <= v.x ) )
	{
		sel.nSelGrid = kRight;
		gp.make( CVector(1,0,0), CVector(0.5,0,0) );
	}
	if( ( v.y <= v.x ) && ( v.x <= -v.y ) &&
		( v.y <= 0.0f ) &&
		( v.y <= v.z ) && ( v.z <= -v.y ) )
	{
		sel.nSelGrid = kFront;
	gp.make( CVector(0,-1,0), CVector(0,-0.5,0) );
	}
	if( ( -v.y <= v.x ) && ( v.x <= v.y ) &&
		( 0.0f <= v.y ) &&
		( -v.y <= v.z ) && ( v.z <= v.y ) )
	{
		sel.nSelGrid = kBack;
		gp.make( CVector(0,1,0), CVector(0,0.5,0) );
	}
	nlassert(sel.nSelGrid!=-1);
	CVector newV = gp.intersect( CVector(0,0,0), v );
	_Grids[sel.nSelGrid].select(newV, newV);
	sel.itSel = _Grids[sel.nSelGrid].begin();
	_Selection.clear();
	_Selection.push_back (sel);
	_CurSel = 0; // Point on the first non-empty grid
	while (	(_CurSel < _Selection.size()) &&
			(_Selection[_CurSel].itSel == _Grids[_Selection[_CurSel].nSelGrid].end()) )
		++_CurSel;
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelPoint::select (CVector &v, float rRadius)
{
	// Perpendicular projection of the circle on each plane


	// TODO

	// TODO

	_CurSel = 0; // Point on the first non-empty grid
	while (	(_CurSel < _Selection.size()) &&
			(_Selection[_CurSel].itSel == _Grids[_Selection[_CurSel].nSelGrid].end()) )
		++_CurSel;
}

// -----------------------------------------------------------------------------------------------
SGridCell CRTLightAccelPoint::getSel ()
{
	return *_Selection[_CurSel].itSel;
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelPoint::nextSel ()
{
	++(_Selection[_CurSel].itSel);
	while (	(_CurSel < _Selection.size()) &&
			(_Selection[_CurSel].itSel == _Grids[_Selection[_CurSel].nSelGrid].end()) )
		++_CurSel;
}

// -----------------------------------------------------------------------------------------------
bool CRTLightAccelPoint::isEndSel ()
{
	return (_CurSel == _Selection.size());
}

// -----------------------------------------------------------------------------------------------
void CRTLightAccelPoint::project (CTriangle &tri, CPlane pyr[4], CPlane &gridPlane, sint32 nGridNb, SGridCell &cell)
{
	CVector vIn[7], vOut[7];
	sint32 i, nOut;
	vIn[0] = tri.V0; vIn[1] = tri.V1; vIn[2] = tri.V2;
	nOut = pyr[0].clipPolygonFront( vIn, vOut, 3 );
	if( nOut == 0 ) return;
	for( i = 0; i < nOut; ++i ) vIn[i] = vOut[i];
	nOut = pyr[1].clipPolygonFront( vIn, vOut, nOut );
	if( nOut == 0 ) return;
	for( i = 0; i < nOut; ++i ) vIn[i] = vOut[i];
	nOut = pyr[2].clipPolygonFront( vIn, vOut, nOut );
	if( nOut == 0 ) return;
	for( i = 0; i < nOut; ++i ) vIn[i] = vOut[i];
	nOut = pyr[3].clipPolygonFront( vIn, vOut, nOut );
	if( nOut >= 3 )
	{
		CVector vMin(1, 1, 1), vMax(-1, -1, -1);
		for( i = 0; i < nOut; ++i )
		{
			vOut[i] = gridPlane.intersect( CVector(0, 0, 0), vOut[i] );
			if( vMin.x > vOut[i].x ) vMin.x = vOut[i].x;
			if( vMin.y > vOut[i].y ) vMin.y = vOut[i].y;
			if( vMin.z > vOut[i].z ) vMin.z = vOut[i].z;
			if( vMax.x < vOut[i].x ) vMax.x = vOut[i].x;
			if( vMax.y < vOut[i].y ) vMax.y = vOut[i].y;
			if( vMax.z < vOut[i].z ) vMax.z = vOut[i].z;
		}
		// Create the bbox
		_Grids[nGridNb].insert( vMin, vMax, cell );
	}
}

