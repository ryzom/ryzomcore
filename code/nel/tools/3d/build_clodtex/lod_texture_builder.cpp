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

#include "lod_texture_builder.h"
#include "nel/misc/aabbox.h"


using namespace std;
using namespace NLMISC;
using namespace NL3D;


// ***************************************************************************
// Quality factor. Useful to get maximum of interseting values in the range 0..255.
#define	NL_LTB_MAX_DISTANCE_QUALITY_FACTOR	0.05f
// The bigger, the lower the influence of the normal is. must be >=0
#define	NL_LTB_NORMAL_BIAS					1.f


// ***************************************************************************
CLodTextureBuilder::CLodTextureBuilder()
{
	_OverSampleDistance= 0.05f;
}


// ***************************************************************************
float			CLodTextureBuilder::computeQualityPixel(const CPixelInfo &p0, const CPixelInfo &p1) const
{
	float	d= (p1.Pos-p0.Pos).norm();
	float	dotProd= p1.Normal*p0.Normal;
	// With NL_NORMAL_BIAS==1, it return froms d*1(normals aligned) to d*3 (opposite normals)
	return d*(NL_LTB_NORMAL_BIAS+1-dotProd);
}

// ***************************************************************************
void			CLodTextureBuilder::setLod(const NL3D::CLodCharacterShapeBuild &lod)
{
	uint	i;

	_CLod= lod;

	// **** compute the max quality distance. ie where CTUVQ.Q== 255
	// build a bbox around the lod.
	CAABBox	bbox;
	bbox.setCenter(lod.Vertices[0]);
	for(i=0;i<lod.Vertices.size();i++)
		bbox.extend(lod.Vertices[i]);
	/* get the opposite vertices/normals and so compute the max qualityPixel "possible" (it is still possible 
		for the CMesh to be completely out of the bbox of the CLod, but doesn't matter)
	*/
	CPixelInfo	p0, p1;
	p0.Pos= bbox.getMin();
	p0.Normal.set(1,0,0);
	p1.Pos= bbox.getMax();
	p1.Normal.set(-1,0,0);
	_MaxDistanceQuality= computeQualityPixel(p0, p1);
	// apply a user factor.
	_MaxDistanceQuality*= NL_LTB_MAX_DISTANCE_QUALITY_FACTOR;
}

// ***************************************************************************
bool			CLodTextureBuilder::computeTexture(const CMesh &mesh, NL3D::CLodCharacterTexture  &text)
{
	// a set to flag if an edge has already been overSampled
	TEdgeSet	edgeSet;

	// **** Over sample the mesh
	_Samples.clear();
	_Samples.reserve(1000);
	// Get vertex info
	const CVertexBuffer	&VB= mesh.getVertexBuffer();
	CVertexBufferRead vba;
	VB.lock (vba);
	const uint8			*srcPos= (const uint8*)vba.getVertexCoordPointer();
	const uint8			*srcNormal= (const uint8*)vba.getNormalCoordPointer();
	const uint8			*srcUV= (const uint8*)vba.getTexCoordPointer();
	uint				vertexSize= VB.getVertexSize();
	// For all matrix blocks..
	for(uint mb=0; mb<mesh.getNbMatrixBlock();mb++)
	{
		// for all rdrPass
		for(uint rp=0; rp<mesh.getNbRdrPass(mb);rp++)
		{
			const CIndexBuffer	&pb= mesh.getRdrPassPrimitiveBlock(mb, rp);
			CIndexBufferRead iba;
			pb.lock (iba);
			uint	matId= mesh.getRdrPassMaterial(mb, rp);
			// samples the tris of this pass			
			if (iba.getFormat() == CIndexBuffer::Indices16)
			{					
				addSampleTris(srcPos, srcNormal, srcUV, vertexSize, (uint16 *) iba.getPtr(), pb.getNumIndexes()/3, matId, edgeSet);
			}
			else
			{
				nlassert(iba.getFormat() == CIndexBuffer::Indices32);
				addSampleTris(srcPos, srcNormal, srcUV, vertexSize, (uint32 *) iba.getPtr(), pb.getNumIndexes()/3, matId, edgeSet);
			}
		}
	}


	// **** compute the texture, with the samples
	computeTextureFromSamples(text);

	return true;
}

// ***************************************************************************
bool			CLodTextureBuilder::computeTexture(const CMeshMRM &meshMRM, NL3D::CLodCharacterTexture  &text)
{
	// a set to flag if an edge has already been overSampled
	TEdgeSet	edgeSet;

	// **** Over sample the mesh
	_Samples.clear();
	_Samples.reserve(1000);
	// Get vertex info
	CVertexBuffer		&VB= const_cast<CVertexBuffer&>(meshMRM.getVertexBuffer());
	CVertexBufferRead vba;
	VB.lock (vba);
	const uint8			*srcPos= (const uint8*)vba.getVertexCoordPointer();
	const uint8			*srcNormal= (const uint8*)vba.getNormalCoordPointer();
	const uint8			*srcUV= (const uint8*)vba.getTexCoordPointer();
	uint				vertexSize = VB.getVertexSize();
	// For the more precise lod
	uint	lodId= meshMRM.getNbLod()-1;
	// Resolve Geomoprh problem: copy End to all geomorphs dest. Hence sure that all ids points to good vertex data
	const std::vector<CMRMWedgeGeom>	&geoms= meshMRM.getMeshGeom().getGeomorphs(lodId);
	for(uint gm=0;gm<geoms.size();gm++)
	{
		uint	srcId= geoms[gm].End;
		// copy the geom src to the dest VB place.
		*(CVector*)(srcPos+gm*vertexSize)= *(CVector*)(srcPos+srcId*vertexSize);
		*(CVector*)(srcNormal+gm*vertexSize)= *(CVector*)(srcNormal+srcId*vertexSize);
		*(CUV*)(srcUV+gm*vertexSize)= *(CUV*)(srcUV+srcId*vertexSize);
	}
	// for all rdrPass
	for(uint rp=0; rp<meshMRM.getNbRdrPass(lodId);rp++)
	{
		const CIndexBuffer	&pb= meshMRM.getRdrPassPrimitiveBlock(lodId, rp);
		CIndexBufferRead iba;
		pb.lock (iba);
		uint	matId= meshMRM.getRdrPassMaterial(lodId, rp);
		// samples the tris of this pass
		if (iba.getFormat() == CIndexBuffer::Indices16)
		{
			addSampleTris(srcPos, srcNormal, srcUV, vertexSize, (uint16 *) iba.getPtr(), pb.getNumIndexes()/3, matId, edgeSet);
		}
		else
		{
			nlassert(iba.getFormat() == CIndexBuffer::Indices32);
			addSampleTris(srcPos, srcNormal, srcUV, vertexSize, (uint32 *) iba.getPtr(), pb.getNumIndexes()/3, matId, edgeSet);
		}
	}


	// **** compute the texture, with the samples
	computeTextureFromSamples(text);

	return true;
}


// ***************************************************************************
bool			CLodTextureBuilder::computeTexture(const CMeshMRMSkinned &meshMRM, NL3D::CLodCharacterTexture  &text)
{
	// a set to flag if an edge has already been overSampled
	TEdgeSet	edgeSet;

	// **** Over sample the mesh
	_Samples.clear();
	_Samples.reserve(1000);
	// Get vertex info
	CVertexBuffer tmp;
	meshMRM.getVertexBuffer(tmp);
	CVertexBufferRead vba;
	tmp.lock (vba);
	const uint8			*srcPos= (const uint8*)vba.getVertexCoordPointer();
	const uint8			*srcNormal= (const uint8*)vba.getNormalCoordPointer();
	const uint8			*srcUV= (const uint8*)vba.getTexCoordPointer();
	uint				vertexSize= tmp.getVertexSize();
	// For the more precise lod
	uint	lodId= meshMRM.getNbLod()-1;
	// Resolve Geomoprh problem: copy End to all geomorphs dest. Hence sure that all ids points to good vertex data
	const std::vector<CMRMWedgeGeom>	&geoms= meshMRM.getMeshGeom().getGeomorphs(lodId);
	for(uint gm=0;gm<geoms.size();gm++)
	{
		uint	srcId= geoms[gm].End;
		// copy the geom src to the dest VB place.
		*(CVector*)(srcPos+gm*vertexSize)= *(CVector*)(srcPos+srcId*vertexSize);
		*(CVector*)(srcNormal+gm*vertexSize)= *(CVector*)(srcNormal+srcId*vertexSize);
		*(CUV*)(srcUV+gm*vertexSize)= *(CUV*)(srcUV+srcId*vertexSize);
	}
	// for all rdrPass
	for(uint rp=0; rp<meshMRM.getNbRdrPass(lodId);rp++)
	{
		CIndexBuffer	pb;
		meshMRM.getRdrPassPrimitiveBlock(lodId, rp, pb);
		uint	matId= meshMRM.getRdrPassMaterial(lodId, rp);
		CIndexBufferRead iba;
		pb.lock (iba);
		// samples the tris of this pass
		if (iba.getFormat() == CIndexBuffer::Indices16)
		{
			addSampleTris(srcPos, srcNormal, srcUV, vertexSize, (uint16 *) iba.getPtr(), pb.getNumIndexes()/3, matId, edgeSet);
		}
		else
		{
			addSampleTris(srcPos, srcNormal, srcUV, vertexSize, (uint32 *) iba.getPtr(), pb.getNumIndexes()/3, matId, edgeSet);
		}
	}


	// **** compute the texture, with the samples
	computeTextureFromSamples(text);

	return true;
}

// ***************************************************************************
void			CLodTextureBuilder::addSampleTris(const uint8 *srcPos, const uint8 *srcNormal, const uint8 *srcUV, uint vertexSize, 
	const uint16 *triPointer, uint numTris, uint materialId, TEdgeSet &edgeSet)
{
	std::vector<uint32> indices32(triPointer, triPointer + numTris * 3);
	addSampleTris(srcPos, srcNormal, srcUV, vertexSize, &indices32[0], numTris, materialId, edgeSet);
}

// ***************************************************************************
void			CLodTextureBuilder::addSampleTris(const uint8 *srcPos, const uint8 *srcNormal, const uint8 *srcUV, uint vertexSize, 
	const uint32 *triPointer, uint numTris, uint materialId, TEdgeSet &edgeSet)
{
	nlassert(srcPos);

	// ensure that the material is in 0..255
	clamp(materialId, 0U, 255U);

	for(uint i=0;i<numTris;i++)
	{
		uint c;

		// **** get tri indices and tri values.
		uint		idx[3];
		CVector		pos[3];
		CVector		normal[3];
		CUV			uv[3];
		for(c=0;c<3;c++)
		{
			idx[c]= *triPointer++;
			pos[c]= *(CVector*)(srcPos + idx[c]*vertexSize);
			if(srcNormal)
				normal[c]= *(CVector*)(srcNormal + idx[c]*vertexSize);
			else
				normal[c]= CVector::K;
			if(srcUV)
				uv[c]= *(CUV*)(srcUV + idx[c]*vertexSize);
			else
				uv[c].set(0,0);
		}

		// **** Append the 3 vertices in the samples. only if not already done
		for(c=0;c<3;c++)
		{
			// special edgeId: vertStart==vertEnd
			TEdge	edgeId;
			edgeId.first= idx[c];
			edgeId.second= idx[c];
			// if success to insert in the set, then must add it to samples
			if(edgeSet.insert(edgeId).second)
			{
				CSample		sample;
				sample.P.Pos= pos[c];
				sample.P.Normal= normal[c];
				sample.P.Normal.normalize();
				sample.UV= uv[c];
				sample.MaterialId= materialId;
				_Samples.push_back(sample);
			}
		}

		// **** Append the interior of each edges, according to _OverSampleDistance
		uint	numEdgeOverSamples[3];
		for(c=0;c<3;c++)
		{
			uint	cNext= (c+1)%3;
			// get the edgeId
			TEdge	edgeId;
			edgeId.first= idx[c];
			edgeId.second= idx[cNext];
			// compute the number of overSamples to apply to the edge
			numEdgeOverSamples[c]= (uint)floor( (pos[cNext]-pos[c]).norm()/_OverSampleDistance );

			// if success to insert in the set, then must add it to samples
			if(edgeSet.insert(edgeId).second)
			{
				// Do it only if numOverSamples>=2
				if(numEdgeOverSamples[c]>=2)
				{
					// Sample the edge, exclude endPoints.
					for(uint s=1;s<numEdgeOverSamples[c];s++)
					{
						float	f= s/(float)numEdgeOverSamples[c];
						// interpoalte the sample
						CSample		sample;
						sample.P.Pos= pos[c]*(1-f) + pos[cNext]*f;
						sample.P.Normal= normal[c]*(1-f) + normal[cNext]*f;
						sample.P.Normal.normalize();
						sample.UV= uv[c]*(1-f) + uv[cNext]*f;
						sample.MaterialId= materialId;
						// add it
						_Samples.push_back(sample);
					}
				}
			}
		}

		// **** Append the interior of the triangle, if too many overSample
		// compute min of over samples
		uint	triOverSample= min(numEdgeOverSamples[0], numEdgeOverSamples[1]);
		triOverSample= min(triOverSample, numEdgeOverSamples[2]);
		// If >=3 (at least one vertex in the middle)
		if(triOverSample>=2)
		{
			// Interpolate with barycentric coordinate rules
			uint	s,t,v;
			// Donc't compute on triangles edges.
			for(s=1;s<triOverSample;s++)
			{
				for(t=1;t<triOverSample-s;t++)
				{
					// barycentric sum==1 guaranteed.
					v= triOverSample-s-t;
					float	fs= s/(float)triOverSample;
					float	ft= t/(float)triOverSample;
					float	fv= v/(float)triOverSample;
					// compute the sample
					CSample		sample;
					sample.P.Pos= pos[0]*fs + pos[1]*ft + pos[2]*fv;
					sample.P.Normal= normal[0]*fs + normal[1]*ft + normal[2]*fv;
					sample.P.Normal.normalize();
					sample.UV= uv[0]*fs + uv[1]*ft + uv[2]*fv;
					sample.MaterialId= materialId;
					// add it
					_Samples.push_back(sample);
				}
			}
		}

	}
}


// ***************************************************************************
bool			CLodTextureBuilder::computeTextureFromSamples(NL3D::CLodCharacterTexture  &text)
{
	// The lod must have correct width/height
	if(NL3D_CLOD_TEXT_WIDTH!=_CLod.getTextureInfoWidth() || NL3D_CLOD_TEXT_HEIGHT!=_CLod.getTextureInfoHeight() )
	{
		nlwarning("ERROR: the _CLod must have a textureInfo size of %d/%d", NL3D_CLOD_TEXT_WIDTH, NL3D_CLOD_TEXT_HEIGHT);
		return false;
	}

	// prepare dest.
	text.Texture.resize(NL3D_CLOD_TEXT_SIZE);
	CLodCharacterTexture::CTUVQ		emptyTUVQ;
	emptyTUVQ.T= 0;
	emptyTUVQ.U= 0;
	emptyTUVQ.V= 0;
	emptyTUVQ.Q= 255;
	fill(text.Texture.begin(), text.Texture.end(), emptyTUVQ);

	// For all pixels.
	const	CPixelInfo		*srcPixel= _CLod.getTextureInfoPtr();
	for(uint i=0;i<NL3D_CLOD_TEXT_SIZE;i++)
	{
		// if the src pixel is not empty
		if( !(srcPixel[i]==CPixelInfo::EmptyPixel) )
		{
			CLodCharacterTexture::CTUVQ		&dstTUVQ= text.Texture[i];

			// For All samples, search the best.
			float	minQuality= FLT_MAX;
			uint	bestIdx= 0;
			for(uint j=0;j<_Samples.size();j++)
			{
				float	q= computeQualityPixel(srcPixel[i], _Samples[j].P);
				if(q<minQuality)
				{
					minQuality= q;
					bestIdx= j;
				}
			}

			// Then compress the sample info in the TUVQ.
			dstTUVQ.T= _Samples[bestIdx].MaterialId;
			dstTUVQ.U= (uint8)(sint)(_Samples[bestIdx].UV.U*256);
			dstTUVQ.V= (uint8)(sint)(_Samples[bestIdx].UV.V*256);
			minQuality= minQuality*255/_MaxDistanceQuality;
			clamp(minQuality, 0, 255);
			dstTUVQ.Q= (uint8)minQuality;
		}
	}

	return true;
}

