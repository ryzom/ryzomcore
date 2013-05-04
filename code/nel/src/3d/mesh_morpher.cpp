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

#include "nel/3d/mesh_morpher.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/raw_skin.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{

// ***************************************************************************
void CBlendShape::serial (NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// version 1 : added tangent space support
	sint ver = f.serialVersion (1);

	f.serial (Name);

	f.serialCont (deltaPos);
	f.serialCont (deltaNorm);
	f.serialCont (deltaUV);
	f.serialCont (deltaCol);

	if (ver >= 1) f.serialCont(deltaTgSpace);

	f.serialCont (VertRefs);
}

// ***************************************************************************
CMeshMorpher::CMeshMorpher()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	_VBOri = NULL;
	_VBDst = NULL;

	_Vertices = NULL;
	_Normals = NULL;
	_TgSpace= NULL;
	_SkinApplied= false;
}

// ***************************************************************************
void CMeshMorpher::init (CVertexBuffer *vbOri, CVertexBuffer *vbDst, bool hasTgSpace)
{
	_VBOri = vbOri;
	_VBDst = vbDst;
	_UseTgSpace = hasTgSpace;
}

// ***************************************************************************
void CMeshMorpher::initSkinned (CVertexBuffer *vbOri,
							CVertexBuffer *vbDst,
							bool hasTgSpace,
							std::vector<CVector> *vVertices,
							std::vector<CVector> *vNormals,
							std::vector<CVector> *vTgSpace, /* NULL if none */
							bool bSkinApplied	)
{
	_VBOri = vbOri;
	_VBDst = vbDst;
	_UseTgSpace = hasTgSpace;

	_Vertices = vVertices;
	_Normals = vNormals;
	_TgSpace = vTgSpace;
	_SkinApplied = bSkinApplied;
}

// ***************************************************************************
void CMeshMorpher::update (std::vector<CAnimatedMorph> *pBSFactor)
{
	uint32 i, j;

	if (_VBOri == NULL)
		return;
	if (BlendShapes.size() == 0)
		return;

	if (_VBOri->getNumVertices() != _VBDst->getNumVertices())
	{	// Because the original vertex buffer is not initialized by default
		// we must init it here (if there are some blendshapes)
		*_VBOri = *_VBDst;
	}

	// Does the flags are reserved ?
	if (_Flags.size() != _VBOri->getNumVertices())
	{
		_Flags.resize (_VBOri->getNumVertices());
		for (i = 0; i < _Flags.size(); ++i)
			_Flags[i] = Modified; // Modified to update all
	}

	nlassert(_VBOri->getVertexFormat() == _VBDst->getVertexFormat());

	// Cleaning with original vertex buffer
	uint32 VBVertexSize = _VBOri->getVertexSize();
	CVertexBufferRead srcvba;
	_VBOri->lock (srcvba);
	CVertexBufferReadWrite dstvba;
	_VBDst->lock (dstvba);
	const uint8 *pOri = (const uint8*)srcvba.getVertexCoordPointer ();
	uint8 *pDst = (uint8*)dstvba.getVertexCoordPointer ();

	for (i= 0; i < _Flags.size(); ++i)
	if (_Flags[i] >= Modified)
	{
		_Flags[i] = OriginalVBDst;

		for(j = 0; j < VBVertexSize; ++j)
			pDst[j+i*VBVertexSize] = pOri[j+i*VBVertexSize];
	}

	uint tgSpaceStage = 0;
	if (_UseTgSpace)
	{
		tgSpaceStage = _VBDst->getNumTexCoordUsed() - 1;
	}

	// Blending with blendshape
	for (i = 0; i < BlendShapes.size(); ++i)
	{
		CBlendShape &rBS = BlendShapes[i];
		float rFactor = pBSFactor->operator[](i).getFactor()/100.0f;

		// todo hulud check it works
		// if (rFactor > 0.0f)
		if (rFactor != 0.0f)
		for (j = 0; j < rBS.VertRefs.size(); ++j)
		{
			uint32 vp = rBS.VertRefs[j];

			// Modify Pos/Norm/TgSpace.
			//------------
			if (_VBDst->getVertexFormat() & CVertexBuffer::PositionFlag)
			if (rBS.deltaPos.size() > 0)
			{
				CVector *pV = dstvba.getVertexCoordPointer (vp);
				*pV += rBS.deltaPos[j] * rFactor;
			}

			if (_VBDst->getVertexFormat() & CVertexBuffer::NormalFlag)
			if (rBS.deltaNorm.size() > 0)
			{
				CVector *pV = dstvba.getNormalCoordPointer (vp);
				*pV += rBS.deltaNorm[j] * rFactor;
			}

			if (_UseTgSpace)
			if (rBS.deltaTgSpace.size() > 0)
			{
				CVector *pV = (CVector*)dstvba.getTexCoordPointer (vp, tgSpaceStage);
				*pV += rBS.deltaTgSpace[j] * rFactor;
			}

			// Modify UV0 / Color
			//------------
			if (_VBDst->getVertexFormat() & CVertexBuffer::TexCoord0Flag)
			if (rBS.deltaUV.size() > 0)
			{
				CUV *pUV = dstvba.getTexCoordPointer (vp);
				*pUV += rBS.deltaUV[j] * rFactor;
			}

			if (_VBDst->getVertexFormat() & CVertexBuffer::PrimaryColorFlag)
			if (rBS.deltaCol.size() > 0)
			{
				// todo hulud d3d vertex color RGBA / BGRA
				CRGBA *pRGBA = (CRGBA*)dstvba.getColorPointer (vp);
				CRGBAF rgbf(*pRGBA);
				rgbf.R += rBS.deltaCol[j].R * rFactor;
				rgbf.G += rBS.deltaCol[j].G * rFactor;
				rgbf.B += rBS.deltaCol[j].B * rFactor;
				rgbf.A += rBS.deltaCol[j].A * rFactor;
				clamp(rgbf.R, 0.0f, 1.0f);
				clamp(rgbf.G, 0.0f, 1.0f);
				clamp(rgbf.B, 0.0f, 1.0f);
				clamp(rgbf.A, 0.0f, 1.0f);
				*pRGBA = rgbf;
			}

			// Modified
			_Flags[vp] = Modified;
		}
	}
}


// ***************************************************************************
void CMeshMorpher::updateSkinned (std::vector<CAnimatedMorph> *pBSFactor)
{
	uint32 i, j;

	if (_VBOri == NULL)
		return;
	if (BlendShapes.size() == 0)
		return;

	if (_VBOri->getNumVertices() != _VBDst->getNumVertices())
	{	// Because the original vertex buffer is not initialized by default
		// we must init it here (if there are some blendshapes)
		*_VBOri = *_VBDst;
	}

	// Does the flags are reserved ?
	if (_Flags.size() != _VBOri->getNumVertices())
	{
		_Flags.resize (_VBOri->getNumVertices());
		for (i = 0; i < _Flags.size(); ++i)
			_Flags[i] = Modified; // Modified to update all
	}

	nlassert(_VBOri->getVertexFormat() == _VBDst->getVertexFormat());

	uint tgSpaceStage;
	uint tgSpaceOff = 0;
	if (_UseTgSpace && _TgSpace)
	{
		tgSpaceStage = _VBDst->getNumTexCoordUsed() - 1;
		tgSpaceOff = _VBDst->getTexCoordOff(tgSpaceStage);
	}

	// Cleaning with original vertex buffer
	uint32 VBVertexSize = _VBOri->getVertexSize();
	CVertexBufferRead srcvba;
	_VBOri->lock (srcvba);
	CVertexBufferReadWrite dstvba;
	_VBDst->lock (dstvba);
	const uint8 *pOri = (const uint8*)srcvba.getVertexCoordPointer ();
	uint8 *pDst = (uint8*)dstvba.getVertexCoordPointer ();

	for (i= 0; i < _Flags.size(); ++i)
	if (_Flags[i] >= Modified)
	{
		for(j = 0; j < VBVertexSize; ++j)
			pDst[j+i*VBVertexSize] = pOri[j+i*VBVertexSize];

		if (_Vertices != NULL)
			_Vertices->operator[](i) = ((CVector*)(pOri+i*VBVertexSize))[0];

		if (_Normals != NULL)
			_Normals->operator[](i) = ((CVector*)(pOri+i*VBVertexSize))[1];

		if (_TgSpace != NULL)
			(*_TgSpace)[i] = * (CVector*)(pOri + i * VBVertexSize + tgSpaceOff);

		_Flags[i] = OriginalVBDst;
	}

	// Blending with blendshape
	for (i = 0; i < BlendShapes.size(); ++i)
	{
		CBlendShape &rBS = BlendShapes[i];
		float rFactor = pBSFactor->operator[](i).getFactor()/100.0f;

		if (rFactor != 0.0f)
		for (j = 0; j < rBS.VertRefs.size(); ++j)
		{
			uint32 vp = rBS.VertRefs[j];

			// Modify Pos/Norm/TgSpace.
			//------------
			if (_Vertices != NULL)
			if (rBS.deltaPos.size() > 0)
			{
				CVector *pV = &(_Vertices->operator[](vp));
				*pV += rBS.deltaPos[j] * rFactor;
			}

			if (_Normals != NULL)
			if (rBS.deltaNorm.size() > 0)
			{
				CVector *pV = &(_Normals->operator[](vp));
				*pV += rBS.deltaNorm[j] * rFactor;
			}

			if (_UseTgSpace && _TgSpace != NULL)
			if (rBS.deltaTgSpace.size() > 0)
			{
				CVector *pV = &((*_TgSpace)[vp]);
				*pV += rBS.deltaTgSpace[j] * rFactor;
			}

			// Modify UV0 / Color
			//------------
			if (_VBDst->getVertexFormat() & CVertexBuffer::TexCoord0Flag)
			if (rBS.deltaUV.size() > 0)
			{
				CUV *pUV = dstvba.getTexCoordPointer (vp);
				*pUV += rBS.deltaUV[j] * rFactor;
			}

			if (_VBDst->getVertexFormat() & CVertexBuffer::PrimaryColorFlag)
			if (rBS.deltaCol.size() > 0)
			{
				// todo hulud d3d vertex color RGBA / BGRA
				CRGBA *pRGBA = (CRGBA*)dstvba.getColorPointer (vp);
				CRGBAF rgbf(*pRGBA);
				rgbf.R += rBS.deltaCol[j].R * rFactor;
				rgbf.G += rBS.deltaCol[j].G * rFactor;
				rgbf.B += rBS.deltaCol[j].B * rFactor;
				rgbf.A += rBS.deltaCol[j].A * rFactor;
				clamp(rgbf.R, 0.0f, 1.0f);
				clamp(rgbf.G, 0.0f, 1.0f);
				clamp(rgbf.B, 0.0f, 1.0f);
				clamp(rgbf.A, 0.0f, 1.0f);
				*pRGBA = rgbf;
			}

			// Modified
			_Flags[vp] = Modified;
		}
	}
}

// ***************************************************************************
void CMeshMorpher::serial (NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	(void)f.serialVersion (0);

	f.serialCont (BlendShapes);
}


// ***************************************************************************
#define	NL3D_RAWSKIN_NORMAL_OFF		12
#define	NL3D_RAWSKIN_UV_OFF			24
#define	NL3D_RAWSKIN_VERTEX_SIZE	32

void CMeshMorpher::updateRawSkin (CVertexBuffer *vbOri,
					NLMISC::CObjectVector<CRawSkinVertex*, false>	&vertexRemap,
					std::vector<CAnimatedMorph> *pBSFactor)
{
	uint32 i, j;

	if (vbOri == NULL)
		return;
	if (BlendShapes.size() == 0)
		return;

	nlassert(vbOri->getVertexFormat() == (CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag |CVertexBuffer::TexCoord0Flag) );
	nlassert(NL3D_RAWSKIN_VERTEX_SIZE == vbOri->getVertexSize());
	nlassert(NL3D_RAWSKIN_NORMAL_OFF == vbOri->getNormalOff());
	nlassert(NL3D_RAWSKIN_UV_OFF == vbOri->getTexCoordOff(0));

	// Cleaning with original vertex buffer
	CVertexBufferRead srcvba;
	vbOri->lock (srcvba);
	const uint8			*pOri = (const uint8*)srcvba.getVertexCoordPointer ();
	CRawSkinVertex	**vRemap= vertexRemap.getPtr();
	uint			numVertices= vbOri->getNumVertices();

	// Update only the vertices of this lod
	for (i= 0; i < numVertices; ++i)
	{
		if(*vRemap)
		{
			(*vRemap)->Pos= *(CVector*)(pOri);
			(*vRemap)->Normal= *(CVector*)(pOri + NL3D_RAWSKIN_NORMAL_OFF);
			(*vRemap)->UV= *(CUV*)(pOri + NL3D_RAWSKIN_UV_OFF);
		}
		pOri+= NL3D_RAWSKIN_VERTEX_SIZE;
		vRemap++;
	}

	// Blending with blendshape
	for (i = 0; i < BlendShapes.size(); ++i)
	{
		CBlendShape		&rBS = BlendShapes[i];
		float			rFactor = pBSFactor->operator[](i).getFactor();

		if (rFactor != 0.0f)
		{
			rFactor*= 0.01f;
			uint32		numVertices= (uint32)rBS.VertRefs.size();
			// don't know why, but cases happen where deltaNorm not empty while deltaPos is
			bool		hasPos= rBS.deltaPos.size()>0;
			bool		hasNorm= rBS.deltaNorm.size()>0;
			bool		hasUV= rBS.deltaUV.size()>0;
			for (j = 0; j < numVertices; ++j)
			{
				// Get the vertex Index in the VBufferFinal
				uint	vid= rBS.VertRefs[j];
				// Then get the RawSkin vertex to modify
				CRawSkinVertex	*rsVert= vertexRemap[vid];

				// If exist in this Lod RawSkin, apply
				if(rsVert)
				{
					if(hasPos)
						rsVert->Pos+= rBS.deltaPos[j] * rFactor;
					if(hasNorm)
						rsVert->Normal+= rBS.deltaNorm[j] * rFactor;
					if(hasUV)
						rsVert->UV+= rBS.deltaUV[j] * rFactor;
				}
			}
		}

	}

}


} // NL3D










