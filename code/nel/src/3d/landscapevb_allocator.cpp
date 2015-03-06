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

#include "nel/3d/landscapevb_allocator.h"
#include "nel/3d/driver.h"
#include "nel/misc/fast_mem.h"


using namespace std;
using namespace NLMISC;

namespace NL3D
{

/*
	Once a reallocation of a VBHard occurs, how many vertices we add to the re-allocation, to avoid
	as possible reallocations.
*/
#define	NL3D_LANDSCAPE_VERTEX_ALLOCATE_SECURITY		1024
/*
	The start size of the array.
*/
#define	NL3D_LANDSCAPE_VERTEX_ALLOCATE_START		4048


#define	NL3D_VERTEX_FREE_MEMORY_RESERVE	1024
/*// 65000 is a maximum because of GeForce limitations.
#define	NL3D_VERTEX_MAX_VERTEX_VBHARD	40000*/


// ***************************************************************************
CLandscapeVBAllocator::CLandscapeVBAllocator(TType type, const std::string &vbName)
{
	_Type= type;
	_VBName= vbName;
	_VertexFreeMemory.reserve(NL3D_VERTEX_FREE_MEMORY_RESERVE);

	_ReallocationOccur= false;
	_NumVerticesAllocated= 0;
	_BufferLocked= false;
	_LastFarVB = NULL;
	_LastNearVB = NULL;

	for(uint i=0;i<MaxVertexProgram;i++)
		_VertexProgram[i]= NULL;
}

// ***************************************************************************
CLandscapeVBAllocator::~CLandscapeVBAllocator()
{
	clear();
}


// ***************************************************************************
void			CLandscapeVBAllocator::updateDriver(IDriver *driver)
{
	// test change of driver.
	nlassert(driver);
	if( _Driver==NULL || driver!=_Driver )
	{
		deleteVertexBuffer();
		_Driver= driver;


		// If change of driver, delete the VertexProgram first, if any
		deleteVertexProgram();
		// Then rebuild VB format, and VertexProgram, if needed.
		// Do it only if VP supported by GPU.
		setupVBFormatAndVertexProgram(!_Driver->isVertexProgramEmulated() && (
			_Driver->supportVertexProgram(CVertexProgram::nelvp)
			// || _Driver->supportVertexProgram(CVertexProgram::glsl330v) // TODO_VP_GLSL
			));

		// must reallocate the VertexBuffer.
		if( _NumVerticesAllocated>0 )
			allocateVertexBuffer(_NumVerticesAllocated);
	}
}


// ***************************************************************************
void			CLandscapeVBAllocator::clear()
{
	// clear list.
	_VertexFreeMemory.clear();
	_NumVerticesAllocated= 0;

	// delete the VB.
	deleteVertexBuffer();

	// delete vertex Program, if any
	deleteVertexProgram();

	// clear other states.
	_ReallocationOccur= false;
	_Driver= NULL;
}



// ***************************************************************************
void			CLandscapeVBAllocator::resetReallocation()
{
	_ReallocationOccur= false;
}



// ***************************************************************************
// ***************************************************************************
// allocation.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
uint			CLandscapeVBAllocator::allocateVertex()
{
	// if no more free, allocate.
	if( _VertexFreeMemory.size()==0 )
	{
		// enlarge capacity.
		uint	newResize;
		if(_NumVerticesAllocated==0)
			newResize= NL3D_LANDSCAPE_VERTEX_ALLOCATE_START;
		else
			newResize= NL3D_LANDSCAPE_VERTEX_ALLOCATE_SECURITY;
		_NumVerticesAllocated+= newResize;
		// re-allocate VB.
		#ifdef NL_LANDSCAPE_INDEX16
			nlassert(_NumVerticesAllocated <= 65535);
		#endif
		allocateVertexBuffer(_NumVerticesAllocated);
		// resize infos on vertices.
		_VertexInfos.resize(_NumVerticesAllocated);

		// Fill list of free elements.
		for(uint i=0;i<newResize;i++)
		{
			// create a new entry which points to this vertex.
			// the list is made so allocation is in growing order.
			_VertexFreeMemory.push_back( _NumVerticesAllocated - (i+1) );

			// Mark as free the new vertices. (Debug).
			_VertexInfos[_NumVerticesAllocated - (i+1)].Free= true;
		}
	}

	// get a vertex (pop_back).
	uint	id= _VertexFreeMemory.back();
	// delete this vertex free entry.
	_VertexFreeMemory.pop_back();

	// check and Mark as not free the vertex. (Debug).
	nlassert(id<_NumVerticesAllocated);
	nlassert(_VertexInfos[id].Free);
	_VertexInfos[id].Free= false;




	return id;
}

// ***************************************************************************
void			CLandscapeVBAllocator::deleteVertex(uint vid)
{
	// check and Mark as free the vertex. (Debug).
	nlassert(vid<_NumVerticesAllocated);
	nlassert(!_VertexInfos[vid].Free);
	_VertexInfos[vid].Free= true;

	// Add this vertex to the free list.
	// create a new entry which points to this vertex.
	_VertexFreeMemory.push_back( vid );
}


// ***************************************************************************
void			CLandscapeVBAllocator::lockBuffer(CFarVertexBufferInfo &farVB)
{
	nlassert( _Type==Far0 || _Type==Far1 );

	// force unlock
	unlockBuffer();

	_LastFarVB = &farVB;

	farVB.setupVertexBuffer(_VB, _VertexProgram[0]!=NULL );

	_BufferLocked= true;
}
// ***************************************************************************
void			CLandscapeVBAllocator::lockBuffer(CNearVertexBufferInfo &tileVB)
{
	nlassert(_Type==Tile);

	// force unlock
	unlockBuffer();

	_LastNearVB = &tileVB;

	tileVB.setupVertexBuffer(_VB, _VertexProgram[0]!=NULL );

	_BufferLocked= true;
}
// ***************************************************************************
void			CLandscapeVBAllocator::unlockBuffer()
{
	if(_BufferLocked)
	{
		if (_LastFarVB)
			_LastFarVB->setupNullPointers();
		_LastFarVB = NULL;
		if (_LastNearVB)
			_LastNearVB->setupNullPointers();
		_LastNearVB = NULL;
		_BufferLocked= false;
	}
}


// ***************************************************************************
// ***************************************************************************
// VertexBuffer mgt.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLandscapeVBAllocator::activate(uint vpId)
{
	nlassert(_Driver);
	nlassert(!_BufferLocked);

	activateVP(vpId);

	_Driver->activeVertexBuffer(_VB);
}


// ***************************************************************************
void			CLandscapeVBAllocator::activateVP(uint vpId)
{
	nlassert(_Driver);

	// If enabled, activate Vertex program first.
	if (_VertexProgram[vpId])
	{
		//nlinfo("\nSTARTVP\n%s\nENDVP\n", _VertexProgram[vpId]->getProgram().c_str());
		nlverify(_Driver->activeVertexProgram(_VertexProgram[vpId]));
	}
}


// ***************************************************************************
void				CLandscapeVBAllocator::deleteVertexBuffer()
{
	// must unlock VBhard before.
	unlockBuffer();

	// delete the soft one.
	_VB.deleteAllVertices();
}


// ***************************************************************************
void				CLandscapeVBAllocator::allocateVertexBuffer(uint32 numVertices)
{
	// no allocation must be done if the Driver is not setuped, or if the driver has been deleted by refPtr.
	nlassert(_Driver);

	// allocate() =>_ReallocationOccur= true;
	_ReallocationOccur= true;
	// must unlock VBhard before.
	unlockBuffer();

	// This always works.
	_VB.setPreferredMemory(CVertexBuffer::AGPPreferred, false);
	_VB.setNumVertices(numVertices);
	_VB.setName (_VBName);
}


// ***************************************************************************
// ***************************************************************************
// Vertex Program.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/*
	Common Part. Inputs and constants.

	Inputs
	--------
	Standard:
	v[0] == StartPos.	Hence, It is the SplitPoint of the father face.
	v[8] == Tex0 (xy)
	v[9] == Tex1 (xy) (different meanings for Far and Tile).
	v[13] == Tex2 (xy) (just for Tile mode).

	Geomorph:
	v[10] == { GeomFactor, MaxNearLimit }
		* where GeomFactor == max(SizeFaceA, SizeFaceB) * OORefineThreshold.
		It's means vertices are re-computed when the RefineThreshold setup change.

		* MaxNearLimit= max(nearLimitFaceA, nearLimitFaceB)

	v[11].xyz == EndPos-StartPos

	Alpha: NB: Since only useful for Far1, v[12] is not in the VB for Far0 and Tile VertexBuffer.
	v[12] == { TransitionSqrMin, OOTransitionSqrDelta}
		* TransitionSqrMin, OOTransitionSqrDelta : Alpha transition, see preRender().
			There is only 3 values possibles. It depends on Far1 type. Changed in preRender()


	Constant:
	--------
	Setuped at beginning of CLandscape::render()
	c[0..3]= ModelViewProjection Matrix.
	c[4]= {0, 1, 0.5, 0}
	c[5]= RefineCenter
	c[6]= {TileDistFarSqr, OOTileDistDeltaSqr, *, *}
	c[7]= ???
	c[8..11]= ModelView Matrix (for Fog).
	c[12]= PZBModelPosition: landscape center / delta Position to apply before multipliying by mviewMatrix


	Fog Note:
	-----------
	Fog is computed on geomorphed position R1.
	R1.w==1, and suppose that ModelViewMatrix has no Projection Part.
	Then Homogenous-coordinate == Non-Homogenous-coordinate.
	Hence we need only (FogVector*R1).z to get the FogC value.
	=> computed in just on instruction.
*/


// ***********************
/*
	Common start of the program for Far0, Far1 and Tile mode.
	It compute the good Geomorphed position.
	At the end of this program, nothing is written in the output register, and we have in the Temp Registers:

	- R0= scratch
	- R1= CurrentPos geomorphed
	- R2.x= sqrDist= (startPos - RefineCenter).sqrnorm(). Useful for alpha computing.

Pgr Len= 18.
NB: 9 ope for normal errorMetric, and 9 ope for smoothing with TileNear.

	The C code for this Program is:  (v[] means data is a vertex input, c[] means it is a constant)
	{
		// Compute Basic ErrorMetric.
		sqrDist= (v[StartPos] - c[RefineCenter]).sqrnorm()
		ErrorMetric= v[GeomFactor] / sqrDist

		// Compute ErrorMetric modified by TileNear transition.
		f= (c[TileDistFarSqr] - sqrDist) * c[OOTileDistDeltaSqr]
		clamp(f, 0, 1);
		// ^4 gives better smooth result
		f= sqr(f);	f= sqr(f);
		// interpolate the errorMetric
		ErrorMetricModified= v[MaxNearLimit]*f + ErrorMetric*(1-f);

		// Take the max errorMetric.
		ErrorMetric= max(ErrorMetric, ErrorMetricModified);

		// Interpolate StartPos to EndPos, between 1 and 2.
		f= ErrorMetric - 1;
		clamp(f, 0, 1);
		R1= f * v[EndPos-StartPos] + StartPos;
	}

*/
const char* NL3D_LandscapeCommonStartProgram=
"!!VP1.0																				\n\
	# compute Basic geomorph into R0.x													\n\
	ADD	R0, v[0], -c[5];			# R0 = startPos - RefineCenter						\n\
	DP3	R2.x, R0, R0;				# R2.x= sqrDist= (startPos - RefineCenter).sqrnorm()\n\
	RCP	R0.x, R2.x;					# R0.x= 1 / sqrDist									\n\
	MUL	R0.x, v[10].x, R0.x;		# R0.x= ErrorMetric= GeomFactor / sqrDist			\n\
																						\n\
	# compute Transition Factor To TileNear Geomorph, into R0.z							\n\
	ADD	R0.z, c[6].x, -R2.x;		# R0.z= TileDistFarSqr - sqrDist					\n\
	MUL	R0.z, R0.z, c[6].y;			# R0.z= f= (TileDistFarSqr - sqrDist ) * OOTileDistDeltaSqr	\n\
	MAX R0.z, R0.z, c[4].x;																\n\
	MIN R0.z, R0.z, c[4].y;			# R0.z= f= clamp(f, 0, 1);							\n\
	MUL R0.z, R0.z, R0.z;																\n\
	MUL R0.z, R0.z, R0.z;			# R0.z= finalFactor= f^4							\n\
																						\n\
	# Apply the transition factor to the ErrorMetric => R0.w= ErrorMetricModified.		\n\
	ADD	R0.w, v[10].y, -R0.x;		# R0.w= maxNearLimit - ErrorMetric					\n\
	MAD	R0.w, R0.z, R0.w, R0.x;	# R0.w= finalFactor * (maxNearLimit - ErrorMetric) + ErrorMetric \n\
																						\n\
	# R0.w may be < R0.x; (when the point is very near). Must take the bigger errorMetric.	\n\
	MAX	R0.x, R0.x, R0.w;			# R0.x= ErrorMetric Max								\n\
																						\n\
	# apply geomorph into R1															\n\
	ADD R0.x, R0.x, -c[4].y;		# R0.x= ErrorMetric Max - 1							\n\
	MAX R0.x, R0.x, c[4].x;																\n\
	MIN R0.x, R0.x, c[4].y;			# R0.x= geomBlend= clamp(R0.x, 0, 1);				\n\
																						\n\
	# NB: Can't use MAD R1.xyz, v[11], R0.x, v[0], because can't acces 2 inputs in one op.	\n\
	# Hence, can use a MAD to Sub the Landscape Center _PZBModelPosition				\n\
	# write to R1.w is useless (but needed to avoid a read error when multiplied by 0)  \n\
	# in the next instruction															\n\
	MAD	R1.xyzw, v[11], R0.x, -c[12];													\n\
	# set w to 1 by using c[4] = { 0, 1, 0.5, 0}										\n\
	MAD	R1, R1, c[4].yyyx, v[0];	# R1= geomBlend * (EndPos-StartPos) + StartPos		\n\
";


// ***********************
// Test Speed.
/*"!!VP1.0																				\n\
	# compute Basic geomorph into R0.x													\n\
	ADD	R0, v[0], -c[5];			# R0 = startPos - RefineCenter						\n\
	DP3	R2.x, R0, R0;				# R2.x= sqrDist= (startPos - RefineCenter).sqrnorm()\n\
	MOV	R1, v[0];					# R1= geomBlend * (EndPos-StartPos) + StartPos		\n\
";
*/
const string NL3D_LandscapeTestSpeedProgram=
"	MOV R1, R1; \n	MOV R1, R1; \n	MOV R1, R1; \n	MOV R1, R1; \n	MOV R1, R1; \n\
	MOV R1, R1; \n	MOV R1, R1; \n	MOV R1, R1; \n	MOV R1, R1; \n	MOV R1, R1; \n\
";


// ***********************
/*
	Far0:
		just project, copy uv0 and uv1
		NB: leave o[COL0] undefined because the material don't care diffuse RGBA here
*/
// ***********************
const char* NL3D_LandscapeFar0EndProgram=
"	# compute in Projection space														\n\
	DP4 o[HPOS].x, c[0], R1;															\n\
	DP4 o[HPOS].y, c[1], R1;															\n\
	DP4 o[HPOS].z, c[2], R1;															\n\
	DP4 o[HPOS].w, c[3], R1;															\n\
	MOV o[TEX0], v[8];																\n\
	MOV o[TEX1], v[9];																\n\
	DP4	o[FOGC].x, c[10], R1;															\n\
	END																					\n\
";


// ***********************
/*
	Far1:
		Compute Alpha transition.
		Project, copy uv0 and uv1,
		NB: leave o[COL0] RGB undefined because the material don't care diffuse RGB
*/
// ***********************
const char* NL3D_LandscapeFar1EndProgram=
"	# compute Alpha Transition															\n\
	ADD R0.x, R2.x, -v[12].x;		# R0.x= sqrDist-TransitionSqrMin					\n\
	MUL	R0.x, R0.x, v[12].y;		# R0.x= (sqrDist-TransitionSqrMin) * OOTransitionSqrDelta	\n\
	MAX R0.x, R0.x, c[4].x;																\n\
	MIN o[COL0].w, R0.x, c[4].y;	# col.A= clamp(R0.x, 0, 1);							\n\
																						\n\
	# compute in Projection space														\n\
	DP4 o[HPOS].x, c[0], R1;															\n\
	DP4 o[HPOS].y, c[1], R1;															\n\
	DP4 o[HPOS].z, c[2], R1;															\n\
	DP4 o[HPOS].w, c[3], R1;															\n\
	MOV o[TEX0], v[8];																\n\
	MOV o[TEX1], v[9];																\n\
	DP4	o[FOGC].x, c[10], R1;															\n\
	END																					\n\
";


// ***********************
/*
	Tile:
		just project, copy uv0, uv1.
		NB: leave o[COL0] undefined because the material don't care diffuse RGBA here
*/
// ***********************
const char* NL3D_LandscapeTileEndProgram=
"	# compute in Projection space														\n\
	DP4 o[HPOS].x, c[0], R1;															\n\
	DP4 o[HPOS].y, c[1], R1;															\n\
	DP4 o[HPOS].z, c[2], R1;															\n\
	DP4 o[HPOS].w, c[3], R1;															\n\
	MOV o[TEX0], v[8];																	\n\
	MOV o[TEX1], v[9];																	\n\
	DP4	o[FOGC].x, c[10], R1;															\n\
	END																					\n\
";

/// Same version but write Tex0 to take uv2, ie v[13], for lightmap pass
const char* NL3D_LandscapeTileLightMapEndProgram=
"	# compute in Projection space														\n\
	DP4 o[HPOS].x, c[0], R1;															\n\
	DP4 o[HPOS].y, c[1], R1;															\n\
	DP4 o[HPOS].z, c[2], R1;															\n\
	DP4 o[HPOS].w, c[3], R1;															\n\
	MOV o[TEX0], v[12];																\n\
	MOV o[TEX1], v[9];																\n\
	DP4	o[FOGC].x, c[10], R1;															\n\
	END																					\n\
";


// ***************************************************************************
void			CLandscapeVBAllocator::deleteVertexProgram()
{
	for (uint i = 0; i < MaxVertexProgram; ++i)
	{
		if (_VertexProgram[i])
		{
			_VertexProgram[i] = NULL; // smartptr
		}
	}
}


// ***************************************************************************
void			CLandscapeVBAllocator::setupVBFormatAndVertexProgram(bool withVertexProgram)
{
	// If not vertexProgram mode
	if(!withVertexProgram)
	{
		// setup normal VB format.
		if(_Type==Far0)
			// v3f/t2f0/t2f1
			_VB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag);
		else if(_Type==Far1)
			// v3f/t2f/t2f1/c4ub
			_VB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag | CVertexBuffer::PrimaryColorFlag );
		else
			// v3f/t2f0/t2f1/t2f2
			_VB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag | CVertexBuffer::TexCoord2Flag);
	}
	else
	{
		// Else Setup our Vertex Program, and good VBuffers, according to _Type.

		if(_Type==Far0)
		{
			// Build the Vertex Format.
			_VB.clearValueEx();
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_STARTPOS,	CVertexBuffer::Float3);	// v[0]= StartPos.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_TEX0,		CVertexBuffer::Float2);	// v[8]= Tex0.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_TEX1,		CVertexBuffer::Float2);	// v[9]= Tex1.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_GEOMINFO,	CVertexBuffer::Float2);	// v[10]= GeomInfos.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_DELTAPOS,	CVertexBuffer::Float3);	// v[11]= EndPos-StartPos.
			_VB.initEx();

			// Init the Vertex Program.
			_VertexProgram[0] = new CVertexProgramLandscape(Far0);
			nlverify(_Driver->compileVertexProgram(_VertexProgram[0]));
		}
		else if(_Type==Far1)
		{
			// Build the Vertex Format.
			_VB.clearValueEx();
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_STARTPOS,	CVertexBuffer::Float3);	// v[0]= StartPos.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_TEX0,		CVertexBuffer::Float2);	// v[8]= Tex0.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_TEX1,		CVertexBuffer::Float2);	// v[9]= Tex1.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_GEOMINFO,	CVertexBuffer::Float2);	// v[10]= GeomInfos.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_DELTAPOS,	CVertexBuffer::Float3);	// v[11]= EndPos-StartPos.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_ALPHAINFO,	CVertexBuffer::Float2);	// v[12]= AlphaInfos.
			_VB.initEx();

			// Init the Vertex Program.
			_VertexProgram[0] = new CVertexProgramLandscape(Far1);
			nlverify(_Driver->compileVertexProgram(_VertexProgram[0]));
		}
		else
		{
			// Build the Vertex Format.
			_VB.clearValueEx();
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_STARTPOS,	CVertexBuffer::Float3);	// v[0]= StartPos.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_TEX0,		CVertexBuffer::Float2);	// v[8]= Tex0.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_TEX1,		CVertexBuffer::Float2);	// v[9]= Tex1.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_TEX2,		CVertexBuffer::Float2);	// v[12]= Tex2.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_GEOMINFO,	CVertexBuffer::Float2);	// v[10]= GeomInfos.
			_VB.addValueEx(NL3D_LANDSCAPE_VPPOS_DELTAPOS,	CVertexBuffer::Float3);	// v[11]= EndPos-StartPos.
			_VB.initEx();

			// Init the Vertex Program.
			_VertexProgram[0] = new CVertexProgramLandscape(Tile, false);
			nlverify(_Driver->compileVertexProgram(_VertexProgram[0]));

			// Init the Vertex Program for lightmap pass
			_VertexProgram[1] = new CVertexProgramLandscape(Tile, true);
			nlverify(_Driver->compileVertexProgram(_VertexProgram[1]));
		}
	}
}


CVertexProgramLandscape::CVertexProgramLandscape(CLandscapeVBAllocator::TType type, bool lightMap)
{
	// nelvp
	{
		CSource *source = new CSource();
		source->Profile = nelvp;
		source->DisplayName = "Landscape/nelvp";
		switch (type)
		{
		case CLandscapeVBAllocator::Far0:
			source->DisplayName += "/far0";
			source->setSource(std::string(NL3D_LandscapeCommonStartProgram) 
				+ std::string(NL3D_LandscapeFar0EndProgram));
			break;
		case CLandscapeVBAllocator::Far1:
			source->DisplayName += "/far1";
			source->setSource(std::string(NL3D_LandscapeCommonStartProgram) 
				+ std::string(NL3D_LandscapeFar1EndProgram));
			break;
		case CLandscapeVBAllocator::Tile:
			source->DisplayName += "/tile";
			if (lightMap)
			{
				source->DisplayName += "/lightmap";
				source->setSource(std::string(NL3D_LandscapeCommonStartProgram) 
					+ std::string(NL3D_LandscapeTileLightMapEndProgram));
			}
			else
			{
				source->setSource(std::string(NL3D_LandscapeCommonStartProgram) 
					+ std::string(NL3D_LandscapeTileEndProgram));
			}
			break;
		}
		source->ParamIndices["modelViewProjection"] = 0;
		source->ParamIndices["programConstants0"] = 4;
		source->ParamIndices["refineCenter"] = 5;
		source->ParamIndices["tileDist"] = 6;
		source->ParamIndices["fog"] = 10;
		source->ParamIndices["pzbModelPosition"] = 12;
		addSource(source);
	}
	// TODO_VP_GLSL
	{
		// ....
	}
}

void CVertexProgramLandscape::buildInfo()
{
	m_Idx.ProgramConstants0 = getUniformIndex("programConstants0");
	nlassert(m_Idx.ProgramConstants0 != ~0);
	m_Idx.RefineCenter = getUniformIndex("refineCenter");
	nlassert(m_Idx.RefineCenter != ~0);
	m_Idx.TileDist = getUniformIndex("tileDist");
	nlassert(m_Idx.TileDist != ~0);
	m_Idx.PZBModelPosition = getUniformIndex("pzbModelPosition");
	nlassert(m_Idx.PZBModelPosition != ~0);
}

} // NL3D
