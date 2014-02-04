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

#include "nel/3d/vegetable_shape.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
CVegetableShape::CVegetableShape()
{
	Lighted= false;
	DoubleSided= false;
	PreComputeLighting= false;
	AlphaBlend= false;
	BestSidedPreComputeLighting= false;
}

// ***************************************************************************
void		CVegetableShape::build(CVegetableShapeBuild &vbuild)
{
	// Must have TexCoord0.
	nlassert( vbuild.VB.getVertexFormat() & CVertexBuffer::TexCoord0Flag );

	// Header
	//---------

	// Lighted ?
	if(vbuild.Lighted && ( vbuild.VB.getVertexFormat() & CVertexBuffer::NormalFlag) )
		Lighted= true;
	else
		Lighted= false;

	// DoubleSided
	DoubleSided= vbuild.DoubleSided;

	// PreComputeLighting.
	PreComputeLighting= Lighted && vbuild.PreComputeLighting;

	// AlphaBlend: valid only for 2Sided and Unlit (or similar PreComputeLighting) mode
	AlphaBlend= vbuild.AlphaBlend && DoubleSided && (!Lighted || PreComputeLighting);

	// BestSidedPreComputeLighting
	BestSidedPreComputeLighting= PreComputeLighting && vbuild.BestSidedPreComputeLighting;

	// BendCenterMode
	BendCenterMode= vbuild.BendCenterMode;

	// Format of the VB.
	uint32	format;
	format= CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag;
	// lighted?
	if(Lighted)
		format|= CVertexBuffer::NormalFlag;
	// set VB.
	VB.setVertexFormat(format);


	// Fill triangles.
	//---------
	uint	i;
	// resisz
	TriangleIndices.resize(vbuild.PB.getNumIndexes());
	CIndexBufferRead ibaRead;
	vbuild.PB.lock (ibaRead);
	const uint32	*srcTri= (const uint32 *) ibaRead.getPtr();
	// fill
	for(i=0; i<vbuild.PB.getNumIndexes()/3; i++)
	{
		TriangleIndices[i*3+0]= *(srcTri++);
		TriangleIndices[i*3+1]= *(srcTri++);
		TriangleIndices[i*3+2]= *(srcTri++);
	}

	// Fill vertices.
	//---------
	// resize
	uint32		nbVerts= vbuild.VB.getNumVertices();
	VB.setNumVertices(nbVerts);

	CVertexBufferRead vba;
	vbuild.VB.lock (vba);
	CVertexBufferReadWrite vbaOut;
	VB.lock (vbaOut);

	// if no vertex color,
	float	maxZ= 0;
	bool	bendFromColor= true;
	if(! (vbuild.VB.getVertexFormat() & CVertexBuffer::PrimaryColorFlag) )
	{
		// must compute bendWeight from z.
		bendFromColor= false;
		// get the maximum Z.
		for(i=0;i<nbVerts;i++)
		{
			float	z= (vba.getVertexCoordPointer(i))->z;
			maxZ= max(z, maxZ);
		}
		// if no positive value, bend will always be 0.
		if(maxZ==0)
			maxZ= 1;
	}

	// For all vertices, fill
	for(i=0;i<nbVerts;i++)
	{
		// Position.
		const CVector		*srcPos= vba.getVertexCoordPointer(i);
		CVector		*dstPos= vbaOut.getVertexCoordPointer(i);
		*dstPos= *srcPos;

		// Normal
		if(Lighted)
		{
			const CVector *srcNormal= vba.getNormalCoordPointer(i);
			CVector		*dstNormal= vbaOut.getNormalCoordPointer(i);
			*dstNormal= *srcNormal;
		}

		// Texture.
		const CUV		*srcUV= vba.getTexCoordPointer(i, 0);
		CUV		*dstUV= vbaOut.getTexCoordPointer(i, 0);
		*dstUV= *srcUV;

		// Bend.
		// copy to texture stage 1.
		CUV		*dstUVBend= vbaOut.getTexCoordPointer(i, 1);
		if(bendFromColor)
		{
			// todo hulud d3d vertex color RGBA / BGRA
			const CRGBA	*srcColor= (const CRGBA*)vba.getColorPointer(i);
			// Copy and scale by MaxBendWeight
			dstUVBend->U= (srcColor->R / 255.f) * vbuild.MaxBendWeight;
		}
		else
		{
			float	w= srcPos->z / maxZ;
			w= max(w, 0.f);
			// Copy and scale by MaxBendWeight
			dstUVBend->U= w * vbuild.MaxBendWeight;
		}
	}


	// Misc.
	//---------
	// prepare for instanciation
	InstanceVertices.resize(VB.getNumVertices());

}

// ***************************************************************************
bool		CVegetableShape::loadShape(const std::string &shape)
{
	string	path= CPath::lookup(shape, false);
	if( path.empty() )
		return false;
	// read this file
	CIFile	f(path);
	serial(f);
	return true;
}

// ***************************************************************************
void		CVegetableShape::serial(NLMISC::IStream &f)
{
	/*
	Version 1:
		- BestSidedPreComputeLighting
	*/
	sint	ver= f.serialVersion(1);
	f.serialCheck(NELID("_LEN"));
	f.serialCheck(NELID("GEV_"));
	f.serialCheck(NELID("BATE"));
	f.serialCheck(NELID("__EL"));

	f.serial(Lighted);
	f.serial(DoubleSided);
	f.serial(PreComputeLighting);
	f.serial(AlphaBlend);
	f.serialEnum(BendCenterMode);
	f.serial(VB);
	f.serialCont(TriangleIndices);

	if(ver>=1)
		f.serial(BestSidedPreComputeLighting);
	else if(f.isReading())
		BestSidedPreComputeLighting= false;

	// if reading
	if(f.isReading())
	{
		// prepare for instanciation
		InstanceVertices.resize(VB.getNumVertices());
	}
}



} // NL3D
