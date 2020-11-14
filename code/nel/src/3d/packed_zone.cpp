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
//
#include "nel/3d/packed_zone.h"
#include "nel/3d/driver.h"
#include "nel/3d/material.h"
//
#include "nel/misc/matrix.h"
#include "nel/misc/polygon.h"
#include "nel/misc/path.h"
#include "nel/misc/grid_traversal.h"
#include "nel/misc/bit_mem_stream.h"
//
#include <limits>
#include <iterator>
//


using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

#define PACKED_COL_RANGE 4095

class CVectorPacker
{
public:
	struct CFormat
	{
		uint Rel0NumBits; // 2 for - 2 .. + 1 range
		uint Rel1NumBits; // 4 for - 8 .. + 7 range
		uint Rel2NumBits; // 8 for - 128 .. + 127 range
		uint AbsNumBits; // num bits for absolute value
	};
	void serialPackedVector16(std::vector<uint16> &v, NLMISC::IStream &f, const CFormat &format);


private:
	// format is stored as 2 bits at each entry
	enum TFormat
	{
		Rel0 = 0, // - 2 .. + 1 range
		Rel1 = 1, // - 8 .. + 7 range
		Rel2 = 2, // - 128 .. + 127 range
		AbsOrRLE = 3 // full precision absolute value
	};
	//
	std::vector<sint32> _LastDeltas;
	uint32				_LastTag;
	uint				_ReadIndex;
	uint				_Count[4]; // statistics
	uint				_Repeated[4]; // statistics
	CFormat				_Format;
private:
	void bufferizeDelta(uint32 tag, sint32 delta, CBitMemStream &bits);
	void flush(CBitMemStream &bits);
	void writeSimpleValue(CBitMemStream &bits, sint32 delta);
	void readSimpleValue(uint32 tag, std::vector<uint16> &v, CBitMemStream &bits);
	static sint32 getMax(uint numBits)
	{
		return (uint32) ((1 << (numBits - 1)) - 1);
	}
	static sint32 getMin(uint numBits)
	{
		return (uint32) (0xfffffffe << (numBits - 2));
	}
	static void extendSign(uint numBits, uint32 &value)
	{
		if (value & (1 << (numBits - 1)))
		{
			value |= (uint32) (0xfffffffe << (numBits - 1));
		}
	}
};

// ********************************************************************************************
void CVectorPacker::writeSimpleValue(CBitMemStream &bits, sint32 delta)
{
	uint32 croppedDelta = (uint32) delta;
	switch(_LastTag)
	{
		case Rel0:
		{
			bits.serial(croppedDelta, _Format.Rel0NumBits);
		}
		break;
		case Rel1:
		{
			bits.serial(croppedDelta, _Format.Rel1NumBits);
		}
		break;
		case Rel2:
		{
			bits.serial(croppedDelta, _Format.Rel2NumBits);
		}
		break;
		case AbsOrRLE:
		{
			// not a delta but a value ...
			uint32 value = (uint16) delta;
			bits.serial(value, _Format.AbsNumBits);
		}
		break;
		default:
			nlassert(0);
		break;
	}
	if (_LastTag == AbsOrRLE)
	{
		//nlwarning("writing simple value %d (tag = %d)", (int) (uint16) delta, (int) _LastTag);
	}
	else
	{
		//nlwarning("writing simple delta %d (tag = %d)", (int) delta, (int) _LastTag);
	}
	++ _Count[_LastTag];
}

// ********************************************************************************************
void CVectorPacker::readSimpleValue(uint32 tag, std::vector<uint16> &v, CBitMemStream &bits)
{
	uint32 croppedDelta;
	switch(tag)
	{
		case Rel0:
			bits.serial(croppedDelta, _Format.Rel0NumBits);
			extendSign(_Format.Rel0NumBits, croppedDelta);
			v[_ReadIndex] = (uint16) (v[_ReadIndex - 1] + (sint16) croppedDelta);
		break;
		case Rel1:
			bits.serial(croppedDelta, _Format.Rel1NumBits);
			extendSign(_Format.Rel1NumBits, croppedDelta);
			v[_ReadIndex] = (uint16) (v[_ReadIndex - 1] + (sint16) croppedDelta);
		break;
		case Rel2:
			bits.serial(croppedDelta, _Format.Rel2NumBits);
			extendSign(_Format.Rel2NumBits, croppedDelta);
			v[_ReadIndex] = (uint16) (v[_ReadIndex - 1] + (sint16) croppedDelta);
		break;
		default:
			nlassert(0);
		break;
	}
	//nlwarning("reading simple delta %d (tag = %d)", (int) (v[_ReadIndex] - v[_ReadIndex - 1]), (int) tag);
	++ _ReadIndex;
}

// ********************************************************************************************
void CVectorPacker::flush(CBitMemStream &bits)
{
	if (_LastDeltas.empty()) return;
	if (_LastDeltas.size() > 4)
	{
		// put a 'repeat tag'
		uint32 repeatTag = AbsOrRLE;
		bits.serial(repeatTag, 2);
		uint32 isRLE = 1;
		bits.serial(isRLE, 1);
		bits.serial(_LastTag, 2);
		nlassert(_LastDeltas.size() <= 255);
		uint8 length = (uint8)_LastDeltas.size();
		//nlwarning("begin RLE, length = %d, tag = %d", (int) length, (int) repeatTag);
		bits.serial(length);
		for(uint k = 0; k < _LastDeltas.size(); ++k)
		{
			writeSimpleValue(bits, _LastDeltas[k]);
		}
		_LastDeltas.clear();
		++_Repeated[_LastTag];
	}
	else
	{
		// flush separate values
		for(uint k = 0; k < _LastDeltas.size(); ++k)
		{
			//nlwarning("write single value tag %d", (int) _LastTag);
			bits.serial(_LastTag, 2);
			if (_LastTag == (uint32) AbsOrRLE)
			{
				uint32 isRLE = 0;
				bits.serial(isRLE, 1);
			}
			writeSimpleValue(bits, _LastDeltas[k]);
		}
		_LastDeltas.clear();
	}
}

// ********************************************************************************************
void CVectorPacker::bufferizeDelta(uint32 tag, sint32 delta, CBitMemStream &bits)
{
	if (tag != _LastTag || _LastDeltas.size() == 255)
	{
		flush(bits);
		_LastTag = tag;
	}
	_LastDeltas.push_back(delta);
}

// ********************************************************************************************
void CVectorPacker::serialPackedVector16(std::vector<uint16> &v,NLMISC::IStream &f, const CFormat &format)
{
	_Format = format;
	if (f.isReading())
	{
		CBitMemStream bits(true);
		std::vector<uint8> datas;
		f.serialCont(datas);
		bits.fill(&datas[0], (uint32)datas.size());
		uint32 numValues = 0;
		bits.serial(numValues);
		v.resize(numValues);
		_ReadIndex = 0;
		while (_ReadIndex != numValues)
		{
			if (_ReadIndex == 0)
			{
				uint32 value;
				bits.serial(value, _Format.AbsNumBits);
				v[0] = (uint16) value;
				//nlwarning("v[0] = %d", (int) v[0]);
				++ _ReadIndex;
			}
			else
			{
				uint32 tag;
				bits.serial(tag, 2);
				//nlwarning("read tag %d", (int) tag);
				switch(tag)
				{

					case Rel0:
					case Rel1:
					case Rel2:
						readSimpleValue(tag, v, bits);
					break;
					case AbsOrRLE:
					{
						// serial one more bit to see if there's RLE here
						uint32 isRLE = 0;
						bits.serial(isRLE, 1);
						if (isRLE)
						{
							uint32 repeatTag;
							bits.serial(repeatTag, 2);
							uint8 length = 0;
							bits.serial(length);
							//nlwarning("begin RLE, length = %d", (int) length);
							for(uint l = 0; l < length; ++l)
							{
								switch(repeatTag)
								{
									case Rel0:
									case Rel1:
									case Rel2:
										readSimpleValue(repeatTag, v, bits);
									break;
									case AbsOrRLE:
										uint32 value;
										bits.serial(value, _Format.AbsNumBits);
										v[_ReadIndex] = (uint16) value;
										//nlwarning("reading abs 16 value : %d", (int) v[_ReadIndex]);
										++ _ReadIndex;
									break;
									default:
										throw NLMISC::EInvalidDataStream();
									break;
								}
							}
						}
						else
						{
							// absolute value
							uint32 value = 0;
							bits.serial(value, _Format.AbsNumBits);
							v[_ReadIndex] = (uint16) value;
							//nlwarning("reading abs 16 value : %d", (int) v[_ReadIndex]);
							++ _ReadIndex;
						}
					}
					break;
				}
			}
		}
	}
	else
	{
		_Count[Rel0] = 0;
		_Count[Rel1] = 0;
		_Count[Rel2] = 0;
		_Count[AbsOrRLE] = 0;
		_Repeated[Rel0] = 0;
		_Repeated[Rel1] = 0;
		_Repeated[Rel2] = 0;
		_Repeated[AbsOrRLE] = 0;
		//
		CBitMemStream bits(false);
		uint32 numValues = (uint32)v.size();
		bits.serial(numValues);
		_LastTag = std::numeric_limits<uint32>::max();
		_LastDeltas.clear();
		for(uint k = 0; k < v.size(); ++k)
		{
			if (k == 0)
			{
				uint32 value = v[0];
				bits.serial(value, _Format.AbsNumBits);
				//nlwarning("v[0] = %d", (int) v[0]);
			}
			else
			{
				sint32 delta = v[k] - v[k - 1];
				if (delta >= getMin(_Format.Rel0NumBits) && delta <= getMax(_Format.Rel0NumBits))
				{
					bufferizeDelta(Rel0, delta, bits);
				}
				else if (delta >= getMin(_Format.Rel1NumBits) && delta <= getMax(_Format.Rel1NumBits))
				{
					bufferizeDelta(Rel1, delta, bits);
				}
				else if (delta >= getMin(_Format.Rel2NumBits) && delta <= getMax(_Format.Rel2NumBits))
				{
					bufferizeDelta(Rel2, delta, bits);
				}
				else
				{
					bufferizeDelta(AbsOrRLE, v[k], bits); // absolute value here ...
				}
			}
		}
		flush(bits);
		std::vector<uint8> datas(bits.buffer(), bits.buffer() + bits.length());
		f.serialCont(datas);
		/*
		if (_Count[Rel0] != 0 || _Count[Rel1] != 0 || _Count[Rel2] != 0 || _Count[AbsOrRLE] != 0)
		{
			nlwarning("count0 = %d", _Count[Rel0]);
			nlwarning("count1 = %d", _Count[Rel1]);
			nlwarning("count2 = %d", _Count[Rel2]);
			nlwarning("countAbs = %d", _Count[AbsOrRLE]);
			nlwarning("*");
			nlwarning("repeat 0 = %d", _Repeated[Rel0]);
			nlwarning("repeat 1 = %d", _Repeated[Rel1]);
			nlwarning("repeat 2 = %d", _Repeated[Rel2]);
			nlwarning("repeat Abs = %d", _Repeated[AbsOrRLE]);
			nlwarning("*");
		}
		*/
	}
}



// helper function : serialize a uint16 vector
void serialPackedVector16(std::vector<uint16> &v, NLMISC::IStream &f)
{
	CVectorPacker packer;
	CVectorPacker::CFormat format;
	format.Rel0NumBits = 2;
	format.Rel1NumBits = 4;
	format.Rel2NumBits = 8;
	format.AbsNumBits = 16;
	packer.serialPackedVector16(v, f, format);
	/*
	if (!v.empty())
	{
		nlwarning("*");
		uint num = std::min((uint) v.size(), (uint) 1000);
		for(uint k = 0; k < num; ++k)
		{
			nlwarning("[%d] = %d", (int) k, (int) v[k]);
		}
		nlwarning("*");
	}
	*/
}


void serialPackedVector12(std::vector<uint16> &v, NLMISC::IStream &f)
{
	CVectorPacker packer;
	CVectorPacker::CFormat format;
	format.Rel0NumBits = 2;
	format.Rel1NumBits = 6;
	format.Rel2NumBits = 9;
	format.AbsNumBits = 12;
	packer.serialPackedVector16(v, f, format);
	/*
	if (!v.empty())
	{
		nlwarning("*");
		uint num = std::min((uint) v.size(), (uint) 1000);
		for(uint k = 0; k < num; ++k)
		{
			nlwarning("[%d] = %d", (int) k, (int) v[k]);
		}
		nlwarning("*");
	}
	*/
}

// some function to ease writing of some primitives into a vertex buffer
static inline void pushVBLine2D(NLMISC::CVector *&dest, const NLMISC::CVector &v0, const NLMISC::CVector &v1)
{
	dest->x = v0.x;
	dest->y = v0.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = v1.x;
	dest->y = v1.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = v0.x;
	dest->y = v0.y;
	dest->z = -0.5f;
	++ dest;
}

static inline void pushVBTri2D(NLMISC::CVector *&dest, const NLMISC::CTriangle &tri)
{
	dest->x = tri.V0.x;
	dest->y = tri.V0.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = tri.V1.x;
	dest->y = tri.V1.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = tri.V2.x;
	dest->y = tri.V2.y;
	dest->z = -0.5f;
	++ dest;
}


static inline void pushVBQuad2D(NLMISC::CVector *&dest, const NLMISC::CQuad &quad)
{
	dest->x = quad.V0.x;
	dest->y = quad.V0.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = quad.V1.x;
	dest->y = quad.V1.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = quad.V2.x;
	dest->y = quad.V2.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = quad.V3.x;
	dest->y = quad.V3.y;
	dest->z = -0.5f;
	++ dest;
}

static inline void pushVBQuad(NLMISC::CVector *&dest, const NLMISC::CQuad &quad)
{
	*dest++ = quad.V0;
	*dest++ = quad.V1;
	*dest++ = quad.V2;
	*dest++ = quad.V3;
}


// compute rasters union.
static void computeRastersUnion(const CPolygon2D::TRasterVect &inRaster0, CPolygon2D::TRasterVect &inRaster1, sint minY0, sint minY1,
						 CPolygon2D::TRasterVect &outRaster, sint &finalMinY)
{
	if (inRaster0.empty())
	{
		if (inRaster1.empty())
		{
			outRaster.empty();
			finalMinY = -1;
			return;
		}
		outRaster = inRaster1;
		finalMinY = minY1;
		return;
	}
	else if (inRaster1.empty())
	{
		outRaster = inRaster0;
		finalMinY = minY0;
		return;
	}
	nlassert(&outRaster != &inRaster0);
	nlassert(&outRaster != &inRaster1);
	finalMinY = std::min(minY0, minY1);
	sint maxY = std::max(minY0 + (sint) inRaster0.size(), minY1 + (sint) inRaster1.size());
	outRaster.resize(maxY - finalMinY);
	for(sint y = 0; y < (sint) outRaster.size(); ++y)
	{
		outRaster[y].first  = INT_MAX;
		outRaster[y].second = INT_MIN;
		sint raster0Y = y + finalMinY - minY0;
		if (raster0Y >= 0  && raster0Y < (sint) inRaster0.size())
		{
			//if (inRaster0[raster0Y].second >= inRaster0[raster0Y].first)
			{
				outRaster[y].first = std::min(outRaster[y].first, inRaster0[raster0Y].first);
				outRaster[y].second = std::max(outRaster[y].second, inRaster0[raster0Y].second);
			}
		}
		sint raster1Y = y + finalMinY - minY1;
		if (raster1Y >= 0  && raster1Y < (sint) inRaster1.size())
		{
			//if (inRaster1[raster1Y].second >= inRaster1[raster1Y].first)
			{
				outRaster[y].first = std::min(outRaster[y].first, inRaster1[raster1Y].first);
				outRaster[y].second = std::max(outRaster[y].second, inRaster1[raster1Y].second);
			}
		}
	}
}

static void addQuadToSilhouette(const CVector &v0, const CVector &v1, const CVector &v2, CVector &v3, CPolygon2D::TRasterVect &sil, sint &minY, float cellSize)
{
	static CPolygon2D quad;
	quad.Vertices.resize(4);
	quad.Vertices[0] = v0 / cellSize;
	quad.Vertices[1] = v1 / cellSize;
	quad.Vertices[2] = v2 / cellSize;
	quad.Vertices[3] = v3 / cellSize ;
	//
	static CPolygon2D::TRasterVect newQuad;
	static CPolygon2D::TRasterVect result;
	sint newMinY = -1;
	sint resultMinY;
	quad.computeOuterBorders(newQuad, newMinY);
	computeRastersUnion(sil, newQuad, minY, newMinY, result, resultMinY);
	minY = resultMinY;
	sil.swap(result);
}

uint16 CPackedZone16::UndefIndex = 0xffff;

// ***************************************************************************************
CPackedZone16::CPackedZone16()
{
	CellSize = 0.f;
	_Origin = CVector::Null;
}

// ***************************************************************************************
void CPackedZone32::unpackTri(const CPackedTri &src, CVector dest[3]) const
{
	// TODO: add 'multiply-add' operator
	dest[0].set(Verts[src.V0].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V0].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V0].Z * _PackedLocalToWorld.z + _Origin.z);
	dest[1].set(Verts[src.V1].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V1].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V1].Z * _PackedLocalToWorld.z + _Origin.z);
	dest[2].set(Verts[src.V2].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V2].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V2].Z * _PackedLocalToWorld.z + _Origin.z);

}

uint32 CPackedZone32::UndefIndex = 0xffffffff;

// ***************************************************************************************
CPackedZone32::CPackedZone32()
{
	CellSize = 0;
	_Origin = CVector::Null;
}

// ***************************************************************************************
void CPackedZone32::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);
	f.serial(Box);
	f.serialCont(Verts);
	f.serialCont(Tris);
	f.serialCont(TriLists);
	f.serial(Grid);
	f.serial(CellSize);
	f.serial(_Origin);
	f.serial(_WorldToLocal);
	f.serial(ZoneX);
	f.serial(ZoneY);
	if (f.isReading())
	{
		_PackedLocalToWorld.set(1.f / (_WorldToLocal.x * (float) PACKED_COL_RANGE),
								1.f / (_WorldToLocal.y * (float) PACKED_COL_RANGE),
								1.f / (_WorldToLocal.z * (float) PACKED_COL_RANGE));
	}
}

// used by CPackedZone::build
struct CZoneInstance
{
	const CShapeInfo *SI;
	CMatrix    WorldMat;
};

// ***************************************************************************************
void CPackedZone32::build(std::vector<const CTessFace*> &leaves,
						float cellSize,
						std::vector<CInstanceGroup *> igs,
						const TShapeCache &shapeCache,
						const NLMISC::CAABBox &baseZoneBBox,
						sint32	zoneX,
						sint32	zoneY
					   )
{
	nlassert(cellSize > 0);
	Verts.clear();
	Tris.clear();
	TriLists.clear();
	Grid.clear();
	if (leaves.empty()) return;
	for(uint k = 0; k < leaves.size(); ++k)
	{
		if (k == 0)
		{
			Box.setMinMax(leaves[k]->VBase->EndPos, leaves[k]->VBase->EndPos);
			Box.extend(leaves[k]->VLeft->EndPos);
			Box.extend(leaves[k]->VRight->EndPos);
		}
		else
		{
			Box.extend(leaves[k]->VBase->EndPos);
			Box.extend(leaves[k]->VLeft->EndPos);
			Box.extend(leaves[k]->VRight->EndPos);
		}
	}
	CAABBox landBBox = Box;
	landBBox.extend(baseZoneBBox.getMin());
	landBBox.extend(baseZoneBBox.getMax());
	// list of instances that contribute to that zone
	std::vector<CZoneInstance> instances;
	// extends with instances that intersect the land bbox with respect to x/y
	for(uint k = 0; k < igs.size(); ++k)
	{
		if (!igs[k]) continue;
		for(uint l = 0; l < igs[k]->getNumInstance(); ++l)
		{
			CMatrix instanceMatrix;
			igs[k]->getInstanceMatrix(l, instanceMatrix);
			if (NLMISC::toLower(NLMISC::CFile::getExtension(igs[k]->getShapeName(l))) == "pacs_prim") continue;
			std::string stdShapeName = standardizeShapeName(igs[k]->getShapeName(l));
			TShapeCache::const_iterator it = shapeCache.find(stdShapeName);
			if (it != shapeCache.end())
			{
				CAABBox xformBBox = CAABBox::transformAABBox(instanceMatrix, it->second.LocalBBox);
				if (xformBBox.getMin().x < landBBox.getMax().x &&
					xformBBox.getMin().y < landBBox.getMax().y &&
					xformBBox.getMax().y >= landBBox.getMin().y &&
					xformBBox.getMax().x >= landBBox.getMin().x)
				{
					Box.extend(xformBBox.getMin());
					Box.extend(xformBBox.getMax());
					CZoneInstance zi;
					zi.SI = &(it->second);
					zi.WorldMat = instanceMatrix;
					instances.push_back(zi);
				}
			}
		}
	}
	//
	/*float delta = 5.f;
	Box.extend(Box.getMin() + CVector(- delta, - delta, - delta));
	Box.extend(Box.getMax() + CVector(delta, delta, delta));*/
	//
	CVector cornerMin = Box.getMin();
	CVector cornerMax = Box.getMax();
	//
	uint width = (uint) ceilf((cornerMax.x - cornerMin.x) / cellSize);
	uint height = (uint) ceilf((cornerMax.y - cornerMin.y) / cellSize);
	float depth = cornerMax.z - cornerMin.z;
	if (width == 0 || height == 0) return;
	Grid.init(width, height, UndefIndex);
	//
	TVertexGrid   vertexGrid; // grid for fast sharing of vertices
	TTriListGrid  triListGrid; // grid for list of tris per grid cell (before packing in a single vector)
	vertexGrid.init(width, height);
	triListGrid.init(width, height);
	CellSize = cellSize;
	//
	_Origin = cornerMin;
	_WorldToLocal.x = 1.f / (width * cellSize);
	_WorldToLocal.y = 1.f / (height * cellSize);
	_WorldToLocal.z = depth != 0  ? 1.f / depth : 0.f;
	//
	for(uint k = 0; k < leaves.size(); ++k)
	{
		CTriangle tri;
		tri.V0 = leaves[k]->VBase->EndPos;
		tri.V1 = leaves[k]->VLeft->EndPos;
		tri.V2 = leaves[k]->VRight->EndPos;
		addTri(tri, vertexGrid, triListGrid);
	}
	// add each ig
	for(uint k = 0; k < instances.size(); ++k)
	{
		addInstance(*instances[k].SI, instances[k].WorldMat, vertexGrid, triListGrid);
	}
	// pack tri lists
	for (uint y = 0; y < height; ++y)
	{
		for (uint x = 0; x < width; ++x)
		{
			if (!triListGrid(x, y).empty())
			{
				Grid(x, y) = (uint32)TriLists.size();
				std::copy(triListGrid(x, y).begin(), triListGrid(x, y).end(), std::back_inserter(TriLists));
				TriLists.push_back(UndefIndex); // mark the end of the list
			}
		}
	}
	//
	ZoneX = zoneX;
	ZoneX = zoneY;
	//
	_PackedLocalToWorld.set(1.f / (_WorldToLocal.x * (float) PACKED_COL_RANGE),
						    1.f / (_WorldToLocal.y * (float) PACKED_COL_RANGE),
						    1.f / (_WorldToLocal.z * (float) PACKED_COL_RANGE));
	// reorder vertices by distance for better compression : start with first vertex then found closest vertex
	//
	/*
	std::vector<uint32> triRemapping(Verts.size());
	std::vector<uint32> vertRemapping(Verts.size());
	std::vector<uint32> todo(Verts.size());
	for(uint k = 0; k < Verts.size(); ++k)
	{
		todo[k] = k;
	}
	CPackedVertex lastPos;
	for(uint k = 0; k < Verts.size(); ++k)
	{
		// find vertex with closest dist
		uint best;
		if (k == 0)
		{
			best = 0;
		}
		else
		{
			uint32 bestDist = INT_MAX;
			for(uint l = 0; l < todo.size(); ++l)
			{
				uint32 dist = (uint32) abs((sint32) Verts[todo[l]].X - (sint32) lastPos.X)
							  + (uint32) abs((sint32) Verts[todo[l]].Y - (sint32) lastPos.Y)
							  + (uint32) abs((sint32) Verts[todo[l]].Z - (sint32) lastPos.Z);
				if (dist < bestDist)
				{
					bestDist = dist;
					best = l;
				}
			}
		}
		lastPos = Verts[todo[best]];
		vertRemapping[k] = todo[best];
		triRemapping[todo[best]] = k;
		todo[best] = todo[todo.size() - 1];
		todo.pop_back();
	}
	// remap all tris
	for(uint k = 0; k < Tris.size(); ++k)
	{
		Tris[k].V0 = triRemapping[Tris[k].V0];
		Tris[k].V1 = triRemapping[Tris[k].V1];
		Tris[k].V2 = triRemapping[Tris[k].V2];
	}
	// reorder vertices
	std::vector<CPackedVertex>  remappedVerts(Verts.size());
	for(uint k = 0; k < Verts.size(); ++k)
	{
		remappedVerts[k] = Verts[vertRemapping[k]];
	}
	Verts.swap(remappedVerts);
	/////////////////////////////////////////////////////////////////////
	// reorder tris
	triRemapping.resize(Tris.size());
	std::vector<uint32> triListRemapping(Tris.size());
	//
	todo.resize(Tris.size());
	for(uint k = 0; k < Tris.size(); ++k)
	{
		todo[k] = k;
	}
	uint32 lastIndex = 0;
	for(uint k = 0; k < Tris.size(); ++k)
	{
		uint32 best = 0;
		uint32 bestScore = INT_MAX;
		for(uint l = 0; l < todo.size(); ++l)
		{
			uint32 score = abs(Tris[todo[l]].V0 - lastIndex) +
						   abs(Tris[todo[l]].V1 - Tris[todo[l]].V0) +
						   abs(Tris[todo[l]].V2 - Tris[todo[l]].V1);
			if (score < bestScore)
			{
				bestScore = score;
				best = l;
			}
		}
		lastIndex = Tris[todo[best]].V2;
		triRemapping[k] = todo[best];
		triListRemapping[todo[best]] = k;
		todo[best] = todo[todo.size() - 1];
		todo.pop_back();
	}
	// remap tri lists
	for(uint k = 0; k < TriLists.size(); ++k)
	{
		if (TriLists[k] != std::numeric_limits<uint32>::max())
		{
			TriLists[k] = triListRemapping[TriLists[k]];
		}
	}
	// reorder tris
	std::vector<CPackedTri>  remappedTris(Tris.size());
	for(uint k = 0; k < Tris.size(); ++k)
	{
		remappedTris[k] = Tris[triRemapping[k]];
	}
	Tris.swap(remappedTris);
	*/

	// reorder tri lists for better compression
	std::vector<uint32>::iterator firstIt = TriLists.begin();
	std::vector<uint32>::iterator currIt = firstIt;
	std::vector<uint32>::iterator lastIt = TriLists.end();

	while (currIt != lastIt)
	{
		if (*currIt == UndefIndex)
		{
			std::sort(firstIt, currIt);
			++ currIt;
			if (currIt == lastIt) break;
			firstIt = currIt;
		}
		else
		{
			++ currIt;
		}
	}
	//
	/*int vertsSize = sizeof(CPackedVertex) * Verts.size();
	int trisSize = sizeof(CPackedTri) * Tris.size();
	int triListSize = sizeof(uint32) * TriLists.size();
	int gridSize = sizeof(uint32) * Grid.getWidth() * Grid.getHeight();
	*/
	/*printf("Verts Size = %d\n", vertsSize);
	printf("Tris Size = %d\n", trisSize);
	printf("Tri List Size = %d\n", triListSize);
	printf("Grid size = %d\n", gridSize);
	printf("Total = %d\n", vertsSize + trisSize + triListSize + gridSize);*/
}

// ***************************************************************************************
void CPackedZone32::addTri(const CTriangle &tri, TVertexGrid &vertexGrid, TTriListGrid &triListGrid)
{
	CPackedTri pt;
	pt.V0 = allocVertex(tri.V0, vertexGrid);
	pt.V1 = allocVertex(tri.V1, vertexGrid);
	pt.V2 = allocVertex(tri.V2, vertexGrid);

	//
	static CPolygon2D::TRasterVect  rasters;
	static CPolygon2D				polyTri;
	sint							minY;
	polyTri.Vertices.resize(3);
	//
	polyTri.Vertices[0].set((tri.V0.x - _Origin.x) / CellSize, (tri.V0.y - _Origin.y) / CellSize);
	polyTri.Vertices[1].set((tri.V1.x - _Origin.x) / CellSize, (tri.V1.y - _Origin.y) / CellSize);
	polyTri.Vertices[2].set((tri.V2.x - _Origin.x) / CellSize, (tri.V2.y - _Origin.y) / CellSize);
	//
	polyTri.computeOuterBorders(rasters, minY);
	if (rasters.empty()) return;
	Tris.push_back(pt);
	//
	for (sint y = 0; y < (sint) rasters.size(); ++y)
	{
		sint gridY = y + minY;
		if (gridY < 0) continue;
		if (gridY >= (sint) triListGrid.getHeight()) break;
		for (sint x = rasters[y].first; x <= rasters[y].second; ++x)
		{
			if (x < 0) continue;
			if (x >= (sint) triListGrid.getWidth()) break;
			triListGrid(x, gridY).push_back((uint32)Tris.size() - 1);
		}
	}
}



// ***************************************************************************************
uint32 CPackedZone32::allocVertex(const CVector &src, TVertexGrid &vertexGrid)
{
	CVector local((src.x - _Origin.x) * _WorldToLocal.x,
		          (src.y - _Origin.y) * _WorldToLocal.y,
				  (src.z - _Origin.z) * _WorldToLocal.z);
	sint x = (sint) (local.x * vertexGrid.getWidth());
	sint y = (sint) (local.y * vertexGrid.getHeight());
	if (x == (sint) vertexGrid.getWidth()) x = (sint) vertexGrid.getWidth() - 1;
	if (y == (sint) vertexGrid.getHeight()) y = (sint) vertexGrid.getHeight() - 1;
	//
	CPackedVertex pv;
	sint32 ix = (sint32) (local.x * PACKED_COL_RANGE);
	sint32 iy = (sint32) (local.y * PACKED_COL_RANGE);
	sint32 iz = (sint32) (local.z * PACKED_COL_RANGE);
	clamp(ix, 0, PACKED_COL_RANGE);
	clamp(iy, 0, PACKED_COL_RANGE);
	clamp(iz, 0, PACKED_COL_RANGE);
	pv.X = (uint16) ix;
	pv.Y = (uint16) iy;
	pv.Z = (uint16) iz;
	//
	std::list<uint32> &vertList = vertexGrid(x, y);
	for(std::list<uint32>::iterator it = vertexGrid(x, y).begin(); it != vertexGrid(x, y).end(); ++it)
	{
		if (Verts[*it] == pv) return *it; // exists already
	}


	// create a new vertex
	Verts.push_back(pv);
	vertList.push_front((uint32)Verts.size() - 1);
	return (uint32)Verts.size() - 1;
}

// ***************************************************************************************
void CPackedZone32::render(CVertexBuffer &vb, IDriver &drv, CMaterial &material, CMaterial &wiredMaterial, const CMatrix &camMat, uint batchSize, const CVector localFrustCorners[8])
{
	if (Tris.empty()) return;
	IDriver::TPolygonMode oldPolygonMode = drv.getPolygonMode();
	//
	CVector frustCorners[8];
	for(uint k = 0; k < sizeofarray(frustCorners); ++k)
	{
		frustCorners[k] = camMat * localFrustCorners[k];
		frustCorners[k].x -= _Origin.x;
		frustCorners[k].y -= _Origin.y;
	}
	// project frustum on x/y plane to see where to test polys
	sint minY = INT_MAX;
	static CPolygon2D::TRasterVect silhouette;
	silhouette.clear();
	addQuadToSilhouette(frustCorners[0], frustCorners[1], frustCorners[2], frustCorners[3], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[1], frustCorners[5], frustCorners[6], frustCorners[2], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[4], frustCorners[5], frustCorners[6], frustCorners[7], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[0], frustCorners[4], frustCorners[7], frustCorners[3], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[0], frustCorners[1], frustCorners[5], frustCorners[4], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[3], frustCorners[7], frustCorners[6], frustCorners[2], silhouette, minY, CellSize);
	//
	{
		CVertexBufferReadWrite vba;
		vb.setNumVertices(batchSize * 3);
		vb.lock(vba);
		CVector *dest = vba.getVertexCoordPointer(0);
		const CVector *endDest = dest + batchSize * 3;
		for(sint y = 0; y < (sint) silhouette.size(); ++y)
		{
			sint gridY = y + minY;
			if (gridY < 0) continue;
			if (gridY >= (sint) Grid.getHeight()) continue;
			sint minX = silhouette[y].first;
			sint maxX = silhouette[y].second;
			for (sint x = minX; x <= maxX; ++x)
			{
				if (x < 0) continue;
				if (x >= (sint) Grid.getWidth()) break;
				uint32 triRefIndex = Grid(x, gridY);
				if (triRefIndex == UndefIndex) continue;
				for (;;)
				{
					uint32 triIndex = TriLists[triRefIndex];
					if (triIndex == UndefIndex) break; // end of list
					unpackTri(Tris[triIndex], dest);
					dest += 3;
					if (dest == endDest)
					{
						// flush batch
						vba.unlock();
						drv.setPolygonMode(IDriver::Filled);
						drv.activeVertexBuffer(vb);
						drv.renderRawTriangles(material, 0, batchSize);
						drv.setPolygonMode(IDriver::Line);
						//drv.renderRawTriangles(wiredMaterial, 0, batchSize);
						// reclaim a new batch
						vb.setNumVertices(batchSize * 3);
						vb.lock(vba);
						dest = vba.getVertexCoordPointer(0);
						endDest = dest + batchSize * 3;
					}
					++ triRefIndex;
				}
			}
		}
		vba.unlock();
		uint numRemainingTris = batchSize - (uint)((endDest - dest) / 3);
		if (numRemainingTris)
		{
			drv.setPolygonMode(IDriver::Filled);
			drv.activeVertexBuffer(vb);
			drv.renderRawTriangles(material, 0, numRemainingTris);
			drv.setPolygonMode(IDriver::Line);
			//drv.renderRawTriangles(wiredMaterial, 0, numRemainingTris);
		}
	}
	drv.setPolygonMode(oldPolygonMode);
}

// ***************************************************************************************
void CPackedZone32::addInstance(const CShapeInfo &si, const NLMISC::CMatrix &matrix, TVertexGrid &vertexGrid, TTriListGrid &triListGrid)
{
	for(uint k = 0; k < si.Tris.size(); ++k)
	{
		CTriangle worldTri;
		si.Tris[k].applyMatrix(matrix, worldTri);
		addTri(worldTri, vertexGrid, triListGrid);
	}
}

// ***************************************************************************************
CSmartPtr<CPackedZone16> CPackedZone32::buildPackedZone16()
{
	if (Verts.size() > 65535) return NULL;
	if (Tris.size() > 65534) return NULL; // NB : not 65534 here because -1 is used to mark the end of a list
	if (TriLists.size() > 65534) return NULL;
	// can convert
	CSmartPtr<CPackedZone16> dest = new CPackedZone16;
	dest->Box = Box;
	dest->Verts = Verts;
	dest->Tris.resize(Tris.size());
	for(uint k = 0; k < Tris.size(); ++k)
	{
		dest->Tris[k].V0 = (uint16) Tris[k].V0;
		dest->Tris[k].V1 = (uint16) Tris[k].V1;
		dest->Tris[k].V2 = (uint16) Tris[k].V2;
	}
	dest->TriLists.resize(TriLists.size());
	for(uint k = 0; k < TriLists.size(); ++k)
	{
		dest->TriLists[k] = (uint16) TriLists[k];
	}
	dest->CellSize = CellSize;
	dest->Grid.init(Grid.getWidth(), Grid.getHeight());
	for (sint y = 0; y < (sint) Grid.getHeight(); ++y)
	{
		for (sint x = 0; x < (sint) Grid.getWidth(); ++x)
		{
			dest->Grid(x, y) = (uint16) Grid(x, y);
		}
	}
	dest->_Origin = _Origin;
	dest->_WorldToLocal = _WorldToLocal;
	dest->ZoneX = ZoneX;
	dest->ZoneY = ZoneY;

	dest->_PackedLocalToWorld.set(1.f / (dest->_WorldToLocal.x * (float) PACKED_COL_RANGE),
								  1.f / (dest->_WorldToLocal.y * (float) PACKED_COL_RANGE),
								  1.f / (dest->_WorldToLocal.z * (float) PACKED_COL_RANGE));

	return dest;
}

// ***************************************************************************************
void CPackedZone16::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);
	f.serial(Box);
	std::vector<uint16> datas;
	if (f.isReading())
	{
		// vertices
		serialPackedVector12(datas, f);
		Verts.resize(datas.size() / 3);
		for(uint k = 0; k < Verts.size(); ++k)
		{
			Verts[k].X	= datas[k];
			Verts[k].Y	= datas[Verts.size() + k];
			Verts[k].Z	= datas[2 * Verts.size() + k];
		}
		// triangles
		serialPackedVector16(datas, f);
		Tris.resize(datas.size() / 3);
		for(uint k = 0; k < Tris.size(); ++k)
		{
			Tris[k].V0	 = datas[3 * k];
			Tris[k].V1	 = datas[3 * k + 1];
			Tris[k].V2	 = datas[3 * k + 2];
		}
		// tris list
		serialPackedVector16(TriLists, f);
		// grid
		uint16 width = (uint16) Grid.getWidth();
		uint16 height = (uint16) Grid.getHeight();
		f.serial(width, height);
		Grid.init(width, height);
		serialPackedVector16(datas, f);
		if ((uint) datas.size() != (uint) width * (uint) height)
		{
			throw NLMISC::EInvalidDataStream();
		}
		std::copy(datas.begin(), datas.end(), Grid.begin());
	}
	else
	{
		// vertices
		datas.resize(Verts.size() * 3);
		for(uint k = 0; k < Verts.size(); ++k)
		{
			datas[k] = Verts[k].X;
			datas[Verts.size() + k ] = Verts[k].Y;
			datas[2 * Verts.size() + k] = Verts[k].Z;
		}
		//nlwarning("serial verts");
		serialPackedVector12(datas, f);
		// triangles
		datas.resize(Tris.size() * 3);
		for(uint k = 0; k < Tris.size(); ++k)
		{
			datas[3 * k] = Tris[k].V0;
			datas[3 * k + 1] = Tris[k].V1;
			datas[3 * k + 2] = Tris[k].V2;
		}
		//nlwarning("serial tris");
		serialPackedVector16(datas, f);
		// tris list
		//nlwarning("serial tri lists");
		serialPackedVector16(TriLists, f);
		// grid
		uint16 width = (uint16) Grid.getWidth();
		uint16 height = (uint16) Grid.getHeight();
		f.serial(width, height);
		datas.resize((uint) width * (uint) height);
		std::copy(Grid.begin(), Grid.end(), datas.begin());
		//nlwarning("serial grid");
		serialPackedVector16(datas, f);
	}
	f.serial(CellSize);
	f.serial(_Origin);
	f.serial(_WorldToLocal);
	f.serial(ZoneX);
	f.serial(ZoneY);

	if (f.isReading())
	{
		_PackedLocalToWorld.set(1.f / (_WorldToLocal.x * (float) PACKED_COL_RANGE),
								1.f / (_WorldToLocal.y * (float) PACKED_COL_RANGE),
								1.f / (_WorldToLocal.z * (float) PACKED_COL_RANGE));
	}
}

// ***************************************************************************************
void CPackedZone16::render(CVertexBuffer &vb, IDriver &drv, CMaterial &material, CMaterial &wiredMaterial, const CMatrix &camMat, uint batchSize, const CVector localFrustCorners[8])
{
	if (Tris.empty()) return;
	IDriver::TPolygonMode oldPolygonMode = drv.getPolygonMode();
	CVector frustCorners[8];
	for(uint k = 0; k < sizeofarray(frustCorners); ++k)
	{
		frustCorners[k] = camMat * localFrustCorners[k];
		frustCorners[k].x -= _Origin.x;
		frustCorners[k].y -= _Origin.y;
	}
	// project frustum on x/y plane to see where to test polys
	sint minY = INT_MAX;
	static CPolygon2D::TRasterVect silhouette;
	silhouette.clear();
	addQuadToSilhouette(frustCorners[0], frustCorners[1], frustCorners[2], frustCorners[3], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[1], frustCorners[5], frustCorners[6], frustCorners[2], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[4], frustCorners[5], frustCorners[6], frustCorners[7], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[0], frustCorners[4], frustCorners[7], frustCorners[3], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[0], frustCorners[1], frustCorners[5], frustCorners[4], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[3], frustCorners[7], frustCorners[6], frustCorners[2], silhouette, minY, CellSize);
	//
	{
		CVertexBufferReadWrite vba;
		vb.setNumVertices(batchSize * 3);
		vb.lock(vba);
		CVector *dest = vba.getVertexCoordPointer(0);
		const CVector *endDest = dest + batchSize * 3;
		for(sint y = 0; y < (sint) silhouette.size(); ++y)
		{
			sint gridY = y + minY;
			if (gridY < 0) continue;
			if (gridY >= (sint) Grid.getHeight()) continue;
			sint minX = silhouette[y].first;
			sint maxX = silhouette[y].second;
			for (sint x = minX; x <= maxX; ++x)
			{
				if (x < 0) continue;
				if (x >= (sint) Grid.getWidth()) break;
				uint32 triRefIndex = Grid(x, gridY);
				if (triRefIndex == UndefIndex) continue;
				for (;;)
				{
					uint16 triIndex = TriLists[triRefIndex];
					if (triIndex == UndefIndex) break; // end of list
					unpackTri(Tris[triIndex], dest);
					dest += 3;
					if (dest == endDest)
					{
						// flush batch
						vba.unlock();
						drv.setPolygonMode(IDriver::Filled);
						drv.activeVertexBuffer(vb);
						drv.renderRawTriangles(material, 0, batchSize);
						drv.setPolygonMode(IDriver::Line);
						//drv.renderRawTriangles(wiredMaterial, 0, batchSize);
						// reclaim a new batch
						vb.setNumVertices(batchSize * 3);
						vb.lock(vba);
						dest = vba.getVertexCoordPointer(0);
						endDest = dest + batchSize * 3;
					}
					++ triRefIndex;
				}
			}
		}
		vba.unlock();
		uint numRemainingTris = batchSize - (uint)((endDest - dest) / 3);
		if (numRemainingTris)
		{
			drv.setPolygonMode(IDriver::Filled);
			drv.activeVertexBuffer(vb);
			drv.renderRawTriangles(material, 0, numRemainingTris);
			drv.setPolygonMode(IDriver::Line);
			//drv.renderRawTriangles(wiredMaterial, 0, numRemainingTris);
		}
	}
	drv.setPolygonMode(oldPolygonMode);
}





// ***************************************************************************************
void CPackedZone16::unpackTri(const CPackedTri16 &src, CVector dest[3]) const
{
	// yes this is ugly code duplication of CPackedZone16::unpackTri but this code is temporary anyway...
	// TODO: add 'multiply-add' operator
	dest[0].set(Verts[src.V0].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V0].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V0].Z * _PackedLocalToWorld.z + _Origin.z);
	dest[1].set(Verts[src.V1].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V1].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V1].Z * _PackedLocalToWorld.z + _Origin.z);
	dest[2].set(Verts[src.V2].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V2].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V2].Z * _PackedLocalToWorld.z + _Origin.z);

}

// raytrace code, common to CPackedZone32 & CPackedZone16
template <class T> bool raytrace(T &packedZone, const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<CTriangle> *testedTriangles = NULL, NLMISC::CVector *normal = NULL)
{
	if (packedZone.Grid.empty()) return false;
	CVector2f start2f((start.x - packedZone.Box.getMin().x) / packedZone.CellSize, (start.y - packedZone.Box.getMin().y) / packedZone.CellSize);
	CVector2f dir2f((end.x - start.x) / packedZone.CellSize, (end.y - start.y) / packedZone.CellSize);
	sint x, y;
	CGridTraversal::startTraverse(start2f, x, y);
	do
	{
		if (x < 0) continue;
		if (x >= (sint) packedZone.Grid.getWidth()) continue;
		if (y < 0) continue;
		if (y >= (sint) packedZone.Grid.getHeight()) continue;
		typename T::TIndexType triListIndex = packedZone.Grid(x, y);
		if (triListIndex != T::UndefIndex)
		{
			CTriangle tri;
			CPlane triPlane;
			float bestInterDist = FLT_MAX;
			CVector bestNormal(0.f, 0.f, 0.f);
			CVector currInter;
			do
			{
				packedZone.unpackTri(packedZone.Tris[packedZone.TriLists[triListIndex]], &tri.V0);
				if (testedTriangles)
				{
					testedTriangles->push_back(tri);
				}
				triPlane.make(tri.V0, tri.V1, tri.V2);
				if (tri.intersect(start, end, currInter, triPlane))
				{
					float dist = (currInter - start).norm();
					if (dist < bestInterDist)
					{
						bestInterDist = dist;
						inter = currInter;
						bestNormal.set(triPlane.a, triPlane.b, triPlane.c);
					}
				}
				++ triListIndex;
			}
			while (packedZone.TriLists[triListIndex] != T::UndefIndex);
			if (bestInterDist != FLT_MAX)
			{
				if (normal)
				{
					*normal = bestNormal.normed();
				}
				return true;
			}
		}
	}
	while(CGridTraversal::traverse(start2f, dir2f, x, y));
	return false;
}

// ***************************************************************************************
bool CPackedZone32::raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<CTriangle> *testedTriangles /*= NULL*/, NLMISC::CVector *normal /*= NULL*/) const
{
	return NL3D::raytrace(*this, start, end, inter, testedTriangles, normal);
}

// ***************************************************************************************
bool CPackedZone16::raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<CTriangle> *testedTriangles /*= NULL*/, NLMISC::CVector *normal /*= NULL*/) const
{
	return NL3D::raytrace(*this, start, end, inter, testedTriangles, normal);
}

// ***************************************************************************************
void CPackedZone16::appendSelection(const NLMISC::CPolygon2D &poly, std::vector<NLMISC::CTriangle> &selectedTriangles) const
{
	// compute covered zones
	NLMISC::CPolygon2D localPoly = poly;
	for (uint k = 0; k < localPoly.Vertices.size(); ++k)
	{
		localPoly.Vertices[k].x = (localPoly.Vertices[k].x - Box.getMin().x) / CellSize;
		localPoly.Vertices[k].y = (localPoly.Vertices[k].y - Box.getMin().y) / CellSize;
	}
	NLMISC::CPolygon2D::TRasterVect borders;
	sint minY;
	localPoly.computeOuterBorders(borders, minY);
	CTriangle newTri;
	//
	std::vector<bool> done(Tris.size(), false); // avoid double insertion
	//
	for (sint y = minY; y < (sint) (minY + borders.size()); ++y)
	{
		if (y < 0 || y >= (sint) Grid.getHeight()) continue;
		for (sint x = borders[y - minY].first; x <= borders[y - minY].second; ++x)
		{
			if (x < 0 || x >= (sint) Grid.getWidth()) continue;
			{
				if (Grid(x, y) != UndefIndex)
				{
					uint16 currTriIndex = Grid(x, y);
					while (TriLists[currTriIndex] != UndefIndex)
					{
						if (!done[TriLists[currTriIndex]])
						{
							unpackTri(Tris[TriLists[currTriIndex]], &newTri.V0);
							selectedTriangles.push_back(newTri);
							done[TriLists[currTriIndex]] = true;
						}
						++ currTriIndex;
					}
				}
			}
		}
	}
}

// ***************************************************************************************
void CPackedZone32::appendSelection(const NLMISC::CPolygon2D &poly, std::vector<NLMISC::CTriangle> &selectedTriangles) const
{
	// TODO nico : factorize with CPackedZone16::appendSelection
	selectedTriangles.clear();
	// compute covered zones
	NLMISC::CPolygon2D localPoly = poly;
	for (uint k = 0; k < localPoly.Vertices.size(); ++k)
	{
		localPoly.Vertices[k].x = (localPoly.Vertices[k].x - Box.getMin().x) / CellSize;
		localPoly.Vertices[k].y = (localPoly.Vertices[k].y - Box.getMin().y) / CellSize;
	}
	NLMISC::CPolygon2D::TRasterVect borders;
	sint minY;
	localPoly.computeOuterBorders(borders, minY);
	CTriangle newTri;
	//
	std::vector<bool> done(Tris.size(), false); // avoid double insertion
	//
	for (sint y = minY; y < (sint) (minY + borders.size()); ++y)
	{
		if (y < 0 || y >= (sint) Grid.getHeight()) continue;
		for (sint x = borders[y - minY].first; x <= borders[y - minY].second; ++x)
		{
			if (x < 0 || x >= (sint) Grid.getWidth()) continue;
			{
				if (Grid(x, y) != UndefIndex)
				{
					uint32 currTriIndex = Grid(x, y);
					while (TriLists[currTriIndex] != UndefIndex)
					{
						if (!done[TriLists[currTriIndex]])
						{
							unpackTri(Tris[TriLists[currTriIndex]], &newTri.V0);
							selectedTriangles.push_back(newTri);
							done[TriLists[currTriIndex]] = true;
						}
						++ currTriIndex;
					}
				}
			}
		}
	}
}

} // NL3D
