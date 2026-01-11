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

#ifndef NL_CLOUD_H
#define NL_CLOUD_H

// ------------------------------------------------------------------------------------------------

#include "nel/3d/camera.h"
#include "nel/3d/material.h"
#include "nel/3d/texture.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/geom_ext.h"

namespace NL3D
{

// ------------------------------------------------------------------------------------------------
class CNoise3d;
class CCloudScape;
struct SCloudTexture3D;
struct SCloudTextureClamp;

// ------------------------------------------------------------------------------------------------
class CCloud
{
public:

	CCloud (CCloudScape *pCloudScape);

	~CCloud ();

	void init (uint32 nVoxelW, uint32 nVoxelH, uint32 nVoxelD, float rBaseFreq, uint32 nNbOctave);
	void generate (CNoise3d &noise);
	void light ();

	void reset (NL3D::CCamera *pViewer);

	void anim (double dt, double dt2);

	// Debug
	void disp ();
	void dispXYZ (NL3D::CMaterial *pMat = NULL);
	// Debug

	// Create the billboard (in the screen at pos (NbW*Width, 0)
	void genBill (NL3D::CCamera *pViewer, uint32 nBillSize=128);

	// Display billboard to the screen
	void dispBill (NL3D::CCamera *pViewer);

	// Accessors

	float getX() { return _Pos.x; }
	float getLastX() { return _LastX; }
	float getY() { return _Pos.y; }
	float getZ() { return _Pos.z; }

	float getSizeX() { return _Size.x; }
	float getSizeY() { return _Size.y; }
	float getSizeZ() { return _Size.z; }

	void setX(float x) { _Pos.x = x; _LastX = x; }
	void setY(float y) { _Pos.y = y; }
	void setZ(float z) { _Pos.z = z; }

	void setSizeX(float x) { _Size.x = x; }
	void setSizeY(float y) { _Size.y = y; }
	void setSizeZ(float z) { _Size.z = z; }

	void setTexClamp (SCloudTextureClamp &t) { _CloudTexClamp = &t; }
	void setTex3DTemp (SCloudTexture3D &t) { _CloudTexTmp = &t; }

	void setLooping () { _WaitState = 2; }

	uint32 getBillSize() { return _BillSize; }

	uint32 getMemSize ()
	{
		return _OldBillSize*_OldBillSize*4 + _BillSize*_BillSize*4;
	}

	//uint32 Trans, TransTotal;
	double Time, FuturTime;

	NLMISC::CRGBA	CloudDiffuse;
	NLMISC::CRGBA	CloudAmbient;
	uint8			CloudPower;
	uint8			CloudDistAtt;

	uint8			LastCloudPower; // Cloud power of the old bill board
private:

	void setMode2D ();

	// in ; viewer, center
	// out I,J,K, left,right,top,bottom,near,far
	void calcBill (const NLMISC::CVector &Viewer, const NLMISC::CVector &Center, const NLMISC::CVector &Size,
					NLMISC::CVector &I, NLMISC::CVector &J, NLMISC::CVector &K,
					float &Left, float &Right, float &Top, float &Bottom, float &Near, float &Far);

private:

	uint32 _Width, _Height, _Depth;
	uint32 _NbW, _NbH; // Number of slice in width and height (NbW*NbH = Depth

	NLMISC::CVector _Size;
	NLMISC::CVector _Pos;
	float _LastX;

	float	_BaseFreq; // 1 -> 1 voxel is 1 noise3d pixel 0.5 -> 2 voxels are 1 noise3d pixel (Lowest octave freq)
	uint32	_NbOctave;
	double	*_UStart, *_VStart, *_WStart; // 1st Lowest octave

	CCloudScape			*_CloudScape;
	SCloudTexture3D		*_CloudTexTmp;		// Temporary cloud texture 3D
	SCloudTextureClamp	*_CloudTexClamp;

	// BillBoard
public:
	// The billboard is a texture where the cloud is rendered
	uint32									_BillSize; // ( The texture is always sqare)
	uint8									*_MemBill;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	_TexBill;

	// The last texture
	uint32									_OldBillSize;
	uint8									*_MemOldBill;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	_TexOldBill;

	uint8 _WaitState;

	// Accel for calc bill

	NLMISC::CVector _BillViewer, _BillCenter, _BillOldCenter;

private:

	NL3D::IDriver *_Driver;
};

// ------------------------------------------------------------------------------------------------

} // namespace NL3D

#endif // NL_CLOUD_H

