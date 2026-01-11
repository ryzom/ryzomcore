// --------------------
// LIGHTMAP CALCULATION
// --------------------
// OLD VERSION 
// This version handled a group of object at one time to optimize the precalculation
// of the accelerated world raytrace structure


#include "nel/misc/time_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/common.h"
#include "nel/misc/triangle.h"
#include "nel/misc/bsphere.h"
#include "3d/quad_tree.h"
#include "3d/skeleton_shape.h"
#include "3d/texture_file.h"
#include "3d/light.h"

#include "nel_export_scene.h"

#include "3d/bsp_tree.h"
#include "3d/quad_grid.h"
// TOOLS
// *****

#define NEL_LIGHT_CLASS_ID_A	0x36e3181f
#define NEL_LIGHT_CLASS_ID_B	0x3ac24049

#define MAXLIGHTMAPSIZE 1024
//#define RATIOLIGHTMAP	0.25		// 0.25 meters in real world represent 1 Lumel in UV world

// -----------------------------------------------------------------------------------------------
struct SLightBuild
{
	string GroupName;
	enum EType { LightAmbient, LightPoint, LightDir, LightSpot };
	EType Type;
	CVector Position;				// Used by LightPoint and LightSpot
	CVector Direction;				// Used by LightSpot and LightDir
	float rRadiusMin, rRadiusMax;	// Used by LightPoint and LightSpot
	float rHotspot, rFallof;		// Used by LightSpot
	CRGBA Ambient;
	CRGBA Diffuse;
	CRGBA Specular;
	bool bAnimate;
	bool bCastShadow;
	float rMult;
	// ------------------------------------
	SLightBuild()
	{
		Type = LightPoint;
		Position = CVector(0.0, 0.0, 0.0);
		Direction = CVector(1.0, 0.0, 0.0);
		rRadiusMin = 1.0f;
		rRadiusMax = 2.0f;
		Ambient = CRGBA(0, 0, 0, 0);
		Diffuse = CRGBA(0, 0, 0, 0);
		Specular = CRGBA(0, 0, 0, 0);
		bAnimate = false; // TEMP MAT
		rMult = 1.0f;
	}
};

// -----------------------------------------------------------------------------------------------
struct SLMPixel
{
	CRGBAF p[8]; // 8 Layers of lightmap possible
};

// -----------------------------------------------------------------------------------------------
struct SLMPlane
{
	sint32 x, y; // Pos in lightmap
	sint32 w, h; // Size
	vector<uint8> msk;	// 0 - No pixel
						// 1 - Pixel must be calculated
						// 2 - Pixel is interior and is calculated
						// 3 - Pixel is exterior in this plane but interior in another of the same smooth group
						// 4 - Pixel is exterior and is extrapolated
	vector<SLMPixel> col; // 32 bits value for each pixel of each layer
	sint32 nNbLayerUsed;
	vector<CMesh::CFace*> faces;	
	// ------------------------------------
	SLMPlane()
	{ 
		nNbLayerUsed = 0;
		x = y = 0; w = h = 1; msk.resize(1); msk[0] = 0; 
		col.resize(1); 
		for( sint32 i = 0; i < 8; ++i )
		{ col[0].p[i].R = col[0].p[i].G = col[0].p[i].B = col[0].p[i].A = 0.0f; }
	}
};

// -----------------------------------------------------------------------------------------------
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
	// Initial u,v
	double InitU, InitV;
};

// -----------------------------------------------------------------------------------------------
struct SCubeGridCell
{
	CMesh::CFace* pF;
	CMesh::CMeshBuild* pMB;
};

// -----------------------------------------------------------------------------------------------
// Represent a cube made of grids centered on (0,0,0) with a size of 1
struct SCubeGrid
{
	enum gridPos { kUp = 0, kDown, kLeft, kRight, kFront, kBack };
	CQuadGrid<SCubeGridCell> grids[6];

	sint32 nSelGrid;
	CQuadGrid<SCubeGridCell>::CIterator itSel;

	// =======================================================================
	SCubeGrid()
	{
		CMatrix	tmp;
		CVector	I, J, K;

		// grids[kUp].changeBase(  );
		I = CVector(  1,  0,  0 );
		J = CVector(  0, -1,  0 );
		K = CVector(  0,  0, -1 );
		tmp.identity(); tmp.setRot( I, J, K, true );
		grids[kDown].changeBase( tmp );

		I = CVector(  0,  0,  1 );
		J = CVector(  0,  1,  0 );
		K = CVector( -1,  0,  0 );
		tmp.identity(); tmp.setRot( I, J, K, true);
		grids[kLeft].changeBase( tmp );

		I = CVector(  0,  0, -1 );
		J = CVector(  0,  1,  0 );
		K = CVector(  1,  0,  0 );
		tmp.identity(); tmp.setRot( I, J, K, true);
		grids[kRight].changeBase( tmp );

		I = CVector(  1,  0,  0 );
		J = CVector(  0,  0,  1 );
		K = CVector(  0, -1,  0 );
		tmp.identity(); tmp.setRot( I, J, K, true);
		grids[kFront].changeBase( tmp );

		I = CVector(  1,  0,  0 );
		J = CVector(  0,  0, -1 );
		K = CVector(  0,  1,  0 );
		tmp.identity(); tmp.setRot( I, J, K, true);
		grids[kBack].changeBase( tmp );
	}

	// =======================================================================
	void create( int nSize )
	{
		grids[kUp].create	( nSize, 1.0f / ((float)nSize) );
		grids[kDown].create	( nSize, 1.0f / ((float)nSize) );
		grids[kLeft].create	( nSize, 1.0f / ((float)nSize) );
		grids[kRight].create( nSize, 1.0f / ((float)nSize) );
		grids[kFront].create( nSize, 1.0f / ((float)nSize) );
		grids[kBack].create	( nSize, 1.0f / ((float)nSize) );
	}

	// =======================================================================
	void project( CTriangle &tri, CPlane pyr[4], CPlane &gridPlane, 
					sint32 nGridNb, SCubeGridCell &cell )
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
			CVector vMin(1,1,1), vMax(-1,-1,-1);
			for( i = 0; i < nOut; ++i )
			{
				vOut[i] = gridPlane.intersect( CVector(0,0,0), vOut[i] );
				if( vMin.x > vOut[i].x ) vMin.x = vOut[i].x;
				if( vMin.y > vOut[i].y ) vMin.y = vOut[i].y;
				if( vMin.z > vOut[i].z ) vMin.z = vOut[i].z;
				if( vMax.x < vOut[i].x ) vMax.x = vOut[i].x;
				if( vMax.y < vOut[i].y ) vMax.y = vOut[i].y;
				if( vMax.z < vOut[i].z ) vMax.z = vOut[i].z;
			}
			// Create the bbox
			grids[nGridNb].insert( vMin, vMax, cell );
		}
	}

	// =======================================================================
	void insert( CTriangle &tri, SCubeGridCell &cell )
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

	// =======================================================================
	void select( CVector &v )
	{
		CPlane gp;
		// Get the plane
		if( ( -v.z <= v.x ) && ( v.x <= v.z ) &&
			( -v.z <= v.y ) && ( v.y <= v.z ) &&
			( 0.0f <= v.z ) )
		{
			nSelGrid = kUp;
			gp.make( CVector(0,0,1), CVector(0,0,0.5) );
		}
		if( ( v.z <= v.x ) && ( v.x <= -v.z ) &&
			( v.z <= v.y ) && ( v.y <= -v.z ) &&
			( v.z <= 0.0f ) )
		{
			nSelGrid = kDown;
			gp.make( CVector(0,0,-1), CVector(0,0,-0.5) );
		}
		if( ( v.x <= 0.0f ) &&
			( v.x <= v.y ) && ( v.y <= -v.x ) &&
			( v.x <= v.z ) && ( v.z <= -v.x ) )
		{
			nSelGrid = kLeft;
			gp.make( CVector(-1,0,0), CVector(-0.5,0,0) );
		}
		if( ( 0.0f <= v.x ) &&
			( -v.x <= v.y ) && ( v.y <= v.x ) &&
			( -v.x <= v.z ) && ( v.z <= v.x ) )
		{
			nSelGrid = kRight;
			gp.make( CVector(1,0,0), CVector(0.5,0,0) );
		}
		if( ( v.y <= v.x ) && ( v.x <= -v.y ) &&
			( v.y <= 0.0f ) &&
			( v.y <= v.z ) && ( v.z <= -v.y ) )
		{
			nSelGrid = kFront;
		gp.make( CVector(0,-1,0), CVector(0,-0.5,0) );
		}
		if( ( -v.y <= v.x ) && ( v.x <= v.y ) &&
			( 0.0f <= v.y ) &&
			( -v.y <= v.z ) && ( v.z <= v.y ) )
		{
			nSelGrid = kBack;
			gp.make( CVector(0,1,0), CVector(0,0.5,0) );
		}
		nlassert(nSelGrid!=-1);
		CVector newV = gp.intersect( CVector(0,0,0), v );
		grids[nSelGrid].select(newV, newV);
		itSel = grids[nSelGrid].begin();
	}

	SCubeGridCell getSel()
	{
		return *itSel;
	}

	void nextSel()
	{
		++itSel;
	}

	bool isEndSel()
	{
		return (itSel == grids[nSelGrid].end());
	}
};

// -----------------------------------------------------------------------------------------------
struct SWorldRT
{
	vector<CMesh::CMeshBuild *> vMB;
	vector<SCubeGrid> cgAccel; // One cube grid by light

	// vector<CAABBox> bbBoxes;
	// vector<sint32> MeshSel;
	// vector<CQuadTree<SWorldRTCell> > qtAccel;
	// vector<CBSPTree<SWorldRTCell> > btAccel;
};

TTicks timerCalcRT = 0;
TTicks timerExportLighting = 0;
TTicks timerInit = 0;
TTicks timerCalc = 0;
TTicks timerPlac = 0;
TTicks timerSave = 0;

// -----------------------------------------------------------------------------------------------
void SortFaceByTextureName(vector<CMesh::CFace*> &AllFaces, CMesh::CMeshBuild *pMB)
{
	int i, j;
	int nNbFace = AllFaces.size();

	for( i = 0; i < nNbFace-1; ++i )
	for( j = i+1; j < nNbFace; ++j )
	{
		ITexture *pT = pMB->Materials[AllFaces[i]->MaterialId].getTexture(0);
		CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
		string namei = "Default";
		if( pTF != NULL )
			namei = pTF->getFileName();
		pT = pMB->Materials[AllFaces[j]->MaterialId].getTexture(0);
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
void ComputeAreaOfTextureName(vector<sint32> &TextureNames, vector<CMesh::CFace*> &AllFaces, CMesh::CMeshBuild *pMB)
{
	int i, nNbFace = AllFaces.size();
	TextureNames.resize(nNbFace);
	ITexture *pT = pMB->Materials[AllFaces[0]->MaterialId].getTexture(0);
	CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
	string CurrentName = "Default";
	sint32 lastface = 0, nNbTexName = 0;
	if( pTF != NULL )
		CurrentName = pTF->getFileName();
	for( i = 0; i < nNbFace; ++i )
	{
		ITexture *pT = pMB->Materials[AllFaces[i]->MaterialId].getTexture(0);
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
	sint32 i, j;
	sint32 F1c[2] = { -1, -1 };
	sint32 F2c[2] = { -1, -1 };

	// Is there a vertices continuity
	for( j = 0; j < 3; ++j )
	for( i = 0; i < 3; ++i )
	{
		if( (pF1->Corner[j].Vertex == pF2->Corner[i].Vertex) && 
			(pF1->Corner[(j+1)%3].Vertex == pF2->Corner[(i+1)%3].Vertex) )
		{
			F1c[0] = j; F1c[1] = (j+1)%3;
			F2c[0] = i; F2c[1] = (i+1)%3;
		}
		if( (pF1->Corner[j].Vertex == pF2->Corner[(i+1)%3].Vertex) && 
			(pF1->Corner[(j+1)%3].Vertex == pF2->Corner[i].Vertex) )
		{
			F1c[0] = (j+1)%3; F1c[1] = j;
			F2c[0] = i;		  F2c[1] = (i+1)%3;
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

// -----------------------------------------------------------------------------------------------
void SortFaceBySMoothGroup( vector<sint32> &FaceGroup, vector<CMesh::CFace*>::iterator ItFaces, sint32 nNbFace )
{
	sint32 j, k, nGroupNb = 0, nGroupOffset = 1;
	bool bFaceAdded;

	// Bubble sort face
	FaceGroup.resize(nNbFace);
	for( j = 0; j < nNbFace; ++j )
		FaceGroup[j] = 1;
	vector<CMesh::CFace*>::iterator CurGrpBeg = ItFaces;
	vector<CMesh::CFace*>::iterator CurGrpEnd = ItFaces;

	for( nGroupOffset = 1; nGroupOffset <= nNbFace; )
	{
		do 
		{
			bFaceAdded = false;
			vector<CMesh::CFace*>::iterator ItParseJ = CurGrpEnd;
			++ItParseJ;
			for( j = nGroupOffset; j < nNbFace; ++j )
			{
				// Is the face is connected to one of the current group
				vector<CMesh::CFace*>::iterator ItParseK = CurGrpBeg;
				for( k = 0; k < FaceGroup[nGroupNb]; ++k )
				{
					if( FaceContinuous( *ItParseK, *ItParseJ, false ) )
					{
						// Yes the face must be added at the end of the group
						++CurGrpEnd;
						CMesh::CFace *pFaceTemp = *CurGrpEnd;
						*CurGrpEnd = *ItParseJ;
						*ItParseJ = pFaceTemp;
						nGroupOffset += 1;
						FaceGroup[nGroupNb] += 1;
						bFaceAdded = true;
						break;
					}
					++ItParseK;
				}
				++ItParseJ;
			}
		} while( bFaceAdded ); // In this pass have we added faces ?
		// No -> Next smooth group
		++CurGrpEnd;
		CurGrpBeg = CurGrpEnd;
		++nGroupNb;
		nGroupOffset += 1;
	}
	FaceGroup.resize(nGroupNb);
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
bool intersectionTriangleSphere( CTriangle &t, CBSphere &s )
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
void CreatePiece( vector<uint8>& Piece, sint32& nSizeX, sint32& nSizeY, sint32 &nPosX, sint32 &nPosY,
				 float lumx1, float lumy1,
				 float lumx2, float lumy2, 
				 float lumx3, float lumy3, uint8 nCol )
{
	double minx, miny;
	double maxx, maxy;
	int j,k;

	if( nCol == 0 )
		nCol = 1;
	minx = lumx1;
	if( minx > lumx2 ) minx = lumx2;
	if( minx > lumx3 ) minx = lumx3;
	maxx = lumx1;
	if( maxx < lumx2 ) maxx = lumx2;
	if( maxx < lumx3 ) maxx = lumx3;
	miny = lumy1;
	if( miny > lumy2 ) miny = lumy2;
	if( miny > lumy3 ) miny = lumy3;
	maxy = lumy1;
	if( maxy < lumy2 ) maxy = lumy2;
	if( maxy < lumy3 ) maxy = lumy3;

	// Put the piece in the new basis (nPosX,nPosY)
	nPosX = ((sint32)floor(minx-0.5));
	nPosY = ((sint32)floor(miny-0.5));

	lumx1 -= nPosX; lumy1 -= nPosY;
	lumx2 -= nPosX; lumy2 -= nPosY;
	lumx3 -= nPosX;	lumy3 -= nPosY;

	nSizeX = 1 + ((sint32)floor(maxx+0.5)) - ((sint32)floor(minx-0.5));
	nSizeY = 1 + ((sint32)floor(maxy+0.5)) - ((sint32)floor(miny-0.5));
	Piece.resize( nSizeX*nSizeY );
	for( j = 0; j < nSizeX*nSizeY; ++j )
		Piece[j] = 0;

// The square interact with the triangle if an edge of the square is cut by an edge of the triangle
// Or the square is in the triangle
	
	for( j = 0; j < nSizeY-1; ++j )
	for( k = 0; k < nSizeX-1; ++k )
	{
		// Is the square (j,k) is interacting with the triangle
		// This means : The square contains a point of the triangle (can be done for the 3 points)
		//              The triangle contains a point of the square
		// If so then we have to turn on all the 4 pixels of the square
		if( isInTriangleOrEdge(k+0.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+1.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+0.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+1.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) )
		{
			Piece[k   + j    *nSizeX] = nCol;
			Piece[1+k + j    *nSizeX] = nCol;
			Piece[k   + (1+j)*nSizeX] = nCol;
			Piece[1+k + (1+j)*nSizeX] = nCol;
		}

		if( segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) )
		{
			Piece[k   + j    *nSizeX] = nCol;
			Piece[1+k + j    *nSizeX] = nCol;
			Piece[k   + (1+j)*nSizeX] = nCol;
			Piece[1+k + (1+j)*nSizeX] = nCol;
		}

	}
	// For all the points of the triangle update the square
	Piece[((sint32)(lumx1-0.5)) + ((sint32)(lumy1-0.5))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx1-0.5)) + ((sint32)(lumy1-0.5))*nSizeX] = nCol;
	Piece[((sint32)(lumx1-0.5)) + (1+((sint32)(lumy1-0.5)))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx1-0.5)) + (1+((sint32)(lumy1-0.5)))*nSizeX] = nCol;

	Piece[((sint32)(lumx2-0.5)) + ((sint32)(lumy2-0.5))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx2-0.5)) + ((sint32)(lumy2-0.5))*nSizeX] = nCol;
	Piece[((sint32)(lumx2-0.5)) + (1+((sint32)(lumy2-0.5)))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx2-0.5)) + (1+((sint32)(lumy2-0.5)))*nSizeX] = nCol;
	
	Piece[((sint32)(lumx3-0.5)) + ((sint32)(lumy3-0.5))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx3-0.5)) + ((sint32)(lumy3-0.5))*nSizeX] = nCol;
	Piece[((sint32)(lumx3-0.5)) + (1+((sint32)(lumy3-0.5)))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx3-0.5)) + (1+((sint32)(lumy3-0.5)))*nSizeX] = nCol;
}

// -----------------------------------------------------------------------------------------------
void ResizeBitmap( vector<uint8> &vBitmap, sint32 &nSizeX, sint32 &nSizeY, sint32 nNewSizeX, sint32 nNewSizeY )
{
	vector<uint8> vImgTemp;
	int i, j;

	vImgTemp.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vImgTemp[i] = 0;

	for( j = 0; j < min(nSizeY,nNewSizeY); ++j )
	for( i = 0; i < min(nSizeX,nNewSizeX); ++i )
	{
		vImgTemp[i+j*nNewSizeX] = vBitmap[i+j*nSizeX];
	}

	vBitmap.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vBitmap[i] = vImgTemp[i];

	nSizeX = nNewSizeX;
	nSizeY = nNewSizeY;
}

// -----------------------------------------------------------------------------------------------
// Same as ResizeBitmap but for 32 bits image
void ResizeBitmap32( CBitmap *pImage, sint32 nNewSizeX, sint32 nNewSizeY )
{
	vector<uint8> vImgTemp;
	int i, j;

	vImgTemp.resize( 4*nNewSizeX*nNewSizeY );
	for( i = 0; i < 4*nNewSizeX*nNewSizeY; ++i )
		vImgTemp[i] = 0;

	CObjectVector<uint8> &vBitmap = pImage->getPixels();
	sint32 nCurSizeX = pImage->getWidth();
	sint32 nCurSizeY = pImage->getHeight();
	for( j = 0; j < min(nCurSizeY,nNewSizeY); ++j )
	for( i = 0; i < min(nCurSizeX,nNewSizeX); ++i )
	{
		vImgTemp[4*(i+j*nNewSizeX)+0] = vBitmap[4*(i+j*pImage->getWidth())+0];
		vImgTemp[4*(i+j*nNewSizeX)+1] = vBitmap[4*(i+j*pImage->getWidth())+1];
		vImgTemp[4*(i+j*nNewSizeX)+2] = vBitmap[4*(i+j*pImage->getWidth())+2];
		vImgTemp[4*(i+j*nNewSizeX)+3] = vBitmap[4*(i+j*pImage->getWidth())+3];
	}

	pImage->resize(nNewSizeX,nNewSizeY);
	vBitmap = pImage->getPixels();
	for( i = 0; i < 4*nNewSizeX*nNewSizeY; ++i )
		vBitmap[i] = vImgTemp[i];
}

// -----------------------------------------------------------------------------------------------
bool PutPieceInLightMap( vector<uint8>& Piece, sint32 nPieceSizeX, sint32 nPieceSizeY, 
						 vector<uint8>& LightMap, sint32 nLightMapSizeX, sint32 nLightMapSizeY,
						 sint32 &nNewPosX, sint32 &nNewPosY )
{
	sint32 i, j, a, b;
	bool bGoodPosition;

	if( nPieceSizeX > nLightMapSizeX ) return false;
	if( nPieceSizeY > nLightMapSizeY ) return false;

	// For all position test if the piece can be put in
	for( j = 0; j < (nLightMapSizeY-nPieceSizeY); ++j )
	for( i = 0; i < (nLightMapSizeX-nPieceSizeX); ++i )
	{
		bGoodPosition = true;
		for( b = 0; b < nPieceSizeY; ++b )
		{
			for( a = 0; a < nPieceSizeX; ++a )
			{
				if( Piece[a+b*nPieceSizeX] != 0 )
					if( LightMap[(i+a)+(j+b)*nLightMapSizeX] != 0 )
					{
						bGoodPosition = false;
						break;
					}
			}
			if( bGoodPosition == false )
				break;
		}
		if( bGoodPosition )
		{
			// Write the piece in the lightmap !!!
			for( b = 0; b < nPieceSizeY; ++b )
			{
				for( a = 0; a < nPieceSizeX; ++a )
				{
					if( Piece[a+b*nPieceSizeX] != 0 )
						LightMap[(i+a)+(j+b)*nLightMapSizeX] = Piece[a+b*nPieceSizeX];
				}
			}
			nNewPosX = i;
			nNewPosY = j;
			return true;
		}
	}

	return false;
}

// -----------------------------------------------------------------------------------------------
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
CMatrix getObjectToWorldMatrix( CMesh::CMeshBuild *pMB )
{
	CMatrix m1, m2, m3, m4, m5;
	m1.identity();
	m1.setPos( pMB->DefaultPivot );
	m1.invert();

	m2.identity();
	m2.scale( pMB->DefaultScale );

	m3.identity();
	m3.setRot( pMB->DefaultRotQuat );

	m4.identity();
	m4.setPos( pMB->DefaultPivot );

	m5.identity();
	m5.setPos( pMB->DefaultPos );

	m1 = m5*m4*m3*m2*m1;
	
	return m1;
}

// -----------------------------------------------------------------------------------------------
float getUVDist( CUV& UV1, CUV& UV2 )
{
	return sqrtf( (UV2.U - UV1.U)*(UV2.U - UV1.U) + (UV2.V - UV1.V)*(UV2.V - UV1.V) );
}

// -----------------------------------------------------------------------------------------------
void getLightBuildList(std::vector<SLightBuild>& vectLight, TimeValue tvTime, Interface& ip, INode*node=NULL )
{
	if( node == NULL )
		node = ip.GetRootNode();

	// Get a pointer on the object's node
    Object *obj = node->EvalWorldState(tvTime).obj;

	// Check if there is an object
	if (obj)
	{
		// Get a GenLight from the node
		if (obj->SuperClassID()==LIGHT_CLASS_ID)
		{
			GenLight *maxLight = (GenLight *) obj;

			bool deleteIt=false;
			if (obj != maxLight) 
				deleteIt = true;

			SLightBuild nelLight;

			// Is the light is animatable ? (TEMP MAT)
			Modifier *pModifier = CExportNel::getModifier( node, Class_ID(NEL_LIGHT_CLASS_ID_A, NEL_LIGHT_CLASS_ID_B) );
			if( pModifier != NULL )
			{
				int bDynamic;
				string sGroup;
				// Get the value of the parameters
				CExportNel::getValueByNameUsingParamBlock2( *pModifier, "bDynamic", (ParamType2)TYPE_BOOL, &bDynamic, 0);
				CExportNel::getValueByNameUsingParamBlock2( *pModifier, "sGroup", (ParamType2)TYPE_STRING, &sGroup, 0);
				if( bDynamic )
				{
					nelLight.bAnimate = false;
					nelLight.GroupName = sGroup;
				}
			}
			else
			{
				nelLight.bAnimate = false;
				nelLight.GroupName = "GlobalLight";
			}

			// Eval the light state fot this tvTime
			Interval valid=NEVER;
			LightState ls;

			if (maxLight->EvalLightState(tvTime, valid, &ls)==REF_SUCCEED)
			{
				// Set the light mode
				switch (maxLight->Type())
				{
					case OMNI_LIGHT:
						nelLight.Type = SLightBuild::EType::LightPoint;
					break;
					case TSPOT_LIGHT:
					case FSPOT_LIGHT:
						nelLight.Type = SLightBuild::EType::LightSpot;
					break;
					case DIR_LIGHT:
					case TDIR_LIGHT:
						nelLight.Type = SLightBuild::EType::LightDir;
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
				nelFColor   *= maxLight->GetIntensity(tvTime);
				nelColor = nelFColor;

				// Affect the ambiant color ?
				nelLight.Ambient = nelColor;
				nelLight.Diffuse = CRGBA (0,0,0);
				nelLight.Specular = CRGBA (0,0,0);

				/*
				if (maxLight->GetAmbientOnly())
				{
					nelLight.Ambient = nelColor;
				}
				else
				*/
				{
					// Affect the diffuse color ?
					if( maxLight->GetAffectDiffuse() )
						nelLight.Diffuse = nelColor;
					// Affect the specular color ?
					if (maxLight->GetAffectSpecular())
						nelLight.Specular = nelColor;
				}

				// Set the light position
				Point3 pos = node->GetNodeTM(tvTime).GetTrans ();
				CVector position;
				position.x=pos.x;
				position.y=pos.y;
				position.z=pos.z;

				// Set the position
				nelLight.Position = position;

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
				nelLight.Direction = direction;
				/*
				// Set spot light information
				nelLight.setCutoff ((float)(NLMISC::Pi*maxLight.GetFallsize(tvTime)/180.f/2.f));

				// Compute the exponent value
				float angle=(float)(NLMISC::Pi*maxLight.GetHotspot(tvTime)/(2.0*180.0));
				nelLight.setupSpotExponent (angle);

				// *** Set attenuation

				if (maxLight.GetUseAtten())
					nelLight.setupAttenuation (maxLight.GetAtten (tvTime, ATTEN_START), maxLight.GetAtten (tvTime, ATTEN_END));
				else
					nelLight.setNoAttenuation ();
				*/

				nelLight.rHotspot = (float)(Pi * maxLight->GetHotspot(tvTime) /(2.0*180.0));
				nelLight.rFallof =  (float)(Pi * maxLight->GetFallsize(tvTime)/(2.0*180.0));

				if (maxLight->GetUseAtten())
				{
					nelLight.rRadiusMin = maxLight->GetAtten (tvTime, ATTEN_START);
					nelLight.rRadiusMax = maxLight->GetAtten (tvTime, ATTEN_END);
				}
				else
				{	// Limit
					nelLight.rRadiusMin = 10.0;
					nelLight.rRadiusMax = 10.0;
				}

				nelLight.bCastShadow = ( maxLight->GetShadow() != 0 );
				nelLight.rMult = maxLight->GetShadMult (tvTime);
				// Add the light in the list
				vectLight.push_back (nelLight);
			}
			if( deleteIt )
				delete maxLight;
		}
	}

	// Recurse sub node
	for (int i=0; i<node->NumberOfChildren(); i++)
		getLightBuildList(vectLight, tvTime, ip, node->GetChildNode(i));
}

// -----------------------------------------------------------------------------------------------
void getLightBuilds( vector<SLightBuild> &lights, TimeValue tvTime, Interface& ip )
{
	SLightBuild amb;

	amb.Type = SLightBuild::EType::LightAmbient;
	amb.GroupName = "GlobalLight";
	amb.bAnimate = false;
	amb.Ambient.R = (uint8)(ip.GetAmbient( tvTime, FOREVER ).x*255);
	amb.Ambient.G = (uint8)(ip.GetAmbient( tvTime, FOREVER ).y*255);
	amb.Ambient.B = (uint8)(ip.GetAmbient( tvTime, FOREVER ).z*255);
	amb.Ambient.A = 255;
	amb.Specular = amb.Diffuse = CRGBA(0,0,0,0);
	lights.push_back( amb );
	getLightBuildList( lights, tvTime, ip );

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
double calculateTriangleSurface( CVector &p1, CVector &p2, CVector &p3 )
{
	CVector n = ((p2-p1)^(p3-p1));
	return 0.5 * n.norm(); // Half of the norm
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
	if( TextureSurf < 0.0001 )
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
/* // To keep
void SortPlanesBySurface( vector<sint32> &PlaneGroup, vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace )
{
	sint32 i, j, k;
	double rMinU = 1000000.0, rMaxU = -1000000.0, rMinV = 1000000.0, rMaxV = -1000000.0;

	CMesh::CFace *pF;
	vector< pair < sint32, sint32 > > sizes;

	sizes.resize( PlaneGroup.size() );
	// Get the size of surface for each plane
	vector<CMesh::CFace*>::iterator ItParseI = ItFace;
	for( i = 0; i < sizes.size(); ++i )
	{
		rMinU = 1000000.0; rMaxU = -1000000.0;
		rMinV = 1000000.0; rMaxV = -1000000.0;
		for( j = 0; j < PlaneGroup[i]; ++j )
		{
			pF = *ItParseI;
			for( k = 0; k < 3; ++k )
			{
				if( rMinU > pF->Corner[k].Uvws[1].U ) rMinU = pF->Corner[k].Uvws[1].U;
				if( rMaxU < pF->Corner[k].Uvws[1].U ) rMaxU = pF->Corner[k].Uvws[1].U;
				if( rMinV > pF->Corner[k].Uvws[1].V ) rMinV = pF->Corner[k].Uvws[1].V;
				if( rMaxV < pF->Corner[k].Uvws[1].V ) rMaxV = pF->Corner[k].Uvws[1].V;
			}
			++ItParseI;
		}
		sizes[i].first = i;
		sizes[i].second = (rMaxU - rMinU) * (rMaxV - rMinV);
	}
	// Sort surfaces to put the biggest first
	for( i = 0; i < sizes.size()-1; ++i )
	for( j = i+1; j < sizes.size(); ++j )
	{
		if( sizes[i].second < sizes[j].second )
		{
			pair< sint32, sint32 > tmp = sizes[i];
			sizes[i] = sizes[j];
			sizes[j] = tmp;
		}
	}

	vector<CMesh::CFace*> TempGrp;
	vector<CMesh::CFace*>::iterator ItParseOut;
	TempGrp.resize( nNbFace );
	
	ItParseOut = TempGrp.begin();
	for( i = 0; i < sizes.size(); ++i )
	{
		// Treating group 
		j = sizes[i].first;
		ItParseI = ItFace;
		// Positionnement of the pointer to the first face of the j th group
		for( k = 0; k < j; ++k ) 
			ItParseI += PlaneGroup[k];
		// Copy the group j at the end of TempGrp
		for( k = 0; k < PlaneGroup[j]; ++k )
		{
			*ItParseOut = *ItParseI;
			++ItParseOut;
			++ItParseI;
		}
	}
	// So now we just have to copy back the temporary group
	ItParseOut = TempGrp.begin();
	ItParseI = ItFace;
	for( i = 0; i < nNbFace; ++i )
	{
		*ItParseI = *ItParseOut;
		++ItParseOut;
		++ItParseI;
	}
	// And the same with the group delimiter
	for( i = 0; i < sizes.size(); ++i )
	{
		j = sizes[i].first;
		sizes[i].first = PlaneGroup[j];
	}
	for( i = 0; i < sizes.size(); ++i )
		PlaneGroup[i] = sizes[i].first;
}
*/
// -----------------------------------------------------------------------------------------------
void PutLMPlaneInLMPlane( SLMPlane &Dst, SLMPlane &Src, bool bMaskOnly = false )
{
	sint32 a, b;
	if( ( (Src.w + Src.x) > Dst.w ) || ( (Src.h + Src.y) > Dst.h ) )
	{
		a = 0; b = 0;
	}
	for( b = 0; b < Src.h; ++b )
	for( a = 0; a < Src.w; ++a )
		if( Src.msk[a+b*Src.w] != 0 )
		{
			Dst.msk[(Src.x+a)+(Src.y+b)*Dst.w] = Src.msk[a+b*Src.w];
			if( bMaskOnly == false )
				Dst.col[(Src.x+a)+(Src.y+b)*Dst.w] = Src.col[a+b*Src.w];
		}
}

// -----------------------------------------------------------------------------------------------
bool TestLMPLaneInLMPlane( SLMPlane &Dst, SLMPlane &Src )
{
	sint32 a, b;
	for( b = 0; b < Src.h; ++b )
	for( a = 0; a < Src.w; ++a )
		if( Src.msk[a+b*Src.w] != 0 )
			if( Dst.msk[(Src.x+a)+(Src.y+b)*Dst.w] != 0 )
				return false;
	return true;
}

// -----------------------------------------------------------------------------------------------
bool TryAllPosForLMPlaneInLMPlane( SLMPlane &Dst, SLMPlane &Src )
{
	sint32 i, j;

	if( Src.w > Dst.w ) return false;
	if( Src.h > Dst.h ) return false;

	// For all position test if the Src plane can be put in
	for( j = 0; j < (Dst.h-Src.h); ++j )
	for( i = 0; i < (Dst.w-Src.w); ++i )
	{
		Src.x = i; Src.y = j;
		if( TestLMPLaneInLMPlane( Dst, Src ) )
			return true;
	}
	return false;
}

// -----------------------------------------------------------------------------------------------
void CopyPlaneColToBitmap32( CBitmap* pImage, SLMPlane &Plane, sint32 nLayerNb )
{
	if( ( pImage->getWidth() != Plane.w ) ||
		( pImage->getHeight() != Plane.h ) )
		ResizeBitmap32( pImage, Plane.w, Plane.h );

	CObjectVector<uint8> &vBitmap = pImage->getPixels();

	for( sint32 i = 0; i < Plane.w*Plane.h; ++i )
	{
		vBitmap[4*i+0] = (uint8)(127.0*Plane.col[i].p[nLayerNb].R);
		vBitmap[4*i+1] = (uint8)(127.0*Plane.col[i].p[nLayerNb].G);
		vBitmap[4*i+2] = (uint8)(127.0*Plane.col[i].p[nLayerNb].B);
		vBitmap[4*i+3] = (uint8)(255.0*Plane.col[i].p[nLayerNb].A);
	}
	/* // To see contours
	for( sint32 i = 0; i < Plane.w*Plane.h; ++i )
		if( ( Plane.msk[i] == 0 ) || ( Plane.msk[i] == 1 ) || ( Plane.msk[i] == 3 ) )
		{
			vBitmap[i*4+0] = 0;
			vBitmap[i*4+1] = 255;
			vBitmap[i*4+2] = 0;
			vBitmap[i*4+3] = 255;
		}
		else
		{
			vBitmap[i*4+0] = Plane.col[i*4+0];
			vBitmap[i*4+1] = Plane.col[i*4+1];
			vBitmap[i*4+2] = Plane.col[i*4+2];
			vBitmap[i*4+3] = Plane.col[i*4+3];
		}
	*/
}

// -----------------------------------------------------------------------------------------------
// Do not stretch the image inside the plane
void ResizeLMPlane( SLMPlane &Dst, sint32 nNewSizeX, sint32 nNewSizeY )
{
	vector<uint8> vImgTemp;
	vector<SLMPixel> vImgTemp2;
	int i, j;

	vImgTemp.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vImgTemp[i] = 0;

	for( j = 0; j < min(Dst.h,nNewSizeY); ++j )
	for( i = 0; i < min(Dst.w,nNewSizeX); ++i )
	{
		vImgTemp[i+j*nNewSizeX] = Dst.msk[i+j*Dst.w];
	}

	Dst.msk.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		Dst.msk[i] = vImgTemp[i];

	// The same as the mask but for the bitmap
	vImgTemp2.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		for( j = 0; j < 8; ++j )
		{ vImgTemp2[i].p[j].R = vImgTemp2[i].p[j].G = vImgTemp2[i].p[j].B = vImgTemp2[i].p[j].A = 0.0f; }

	for( j = 0; j < min(Dst.h,nNewSizeY); ++j )
	for( i = 0; i < min(Dst.w,nNewSizeX); ++i )
		vImgTemp2[i+j*nNewSizeX] = Dst.col[i+j*Dst.w];

	Dst.col.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		Dst.col[i] = vImgTemp2[i];

	Dst.w = nNewSizeX;
	Dst.h = nNewSizeY;
}

// -----------------------------------------------------------------------------------------------
// Stretch a plane by a given factor 4.0 -> multiply its size by 4 and 0.5 -> halves its size
void stretchLMPlane( SLMPlane *pPlane, double osFactor )
{
	sint32 nNewSizeX = (sint32)(pPlane->w * osFactor);
	sint32 nNewSizeY = (sint32)(pPlane->h * osFactor);
	vector<uint8> vImgTemp;
	vector<SLMPixel> vImgTemp2;
	int i, j, k;

	// Reduce the color
	vImgTemp2.resize( nNewSizeX * nNewSizeY );
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		for( j = 0; j < 8; ++j )
		{ vImgTemp2[i].p[j].R = vImgTemp2[i].p[j].G = vImgTemp2[i].p[j].B = vImgTemp2[i].p[j].A = 0.0f; }

	vImgTemp.resize( nNewSizeX * nNewSizeY );
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vImgTemp[i] = 0;

	double dx, dy, x, y;
	if( osFactor > 1.0 ) // Up
	{
		dx = 1.0/osFactor;
		dy = 1.0/osFactor;
		y = 0.0;
		for( j = 0; j < nNewSizeY; ++j )
		{
			x = 0.0;
			for( i = 0; i < nNewSizeX; ++i )
			{
				if( pPlane->msk[((sint32)x)+((sint32)y)*pPlane->w] != 0 )
				{
					vImgTemp[i+j*nNewSizeX] = 1;
				}
				for( k = 0; k < 8; ++k )
				{
					vImgTemp2[i+j*nNewSizeX].p[k].R += pPlane->col[((sint32)x)+((sint32)y)*pPlane->w].p[k].R;
					vImgTemp2[i+j*nNewSizeX].p[k].G += pPlane->col[((sint32)x)+((sint32)y)*pPlane->w].p[k].G;
					vImgTemp2[i+j*nNewSizeX].p[k].B += pPlane->col[((sint32)x)+((sint32)y)*pPlane->w].p[k].B;
					vImgTemp2[i+j*nNewSizeX].p[k].A += 1.0f;
				}
				x += dx;
			}
			y += dy;
		}
	}
	else // Down
	{
		dx = osFactor;
		dy = osFactor;
		y = 0.0;
		for( j = 0; j < pPlane->h; ++j )
		{
			x = 0.0;
			for( i = 0; i < pPlane->w; ++i )
			{
				if( pPlane->msk[i+j*pPlane->w] != 0 )
				{
					vImgTemp[((sint32)x)+((sint32)y)*nNewSizeX] = 1;
				}
				for( k = 0; k < 8; ++k )
				{
					vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX].p[k].R += pPlane->col[i+j*pPlane->w].p[k].R;
					vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX].p[k].G += pPlane->col[i+j*pPlane->w].p[k].G;
					vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX].p[k].B += pPlane->col[i+j*pPlane->w].p[k].B;
					vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX].p[k].A += 1.0f;
				}
				x += dx;
			}
			y += dy;
		}
	}

	for( j = 0; j < nNewSizeY; ++j )
	for( i = 0; i < nNewSizeX; ++i )
	for( k = 0; k < 8; ++k )
	if( vImgTemp2[i+j*nNewSizeX].p[k].A > 1.0f )
	{
		vImgTemp2[i+j*nNewSizeX].p[k].R /= vImgTemp2[i+j*nNewSizeX].p[k].A;
		vImgTemp2[i+j*nNewSizeX].p[k].G /= vImgTemp2[i+j*nNewSizeX].p[k].A;
		vImgTemp2[i+j*nNewSizeX].p[k].B /= vImgTemp2[i+j*nNewSizeX].p[k].A;
		vImgTemp2[i+j*nNewSizeX].p[k].A = 1.0f;			
	}

	pPlane->col.resize( nNewSizeX * nNewSizeY );
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		pPlane->col[i] = vImgTemp2[i];

	pPlane->msk.resize( nNewSizeX * nNewSizeY );
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		pPlane->msk[i] = vImgTemp[i];

	pPlane->w = nNewSizeX;
	pPlane->h = nNewSizeY;
	pPlane->x = (sint32)(pPlane->x * osFactor);
	pPlane->y = (sint32)(pPlane->y * osFactor);
}

// -----------------------------------------------------------------------------------------------
void CreateLMPlaneFromFace( SLMPlane &Out, CMesh::CFace *pF )
{
	double	lumx1 = pF->Corner[0].Uvws[1].U, lumy1 = pF->Corner[0].Uvws[1].V, 
			lumx2 = pF->Corner[1].Uvws[1].U, lumy2 = pF->Corner[1].Uvws[1].V, 
			lumx3 = pF->Corner[2].Uvws[1].U, lumy3 = pF->Corner[2].Uvws[1].V;
	double minx, miny;
	double maxx, maxy;
	int j,k;

	minx = lumx1;
	if( minx > lumx2 ) minx = lumx2;
	if( minx > lumx3 ) minx = lumx3;
	maxx = lumx1;
	if( maxx < lumx2 ) maxx = lumx2;
	if( maxx < lumx3 ) maxx = lumx3;
	miny = lumy1;
	if( miny > lumy2 ) miny = lumy2;
	if( miny > lumy3 ) miny = lumy3;
	maxy = lumy1;
	if( maxy < lumy2 ) maxy = lumy2;
	if( maxy < lumy3 ) maxy = lumy3;

	// Put the piece in the new basis (nPosX,nPosY)
	Out.x = ((sint32)floor(minx-0.5));
	Out.y = ((sint32)floor(miny-0.5));

	lumx1 -= Out.x; lumy1 -= Out.y;
	lumx2 -= Out.x; lumy2 -= Out.y;
	lumx3 -= Out.x;	lumy3 -= Out.y;

	ResizeLMPlane( Out, 1 + ((sint32)floor(maxx+0.5)) - ((sint32)floor(minx-0.5)),
						1 + ((sint32)floor(maxy+0.5)) - ((sint32)floor(miny-0.5)) );

	for( j = 0; j < Out.w*Out.h; ++j )
	{
		Out.msk[j] = 0;
	}

// The square interact with the triangle if an edge of the square is cut by an edge of the triangle
// Or the square is in the triangle
	
	for( j = 0; j < Out.h-1; ++j )
	for( k = 0; k < Out.w-1; ++k )
	{
		// Is the square (j,k) is interacting with the triangle
		// This means : The square contains a point of the triangle (can be done for the 3 points)
		//              The triangle contains a point of the square
		// If so then we have to turn on all the 4 pixels of the square
		if( isInTriangleOrEdge(k+0.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+1.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+0.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+1.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) )
		{
			Out.msk[k   + j    *Out.w] = 1;
			Out.msk[1+k + j    *Out.w] = 1;
			Out.msk[k   + (1+j)*Out.w] = 1;
			Out.msk[1+k + (1+j)*Out.w] = 1;
		}

		if( segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) )
		{
			Out.msk[k   + j    *Out.w] = 1;
			Out.msk[1+k + j    *Out.w] = 1;
			Out.msk[k   + (1+j)*Out.w] = 1;
			Out.msk[1+k + (1+j)*Out.w] = 1;
		}

	}
	// For all the points of the triangle update the square

// TODO : Test if we need it !

	Out.msk[((sint32)(lumx1-0.5))   + ((sint32)(lumy1-0.5))    *Out.w] = 1;
	Out.msk[1+((sint32)(lumx1-0.5)) + ((sint32)(lumy1-0.5))    *Out.w] = 1;
	Out.msk[((sint32)(lumx1-0.5))   + (1+((sint32)(lumy1-0.5)))*Out.w] = 1;
	Out.msk[1+((sint32)(lumx1-0.5)) + (1+((sint32)(lumy1-0.5)))*Out.w] = 1;

	Out.msk[((sint32)(lumx2-0.5))   + ((sint32)(lumy2-0.5))    *Out.w] = 1;
	Out.msk[1+((sint32)(lumx2-0.5)) + ((sint32)(lumy2-0.5))    *Out.w] = 1;
	Out.msk[((sint32)(lumx2-0.5))   + (1+((sint32)(lumy2-0.5)))*Out.w] = 1;
	Out.msk[1+((sint32)(lumx2-0.5)) + (1+((sint32)(lumy2-0.5)))*Out.w] = 1;
	
	Out.msk[((sint32)(lumx3-0.5))   + ((sint32)(lumy3-0.5))    *Out.w] = 1;
	Out.msk[1+((sint32)(lumx3-0.5)) + ((sint32)(lumy3-0.5))    *Out.w] = 1;
	Out.msk[((sint32)(lumx3-0.5))   + (1+((sint32)(lumy3-0.5)))*Out.w] = 1;
	Out.msk[1+((sint32)(lumx3-0.5)) + (1+((sint32)(lumy3-0.5)))*Out.w] = 1;

}

// -----------------------------------------------------------------------------------------------
// Warning : modify the Faces Uvws[1]
void CreateLMPlaneFromFaceGroup( SLMPlane &Plane, vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace )
{
	sint32 i, j;
	double rMinU = 1000000.0, rMaxU = -1000000.0, rMinV = 1000000.0, rMaxV = -1000000.0;
	vector<CMesh::CFace*>::iterator ItParseI = ItFace;
	CMesh::CFace *pF;

	Plane.faces.resize( nNbFace );

	for( i = 0; i < nNbFace; ++i )
	{
		pF = *ItParseI;
		for( j = 0; j < 3; ++j )
		{
			if( rMinU > pF->Corner[j].Uvws[1].U ) rMinU = pF->Corner[j].Uvws[1].U;
			if( rMaxU < pF->Corner[j].Uvws[1].U ) rMaxU = pF->Corner[j].Uvws[1].U;
			if( rMinV > pF->Corner[j].Uvws[1].V ) rMinV = pF->Corner[j].Uvws[1].V;
			if( rMaxV < pF->Corner[j].Uvws[1].V ) rMaxV = pF->Corner[j].Uvws[1].V;
		}
		Plane.faces[i] = pF;
		++ItParseI;
	}

	sint32 w = ( 1 + ((sint32)floor( rMaxU + 0.5 )) - ((sint32)floor( rMinU - 0.5 )) );
	sint32 h = ( 1 + ((sint32)floor( rMaxV + 0.5 )) - ((sint32)floor( rMinV - 0.5 )) );
	ResizeLMPlane( Plane, w, h );
	Plane.x = ( ((sint32)floor( rMinU - 0.5 )) );
	Plane.y = ( ((sint32)floor( rMinV - 0.5 )) );
	for( j = 0; j < Plane.w*Plane.h; ++j )
		Plane.msk[j] = 0;

	ItParseI = ItFace;
	for( i = 0; i < nNbFace; ++i )
	{
		pF = *ItParseI;
		// Create Mask
		SLMPlane Piece;

		CreateLMPlaneFromFace( Piece, pF );
		// Because all is in absolute coordinate
		Piece.x -= Plane.x;
		Piece.y -= Plane.y;
		PutLMPlaneInLMPlane( Plane, Piece );
		
		++ItParseI;
	}
}

// -----------------------------------------------------------------------------------------------
void ModifyLMPlaneWithOverSampling( SLMPlane *pPlane, double rOverSampling )
{	
	sint32 i, j;
	vector<CMesh::CFace*>::iterator ItFace = pPlane->faces.begin();
	sint32 nNbFace = pPlane->faces.size();

	stretchLMPlane( pPlane, rOverSampling );
	for( j = 0; j < pPlane->w*pPlane->h; ++j ) // Reset the mask
		pPlane->msk[j] = 0;
	MultiplyFaceUV1( ItFace, nNbFace, rOverSampling );
	ItFace = pPlane->faces.begin();
	// Recreate the form
	for( i = 0; i < nNbFace; ++i )
	{
		CMesh::CFace *pF = *ItFace;
		SLMPlane Piece;
		CreateLMPlaneFromFace( Piece, pF );
		Piece.x -= pPlane->x;
		Piece.y -= pPlane->y;
		PutLMPlaneInLMPlane( *pPlane, Piece, true );		
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
void PlaceLMPlaneInLMPLane( SLMPlane &Dst, SLMPlane &Src )
{
	while( true )
	{
		if( !TryAllPosForLMPlaneInLMPlane( Dst, Src ) )
		{
			if( ( Dst.w < MAXLIGHTMAPSIZE ) || ( Dst.h < MAXLIGHTMAPSIZE ) )
			{
				if( Dst.w < Dst.h )
					ResizeLMPlane( Dst, Dst.w*2, Dst.h );
				else
					ResizeLMPlane( Dst, Dst.w, Dst.h*2 );
			}
			else
			{
				// ERROR: we reached the maximum texture size
				nlstop;
			}
		}
		else
		{
			// We found a position
			PutLMPlaneInLMPlane( Dst, Src );
			break;
		}
	}
}

// -----------------------------------------------------------------------------------------------
void CalculateGradient( SGradient &g, CMesh::CFace *pF, vector<CVector>& vVertices, CVector &n1, CVector &n2, CVector &n3 )
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

	double GradDen = 1.0 / ( (u3-u1)*(v2-v1) - (u2-u1)*(v3-v1) );

	g.InitU = u1;
	g.InitV = v1;

	g.InitPx = p1.x;
	g.InitPy = p1.y;
	g.InitPz = p1.z;

	g.InitNx = n1.x;
	g.InitNy = n1.y;
	g.InitNz = n1.z;

	// Gradients for the vertex

	g.GraduPx = ( (p3.x-p1.x)*(v2-v1)-(p2.x-p1.x)*(v3-v1) ) * GradDen;
	g.GradvPx = ( (p2.x-p1.x)*(u3-u1)-(p3.x-p1.x)*(u2-u1) ) * GradDen;

	g.GraduPy = ( (p3.y-p1.y)*(v2-v1)-(p2.y-p1.y)*(v3-v1) ) * GradDen;
	g.GradvPy = ( (p2.y-p1.y)*(u3-u1)-(p3.y-p1.y)*(u2-u1) ) * GradDen;

	g.GraduPz = ( (p3.z-p1.z)*(v2-v1)-(p2.z-p1.z)*(v3-v1) ) * GradDen;
	g.GradvPz = ( (p2.z-p1.z)*(u3-u1)-(p3.z-p1.z)*(u2-u1) ) * GradDen;

	// The same for the normal
					
	g.GraduNx = ( (n3.x-n1.x)*(v2-v1)-(n2.x-n1.x)*(v3-v1) ) * GradDen;
	g.GradvNx = ( (n2.x-n1.x)*(u3-u1)-(n3.x-n1.x)*(u2-u1) ) * GradDen;

	g.GraduNy = ( (n3.y-n1.y)*(v2-v1)-(n2.y-n1.y)*(v3-v1) ) * GradDen;
	g.GradvNy = ( (n2.y-n1.y)*(u3-u1)-(n3.y-n1.y)*(u2-u1) ) * GradDen;

	g.GraduNz = ( (n3.z-n1.z)*(v2-v1)-(n2.z-n1.z)*(v3-v1) ) * GradDen;
	g.GradvNz = ( (n2.z-n1.z)*(u3-u1)-(n3.z-n1.z)*(u2-u1) ) * GradDen;
}

// -----------------------------------------------------------------------------------------------
CVector CalculateInterpolatedVertex( SGradient &g, double u, double v )
{
	CVector vRet;
	vRet.x = (float)(g.GraduPx*(u-g.InitU) + g.GradvPx*(v-g.InitV) + g.InitPx);
	vRet.y = (float)(g.GraduPy*(u-g.InitU) + g.GradvPy*(v-g.InitV) + g.InitPy);
	vRet.z = (float)(g.GraduPz*(u-g.InitU) + g.GradvPz*(v-g.InitV) + g.InitPz);
	return vRet;
}

// -----------------------------------------------------------------------------------------------
CVector CalculateInterpolatedNormal( SGradient &g, double u, double v )
{
	CVector vRet;
	vRet.x = (float)(g.GraduNx*(u-g.InitU) + g.GradvNx*(v-g.InitV) + g.InitNx);
	vRet.y = (float)(g.GraduNy*(u-g.InitU) + g.GradvNy*(v-g.InitV) + g.InitNy);
	vRet.z = (float)(g.GraduNz*(u-g.InitU) + g.GradvNz*(v-g.InitV) + g.InitNz);
	vRet.normalize();
	return vRet;
}

// -----------------------------------------------------------------------------------------------
float TestRay( CVector &vLightPos, CVector &vVertexPos, SWorldRT &wrt, sint32 nLightNb )
{

	// Optim avec Cube Grid
	wrt.cgAccel[nLightNb].select( vVertexPos - vLightPos );
	while( !wrt.cgAccel[nLightNb].isEndSel() )
	{
		SCubeGridCell cell = wrt.cgAccel[nLightNb].getSel();

		CVector hit;
		CTriangle t(cell.pMB->Vertices[cell.pF->Corner[0].Vertex],
					cell.pMB->Vertices[cell.pF->Corner[1].Vertex],
					cell.pMB->Vertices[cell.pF->Corner[2].Vertex] );
		CPlane plane;
		plane.make( t.V0, t.V1, t.V2 );

		if( t.intersect( vLightPos, vVertexPos, hit, plane ) )
		{
			return 0.0f;
		}

		// Next selected element
		wrt.cgAccel[nLightNb].nextSel();
	}
	return 1.0f;

	/* // Optim avec BSP
	for( sint32 i = 0; i < wrt.bbBoxes.size(); ++i )
	{
		if( wrt.bbBoxes[i].intersect( rayP1, rayP2, rayP2 ) )
		{
			sint32 nNbSel = wrt.btAccel[i].select( rayP1, rayP2 );
			for( sint32 j = 0; j < nNbSel; ++j )
			{
				SWorldRTCell cell = wrt.btAccel[i].getSelection( j );

				CVector hit;
				CTriangle t(cell.pMB->Vertices[cell.pF->Corner[0].Vertex],
							cell.pMB->Vertices[cell.pF->Corner[1].Vertex],
							cell.pMB->Vertices[cell.pF->Corner[2].Vertex] );
				CPlane plane;
				plane.make( t.V0, t.V1, t.V2 );

				if( t.intersect( rayP1, rayP2, hit, plane ) )
				{
					return 0.0f;
				}
			}
		}
	}
	return 1.0f;*/


	/*// Optim avec quad tree
	for( sint32 i = 0; i < wrt.bbBoxes.size(); ++i )
	{
		if( wrt.bbBoxes[i].intersect( rayP1, rayP2, rayP2 ) )
		{
			wrt.qtAccel[i].selectSegment( rayP1, rayP2 );

			CQuadTree<SWorldRTCell>::CIterator it = wrt.qtAccel[i].begin();
			while( it != wrt.qtAccel[i].end() )
			{
				SWorldRTCell cell = *it;

				CVector hit;
				CTriangle t(cell.pMB->Vertices[cell.pF->Corner[0].Vertex],
							cell.pMB->Vertices[cell.pF->Corner[1].Vertex],
							cell.pMB->Vertices[cell.pF->Corner[2].Vertex] );
				CPlane plane;
				plane.make( t.V0, t.V1, t.V2 );

				if( t.intersect( rayP1, rayP2, hit, plane ) )
				{
					return 0.0f;
				}

				// Next selected element
				++it;
			}
		}
	}
	return 1.0f;*/
	

	/* // No optim
	for( sint32 i = 0; i < Meshes.size(); ++i )
	{
		CMesh::CMeshBuild *pMB = Meshes[i].first;
		// Test the ray with the bsphere first
		if( segmentIntersectBSphere( rayP1, rayP2, Meshes[i].second ) )
		{
			CMatrix MBMatrix = getObjectToWorldMatrix( pMB );
			for( sint32 j = 0; j < pMB->Faces.size(); ++j )
			{
				CVector hit;
				CTriangle t(MBMatrix * pMB->Vertices[pMB->Faces[j].Corner[0].Vertex],
							MBMatrix * pMB->Vertices[pMB->Faces[j].Corner[1].Vertex],
							MBMatrix * pMB->Vertices[pMB->Faces[j].Corner[2].Vertex] );
				CPlane plane;
				plane.make( t.V0, t.V1, t.V2 );

				if( t.intersect( rayP1, rayP2, hit, plane ) )
				{
					return 0.0f;
				}
			}
		}
	}*/
}

// -----------------------------------------------------------------------------------------------
float RayTraceAVertex( CVector &p, SWorldRT &wrt, sint32 nLightNb, SLightBuild& rLight )
{
	double rFactor = 0.0;
	sint32 nLightForFactor = 0;

	TTicks zeTime = CTime::getPerformanceTime();

	switch( rLight.Type )
	{
		case SLightBuild::LightAmbient:
			rFactor = 1.0;
		break;
		case SLightBuild::LightSpot:
		case SLightBuild::LightPoint:
		{
			CVector light_p = p - rLight.Position;
			float light_p_distance = light_p.norm();
			light_p_distance = light_p_distance - (0.01+(0.05*light_p_distance/100.0)); // Substract n centimeter
			light_p.normalize();
			light_p *= light_p_distance;
			rFactor = TestRay( rLight.Position, rLight.Position + light_p, wrt, nLightNb );
			nLightForFactor++;
		}
		break;
		case SLightBuild::LightDir:
		/*
		{
			CVector r1 = p-rLight.Direction/100;
			CVector r2 = p-100*rLight.Direction;
			rFactor = TestRay( r1, r2, wrt );
			nLightForFactor++;
		}
		*/
			rFactor = 1.0; // Not done for the moment
		break;
		default:
		break;
	}

	timerCalcRT += CTime::getPerformanceTime() - zeTime;

	if( rFactor > 0.0 )	
		return 1.0f;
	else
		return 0.0f;
	//return rFactor / ((double)nLightForFactor);
}

// -----------------------------------------------------------------------------------------------
CRGBAF LightAVertex( CVector &pRT, CVector &p, CVector &n, 
					vector<sint32> &vLights, vector<SLightBuild> &AllLights,
					SWorldRT &wrt, bool bDoubleSided )
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
		float factor = 0.0;
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

				// ??? light_intensity *= light_intensity * light_intensity;
				if( bDoubleSided && (n*p_light < 0.0f) )
				{
					p_light = -p_light;
				}
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
				light_intensity = max(0.0f, n*p_light);
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

				// ??? light_intensity *= light_intensity * light_intensity;
				if( bDoubleSided && (n*p_light < 0.0f) )
				{
					p_light = -p_light;
				}
				light_intensity *= max(0.0f, n*p_light);
				lightDiffCol.R = light_intensity * rLight.Diffuse.R / 255.0f;
				lightDiffCol.G = light_intensity * rLight.Diffuse.G / 255.0f;
				lightDiffCol.B = light_intensity * rLight.Diffuse.B / 255.0f;
				lightDiffCol.A = light_intensity * rLight.Diffuse.A / 255.0f;
			}
			break;
			default:
			break;
		}
		if( light_intensity > 0.0f )
		{
			if( ( rLight.bCastShadow ) && ( theExportSceneStruct.bShadow ) )
				factor = RayTraceAVertex( pRT, wrt, vLights[nLight], rLight );
			else
				factor = 1.0f;
			factor *= rLight.rMult;
		}			
		
		rgbafRet.R += lightAmbiCol.R + lightDiffCol.R * factor;
		if( rgbafRet.R > 1.0f ) rgbafRet.R = 1.0;
		rgbafRet.G += lightAmbiCol.G + lightDiffCol.G * factor;
		if( rgbafRet.G > 1.0f ) rgbafRet.G = 1.0;
		rgbafRet.B += lightAmbiCol.B + lightDiffCol.B * factor;
		if( rgbafRet.B > 1.0f ) rgbafRet.B = 1.0;
		rgbafRet.A += lightAmbiCol.A + lightDiffCol.A * factor;
		if( rgbafRet.A > 1.0f ) rgbafRet.A = 1.0;
	}
	return rgbafRet;
}

// -----------------------------------------------------------------------------------------------
CVector CalcInterpolatedVertexInFace( SGradient &g, double Uin, double Vin, CMesh::CFace *pF )
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
	return CalculateInterpolatedVertex( g, Uout, Vout );
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
void FirstLight( CMesh::CMeshBuild* pMB, SLMPlane &Plane, vector<CVector> &vVertices, 
				CMatrix& ToWorldMat, vector<sint32> &vLights, vector<SLightBuild> &AllLights,
				sint32 nLayerNb, SWorldRT &wrt )
{
	// Fill interiors
	vector<CMesh::CFace*>::iterator ItFace = Plane.faces.begin();
	sint32 nNbFace = Plane.faces.size();
	sint32 i, j, k;
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

		bool doubleSided = pMB->Materials[pF->MaterialId].detDoubleSided();

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
	
		CalculateGradient( g, pF, vVertices, n1, n2, n3 );

		// Process all the interior
		for( k = nPosMinV; k <= nPosMaxV; ++k )
		for( j = nPosMinU; j <= nPosMaxU; ++j )
		{
			if( isInTriangleOrEdge( j+0.5, k+0.5,
									pF->Corner[0].Uvws[1].U, pF->Corner[0].Uvws[1].V,
									pF->Corner[1].Uvws[1].U, pF->Corner[1].Uvws[1].V,
									pF->Corner[2].Uvws[1].U, pF->Corner[2].Uvws[1].V ) )
			{
				CVector p = CalculateInterpolatedVertex( g, j+0.5, k+0.5);
				CVector n = CalculateInterpolatedNormal( g, j+0.5, k+0.5);
				CRGBAF col = LightAVertex( p, p, n, vLights, AllLights, wrt, doubleSided );
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w].p[nLayerNb].R = col.R;
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w].p[nLayerNb].G = col.G;
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w].p[nLayerNb].B = col.B;
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w].p[nLayerNb].A = 1.0f;
				// Darken the plane to indicate pixel is calculated
				Plane.msk[j-Plane.x + (k-Plane.y)*Plane.w] = 2;
			}
		}
		// Next Face
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
void SecondLight( CMesh::CMeshBuild*pMB, vector<SLMPlane*>::iterator ItPlanes, sint32 nNbPlanes,
					vector<CVector> &vVertices, CMatrix& ToWorldMat, 
					vector<sint32> &vLights, vector<SLightBuild> &AllLights,
					sint32 nLayerNb, SWorldRT &wrt)
{
	// Fill interiors
	sint32 i, j, k;
	sint32 nPosMinU, nPosMaxU, nPosMinV, nPosMaxV;
	SGradient g;

	vector<SLMPlane*>::iterator ItPlanes1 = ItPlanes;
	for( sint32 nPlanes1 = 0; nPlanes1 < nNbPlanes; ++nPlanes1 )
	{
		SLMPlane *pPlane1 = *ItPlanes1;
		vector<CMesh::CFace*>::iterator ItParseI = pPlane1->faces.begin();
		sint32 nNbFace1 = pPlane1->faces.size();
		for( i = 0; i < nNbFace1; ++i )
		{
			CMesh::CFace *pF1 = *ItParseI;
			double rMinU = 1000000.0, rMaxU = -1000000.0, rMinV = 1000000.0, rMaxV = -1000000.0;
			bool doubleSided = pMB->Materials[pF1->MaterialId].detDoubleSided();
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
	
			CalculateGradient( g, pF1, vVertices, n1, n2, n3 );

			double	lumx1 = pF1->Corner[0].Uvws[1].U, lumy1 = pF1->Corner[0].Uvws[1].V, 
					lumx2 = pF1->Corner[1].Uvws[1].U, lumy2 = pF1->Corner[1].Uvws[1].V, 
					lumx3 = pF1->Corner[2].Uvws[1].U, lumy3 = pF1->Corner[2].Uvws[1].V;

			// Process all the exterior and try to link with other planes
			for( k = nPosMinV; k < nPosMaxV; ++k )
			for( j = nPosMinU; j < nPosMaxU; ++j )
			if( ( pPlane1->msk[j-pPlane1->x   + (k-pPlane1->y)*pPlane1->w]   == 1 ) ||
				( pPlane1->msk[1+j-pPlane1->x + (k-pPlane1->y)*pPlane1->w]   == 1 ) ||
				( pPlane1->msk[1+j-pPlane1->x + (1+k-pPlane1->y)*pPlane1->w] == 1 ) ||
				( pPlane1->msk[j-pPlane1->x   + (1+k-pPlane1->y)*pPlane1->w] == 1 ) )
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
				segmentIntersection(j+1.5, k+1.5, j+0.5, k+1.5, lumx3, lumy3, lumx1, lumy1) )
			{
				// If all segment of the current face are linked with a face in this plane, no need to continue
				vector<CMesh::CFace*>::iterator ItParseM = pPlane1->faces.begin();
				sint32 nNbSeg = 0;
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
					SLMPlane *pPlane2 = *ItParsePlanes;
					if( pPlane2 != pPlane1 )
					for( n = 0; n < pPlane2->faces.size(); ++n )
					{
						CMesh::CFace *pF2 = pPlane2->faces[n];
						if( FaceContinuous( pF1, pF2 ) )
						{
							for( sint32 o = 0; o < 4; ++o )
							{
								sint32 nAbsX = j + (o/2), nAbsY = k + (o%2);
								// Is it a pixel to treat and pixel in the 2nd plane
								if( ( pPlane1->msk[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w] == 1 ) &&
									(nAbsX >= pPlane2->x) && (nAbsX < (pPlane2->x+pPlane2->w) ) &&
									(nAbsY >= pPlane2->y) && (nAbsY < (pPlane2->y+pPlane2->h) ) )
								{
									// Is it an interior calculated pixel ?
									if( pPlane2->msk[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w] == 2 )
									{ // Yes -> ok so get it
										pPlane1->col[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w].p[nLayerNb] = 
													pPlane2->col[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w].p[nLayerNb];
										pPlane1->msk[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w] = 3;
									}
									else
									if( pPlane2->msk[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w] == 1 )
									{ // No -> Add extrapolated value
										CVector iv = CalculateInterpolatedVertex( g, ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
										CVector in = CalculateInterpolatedNormal( g, ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);										
										CVector rv = CalcInterpolatedVertexInFace( g, ((double)nAbsX)+0.5, ((double)nAbsY)+0.5, pF1 );
										CRGBAF col = LightAVertex( rv, iv, in, vLights, AllLights, wrt, doubleSided );
										//float f = 1.0f;
										pPlane2->col[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w].p[nLayerNb].R += col.R;
										pPlane2->col[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w].p[nLayerNb].G += col.G;
										pPlane2->col[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w].p[nLayerNb].B += col.B;
										pPlane2->col[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w].p[nLayerNb].A += 1.0f;
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
					if( pPlane1->msk[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w] == 1 )
					{
						CVector iv = CalculateInterpolatedVertex( g, ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
						CVector in = CalculateInterpolatedNormal( g, ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
						CVector rv = CalcInterpolatedVertexInFace( g, ((double)nAbsX)+0.5, ((double)nAbsY)+0.5, pF1 );
						CRGBAF col = LightAVertex( rv, iv, in, vLights, AllLights, wrt, doubleSided );
						//float f = 1.0f;
						pPlane1->col[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w].p[nLayerNb].R += col.R;
						pPlane1->col[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w].p[nLayerNb].G += col.G;
						pPlane1->col[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w].p[nLayerNb].B += col.B;
						pPlane1->col[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w].p[nLayerNb].A += 1.0f;
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
		SLMPlane *pPlane1 = *ItPlanes1;
		
		for( k = 0; k < pPlane1->h; ++k )
		for( j = 0; j < pPlane1->w; ++j )
		{
			if( pPlane1->msk[j+k*pPlane1->w] == 1 )
			{
				sint32 nNbNormals = pPlane1->col[j + k*pPlane1->w].p[nLayerNb].A;
				pPlane1->col[j + k*pPlane1->w].p[nLayerNb].R /= nNbNormals;
				pPlane1->col[j + k*pPlane1->w].p[nLayerNb].G /= nNbNormals;
				pPlane1->col[j + k*pPlane1->w].p[nLayerNb].B /= nNbNormals;
				pPlane1->col[j + k*pPlane1->w].p[nLayerNb].A = 1.0f;
				pPlane1->msk[j + k*pPlane1->w] = 4;
			}
		}
		++ItPlanes1;
	}
}

// -----------------------------------------------------------------------------------------------
bool IsAllFaceMapped( vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFaces )
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
	if( fabsf(TextureSurf) < 0.000001 )
		return false;
	return true;
}

// -----------------------------------------------------------------------------------------------
CAABBox getMeshBBox( CMesh::CMeshBuild& rMB, bool bNeedToTransform )
{
	CAABBox meshBox;
	if( bNeedToTransform )
	{
		CMatrix MBMatrix = getObjectToWorldMatrix( &rMB );

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
bool isInteractionLightMesh( SLightBuild &rSLB, CMesh::CMeshBuild &rMB )
{
	CAABBox meshBox;

	if( rSLB.Type == SLightBuild::LightAmbient )
		return true;
	meshBox = getMeshBBox( rMB, true );
	return isLightCanCastShadowOnBox( rSLB, meshBox );
}

bool isInteractionLightMeshWithoutAmbient( SLightBuild &rSLB, CMesh::CMeshBuild &rMB )
{
	CAABBox meshBox;

	if( rSLB.Type == SLightBuild::LightAmbient )
		return false;
	meshBox = getMeshBBox( rMB, true );
	return isLightCanCastShadowOnBox( rSLB, meshBox );
}

// -----------------------------------------------------------------------------------------------
// Get all lights that can cast shadows on the current mesh
void getLightInteract( CMesh::CMeshBuild* pMB, vector<SLightBuild> &AllLights, vector< vector<sint32> >&vvLights )
{
	uint32 nNbGroup = 0;
	vector<sint32> vlbTmp;
	uint32 i, j;

	for( i = 0; i < AllLights.size(); ++i )
	{
		if( isInteractionLightMesh( AllLights[i], *pMB ) )
		{
			// Is the light name already exist
			for( j = 0; j < nNbGroup; ++j )
				if( AllLights[vvLights[j].operator[](0)].GroupName == AllLights[i].GroupName )
					break;
			// The light name does not exist create a new group
			if( ( j == nNbGroup ) && ( nNbGroup < 8 ) )
			{
				vvLights.push_back( vlbTmp ); // Static lighting
				vvLights[nNbGroup].push_back( i );
				++nNbGroup;
			}
			else
			{ // The light name already exist or there is not enought groups
				if( j == nNbGroup )
					j = nNbGroup - 1;
				vvLights[j].push_back( i );
			}
		}
	}
}

// -----------------------------------------------------------------------------------------------
void GetAllNodeInScene( vector< CMesh::CMeshBuild* > &Meshes, vector<SLightBuild> &AllLights, INode* pNode = NULL )
{
	if( pNode == NULL )
		pNode = theCNelExport.ip->GetRootNode();

	// Get a pointer on the object's node
	TimeValue tvTime = theCNelExport.ip->GetTime();

	if( ! RPO::isZone( *pNode, tvTime ) )
	if( CExportNel::isMesh( *pNode, tvTime ) )
	{
		CMesh::CMeshBuild *pMB;
		pMB = CExportNel::createMeshBuild( *pNode, tvTime );
		// If the mesh has no interaction with one of the light selected we do not need it
		bool bInteract = false;
		for( uint32 i = 0; i < AllLights.size(); ++i )
		if( isInteractionLightMeshWithoutAmbient( AllLights[i], *pMB ) )
		{
			bInteract = true;
			break;
		}
		if( bInteract )
			Meshes.push_back( pMB );
		else
			delete pMB; // No interaction so delete the mesh
	}

	for( sint32 i = 0; i < pNode->NumberOfChildren(); ++i )
		GetAllNodeInScene( Meshes, AllLights, pNode->GetChildNode(i) );
}

// -----------------------------------------------------------------------------------------------
void buildWorldRT( SWorldRT &wrt, vector<SLightBuild> &AllLights )
{
	uint32 i, j, k;

	// Get all the nodes in the scene
	GetAllNodeInScene( wrt.vMB, AllLights );

	// Transform the meshbuilds vertices and normals to have world coordinates
	for( i = 0; i < wrt.vMB.size(); ++i )
	{
		CMatrix MBMatrix = getObjectToWorldMatrix( wrt.vMB[i] );
		// Update vertices
		for( j = 0; j < wrt.vMB[i]->Vertices.size(); ++j )
			wrt.vMB[i]->Vertices[j] = MBMatrix * wrt.vMB[i]->Vertices[j];
		// Update normals
		MBMatrix.invert();
		MBMatrix.transpose();
		for( j = 0; j < wrt.vMB[i]->Faces.size(); ++j )
			for( k = 0; k < 3 ; ++k )
				wrt.vMB[i]->Faces[j].Corner[k].Normal = 
									MBMatrix.mulVector( wrt.vMB[i]->Faces[j].Corner[k].Normal );
	}

	// Construct all cube grids from all lights
	wrt.cgAccel.resize( AllLights.size() );
	for( i = 0; i < AllLights.size(); ++i )
	{
		wrt.cgAccel[i].create( 64 ); // width of each grid in number of square
		switch( AllLights[i].Type )
		{
			case SLightBuild::LightAmbient:
				// No ambient handled for the moment
			break;
			case SLightBuild::LightSpot: // For the moment spot like point
			case SLightBuild::LightPoint:
			{
				for( j = 0; j < wrt.vMB.size(); ++j )
				{
				for( k = 0; k < wrt.vMB[j]->Faces.size(); ++k )
				{
					SCubeGridCell cell;
					cell.pF = &(wrt.vMB[j]->Faces[k]);
					cell.pMB = wrt.vMB[j];
					CTriangle tri = CTriangle( 
						cell.pMB->Vertices[cell.pF->Corner[0].Vertex] - AllLights[i].Position,
						cell.pMB->Vertices[cell.pF->Corner[1].Vertex] - AllLights[i].Position,
						cell.pMB->Vertices[cell.pF->Corner[2].Vertex] - AllLights[i].Position );
					if( intersectionTriangleSphere( tri, CBSphere(CVector(0,0,0), AllLights[i].rRadiusMax) ) )
						wrt.cgAccel[i].insert( tri, cell );
				}
				}
			}
			break;
			case SLightBuild::LightDir:
				// No directionnal handled for the moment
			break;
		}
	}	
	


	// Construct the aabboxes of the buildmeshes
	/*	wrt.bbBoxes.resize( wrt.vMB.size() );
	for( i = 0; i < wrt.vMB.size(); ++i )
	{
		wrt.bbBoxes[i] = getMeshBBox( *wrt.vMB[i], false );
	} */

	/* // Construct the quad tree for each mesh
	wrt.qtAccel.resize( wrt.vMB.size() );
	for( i = 0; i < wrt.vMB.size(); ++i )
	{
		wrt.qtAccel[i].create( 4, wrt.bbBoxes[i].getCenter(), max(wrt.bbBoxes[i].getSize().x, wrt.bbBoxes[i].getSize().y) );

		// Put the quadtree in the XY basis
		CMatrix	tmp;
		CVector	I( 1, 0, 0  );
		CVector	J( 0, 0, -1 );
		CVector	K( 0, 1, 0  );
		tmp.identity();
		tmp.setRot( I, J, K, true );
		wrt.qtAccel[i].changeBase( tmp );

		for( j = 0; j < wrt.vMB[i]->Faces.size(); ++j )
		{
			SWorldRTCell cell;
			CAABBox bbFace;
			cell.pF = &(wrt.vMB[i]->Faces[j]);
			cell.pMB = wrt.vMB[i];
			bbFace.setCenter( cell.pMB->Vertices[cell.pF->Corner[0].Vertex] );
			bbFace.extend( cell.pMB->Vertices[cell.pF->Corner[1].Vertex] );
			bbFace.extend( cell.pMB->Vertices[cell.pF->Corner[2].Vertex] );
			wrt.qtAccel[i].insert( bbFace.getMin(), bbFace.getMax(), cell );
		}
	} */

	/* // Construct the bsp tree for each mesh
	wrt.btAccel.resize( wrt.vMB.size() );
	for( i = 0; i < wrt.vMB.size(); ++i )
	{
		for( j = 0; j < wrt.vMB[i]->Faces.size(); ++j )
		{
			SWorldRTCell cell;
			CTriangle tri;
			cell.pF = &(wrt.vMB[i]->Faces[j]);
			cell.pMB = wrt.vMB[i];
			tri.V0 = cell.pMB->Vertices[cell.pF->Corner[0].Vertex];
			tri.V1 = cell.pMB->Vertices[cell.pF->Corner[1].Vertex];
			tri.V2 = cell.pMB->Vertices[cell.pF->Corner[2].Vertex];
			wrt.btAccel[i].insert( tri, cell );
		}
		sint32 nNbNode = wrt.btAccel[i].getNbNode();
	} */
}

// -----------------------------------------------------------------------------------------------
// Is the box b1 can cast shadow on the box b2 with the light l ?
bool isBoxCanCastShadowOnBoxWithLight( CAABBox &b1, CAABBox &b2, SLightBuild &l )
{
	// if the light is included in the box b2
	return isLightCanCastShadowOnBox( l, b1 );
}

// -----------------------------------------------------------------------------------------------
void supprLightNoInteract( vector<SLightBuild> &vLights, 
						  vector< pair < CMesh::CMeshBuild*,INode* > > &AllSelectedMeshes )
{
	uint32 i, j;

	for( i = 0; i < vLights.size(); ++i )
	{
		bool bInteract = false;

		for( j = 0; j < AllSelectedMeshes.size(); ++j )
		if( isInteractionLightMesh( vLights[i], *AllSelectedMeshes[j].first ) )
		{
			bInteract = true;
			break;
		}
		if( !bInteract )
		{
			// Suppress the light because it has no interaction with selected meshes
			for( j = i; j < (vLights.size()-1); ++j )
				vLights[j] = vLights[j+1];
			vLights.resize(vLights.size()-1);
			--i;
		}
	}
}

// -----------------------------------------------------------------------------------------------
void supprLightNoInteractOne( vector<SLightBuild> &vLights, CMesh::CMeshBuild* pMB)
{
	uint32 i, j;

	for( i = 0; i < vLights.size(); ++i )
	{
		bool bInteract = false;

		if( isInteractionLightMesh( vLights[i], *pMB ) )
		{
			bInteract = true;
			break;
		}
		if( !bInteract )
		{
			// Suppress the light because it has no interaction with selected meshes
			for( j = i; j < (vLights.size()-1); ++j )
				vLights[j] = vLights[j+1];
			vLights.resize(vLights.size()-1);
			--i;
		}
	}
}

// -----------------------------------------------------------------------------------------------
// Construct the world accelerator for raytrace
void buildWorldRTAccel( SWorldRT &wrt, CMesh::CMeshBuild* pMB, vector< vector<sint32> > &vvLights )
{
	sint32 i, j, k;
	// Determine all meshes that can cast shadows on the current mesh with current lights
	/*
	vector<sint32> allmesh;
	CAABBox curBBox = getMeshBBox( *pMB, true );
	for( j = 0; j < wrt.vMB.size(); ++j )
	{
		for( k = 0; k < vvLights.size(); ++k )
		{
			vector<SLightBuild*> &vLights = vvLights[k];
			for( i = 0; i < vLights.size(); ++i )
			{
				if( isBoxCanCastShadowOnBoxWithLight( wrt.bbBoxes[j] , curBBox, *vLights[i] ) )
				{
					allmesh.push_back( j );
					break;
				}
			}
		}
	}

	// Clear the container
	wrt.qtAccel.clear();
	// Construct the accel

	CAABBox bbWorld;
	sint32 nNbVertices = 0;
	// Get the BBox of all meshes that can cast shadow on the current mesh
	for( i = 0; i < allmesh.size(); ++i )
	{
		nNbVertices += wrt.vMB[allmesh[i]]->Vertices.size();
		for( j = 0; j < wrt.vMB[allmesh[i]]->Vertices.size(); ++j )
			if( (j == 0) && ( i == 0 ) )
				bbWorld.setCenter( wrt.vMB[allmesh[i]]->Vertices[j] );
			else
				bbWorld.extend( wrt.vMB[allmesh[i]]->Vertices[j] );
	}

	wrt.qtAccel.create( 9, bbWorld.getCenter(), max(bbWorld.getSize().x,bbWorld.getSize().y) );

	// Put the quadtree in the XY basis
	CMatrix	tmp;
	CVector	I( 1, 0, 0  );
	CVector	J( 0, 0, -1 );
	CVector	K( 0, 1, 0  );
	tmp.identity();
	tmp.setRot( I, J, K, true );
	wrt.qtAccel.changeBase( tmp );

	for( i = 0; i < allmesh.size(); ++i )
	{
		for( j = 0; j < wrt.vMB[allmesh[i]]->Faces.size(); ++j )
		{
			SWorldRTCell cell;
			CAABBox bbFace;
			cell.pF = &(wrt.vMB[allmesh[i]]->Faces[j]);
			cell.pMB = wrt.vMB[allmesh[i]];
			bbFace.setCenter( cell.pMB->Vertices[cell.pF->Corner[0].Vertex] );
			bbFace.extend( cell.pMB->Vertices[cell.pF->Corner[1].Vertex] );
			bbFace.extend( cell.pMB->Vertices[cell.pF->Corner[2].Vertex] );
			wrt.qtAccel.insert( bbFace.getMin(), bbFace.getMax(), cell );
		}
	}*/
}

volatile bool bCancelCalculation = false;
float gRatioCalculated;
DWORD gTimeBegin;

int CALLBACK CalculatingDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	DWORD TimeCurrent =  timeGetTime();
	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			CenterWindow( hwndDlg, theCNelExport.ip->GetMAXHWnd() );
			ShowWindow( hwndDlg, SW_SHOWNORMAL );
			gRatioCalculated = 0.0;
			gTimeBegin = timeGetTime();
			bCancelCalculation = false;
		}
		break;

		case WM_PAINT:
		{
			char temp[256];
			SendMessage( GetDlgItem( hwndDlg, IDC_PROGRESS1 ), PBM_SETPOS, gRatioCalculated*100, 0 );

			if( gRatioCalculated > 0.0 )
			{
				DWORD TimeLeft = ((TimeCurrent - gTimeBegin) / gRatioCalculated) * (1.0-gRatioCalculated);
				sprintf( temp, "Time remaining : %02d h %02d m %02d s", TimeLeft/3600000,
																		(TimeLeft/60000)%60,
																		(TimeLeft/1000)%60 );

				SendMessage( GetDlgItem( hwndDlg, IDC_STATICTIMELEFT ), WM_SETTEXT, 0, (long)temp );
				SendMessage( GetDlgItem( hwndDlg, IDC_BUTTONCANCEL ), WM_PAINT, 0, 0 );
			}
		}
		break;

		case WM_DESTROY:
			bCancelCalculation = true;
		break;
		case WM_COMMAND:
		{
			switch( LOWORD(wParam) )
			{
				// ---
				case IDC_BUTTONCANCEL:
					if( HIWORD(wParam) == BN_CLICKED )
						bCancelCalculation = true;
				break;
				default:
				break;
			}
		}		
		break;
		default:
			return FALSE;
		break;
	}
	return TRUE;
}

// -----------------------------------------------------------------------------------------------
void AddLightInfo( CMesh::CMeshBuild *pMB, string &LightName, uint8 nMatNb, uint8 nStageNb )
{
	CMesh::CMatStage ms;
	ms.nMatNb = nMatNb;
	ms.nStageNb = nStageNb;
	CMesh::CLightInfoMapList listTemp;
	//list< pair< uint8, uint8 > > listTemp;
	CMesh::TLightInfoMap::iterator itMap = pMB->LightInfoMap.find( LightName );
	if( itMap == pMB->LightInfoMap.end() )
	{
		listTemp.push_back(	ms );
		pMB->LightInfoMap.insert( pair< string, CMesh::CLightInfoMapList >(LightName, listTemp) );
	}
	else
	{
		itMap->second.push_back( ms );
	}
}

// -----------------------------------------------------------------------------------------------
bool isAllBlack( SLMPlane &Plane, uint8 nLayerNb )
{
	for( sint32 i = 0; i < Plane.w*Plane.h; ++i )
		if( (Plane.col[i].p[nLayerNb].R > 0.06f) || // around 15/255
			(Plane.col[i].p[nLayerNb].G > 0.06f) ||
			(Plane.col[i].p[nLayerNb].B > 0.06f) )
			return false; // Not all is black
	return true;
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CNelExport::deleteLM(INode& ZeNode)
{
	sint32 i;

	// Suppress all lightmap files
	for( i = 0; i < 8; ++i )
	{		
		string sSaveName;
		sSaveName = theExportSceneStruct.sExportLighting;
		if( sSaveName[sSaveName.size()-1] != '\\' ) sSaveName += "\\";
		sSaveName += ZeNode.GetName();
		char tmp[32];
		sprintf( tmp, "%d", i );
		sSaveName += tmp;
		sSaveName += ".tga";
		FILE *file = nlfopen(sSaveName, "rb")
		if (file)
		{
			fclose( file );
			DeleteFile( sSaveName.c_str() );
		}
	}
}


//bool CNelExport::exportMeshLM(const char *sPath, INode& ZeNode, Interface& ip, TimeValue tvTime)
bool CNelExport::calculateLM(CMesh::CMeshBuild *pZeMeshBuild, INode& ZeNode, Interface& ip, TimeValue tvTime, bool absolutePath)
{
	// 1 -> Build all MeshBuild structure
	// ----------------------------------
		//vector< pair < CMesh::CMeshBuild*,INode* > > AllMeshBuilds;
	int nNbMesh = 0;
	int i, j;
/*
	if( RPO::isZone( ZeNode, tvTime ) )
		return false;
	if( ! CExportNel::isMesh( ZeNode, tvTime ) )
		return false;

	pZeMeshBuild = CExportNel::createMeshBuild( ZeNode, tvTime );
*/
	// 2 -> LightMapping V2
	// --------------------
	SWorldRT WorldRT; // The static world for raytrace
	vector<SLightBuild> AllLights;

	// Select meshes to test for raytrace
	// Get all lights from MAX
	getLightBuilds( AllLights, tvTime, *theCNelExport.ip );
	// Get all lights L that have influence over the mesh selected
	// supprLightNoInteract( AllLights, AllMeshBuilds );
	supprLightNoInteractOne( AllLights, pZeMeshBuild );
	// Get all meshes that are influenced by the lights L			
	buildWorldRT( WorldRT, AllLights );

	//for( nNode=0; nNode < nNbMesh; ++nNode )
	{
		// First order face by Material and by texture surface
		CMesh::CMeshBuild *pMB = pZeMeshBuild;
		vector<CMesh::CFace*> AllFaces;
		CMatrix MBMatrix = getObjectToWorldMatrix( pMB );
		vector<CVector> AllVertices; // All vertices in world space
		vector<sint32> FaceGroupByMat; // Number of faces with the same properties
		sint32 nNbFace = pMB->Faces.size(), nNbVertex = pMB->Vertices.size();
		sint32 offsetMat, offsetSmooth, offsetPlane;
		vector<SLMPlane*> AllPlanes;
		sint32 AllPlanesPrevSize;
		vector< vector<sint32> > vvLights;

		// Select Lights interacting with the node
		getLightInteract( pMB, AllLights, vvLights );
		// Construct the raytrace accelerator from those lights
		buildWorldRTAccel( WorldRT, pMB, vvLights );

		//gRatioCalculated = ((float)nNode) / ((float)nNbMesh);
		//for( i = 0; i < 32; ++i )
		//{
		//	MSG msg;
		//	PeekMessage(&msg,(HWND)hwndCalculating,0,0,PM_REMOVE);
		//	{
		//		if( IsDialogMessage(hwndCalculating,&msg) )
		//		{
		//			TranslateMessage(&msg);
		//			DispatchMessage(&msg);
		//		}
		//	}
		//}

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
		SortFaceByMaterialId( FaceGroupByMat, AllFaces.begin(), AllFaces.size() );

		if( ! IsAllFaceMapped( AllFaces.begin(), AllFaces.size() ) )
		{
			string thetext;
			//thetext = "Object ";
			thetext += ZeNode.GetName();
			thetext = "have not all this faces mapped";
			MessageBox( NULL, thetext.c_str(), "Warning", MB_OK|MB_ICONERROR );
			return false;
		}

		// PATCH
		FaceGroupByMat.resize(1);
		FaceGroupByMat[0] = AllFaces.size();

		

		offsetMat = 0;
		for( uint32 nMat = 0; nMat < FaceGroupByMat.size(); ++nMat )
		{
			vector<sint32> FaceGroupBySmooth;

			// Sort faces by smoothing group
			SortFaceBySMoothGroup( FaceGroupBySmooth, AllFaces.begin()+offsetMat, FaceGroupByMat[nMat] );

			offsetSmooth = offsetMat;
			for( uint32 nSmoothNb = 0; nSmoothNb < FaceGroupBySmooth.size(); ++nSmoothNb )
			{
				uint32 nPlaneNb, nLight;
				vector<sint32> FaceGroupByPlane;
				
				if( ! PutFaceUV1InLumelCoord( theExportSceneStruct.rLumelSize, AllVertices, 
										AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] ) )
					continue;

				SortFaceByPlane( FaceGroupByPlane, AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] );
				//AllPlanes.resize( FaceGroupByPlane.size() );

				//SortPlanesBySurface( FaceGroupByPlane, AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] );

				AllPlanesPrevSize = AllPlanes.size();
				AllPlanes.resize( AllPlanesPrevSize + FaceGroupByPlane.size() );

				offsetPlane = offsetSmooth;
				for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
				{
					AllPlanes[AllPlanesPrevSize+nPlaneNb] = new SLMPlane;
					AllPlanes[AllPlanesPrevSize+nPlaneNb]->nNbLayerUsed = vvLights.size();
					// Fill planes (part of lightmap)
					CreateLMPlaneFromFaceGroup( *AllPlanes[AllPlanesPrevSize+nPlaneNb], 
												AllFaces.begin()+offsetPlane, FaceGroupByPlane[nPlaneNb] );
					// Next group of face with the same plane in the same smooth group of the same material
					offsetPlane += FaceGroupByPlane[nPlaneNb];
				}
				// Make join between all planes (all planes must be created)

				for( nLight = 0; nLight < vvLights.size(); ++nLight )
				{
					for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
					{					
						// Light the LightMap for the plane (interior only)
						FirstLight( pMB, *AllPlanes[AllPlanesPrevSize+nPlaneNb], 
									AllVertices, MBMatrix, vvLights[nLight], AllLights,
									nLight, WorldRT );
					}
					SecondLight( pMB, AllPlanes.begin()+AllPlanesPrevSize, FaceGroupByPlane.size(),
								AllVertices, MBMatrix, vvLights[nLight], AllLights,
								nLight, WorldRT );
				}
				if( theExportSceneStruct.nOverSampling > 1 )
				{
					for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
						ModifyLMPlaneWithOverSampling( AllPlanes[AllPlanesPrevSize+nPlaneNb],
														theExportSceneStruct.nOverSampling );
					for( nLight = 0; nLight < vvLights.size(); ++nLight )
					{
						for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
							FirstLight( pMB, *AllPlanes[AllPlanesPrevSize+nPlaneNb], 
										AllVertices, MBMatrix, vvLights[nLight], AllLights,
										nLight, WorldRT );
						SecondLight( pMB, AllPlanes.begin()+AllPlanesPrevSize, FaceGroupByPlane.size(),
									AllVertices, MBMatrix, vvLights[nLight],  AllLights,
									nLight, WorldRT );
					}						
					for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
						ModifyLMPlaneWithOverSampling( AllPlanes[AllPlanesPrevSize+nPlaneNb],
														1.0/((double)theExportSceneStruct.nOverSampling) );
				}
				// Next group of face with the same smooth group and the same material
				offsetSmooth += FaceGroupBySmooth[nSmoothNb];
			}
			// Next group of face with the same material
			offsetMat += FaceGroupByMat[nMat];
		}
		
		// Create the lightmap
		
		SLMPlane LightMap;
		SortPlanesBySurface( AllPlanes );
		for( i = 0; i < AllPlanes.size(); ++i )
		{
			// Put in the basis of the plane
			MoveFaceUV1( AllPlanes[i]->faces.begin(), AllPlanes[i]->faces.size(), 
						-AllPlanes[i]->x, -AllPlanes[i]->y );
			PlaceLMPlaneInLMPLane( LightMap, *AllPlanes[i] );
			LightMap.nNbLayerUsed = AllPlanes[i]->nNbLayerUsed;
			// Put in the new basis
			MoveFaceUV1( AllPlanes[i]->faces.begin(), AllPlanes[i]->faces.size(), 
						AllPlanes[i]->x, AllPlanes[i]->y );
			delete AllPlanes[i];
		}
		
		// Save the lightmap				
		// Assign the name of the lightmap and get the complete save name

		// Update UV coords to Texture space
		PutFaceUV1InTextureCoord( LightMap.w, LightMap.h, AllFaces.begin(), nNbFace );
		sint32 nLightMapNb = 0;
		for( j = 0; j < LightMap.nNbLayerUsed; ++j )
		if( ! isAllBlack( LightMap, j ) )
		{
			CTextureFile *pLightMap = new CTextureFile();
			//string sSaveName = AllMeshBuilds[nNode].second->GetName();
			string sSaveName = ZeNode.GetName();
			char tmp[32];
			sprintf( tmp, "%d", nLightMapNb );
			sSaveName += tmp;
			sSaveName += ".tga";
			pLightMap->setFileName( sSaveName );
			sSaveName = theExportSceneStruct.sExportLighting;
			if( sSaveName[sSaveName.size()-1] != '\\' ) sSaveName += "\\";
			sSaveName += pLightMap->getFileName();
			CopyPlaneColToBitmap32( pLightMap, LightMap, j );
			COFile f( sSaveName );
			pLightMap->writeTGA( f, 32 );	

			for( i = 0; i < pMB->Materials.size(); ++i )
			{
				pMB->Materials[i].setLightMap( nLightMapNb, pLightMap );
				//AllMeshBuilds[nNode].first->Materials[i].setLighting( false );
				AddLightInfo( pMB, AllLights[vvLights[j].operator[](0)].GroupName, i, nLightMapNb );
				int a = pMB->LightInfoMap.size();
			}
			++nLightMapNb;
		}		
		// Next mesh
		
		//if( bCancelCalculation )
		//	break;
	}
	for( i = 0; i < WorldRT.vMB.size(); ++i )
		delete WorldRT.vMB[i];

	// End of the lighting process for this node we have to export the data
	//for( nNode=0; nNode < nNbMesh; ++nNode )
	{
		// First order face by Material and by texture surface
		//CMesh::CMeshBuild *pMB = AllMeshBuilds[nNode].first;
		CMesh::CMeshBuild *pMB = pZeMeshBuild;
		CMesh* mesh = new CMesh;
		pMB->VertexFlags |= CVertexBuffer::TexCoord1Flag;
		// Build the mesh with the build interface

		for( i = 0; i < pMB->Materials.size(); ++i )
		{
			pMB->Materials[i].setLighting( false );
			pMB->Materials[i].setColor( CRGBA(255,255,255,255) );
		}
/*
		mesh->build( *pMB );

		COFile file;
		if (file.open( sPath ))
		{
			try
			{
				// Create a streamable shape
				CShapeStream shapeStream( mesh );
				// Serial the shape
				shapeStream.serial (file);
			}
			catch (...)
			{
			}
		}

		// Delete the pointer
		delete mesh;
*/
	}		
	// Delete the window
	//DestroyWindow( hwndCalculating );

	// ? -> Ending deletion des objets temporaires
	//for( nNode = 0; nNode < nNbMesh; ++nNode )
	//	delete AllMeshBuilds[nNode].first;
	//delete pZeMeshBuild;
	return true;	
}


bool CNelExport::exportScene(std::vector<INode*>& vectNode)
{
	// All inputs are transfered throught the global theExportSceneStruct structure

	// Export the Instance Group (aka scene description)
	// *************************
	if( theExportSceneStruct.bExportInstanceGroup )
		exportInstanceGroup( theExportSceneStruct.sExportInstanceGroup, vectNode );

	// Export all the shapes
	// *********************
	// If lighting enabled we export the shape after lightmapping modification
	if( theExportSceneStruct.bExportShapes && (!theExportSceneStruct.bExportLighting) )
	{
		// Get time
		TimeValue tvTime = theCNelExport.ip->GetTime();

		// Get node count
		int nNumSelNode = vectNode.size();

		// Save all selected objects
		for (int nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=vectNode[nNode];

			// It is a zone ?
			if (RPO::isZone (*pNode, tvTime))
			{
				// Nothing to do if this is a zone
			}
			// Try to export a mesh
			else if (CExportNel::isMesh (*pNode, tvTime))
			{
				// Build the name of the file to export
				char sSavePath[512];
				strcpy( sSavePath, theExportSceneStruct.sExportShapes.c_str() );
				if(theExportSceneStruct.sExportShapes[theExportSceneStruct.sExportShapes.size()-1] != '\\' )
					strcat( sSavePath, "\\" );
				strcat( sSavePath, pNode->GetName() );
				strcat( sSavePath, ".shape" );

				// Export the mesh
				if (!theCNelExport.exportMesh (sSavePath, *pNode, *theCNelExport.ip, tvTime))
				{
					// Error message
					char sErrorMsg[512];
					sprintf (sErrorMsg, "Error exporting the mesh %s in the file\n%s", pNode->GetName(), sSavePath);
					MessageBox (theCNelExport.ip->GetMAXHWnd(), sErrorMsg, "NeL export scene", MB_OK|MB_ICONEXCLAMATION );
				}
			}
		}
	}

	// Export the lighting (aka compute all light maps)
	// *******************
	if( theExportSceneStruct.bExportLighting )
	{
		TTicks zeTime = CTime::getPerformanceTime();
		// 1 -> Build all MeshBuild structure
		// ----------------------------------
		vector< pair < CMesh::CMeshBuild*,INode* > > AllMeshBuilds;
		TimeValue tvTime = theCNelExport.ip->GetTime();
		int nNumSelNode = vectNode.size();
		int nNbMesh = 0;
		int nNode, i, j;


		// Launch the dialog box
		bCancelCalculation = true;
		HWND hwndCalculating = CreateDialog( CNelExportDesc.HInstance(), 
											 MAKEINTRESOURCE(IDD_CALCULATING), 
											 theCNelExport.ip->GetMAXHWnd(), 
											 CalculatingDialogCallback );

		// Count number of mesh in the selection
		for( nNode = 0; nNode < nNumSelNode; ++nNode )
		{
			INode* pNode = vectNode[nNode];
			if( ! RPO::isZone( *pNode, tvTime ) )
			if( CExportNel::isMesh( *pNode, tvTime ) )
				nNbMesh++;
		}
		AllMeshBuilds.resize(nNbMesh);
		nNbMesh = 0;
		// Create all the MeshBuild used to place lightmaps and to raytrace lightmaps
		for( nNode=0; nNode < nNumSelNode; ++nNode )
		{
			INode* pNode = vectNode[nNode];
			if( ! RPO::isZone( *pNode, tvTime ) )
			if( CExportNel::isMesh( *pNode, tvTime ) )
			{
				AllMeshBuilds[nNbMesh].second = pNode;
				AllMeshBuilds[nNbMesh].first = CExportNel::createMeshBuild( *pNode, tvTime );
				nNbMesh++;
			}
		}

		// Suppress all lightmap files
		for( nNode=0; nNode < nNbMesh; ++nNode )
		for( i = 0; i < 8; ++i )
		{		
			string sSaveName;
			sSaveName = theExportSceneStruct.sExportLighting;
			if( sSaveName[sSaveName.size()-1] != '\\' ) sSaveName += "\\";
			sSaveName += AllMeshBuilds[nNode].second->GetName();
			char tmp[32];
			sprintf( tmp, "%d", i );
			sSaveName += tmp;
			sSaveName += ".tga";
			FILE *file = nlfopen(sSaveName, "rb");
			if (file)
			{
				fclose( file );
				DeleteFile( sSaveName.c_str() );
			}
		}
		

		// 2 -> LightMapping V2
		// --------------------
		SWorldRT WorldRT; // The static world for raytrace
//		vector< pair < CMesh::CMeshBuild*, CBSphere > > MeshBuildForRaytrace;
		vector<SLightBuild> AllLights;

		// Select meshes to test for raytrace
		// Get all lights from MAX
		getLightBuilds( AllLights, tvTime, *theCNelExport.ip );
		// Get all lights L that have influence over the mesh selected
		supprLightNoInteract( AllLights, AllMeshBuilds );
		// Get all meshes that are influenced by the lights L			
		buildWorldRT( WorldRT, AllLights );

		for( nNode=0; nNode < nNbMesh; ++nNode )
		{
			// First order face by Material and by texture surface
			CMesh::CMeshBuild *pMB = AllMeshBuilds[nNode].first;
			vector<CMesh::CFace*> AllFaces;
			CMatrix MBMatrix = getObjectToWorldMatrix( pMB );
			vector<CVector> AllVertices; // All vertices in world space
			vector<sint32> FaceGroupByMat; // Number of faces with the same properties
			sint32 nNbFace = pMB->Faces.size(), nNbVertex = pMB->Vertices.size();
			sint32 offsetMat, offsetSmooth, offsetPlane;
			vector<SLMPlane*> AllPlanes;
			sint32 AllPlanesPrevSize;
			vector< vector<sint32> > vvLights;

			TTicks zeTime2 = CTime::getPerformanceTime();

			// Select Lights interacting with the node
			getLightInteract( pMB, AllLights, vvLights );
			// Construct the raytrace accelerator from those lights
			buildWorldRTAccel( WorldRT, pMB, vvLights );

			gRatioCalculated = ((float)nNode) / ((float)nNbMesh);

			for( i = 0; i < 32; ++i )
			{
				MSG msg;
				PeekMessage(&msg,(HWND)hwndCalculating,0,0,PM_REMOVE);
				{
					if( IsDialogMessage(hwndCalculating,&msg) )
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}

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
			SortFaceByMaterialId( FaceGroupByMat, AllFaces.begin(), AllFaces.size() );

			if( ! IsAllFaceMapped( AllFaces.begin(), AllFaces.size() ) )
			{
				string thetext;
				//thetext = "Object ";
				//thetext += AllMeshBuilds[nNode].second->GetName();
				//thetext = "have not all this faces mapped";
				//MessageBox( NULL, thetext.c_str(), "Warning", MB_OK );
				continue;
			}

			// PATCH
			FaceGroupByMat.resize(1);
			FaceGroupByMat[0] = AllFaces.size();

			timerInit += CTime::getPerformanceTime() - zeTime2;

			zeTime2 = CTime::getPerformanceTime();
			offsetMat = 0;
			for( uint32 nMat = 0; nMat < FaceGroupByMat.size(); ++nMat )
			{
				vector<sint32> FaceGroupBySmooth;

				// Sort faces by smoothing group
				SortFaceBySMoothGroup( FaceGroupBySmooth, AllFaces.begin()+offsetMat, FaceGroupByMat[nMat] );

				offsetSmooth = offsetMat;
				for( uint32 nSmoothNb = 0; nSmoothNb < FaceGroupBySmooth.size(); ++nSmoothNb )
				{
					uint32 nPlaneNb, nLight;
					vector<sint32> FaceGroupByPlane;
					
					if( ! PutFaceUV1InLumelCoord( theExportSceneStruct.rLumelSize, AllVertices, 
											AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] ) )
						continue;

					SortFaceByPlane( FaceGroupByPlane, AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] );
					//AllPlanes.resize( FaceGroupByPlane.size() );

					//SortPlanesBySurface( FaceGroupByPlane, AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] );

					AllPlanesPrevSize = AllPlanes.size();
					AllPlanes.resize( AllPlanesPrevSize + FaceGroupByPlane.size() );

					offsetPlane = offsetSmooth;
					for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
					{
						AllPlanes[AllPlanesPrevSize+nPlaneNb] = new SLMPlane;
						AllPlanes[AllPlanesPrevSize+nPlaneNb]->nNbLayerUsed = vvLights.size();
						// Fill planes (part of lightmap)
						CreateLMPlaneFromFaceGroup( *AllPlanes[AllPlanesPrevSize+nPlaneNb], 
													AllFaces.begin()+offsetPlane, FaceGroupByPlane[nPlaneNb] );
						// Next group of face with the same plane in the same smooth group of the same material
						offsetPlane += FaceGroupByPlane[nPlaneNb];
					}
					// Make join between all planes (all planes must be created)

					for( nLight = 0; nLight < vvLights.size(); ++nLight )
					{
						for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
						{					
							// Light the LightMap for the plane (interior only)
							FirstLight( pMB, *AllPlanes[AllPlanesPrevSize+nPlaneNb], 
										AllVertices, MBMatrix, vvLights[nLight], AllLights,
										nLight, WorldRT );
						}
						SecondLight( pMB, AllPlanes.begin()+AllPlanesPrevSize, FaceGroupByPlane.size(),
									AllVertices, MBMatrix, vvLights[nLight], AllLights,
									nLight, WorldRT );
					}
					if( theExportSceneStruct.nOverSampling > 1 )
					{
						for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
							ModifyLMPlaneWithOverSampling( AllPlanes[AllPlanesPrevSize+nPlaneNb],
															theExportSceneStruct.nOverSampling );
						for( nLight = 0; nLight < vvLights.size(); ++nLight )
						{
							for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
								FirstLight( pMB, *AllPlanes[AllPlanesPrevSize+nPlaneNb], 
											AllVertices, MBMatrix, vvLights[nLight], AllLights,
											nLight, WorldRT );
							SecondLight( pMB, AllPlanes.begin()+AllPlanesPrevSize, FaceGroupByPlane.size(),
										AllVertices, MBMatrix, vvLights[nLight],  AllLights,
										nLight, WorldRT );
						}						
						for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
							ModifyLMPlaneWithOverSampling( AllPlanes[AllPlanesPrevSize+nPlaneNb],
															1.0/((double)theExportSceneStruct.nOverSampling) );
					}
					// Next group of face with the same smooth group and the same material
					offsetSmooth += FaceGroupBySmooth[nSmoothNb];
				}
				// Next group of face with the same material
				offsetMat += FaceGroupByMat[nMat];
			}
			timerCalc += CTime::getPerformanceTime() - zeTime2;
			// Create the lightmap
			zeTime2 = CTime::getPerformanceTime();
			SLMPlane LightMap;
			SortPlanesBySurface( AllPlanes );
			for( i = 0; i < AllPlanes.size(); ++i )
			{
				// Put in the basis of the plane
				MoveFaceUV1( AllPlanes[i]->faces.begin(), AllPlanes[i]->faces.size(), 
							-AllPlanes[i]->x, -AllPlanes[i]->y );
				PlaceLMPlaneInLMPLane( LightMap, *AllPlanes[i] );
				LightMap.nNbLayerUsed = AllPlanes[i]->nNbLayerUsed;
				// Put in the new basis
				MoveFaceUV1( AllPlanes[i]->faces.begin(), AllPlanes[i]->faces.size(), 
							AllPlanes[i]->x, AllPlanes[i]->y );
				delete AllPlanes[i];
			}
			timerPlac += CTime::getPerformanceTime() - zeTime2;
			// Save the lightmap				
			// Assign the name of the lightmap and get the complete save name

			zeTime2 = CTime::getPerformanceTime();
			// Update UV coords to Texture space
			PutFaceUV1InTextureCoord( LightMap.w, LightMap.h, AllFaces.begin(), nNbFace );
			sint32 nLightMapNb = 0;
			for( j = 0; j < LightMap.nNbLayerUsed; ++j )
			if( ! isAllBlack( LightMap, j ) )
			{
				CTextureFile *pLightMap = new CTextureFile();
				string sSaveName = AllMeshBuilds[nNode].second->GetName();
				char tmp[32];
				sprintf( tmp, "%d", nLightMapNb );
				sSaveName += tmp;
				sSaveName += ".tga";
				pLightMap->setFileName( sSaveName );
				sSaveName = theExportSceneStruct.sExportLighting;
				if( sSaveName[sSaveName.size()-1] != '\\' ) sSaveName += "\\";
				sSaveName += pLightMap->getFileName();
				CopyPlaneColToBitmap32( pLightMap, LightMap, j );
				COFile f( sSaveName );
				pLightMap->writeTGA( f, 32 );	

				for( i = 0; i < pMB->Materials.size(); ++i )
				{
					pMB->Materials[i].setLightMap( nLightMapNb, pLightMap );
					//AllMeshBuilds[nNode].first->Materials[i].setLighting( false );
					AddLightInfo( pMB, AllLights[vvLights[j].operator[](0)].GroupName, i, nLightMapNb );
					int a = pMB->LightInfoMap.size();
				}
				++nLightMapNb;
			}
			timerSave += CTime::getPerformanceTime() - zeTime2;
			// Next mesh
			//delete pLightMap; // TODO vector<> de lightmap
			if( bCancelCalculation )
				break;
		}
		for( i = 0; i < WorldRT.vMB.size(); ++i )
			delete WorldRT.vMB[i];

		// End of the lighting process for this node we have to export the data
		for( nNode=0; nNode < nNbMesh; ++nNode )
		{
			// First order face by Material and by texture surface
			CMesh::CMeshBuild *pMB = AllMeshBuilds[nNode].first;
			CMesh* mesh = new CMesh;
			pMB->VertexFlags |= CVertexBuffer::TexCoord1Flag;
			// Build the mesh with the build interface

			for( i = 0; i < pMB->Materials.size(); ++i )
			{
				pMB->Materials[i].setLighting( false );
				pMB->Materials[i].setColor( CRGBA(255,255,255,255) );
			}

			mesh->build( *pMB );

			COFile file;
			char sSavePath[512];
			strcpy( sSavePath, theExportSceneStruct.sExportShapes.c_str() );
			if(theExportSceneStruct.sExportShapes[theExportSceneStruct.sExportShapes.size()-1] != '\\' )
				strcat( sSavePath, "\\" );
			strcat( sSavePath, AllMeshBuilds[nNode].second->GetName() );
			strcat( sSavePath, ".shape" );
			if (file.open( sSavePath ))
			{
				try
				{
					// Create a streamable shape
					CShapeStream shapeStream( mesh );
					// Serial the shape
					shapeStream.serial (file);
				}
				catch (...)
				{
				}
			}

			// Delete the pointer
			delete mesh;
		}		
		// Delete the window
		DestroyWindow( hwndCalculating );

		// ? -> Ending deletion des objets temporaires
		for( nNode = 0; nNode < nNbMesh; ++nNode )
			delete AllMeshBuilds[nNode].first;

		timerExportLighting =  CTime::getPerformanceTime() - zeTime;
		{
			string toDisp;
			char tam[1024];
			sprintf( tam, "timerExport = %f sec\n", (float)CTime::ticksToSecond( timerExportLighting ) );
			toDisp += tam;
			sprintf( tam, "timerInit = %f %%\n", 100*((float)timerInit)/((float)timerExportLighting) );
			toDisp += tam;
			sprintf( tam, "timerCalc = %f %%\n", 100*((float)timerCalc)/((float)timerExportLighting) );
			toDisp += tam;
			sprintf( tam, "    timerCalcRT = %f %%\n", 100*((float)timerCalcRT)/((float)timerExportLighting) );
			toDisp += tam;
			sprintf( tam, "timerPlac = %f %%\n", 100*((float)timerPlac)/((float)timerExportLighting) );
			toDisp += tam;
			sprintf( tam, "timerSave = %f %%\n", 100*((float)timerSave)/((float)timerExportLighting) );
			toDisp += tam;
			MessageBox( theCNelExport.ip->GetMAXHWnd(), toDisp.c_str(), "Info", MB_OK );
		}

	} // End if lighting process asked
		
	return true;
}
