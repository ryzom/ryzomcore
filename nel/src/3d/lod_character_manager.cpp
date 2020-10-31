// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/common.h"
#include "nel/3d/lod_character_manager.h"
#include "nel/3d/lod_character_shape.h"
#include "nel/3d/lod_character_shape_bank.h"
#include "nel/3d/lod_character_instance.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/lod_character_texture.h"
#include "nel/3d/ray_mesh.h"
#include "nel/misc/file.h"
#include "nel/misc/algo.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/system_info.h"


using	namespace std;
using	namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// Dest is without Normal because precomputed
#define	NL3D_CLOD_VERTEX_FORMAT	(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::PrimaryColorFlag)
#define	NL3D_CLOD_VERTEX_SIZE	24
#define	NL3D_CLOD_UV_OFF		12
#define	NL3D_CLOD_COLOR_OFF		20

// size (in block) of the big texture.
#define	NL3D_CLOD_TEXT_NLOD_WIDTH	16
#define	NL3D_CLOD_TEXT_NLOD_HEIGHT	16
#define	NL3D_CLOD_TEXT_NUM_IDS		NL3D_CLOD_TEXT_NLOD_WIDTH*NL3D_CLOD_TEXT_NLOD_HEIGHT
#define	NL3D_CLOD_BIGTEXT_WIDTH		NL3D_CLOD_TEXT_NLOD_WIDTH*NL3D_CLOD_TEXT_WIDTH
#define	NL3D_CLOD_BIGTEXT_HEIGHT	NL3D_CLOD_TEXT_NLOD_HEIGHT*NL3D_CLOD_TEXT_HEIGHT

// Default texture color. Alpha must be 255
#define	NL3D_CLOD_DEFAULT_TEXCOLOR	CRGBA(255,255,255,255)


// ***************************************************************************
CLodCharacterManager::CLodCharacterManager()
{
	_MaxNumVertices= 3000;
	_NumVBHard= 8;
	_Rendering= false;
	_LockDone= false;

	// setup the texture.
	_BigTexture= new CTextureBlank;
	// The texture always reside in memory... This take 1Mo of RAM. (16*32*16*32 * 4)
	// NB: this is simplier like that, and this is not a problem, since only 1 or 2 Mo are allocated :o)
	_BigTexture->setReleasable(false);
	// create the bitmap.
	_BigTexture->resize(NL3D_CLOD_BIGTEXT_WIDTH, NL3D_CLOD_BIGTEXT_HEIGHT, CBitmap::RGBA);
	// Format of texture, 16 bits and no mipmaps.
	_BigTexture->setUploadFormat(ITexture::RGB565);
	_BigTexture->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	_BigTexture->setWrapS(ITexture::Clamp);
	_BigTexture->setWrapT(ITexture::Clamp);

	// Alloc free Ids
	_FreeIds.resize(NL3D_CLOD_TEXT_NUM_IDS);
	for(uint i=0;i<_FreeIds.size();i++)
	{
		_FreeIds[i]= i;
	}

	// setup the material
	_Material.initUnlit();
	_Material.setAlphaTest(true);
	_Material.setDoubleSided(true);
	_Material.setTexture(0, _BigTexture);

	// setup for lighting, Default for Ryzom setup
	_LightCorrectionMatrix.rotateZ((float)Pi/2);
	_LightCorrectionMatrix.invert();
	NL_SET_IB_NAME(_Triangles, "CLodCharacterManager::_Triangles");
}


// ***************************************************************************
CLodCharacterManager::~CLodCharacterManager()
{
	reset();
}

// ***************************************************************************
void			CLodCharacterManager::reset()
{
	nlassert(!isRendering());

	// delete shapeBanks.
	for(uint i=0;i<_ShapeBankArray.size();i++)
	{
		if(_ShapeBankArray[i])
			delete _ShapeBankArray[i];
	}

	// clears containers
	contReset(_ShapeBankArray);
	contReset(_ShapeMap);

	// reset render part.
	_VertexStream.release();
}

// ***************************************************************************
uint32			CLodCharacterManager::createShapeBank()
{
	// search a free entry
	for(uint i=0;i<_ShapeBankArray.size();i++)
	{
		// if ree, use it.
		if(_ShapeBankArray[i]==NULL)
		{
			_ShapeBankArray[i]= new CLodCharacterShapeBank;
			return i;
		}
	}

	// no free entrey, resize array.
	_ShapeBankArray.push_back(new CLodCharacterShapeBank);
	return (uint32)_ShapeBankArray.size()-1;
}

// ***************************************************************************
const CLodCharacterShapeBank	*CLodCharacterManager::getShapeBank(uint32 bankId) const
{
	if(bankId>=_ShapeBankArray.size())
		return NULL;
	else
		return _ShapeBankArray[bankId];
}

// ***************************************************************************
CLodCharacterShapeBank	*CLodCharacterManager::getShapeBank(uint32 bankId)
{
	if(bankId>=_ShapeBankArray.size())
		return NULL;
	else
		return _ShapeBankArray[bankId];
}

// ***************************************************************************
void			CLodCharacterManager::deleteShapeBank(uint32 bankId)
{
	if(bankId>=_ShapeBankArray.size())
	{
		if(_ShapeBankArray[bankId])
		{
			delete _ShapeBankArray[bankId];
			_ShapeBankArray[bankId]= NULL;
		}
	}
}

// ***************************************************************************
sint32			CLodCharacterManager::getShapeIdByName(const std::string &name) const
{
	CstItStrIdMap	it= _ShapeMap.find(name);
	if(it==_ShapeMap.end())
		return -1;
	else
		return it->second;
}

// ***************************************************************************
const CLodCharacterShape	*CLodCharacterManager::getShape(uint32 shapeId) const
{
	// split the id
	uint	bankId= shapeId >> 16;
	uint	shapeInBankId= shapeId &0xFFFF;

	// if valid bankId
	const CLodCharacterShapeBank	*shapeBank= getShapeBank(bankId);
	if(shapeBank)
	{
		// return the shape from the bank
		return shapeBank->getShape(shapeInBankId);
	}
	else
		return NULL;
}

// ***************************************************************************
bool			CLodCharacterManager::compile()
{
	bool	error= false;

	// clear the map
	contReset(_ShapeMap);

	// build the map
	for(uint i=0; i<_ShapeBankArray.size(); i++)
	{
		if(_ShapeBankArray[i])
		{
			// Parse all Shapes
			for(uint j=0; j<_ShapeBankArray[i]->getNumShapes(); j++)
			{
				// build the shape Id
				uint	shapeId= (i<<16) + j;

				// get the shape
				const CLodCharacterShape	*shape= _ShapeBankArray[i]->getShape(j);
				if(shape)
				{
					const string &name= shape->getName();
					ItStrIdMap	it= _ShapeMap.find(name);
					if(it == _ShapeMap.end())
						// insert the id in the map
						_ShapeMap.insert(make_pair(name, shapeId));
					else
					{
						error= true;
						nlwarning("Found a Character Lod with same name in the manager: %s", name.c_str());
					}
				}
			}
		}
	}

	return error;
}

// ***************************************************************************
// ***************************************************************************
// Render
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLodCharacterManager::setMaxVertex(uint32 maxVertex)
{
	// we must not be between beginRender() and endRender()
	nlassert(!isRendering());
	_MaxNumVertices= maxVertex;
}

// ***************************************************************************
void			CLodCharacterManager::setVertexStreamNumVBHard(uint32 numVBHard)
{
	// we must not be between beginRender() and endRender()
	nlassert(!isRendering());
	_NumVBHard= numVBHard;
}

// ***************************************************************************
void			CLodCharacterManager::beginRender(IDriver *driver, const CVector &managerPos)
{
	H_AUTO( NL3D_CharacterLod_beginRender );

	// we must not be between beginRender() and endRender()
	nlassert(!isRendering());

	// Reset render
	//=================
	_CurrentVertexId=0;
	_CurrentTriId= 0;

	// update Driver.
	//=================
	nlassert(driver);

	// test change of vertexStream setup
	bool	mustChangeVertexStream= _VertexStream.getDriver() != driver;
	if(!mustChangeVertexStream)
	{
		mustChangeVertexStream= _MaxNumVertices != _VertexStream.getMaxVertices();
		mustChangeVertexStream= mustChangeVertexStream || _NumVBHard != _VertexStream.getNumVB();
	}
	// re-init?
	if( mustChangeVertexStream )
	{
		// chech offset
		CVertexBuffer	vb;
		vb.setVertexFormat(NL3D_CLOD_VERTEX_FORMAT);
		// NB: addRenderCharacterKey() loop hardCoded for Vertex+UV+Normal+Color only.
		nlassert( NL3D_CLOD_UV_OFF == vb.getTexCoordOff());
		nlassert( NL3D_CLOD_COLOR_OFF == vb.getColorOff());

		// Setup the vertex stream
		_VertexStream.release();
		_VertexStream.init(driver, NL3D_CLOD_VERTEX_FORMAT, _MaxNumVertices, _NumVBHard, "CLodManagerVB", false); // nb : don't use volatile lock as we keep the buffer locked
	}

	// prepare for render.
	//=================

	// Do not Lock Buffer now (will be done at the first instance added)
	nlassert(!_LockDone);
	_VertexSize= _VertexStream.getVertexSize();
	// NB: addRenderCharacterKey() loop hardCoded for Vertex+UV+Normal+Color only.
	nlassert( _VertexSize == NL3D_CLOD_VERTEX_SIZE );	// Vector + Normal + UV + RGBA


	// Alloc a minimum of primitives (2*vertices), to avoid as possible reallocation in addRenderCharacterKey
	if(_Triangles.getNumIndexes()<_MaxNumVertices * 2)
	{
		_Triangles.setFormat(NL_LOD_CHARACTER_INDEX_FORMAT);
		_Triangles.setNumIndexes(_MaxNumVertices * 2);
	}

	// Local manager matrix
	_ManagerMatrixPos= managerPos;

	// Ok, start rendering
	_Rendering= true;
}


// ***************************************************************************
static inline void	computeLodLighting(CRGBA &lightRes, const CVector &lightObjectSpace, const CVector &normalPtr, CRGBA ambient, CRGBA diffuse)
{
	float	f= lightObjectSpace * normalPtr;
	sint	f8= NLMISC::OptFastFloor(f);
	fastClamp8(f8);
	sint	r,g,b;
	r= (diffuse.R * f8)>>8;
	g= (diffuse.G * f8)>>8;
	b= (diffuse.B * f8)>>8;
	r+= ambient.R;
	g+= ambient.G;
	b+= ambient.B;
	fastClamp8(r);
	fastClamp8(g);
	fastClamp8(b);
	lightRes.R= r;
	lightRes.G= g;
	lightRes.B= b;
}


// ***************************************************************************
bool			CLodCharacterManager::addRenderCharacterKey(CLodCharacterInstance &instance, const CMatrix &worldMatrix,
	CRGBA paramAmbient, CRGBA paramDiffuse, const CVector &lightDir)
{
	H_AUTO ( NL3D_CharacterLod_AddRenderKey )

	nlassert(_VertexStream.getDriver());
	// we must be between beginRender() and endRender()
	nlassert(isRendering());


	// regroup all variables that will be accessed in the ASM loop (minimize cache problems)
	uint			numVertices;
	const CLodCharacterShape::CVector3s		*vertPtr;
	const CVector	*normalPtr;
	const CUV		*uvPtr;
	const uint8		*alphaPtr;
	CVector			lightObjectSpace;
	CVector			matPos;
	float			a00, a01, a02;
	float			a10, a11, a12;
	float			a20, a21, a22;
	uint64			blank= 0;
	CRGBA			ambient= paramAmbient;
	CRGBA			diffuse= paramDiffuse;
	// For ASM / MMX, must set 0 to alpha part, because replaced by *alphaPtr (with add)
	ambient.A= 0;
	diffuse.A= 0;


	// Get the Shape and current key.
	//=============

	// get the shape
	const CLodCharacterShape	*clod= getShape(instance.ShapeId);
	// if not found quit, return true
	if(!clod)
		return true;

	// get UV/Normal array. NULL => error
	normalPtr= clod->getNormals();
	// get UV of the instance
	uvPtr= instance.getUVs();
	// uvPtr is NULL means that initInstance() has not been called!!
	nlassert(normalPtr && uvPtr);

	// get the anim key
	CVector		unPackScaleFactor;
	vertPtr= clod->getAnimKey(instance.AnimId, instance.AnimTime, instance.WrapMode, unPackScaleFactor);
	// if not found quit, return true
	if(!vertPtr)
		return true;
	// get num verts
	numVertices= clod->getNumVertices();

	// empty shape??
	if(numVertices==0)
		return true;

	// If too many vertices, quit, returning false.
	if(_CurrentVertexId+numVertices > _MaxNumVertices)
		return false;

	// get alpha array
	static	vector<uint8>	defaultAlphaArray;
	// get the instance alpha if correctly setuped
	if(instance.VertexAlphas.size() == numVertices)
	{
		alphaPtr= &instance.VertexAlphas[0];
	}
	// if error, take 255 as alpha.
	else
	{
		// NB: still use an array. This case should never arise, but support it not at full optim.
		if(defaultAlphaArray.size()<numVertices)
			defaultAlphaArray.resize(numVertices, 255);
		alphaPtr= &defaultAlphaArray[0];
	}

	// Lock Buffer if not done
	//=============

	// Do this after code above because we are sure that we will fill something (numVertices>0)
	if(!_LockDone)
	{
		_VertexData= _VertexStream.lock();
		_LockDone= true;
	}

	// After lock, For D3D, the VertexColor may be in BGRA format
	if(_VertexStream.isBRGA())
	{
		// then swap only the B and R (no cpu cycle added per vertex)
		ambient.swapBR();
		diffuse.swapBR();
	}


	// Prepare Transform
	//=============

	// HTimerInfo: all this block takes 0.1%

	// Get matrix pos.
	matPos= worldMatrix.getPos();
	// compute in manager space.
	matPos -= _ManagerMatrixPos;
	// Get rotation line vectors
	const float *worldM= worldMatrix.get();
	a00= worldM[0]; a01= worldM[4]; a02= worldM[8];
	a10= worldM[1]; a11= worldM[5]; a12= worldM[9];
	a20= worldM[2]; a21= worldM[6]; a22= worldM[10];

	// get the light in object space.
	// Multiply light dir with transpose of worldMatrix. This may be not exact (not uniform scale) but sufficient.
	lightObjectSpace.x= a00 * lightDir.x + a10 * lightDir.y + a20 * lightDir.z;
	lightObjectSpace.y= a01 * lightDir.x + a11 * lightDir.y + a21 * lightDir.z;
	lightObjectSpace.z= a02 * lightDir.x + a12 * lightDir.y + a22 * lightDir.z;
	// animation User correction
	lightObjectSpace= _LightCorrectionMatrix.mulVector(lightObjectSpace);
	// normalize, and neg for Dot Product.
	lightObjectSpace.normalize();
	lightObjectSpace= -lightObjectSpace;
	// preMul by 255 for RGBA uint8
	lightObjectSpace*= 255;

	// multiply matrix with scale factor for Pos.
	a00*= unPackScaleFactor.x; a01*= unPackScaleFactor.y; a02*= unPackScaleFactor.z;
	a10*= unPackScaleFactor.x; a11*= unPackScaleFactor.y; a12*= unPackScaleFactor.z;
	a20*= unPackScaleFactor.x; a21*= unPackScaleFactor.y; a22*= unPackScaleFactor.z;

	// get dst Array.
	uint8	*dstPtr;
	dstPtr= _VertexData + _CurrentVertexId * _VertexSize;


	/* PreCaching Note: CFastMem::precache() has been tested (done on the 4 arrays) but not very interesting,
		maybe because the cache miss improve //ism a bit below.
	*/

	// Fill the VB
	//=============
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	// optimized version
	if(CSystemInfo::hasMMX())
	{
		H_AUTO( NL3D_CharacterLod_vertexFill );

		if(numVertices)
		{
			sint f8;

			/* NB: order is important for AGP filling optimisation in dstPtr

				Pentium2+ optimisation notes:

				- "uop" comment formating:
					A/B means "A micro-ops in port 0, and B micro-ops in port 2".  (port 1 is very rare for FPU)
					A/B/C/D means "A micro-ops in port 0, B in port 2, C in port 3 and D in port 4".
					The number in () is the delay (if any).
				- the "compute lighting part" must done first, because of the "fistp f8" mem writes that must
						be place far away from the "mov eax, f8" read in clamp lighting part
						(else seems that it crashes all the //ism)
				- No need to Interleave on Pentium2+. But prevents "write/read stall" by putting the write
					far away from the next read. Else stall of 3 cycles + BIG BREAK OF //ism (I think).
					This had save me 120 cycles / 240 !!!

				BenchResults:
				- The "transform vertex part" and "all next part" cost 42 cycles, but is somewhat optimal:
					63 uop (=> min 21 cycles), but 36 uop in the P0 port (=> this is the bottleneck)
				- The lighting part adds 1 cycle only ?????  (44 cycles) But still relevant and optimal:
					43 uop in port P0!!!!
				- The UV part adds 4 cycles (47) (should not since 0 in Port P0), still acceptable.
				- The clamp part adds 3 cycles (50), and add 11 cycles in "P0 or P1" (but heavy dependency)
					If we assume all goes into P1, it should takes 0... still acceptable (optimal==43?)
				- The alpha part adds 2 cycles (52, optimal=45). OK.
				- The modulate part adds 15 cycles. OK

				TOTAL: 67 cycles in theory (write in RAM, no cache miss problem)
				BENCH: ASM version: 91 cycles (Write in AGP, some cache miss problems, still good against 67)
					   C version: 316 cycles.
			*/
			__asm
			{
				mov		edi, dstPtr
			theLoop:
				// **** compute lighting
				mov		esi,normalPtr			// uop: 0/1
				// dot3
				fld		dword ptr [esi]			// uop: 0/1
				fmul	lightObjectSpace.x		// uop: 1/1 (5)
				fld		dword ptr [esi+4]		// uop: 0/1
				fmul	lightObjectSpace.y		// uop: 1/1 (5)
				faddp	st(1),st				// uop: 1/0 (3)
				fld		dword ptr [esi+8]		// uop: 0/1
				fmul	lightObjectSpace.z		// uop: 1/1 (5)
				faddp	st(1),st				// uop: 1/0 (3)
				fistp	f8						// uop: 2/0/1/1 (5)
				// next
				add		esi, 12					// uop: 1/0
				mov		normalPtr, esi			// uop: 0/0/1/1


				// **** transform vertex, and store
				mov		esi, vertPtr			// uop: 0/1
				fild	word ptr[esi]			// uop: 3/1 (5)
				fild	word ptr[esi+2]			// uop: 3/1 (5)
				fild	word ptr[esi+4]			// uop: 3/1 (5)
				// x
				fld		a00						// uop: 0/1
				fmul	st, st(3)				// uop: 1/0 (5)
				fld		a01						// uop: 0/1
				fmul	st, st(3)				// uop: 1/0 (5)
				faddp	st(1), st				// uop: 1/0 (3)
				fld		a02						// uop: 0/1
				fmul	st, st(2)				// uop: 1/0 (5)
				faddp	st(1), st				// uop: 1/0 (3)
				fld		matPos.x				// uop: 0/1
				faddp	st(1), st				// uop: 1/0 (3)
				fstp	dword ptr[edi]			// uop: 0/0/1/1
				// y
				fld		a10
				fmul	st, st(3)
				fld		a11
				fmul	st, st(3)
				faddp	st(1), st
				fld		a12
				fmul	st, st(2)
				faddp	st(1), st
				fld		matPos.y
				faddp	st(1), st
				fstp	dword ptr[edi+4]
				// z
				fld		a20
				fmul	st, st(3)
				fld		a21
				fmul	st, st(3)
				faddp	st(1), st
				fld		a22
				fmul	st, st(2)
				faddp	st(1), st
				fld		matPos.z
				faddp	st(1), st
				fstp	dword ptr[edi+8]
				// flush stack
				fstp	st						// uop: 1/0
				fstp	st						// uop: 1/0
				fstp	st						// uop: 1/0
				// next
				add		esi, 6					// uop: 1/0
				mov		vertPtr, esi			// uop: 0/0/1/1


				// **** copy uv
				mov		esi, uvPtr							// uop: 0/1
				mov		eax, [esi]							// uop: 0/1
				mov		[edi+NL3D_CLOD_UV_OFF], eax			// uop: 0/0/1/1
				mov		ebx, [esi+4]						// uop: 0/1
				mov		[edi+NL3D_CLOD_UV_OFF+4], ebx		// uop: 0/0/1/1
				// next
				add		esi, 8					// uop: 1/0
				mov		uvPtr, esi				// uop: 0/0/1/1


				// **** Clamp lighting
				// clamp to 0 only. will be clamped to 255 by MMX
				mov		eax, f8					// uop: 0/1
				cmp		eax, 0x80000000			// if>=0 => CF=1
				sbb		ebx, ebx				// if>=0 => CF==1 => ebx=0xFFFFFFFF
				and		eax, ebx				// if>=0 => eax unchanged, else eax=0 (clamped)


				// **** Modulate lighting modulate with diffuse color, add ambient term, using MMX
				movd			mm0, eax		// 0000000L		uop: 1/0
				packuswb		mm0, mm0		// 000L000L		uop: 1/0 (p1)
				packuswb		mm0, mm0		// 0L0L0L0L		uop: 1/0 (p1)
				movd			mm1, diffuse	//				uop: 0/1
				punpcklbw		mm1, blank		//				uop: 1/1 (p1)
				pmullw			mm0, mm1		// diffuse*L	uop: 1/0 (3)
				psrlw			mm0, 8			// 0A0B0G0R		uop: 1/0 (p1)
				packuswb		mm0, blank		// 0000ABGR		uop: 1/1 (p1)
				movd			mm2, ambient	//				uop: 0/1
				paddusb			mm0, mm2		//				uop: 1/0
				movd			ebx, mm0		// ebx= AABBGGRR	uop: 1/0
				// NB: emms is not so bad on P2+: delay of 6, +11 (NB: far better than no MMX instructions)
				emms							// uop: 11/0 (6).  (?????)


				// **** append alpha, and store
				mov		esi, alphaPtr						// uop: 0/1
				movzx	eax, byte ptr[esi]					// uop: 0/1
				shl		eax, 24								// uop: 1/0
				add		ebx, eax							// uop: 1/0
				// now, ebx=  AABBGGRR
				mov		[edi+NL3D_CLOD_COLOR_OFF], ebx		// uop: 0/0/1/1
				// next
				add		esi, 1					// uop: 1/0
				mov		alphaPtr, esi			// uop: 0/0/1/1


				// **** next
				add		edi, NL3D_CLOD_VERTEX_SIZE		// uop: 1/0

				mov		eax, numVertices		// uop: 0/1
				dec		eax						// uop: 1/0
				mov		numVertices, eax		// uop: 0/0/1/1

				jnz		theLoop					// uop: 1/1 (p1)

				// To have same behavior than c code
				mov		dstPtr, edi
			}
		}
	}
	else
#endif
	{
		H_AUTO( NL3D_CharacterLod_vertexFill );

		CVector		fVect;

		for(;numVertices>0;)
		{
			// NB: order is important for AGP filling optimisation
			// transform vertex, and store.
			CVector		*dstVector= (CVector*)dstPtr;
			fVect.x= vertPtr->x; fVect.y= vertPtr->y; fVect.z= vertPtr->z;
			++vertPtr;
			dstVector->x= a00 * fVect.x + a01 * fVect.y + a02 * fVect.z + matPos.x;
			dstVector->y= a10 * fVect.x + a11 * fVect.y + a12 * fVect.z + matPos.y;
			dstVector->z= a20 * fVect.x + a21 * fVect.y + a22 * fVect.z + matPos.z;
			// Copy UV
			*(CUV*)(dstPtr + NL3D_CLOD_UV_OFF)= *uvPtr;
			++uvPtr;

			// Compute Lighting.
			CRGBA	lightRes;
			computeLodLighting(lightRes, lightObjectSpace, *normalPtr, ambient, diffuse);
			++normalPtr;
			lightRes.A= *alphaPtr;
			++alphaPtr;
			// store.
			*((CRGBA*)(dstPtr + NL3D_CLOD_COLOR_OFF))= lightRes;

			// next
			dstPtr+= NL3D_CLOD_VERTEX_SIZE;
			numVertices--;
		}
	}

	// Add Primitives.
	//=============

	{
		H_AUTO( NL3D_CharacterLod_primitiveFill )

		// get number of tri indexes
		uint	numTriIdxs= clod->getNumTriangles() * 3;

		// Yoyo: there is an assert with getPtr(). Not sure, but maybe arise if numTriIdxs==0
		if(numTriIdxs)
		{
			// realloc tris if needed.
			if(_CurrentTriId+numTriIdxs > _Triangles.getNumIndexes())
			{
				_Triangles.setFormat(NL_LOD_CHARACTER_INDEX_FORMAT);
				_Triangles.setNumIndexes(_CurrentTriId+numTriIdxs);
			}

			// reindex and copy tris
			CIndexBufferReadWrite iba;
			_Triangles.lock(iba);
			const TLodCharacterIndexType	*srcIdx= clod->getTriangleArray();
			nlassert(sizeof(TLodCharacterIndexType) == _Triangles.getIndexNumBytes());
			TLodCharacterIndexType		*dstIdx= (TLodCharacterIndexType *) iba.getPtr()+_CurrentTriId;
			for(;numTriIdxs>0;numTriIdxs--, srcIdx++, dstIdx++)
			{
				*dstIdx= *srcIdx + _CurrentVertexId;
			}
		}
	}

	// Next
	//=============

	// Inc Vertex count.
	_CurrentVertexId+= clod->getNumVertices();
	// Inc Prim count.
	_CurrentTriId+= clod->getNumTriangles() * 3;


	// key added
	return true;
}

// ***************************************************************************
void			CLodCharacterManager::endRender()
{
	H_AUTO ( NL3D_CharacterLod_endRender );

	IDriver		*driver= _VertexStream.getDriver();
	nlassert(driver);
	// we must be between beginRender() and endRender()
	nlassert(isRendering());

	// if something rendered
	if(_LockDone)
	{
		// UnLock Buffer.
		_VertexStream.unlock(_CurrentVertexId);
		_LockDone= false;

		// Render the VBuffer and the primitives.
		if(_CurrentTriId>0)
		{
			// setup matrix.
			CMatrix		managerMatrix;
			managerMatrix.setPos(_ManagerMatrixPos);
			driver->setupModelMatrix(managerMatrix);

			// active VB
			_VertexStream.activate();

			// render triangles
			driver->activeIndexBuffer(_Triangles);
			driver->renderTriangles(_Material, 0, _CurrentTriId/3);
		}

		// swap Stream VBHard
		_VertexStream.swapVBHard();
	}

	// Ok, end rendering
	_Rendering= false;
}

// ***************************************************************************
void			CLodCharacterManager::setupNormalCorrectionMatrix(const CMatrix &normalMatrix)
{
	_LightCorrectionMatrix= normalMatrix;
	_LightCorrectionMatrix.setPos(CVector::Null);
	_LightCorrectionMatrix.invert();
}


// ***************************************************************************
// ***************************************************************************
// Texturing.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CLodCharacterTmpBitmap::CLodCharacterTmpBitmap()
{
	reset();
}

// ***************************************************************************
void			CLodCharacterTmpBitmap::reset()
{
	// setup a 1*1 bitmap
	_Bitmap.resize(1);
	_Bitmap[0]= CRGBA::Black;
	_WidthPower=0;
	_UShift= 8;
	_VShift= 8;
}

// ***************************************************************************
void			CLodCharacterTmpBitmap::build(const NLMISC::CBitmap &bmpIn)
{
	uint	width= bmpIn.getWidth();
	uint	height= bmpIn.getHeight();
	nlassert(width>0 && width<=256);
	nlassert(height>0 && height<=256);

	// resize bitmap.
	_Bitmap.resize(width*height);
	_WidthPower= getPowerOf2(width);
	// compute shift
	_UShift= 8-getPowerOf2(width);
	_VShift= 8-getPowerOf2(height);

	// convert the bitmap.
	CBitmap		bmp= bmpIn;
	bmp.convertToType(CBitmap::RGBA);
	CRGBA	*src= (CRGBA*)&bmp.getPixels()[0];
	CRGBA	*dst= _Bitmap.getPtr();
	for(sint nPix= width*height;nPix>0;nPix--, src++, dst++)
	{
		*dst= *src;
	}
}

// ***************************************************************************
void			CLodCharacterTmpBitmap::build(CRGBA col)
{
	// setup a 1*1 bitmap and set it with col
	reset();
	_Bitmap[0]= col;
}


// ***************************************************************************
void			CLodCharacterManager::initInstance(CLodCharacterInstance &instance)
{
	// first release in (maybe) other manager.
	if(instance._Owner)
		instance._Owner->releaseInstance(instance);

	// get the shape
	const CLodCharacterShape	*clod= getShape(instance.ShapeId);
	// if not found quit
	if(!clod)
		return;
	// get Uvs.
	const CUV	*uvSrc= clod->getUVs();
	nlassert(uvSrc);


	// Ok, init header
	instance._Owner= this;
	instance._UVs.resize(clod->getNumVertices());

	// allocate an id. If cannot, then fill Uvs with 0 => filled with Black. (see endTextureCompute() why).
	if(_FreeIds.empty())
	{
		// set a "Not enough memory" id
		instance._TextureId= NL3D_CLOD_TEXT_NUM_IDS;
		CUV		uv(0,0);
		fill(instance._UVs.begin(), instance._UVs.end(), uv);
	}
	// else OK, can instanciate the Uvs.
	else
	{
		// get the id.
		instance._TextureId= _FreeIds.back();
		_FreeIds.pop_back();
		// get the x/y.
		uint	xId= instance._TextureId % NL3D_CLOD_TEXT_NLOD_WIDTH;
		uint	yId= instance._TextureId / NL3D_CLOD_TEXT_NLOD_WIDTH;
		// compute the scale/bias to apply to Uvs.
		float	scaleU= 1.0f / NL3D_CLOD_TEXT_NLOD_WIDTH;
		float	scaleV= 1.0f / NL3D_CLOD_TEXT_NLOD_HEIGHT;
		float	biasU= (float)xId / NL3D_CLOD_TEXT_NLOD_WIDTH;
		float	biasV= (float)yId / NL3D_CLOD_TEXT_NLOD_HEIGHT;
		// apply it to each UVs.
		CUV		*uvDst= &instance._UVs[0];
		for(uint i=0; i<instance._UVs.size();i++)
		{
			uvDst[i].U= biasU + uvSrc[i].U*scaleU;
			uvDst[i].V= biasV + uvSrc[i].V*scaleV;
		}
	}
}

// ***************************************************************************
void			CLodCharacterManager::releaseInstance(CLodCharacterInstance &instance)
{
	if(instance._Owner==NULL)
		return;
	nlassert(this==instance._Owner);

	// if the id is not a "Not enough memory" id, release it.
	if(instance._TextureId>=0 && instance._TextureId<NL3D_CLOD_TEXT_NUM_IDS)
		_FreeIds.push_back(instance._TextureId);

	// reset the instance
	instance._Owner= NULL;
	instance._TextureId= -1;
	contReset(instance._UVs);
}


// ***************************************************************************
CRGBA			*CLodCharacterManager::getTextureInstance(CLodCharacterInstance &instance)
{
	nlassert(instance._Owner==this);
	nlassert(instance._TextureId!=-1);
	// if the texture id is a "not enough memory", quit.
	if(instance._TextureId==NL3D_CLOD_TEXT_NUM_IDS)
		return NULL;

	// get the x/y.
	uint	xId= instance._TextureId % NL3D_CLOD_TEXT_NLOD_WIDTH;
	uint	yId= instance._TextureId / NL3D_CLOD_TEXT_NLOD_WIDTH;

	// get the ptr on the correct pixel.
	CRGBA	*pix= (CRGBA*)&_BigTexture->getPixels(0)[0];
	return pix + yId*NL3D_CLOD_TEXT_HEIGHT*NL3D_CLOD_BIGTEXT_WIDTH + xId*NL3D_CLOD_TEXT_WIDTH;
}


// ***************************************************************************
bool			CLodCharacterManager::startTextureCompute(CLodCharacterInstance &instance)
{
	CRGBA	*dst= getTextureInstance(instance);
	if(!dst)
		return false;

	// erase the texture with 0,0,0,255. Alpha is actually the min "Quality" part of the CTUVQ.
	CRGBA	col= NL3D_CLOD_DEFAULT_TEXCOLOR;
	for(uint y=0;y<NL3D_CLOD_TEXT_HEIGHT;y++)
	{
		// erase the line
		for(uint x=0;x<NL3D_CLOD_TEXT_WIDTH;x++)
			dst[x]= col;
		// Next line
		dst+= NL3D_CLOD_BIGTEXT_WIDTH;
	}

	return true;
}

// ***************************************************************************
void			CLodCharacterManager::addTextureCompute(CLodCharacterInstance &instance, const CLodCharacterTexture &lodTexture)
{
	CRGBA	*dst= getTextureInstance(instance);
	if(!dst)
		return;

	// get lookup ptr.
	nlassert(lodTexture.Texture.size()==NL3D_CLOD_TEXT_SIZE);
	if (lodTexture.Texture.size() < NL3D_CLOD_TEXT_SIZE)
		return;

	const CLodCharacterTexture::CTUVQ		*lookUpPtr= &lodTexture.Texture[0];

	// apply the lodTexture, taking only better quality (ie nearer 0)
	for(uint y=0;y<NL3D_CLOD_TEXT_HEIGHT;y++)
	{
		// erase the line
		for(uint x=0;x<NL3D_CLOD_TEXT_WIDTH;x++)
		{
			CLodCharacterTexture::CTUVQ		lut= *lookUpPtr;
			// if this quality is better than the one stored
			if(lut.Q<dst[x].A)
			{
				// get what texture to read, and read the pixel.
				CRGBA	col= _TmpBitmaps[lut.T].getPixel(lut.U, lut.V);
				// set quality.
				col.A= lut.Q;
				// set in dest
				dst[x]= col;
			}

			// next lookup
			lookUpPtr++;
		}
		// Next line
		dst+= NL3D_CLOD_BIGTEXT_WIDTH;
	}
}

// ***************************************************************************
void			CLodCharacterManager::endTextureCompute(CLodCharacterInstance &instance, uint numBmpToReset)
{
	CRGBA	*dst= getTextureInstance(instance);
	if(!dst)
		return;

	// reset All Alpha values to 255 => no AlphaTest problems
	for(uint y=0;y<NL3D_CLOD_TEXT_HEIGHT;y++)
	{
		// erase the line
		for(uint x=0;x<NL3D_CLOD_TEXT_WIDTH;x++)
		{
			dst[x].A= 255;
		}
		// Next line
		dst+= NL3D_CLOD_BIGTEXT_WIDTH;
	}

	// If the id == 0 then must reset the 0,0 Pixel to black. for the "Not Enough memory" case in initInstance().
	if(instance._TextureId==0)
		*(CRGBA*)&_BigTexture->getPixels(0)[0]= NL3D_CLOD_DEFAULT_TEXCOLOR;

	// get the x/y.
	uint	xId= instance._TextureId % NL3D_CLOD_TEXT_NLOD_WIDTH;
	uint	yId= instance._TextureId / NL3D_CLOD_TEXT_NLOD_WIDTH;
	// touch the texture for Driver update.
	_BigTexture->touchRect(
		CRect(xId*NL3D_CLOD_TEXT_WIDTH, yId*NL3D_CLOD_TEXT_HEIGHT, NL3D_CLOD_TEXT_WIDTH, NL3D_CLOD_TEXT_HEIGHT) );

	// reset tmpBitmaps / free memory.
	for(uint i=0; i<numBmpToReset; i++)
	{
		_TmpBitmaps[i].reset();
	}

	// TestYoyo
	/*NLMISC::COFile	f("tam.tga");
	_BigTexture->writeTGA(f,32);*/
}


// ***************************************************************************
bool	CLodCharacterManager::fastIntersect(const CLodCharacterInstance &instance, const NLMISC::CMatrix &toRaySpace, float &dist2D, float &distZ, bool computeDist2D)
{
	H_AUTO ( NL3D_CharacterLod_fastIntersect )

	uint			numVertices;
	const CLodCharacterShape::CVector3s		*vertPtr;
	CVector			matPos;
	float			a00, a01, a02;
	float			a10, a11, a12;
	float			a20, a21, a22;


	// Get the Shape and current key.
	//=============

	// get the shape
	const CLodCharacterShape	*clod= getShape(instance.ShapeId);
	// if not found quit
	if(!clod)
		return false;

	// get the anim key
	CVector		unPackScaleFactor;
	vertPtr= clod->getAnimKey(instance.AnimId, instance.AnimTime, instance.WrapMode, unPackScaleFactor);
	// if not found quit
	if(!vertPtr)
		return false;
	// get num verts
	numVertices= clod->getNumVertices();

	// empty shape??
	if(numVertices==0)
		return false;

	// Prepare Transform
	//=============

	// Get matrix pos.
	matPos= toRaySpace.getPos();
	// Get rotation line vectors
	const float *rayM= toRaySpace.get();
	a00= rayM[0]; a01= rayM[4]; a02= rayM[8];
	a10= rayM[1]; a11= rayM[5]; a12= rayM[9];
	a20= rayM[2]; a21= rayM[6]; a22= rayM[10];

	// multiply matrix with scale factor for Pos.
	a00*= unPackScaleFactor.x; a01*= unPackScaleFactor.y; a02*= unPackScaleFactor.z;
	a10*= unPackScaleFactor.x; a11*= unPackScaleFactor.y; a12*= unPackScaleFactor.z;
	a20*= unPackScaleFactor.x; a21*= unPackScaleFactor.y; a22*= unPackScaleFactor.z;

	// get dst Array.
	// enlarge temp buffer
	static std::vector<CVector>	lodInRaySpace;
	if(numVertices>lodInRaySpace.size())
		lodInRaySpace.resize(numVertices);
	CVector	*dstPtr= &lodInRaySpace[0];


	// Fill the temp skin
	//=============
	{
		CVector		fVect;

		for(;numVertices>0;)
		{
			// transform vertex, and store.
			fVect.x= vertPtr->x; fVect.y= vertPtr->y; fVect.z= vertPtr->z;
			++vertPtr;
			dstPtr->x= a00 * fVect.x + a01 * fVect.y + a02 * fVect.z + matPos.x;
			dstPtr->y= a10 * fVect.x + a11 * fVect.y + a12 * fVect.z + matPos.y;
			dstPtr->z= a20 * fVect.x + a21 * fVect.y + a22 * fVect.z + matPos.z;

			// next
			dstPtr++;
			numVertices--;
		}
	}

	// Test intersection
	//=============

	return CRayMesh::getRayIntersection(lodInRaySpace, clod->getTriangleIndices(), dist2D, distZ, computeDist2D);
}


} // NL3D
