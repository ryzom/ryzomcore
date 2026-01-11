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

#include "nel/3d/landscapevb_info.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/landscapevb_allocator.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// VertexBufferInfo.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CFarVertexBufferInfo::setupNullPointers()
{
	Accessor.unlock();
	VertexCoordPointer= NULL;
	TexCoordPointer0= NULL;
	TexCoordPointer1= NULL;
	ColorPointer= NULL;
	GeomInfoPointer= NULL;
	DeltaPosPointer= NULL;
	AlphaInfoPointer= NULL;
}


// ***************************************************************************
void		CFarVertexBufferInfo::setupPointersForVertexProgram()
{
	// see CLandscapeVBAllocator for program definition.
	uint8	*vcoord= (uint8*)VertexCoordPointer;

	TexCoordPointer0= vcoord + TexCoordOff0;
	TexCoordPointer1= vcoord + TexCoordOff1;
	GeomInfoPointer= vcoord + GeomInfoOff;
	DeltaPosPointer= vcoord + DeltaPosOff;
	AlphaInfoPointer= vcoord + AlphaInfoOff;
}


// ***************************************************************************
void		CFarVertexBufferInfo::setupVertexBuffer(CVertexBuffer &vb, bool forVertexProgram)
{
	VertexFormat= vb.getVertexFormat();
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();

	if(NumVertices==0)
	{
		setupNullPointers();
		return;
	}

	vb.lock (Accessor);
	VertexCoordPointer = Accessor.getVertexCoordPointer();

	if(forVertexProgram)
	{
		// With VertexCoordPointer setuped, init for VP.
		TexCoordOff0= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_TEX0);				// v[8]= Tex0.
		TexCoordOff1= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_TEX1);				// v[9]= Tex1.
		GeomInfoOff= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_GEOMINFO);			// v[10]= GeomInfos.
		DeltaPosOff= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_DELTAPOS);			// v[11]= EndPos-StartPos
		// Init Alpha Infos only if enabled (enabled if Value 5 are).
		AlphaInfoOff= 0;
		if( vb.getVertexFormat() & (1<<NL3D_LANDSCAPE_VPPOS_ALPHAINFO) )
			AlphaInfoOff= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_ALPHAINFO);		// v[12]= AlphaInfos

		// update Ptrs.
		setupPointersForVertexProgram();
	}
	else
	{
		TexCoordOff0= vb.getTexCoordOff(0);
		TexCoordOff1= vb.getTexCoordOff(1);
		TexCoordPointer0= Accessor.getTexCoordPointer(0, 0);
		TexCoordPointer1= Accessor.getTexCoordPointer(0, 1);

		// In Far0, we don't have Color component.
		if(VertexFormat & CVertexBuffer::PrimaryColorFlag)
		{
			ColorOff= vb.getColorOff();
			// todo hulud d3d vertex color RGBA / BGRA
			ColorPointer= Accessor.getColorPointer();
		}
		else
		{
			ColorOff= 0;
			ColorPointer= NULL;
		}
	}

}


// ***************************************************************************
void		CNearVertexBufferInfo::setupNullPointers()
{
	Accessor.unlock();
	VertexCoordPointer= NULL;
	TexCoordPointer0= NULL;
	TexCoordPointer1= NULL;
	TexCoordPointer2= NULL;
	GeomInfoPointer= NULL;
	DeltaPosPointer= NULL;
}


// ***************************************************************************
void		CNearVertexBufferInfo::setupPointersForVertexProgram()
{
	// see CLandscapeVBAllocator for program definition.
	uint8	*vcoord= (uint8*)VertexCoordPointer;

	TexCoordPointer0= vcoord + TexCoordOff0;
	TexCoordPointer1= vcoord + TexCoordOff1;
	TexCoordPointer2= vcoord + TexCoordOff2;
	GeomInfoPointer= vcoord + GeomInfoOff;
	DeltaPosPointer= vcoord + DeltaPosOff;

}


// ***************************************************************************
void		CNearVertexBufferInfo::setupVertexBuffer(CVertexBuffer &vb, bool forVertexProgram)
{
	VertexFormat= vb.getVertexFormat();
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();

	if(NumVertices==0)
	{
		setupNullPointers();
		return;
	}

	vb.lock (Accessor);
	VertexCoordPointer= Accessor.getVertexCoordPointer();

	if(forVertexProgram)
	{
		// With VertexCoordPointer setuped, init for VP.
		TexCoordOff0= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_TEX0);				// v[8]= Tex0.
		TexCoordOff1= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_TEX1);				// v[9]= Tex1.
		TexCoordOff2= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_TEX2);				// v[13]= Tex1.
		GeomInfoOff= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_GEOMINFO);			// v[10]= GeomInfos.
		DeltaPosOff= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_DELTAPOS);			// v[11]= EndPos-StartPos

		// update Ptrs.
		setupPointersForVertexProgram();
	}
	else
	{
		TexCoordPointer0= Accessor.getTexCoordPointer(0, 0);
		TexCoordPointer1= Accessor.getTexCoordPointer(0, 1);
		TexCoordPointer2= Accessor.getTexCoordPointer(0, 2);

		TexCoordOff0= vb.getTexCoordOff(0);
		TexCoordOff1= vb.getTexCoordOff(1);
		TexCoordOff2= vb.getTexCoordOff(2);
	}
}


} // NL3D
