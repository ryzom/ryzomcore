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

#ifndef NL_CLOUD_SCAPE_H
#define NL_CLOUD_SCAPE_H

// ------------------------------------------------------------------------------------------------

#include "nel/3d/material.h"
#include "nel/3d/texture.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/value_smoother.h"
#include "nel/3d/cloud.h"
#include "nel/3d/noise_3d.h"
#include "nel/3d/u_cloud_scape.h"

namespace NL3D
{

// ------------------------------------------------------------------------------------------------
struct SCloudTexture3D
{
	uint32 Width, Height, Depth;
	uint32 NbW, NbH; // Number of slice in width and height (NbW*NbH = Depth)
	uint8									*Mem;
	uint8									*Mem2;
	uint8									*MemBuffer;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	Tex;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	Tex2;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	TexBuffer;
	NL3D::CMaterial							ToLightRGB;
	NL3D::CMaterial							ToLightAlpha;
	NL3D::CMaterial							ToBill;
	NL3D::CMaterial							MatCopy;

	SCloudTexture3D ();
	void init (uint32 nWidth, uint32 nHeight, uint32 nDepth);
};

// ------------------------------------------------------------------------------------------------
struct SCloudTextureClamp
{
	uint32 Width, Height, Depth;
	uint32 NbW, NbH; // Number of slice in width and height (NbW*NbH = Depth)
	uint8									*Mem;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	Tex;
	NL3D::CMaterial							ToClamp;

	SCloudTextureClamp ();
	void init (uint32 nWidth, uint32 nHeight, uint32 nDepth, const std::string &filename);
};

// ------------------------------------------------------------------------------------------------
class CCloudScape
{
public:

	CCloudScape (NL3D::IDriver *pDriver);

	~CCloudScape ();

	void init (SCloudScapeSetup *pCSS = NULL, NL3D::CCamera *pCamera = NULL);

	void set (SCloudScapeSetup &css);

	// Function that make cloud scape (work with screeen as temp buffer)
	void anim (double dt, NL3D::CCamera *pCamera);

	// Render all clouds to the screen
	void render ();

	uint32 getMemSize();

	void setQuality (float threshold) { _LODQualityThreshold = threshold; }

	void setNbCloudToUpdateIn80ms (uint32 n) { _NbHalfCloudToUpdate = n; }

	bool isDebugQuadEnabled () { return _DebugQuad; }
	void setDebugQuad (bool b) { _DebugQuad = b; }

private:

	void makeHalfCloud ();

private:

	uint32 _NbHalfCloudToUpdate; // In 40 ms
	double _GlobalTime;
	double _DeltaTime;
	double _DTRest;
	NLMISC::CValueSmoother _AverageFrameRate;
	bool _Generate; // or light ?
	CCloud *_CurrentCloudInProcess; // Current cloud that is processing
	double _CurrentCloudInProcessFuturTime;

	std::vector<uint8> _CloudPower;
	std::vector<bool> _ShouldProcessCloud;

	SCloudScapeSetup _CurrentCSS;
	SCloudScapeSetup _NewCSS;
	SCloudScapeSetup _OldCSS;
	double _TimeNewCSS;

	bool				_IsIncomingCSS;
	SCloudScapeSetup	_IncomingCSS;

	CNoise3d	_Noise3D;

	std::vector<CCloud>		_AllClouds;

	// Cloud scheduler
	struct SCloudSchedulerEntry
	{
		sint32 CloudIndex;
		sint32 DeltaNextCalc;
		uint32 Frame;
		NLMISC::CRGBA Ambient;
		NLMISC::CRGBA Diffuse;
		uint8 Power;
		SCloudSchedulerEntry()
		{
			CloudIndex = DeltaNextCalc = -1;
		}
	};
	struct SCloudSchedulerAccel
	{
		bool	ValidPos;
		std::list<SCloudSchedulerEntry>::iterator Pos;
//		uint32	Frame;

		SCloudSchedulerAccel()
		{
			ValidPos = false;
		}
	};
//	std::deque<SCloudSchedulerEntry>	_CloudScheduler;
	std::list<SCloudSchedulerEntry>	_CloudScheduler;
	uint32 _CloudSchedulerSize;
	std::vector<SCloudSchedulerAccel>	_CloudSchedulerLastAdded;
	uint32 _FrameCounter;
	std::vector<float> _ExtrapolatedPriorities;


	// Cloud sort
	struct SSortedCloudEntry
	{
		CCloud *Cloud;
		float Distance;
	};
	std::vector<SSortedCloudEntry> _SortedClouds;



	float _LODQualityThreshold;
	bool _DebugQuad;

	NL3D::IDriver			*_Driver;
	NL3D::CVertexBuffer		_VertexBuffer;
	NL3D::CMaterial			_MatClear;
	NL3D::CMaterial			_MatBill;

	NL3D::CCamera			*_ViewerCam;

	SCloudTexture3D			Tex3DTemp;
	SCloudTextureClamp		TexClamp;

	// Driver reset counter initial value
	uint					_ResetCounter;

	friend class CCloud;

	double					_LastAnimRenderTime;
	double					_MaxDeltaTime;
};

// ------------------------------------------------------------------------------------------------
} // namespace NL3D

#endif // NL_CLOUD_SCAPE_H

