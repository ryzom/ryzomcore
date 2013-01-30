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
#include "nel/3d/cloud_scape.h"
#include "nel/3d/driver.h"
#include "nel/3d/scissor.h"
#include "nel/3d/viewport.h"

// ------------------------------------------------------------------------------------------------
using namespace NLMISC;

namespace NL3D
{

// ------------------------------------------------------------------------------------------------
#define SQR(x) (x)*(x)

#define MAX_DIST	400.0f
#define MAX_CLOUDS	256
// QUEUE_SIZE must be at least 2*MAX_CLOUDS
#define QUEUE_SIZE	512
static const double MAX_CLOUDS_ANIM_DELTA_TIME = 0.075;				  // maximum delta time handled by cloud animation, delta t above that are clamped
static const double MIN_CLOUDS_ANIM_DELTA_TIME = 0.005;				  // minimum delta time handled by clouds animation
static const double MAX_TIME_FOR_CLOUD_ANIM = 0.02;					  // max number of second spent for cloud render before we check if too slow
static const double MAX_FRAME_PERCENT_FOR_CLOUD_RENDERING = 20 / 100; // at most 20 % of the frame for cloud render


// ------------------------------------------------------------------------------------------------
// SCloudTexture3D
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
SCloudTexture3D::SCloudTexture3D ()
{
	Mem = NULL;
	Mem2 = NULL;
	MemBuffer = NULL;
	ToLightRGB.initUnlit();
	ToLightRGB.setShader (CMaterial::Normal);
	ToLightRGB.setZFunc (CMaterial::always);
	ToLightRGB.setZWrite (false);
	ToLightRGB.texEnvOpRGB (0, CMaterial::Replace);
	ToLightRGB.texEnvArg0RGB (0, CMaterial::Diffuse, CMaterial::SrcColor);
	ToLightRGB.texEnvOpAlpha (0, CMaterial::Replace);
	ToLightRGB.texEnvArg0Alpha (0, CMaterial::Texture, CMaterial::SrcAlpha);
	ToLightRGB.setBlend (true);
	ToLightRGB.setBlendFunc (CMaterial::invsrcalpha, CMaterial::srcalpha);
	ToLightRGB.setColor (CRGBA(0,0,0,255));
	ToLightAlpha.initUnlit();
	ToLightAlpha.setShader (CMaterial::Normal);
	ToLightAlpha.setZFunc (CMaterial::always);
	ToLightAlpha.setZWrite (false);
	ToLightAlpha.texEnvOpRGB (0, CMaterial::Replace);
	ToLightAlpha.texEnvArg0RGB (0, CMaterial::Diffuse, CMaterial::SrcColor);
	ToLightAlpha.texEnvOpAlpha (0, CMaterial::Replace);
	ToLightAlpha.texEnvArg0Alpha (0, CMaterial::Texture, CMaterial::InvSrcAlpha);
	ToLightAlpha.setColor (CRGBA(0,0,0,255));

	ToBill.initUnlit();
	ToBill.setZFunc (CMaterial::always);
	ToBill.setZWrite (false);
	ToBill.setDoubleSided(true);

	ToBill.texEnvOpRGB (0, CMaterial::Add);
	ToBill.texEnvArg0RGB (0, CMaterial::Texture, CMaterial::SrcColor);
	ToBill.texEnvArg1RGB (0, CMaterial::Diffuse, CMaterial::SrcColor);
	ToBill.setColor (CRGBA(80,80,80,255));

	ToBill.texEnvOpAlpha (0, CMaterial::Replace);
	ToBill.texEnvArg0Alpha (0, CMaterial::Texture, CMaterial::SrcAlpha);

	ToBill.texEnvOpRGB (1, CMaterial::Modulate);
	ToBill.texEnvArg0RGB (1, CMaterial::Previous, CMaterial::SrcColor);
	ToBill.texEnvArg1RGB (1, CMaterial::Previous, CMaterial::SrcAlpha);
	ToBill.texEnvOpAlpha (1, CMaterial::Replace);
	ToBill.texEnvArg0Alpha (1, CMaterial::Previous, CMaterial::SrcAlpha);

	ToBill.setBlendFunc (CMaterial::one, CMaterial::invsrcalpha);
	ToBill.setBlend (true);

	MatCopy.initUnlit();
	MatCopy.setShader (CMaterial::Normal);
	MatCopy.setZFunc (CMaterial::always);
	MatCopy.setZWrite (false);
	MatCopy.texEnvOpRGB (0, CMaterial::Replace);
	MatCopy.texEnvArg0RGB (0, CMaterial::Texture, CMaterial::SrcColor);
	MatCopy.texEnvOpAlpha (0, CMaterial::Replace);
	MatCopy.texEnvArg0Alpha (0, CMaterial::Texture, CMaterial::SrcAlpha);
	MatCopy.setBlend (false);
	MatCopy.setColor (CRGBA(255,255,255,255));
}

// ------------------------------------------------------------------------------------------------
void SCloudTexture3D::init (uint32 nWidth, uint32 nHeight, uint32 nDepth)
{
	if (Mem != NULL)
		return;

	Width = raiseToNextPowerOf2 (nWidth);
	Height = raiseToNextPowerOf2 (nHeight);
	Depth = raiseToNextPowerOf2 (nDepth);
	uint32 vdpo2 = getPowerOf2(Depth);
	NbW = 1 << (vdpo2 / 2);
	if ((vdpo2 & 1) != 0)
		NbH = 2 << (vdpo2 / 2);
	else
		NbH = 1 << (vdpo2 / 2);

	Mem = new uint8[4*NbW*Width*NbH*Height];
	Mem2 = new uint8[4*NbW*Width*NbH*Height];
	MemBuffer = new uint8[4*Width*Height];
	Tex = new CTextureMem (Mem, 4*NbW*Width*NbH*Height, true, false, NbW*Width, NbH*Height, CBitmap::RGBA);
	Tex2 = new CTextureMem (Mem2, 4*NbW*Width*NbH*Height, true, false, NbW*Width, NbH*Height, CBitmap::RGBA);
	TexBuffer = new CTextureMem (MemBuffer, 4*Width*Height, true, false, Width, Height, CBitmap::RGBA);

	Tex->setWrapS (ITexture::Clamp);
	Tex->setWrapT (ITexture::Clamp);
	Tex->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
	Tex->setReleasable (false);
	Tex->setRenderTarget (true);
	Tex->generate ();
	Tex2->setWrapS (ITexture::Clamp);
	Tex2->setWrapT (ITexture::Clamp);
	Tex2->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
	Tex2->setReleasable (false);
	Tex2->setRenderTarget (true);
	Tex2->generate ();
	TexBuffer->setWrapS (ITexture::Clamp);
	TexBuffer->setWrapT (ITexture::Clamp);
	TexBuffer->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
	TexBuffer->setReleasable (false);
	TexBuffer->setRenderTarget (true);
	TexBuffer->generate ();

	ToLightRGB.setTexture (0, Tex);
	ToLightAlpha.setTexture (0, Tex);
	ToBill.setTexture(0, Tex2);
	ToBill.setTexture(1, Tex2);
	MatCopy.setTexture(0, TexBuffer);
}

// ------------------------------------------------------------------------------------------------
// SCloudTextureClamp
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
SCloudTextureClamp::SCloudTextureClamp ()
{
	Mem = NULL;
	ToClamp.initUnlit();
	ToClamp.setShader (CMaterial::Normal);
	ToClamp.texEnvOpAlpha (0, CMaterial::Add);
	ToClamp.texEnvArg0Alpha (0, CMaterial::Texture, CMaterial::SrcAlpha);
	ToClamp.texEnvArg1Alpha (0, CMaterial::Diffuse, CMaterial::SrcAlpha);
	ToClamp.setColor (CRGBA(255,255,255,255));
	ToClamp.setBlend (true);
	ToClamp.setBlendFunc (CMaterial::one, CMaterial::one);
	ToClamp.setZFunc (CMaterial::always);
	ToClamp.setZWrite (false);

}

// ------------------------------------------------------------------------------------------------
void SCloudTextureClamp::init (uint32 nWidth, uint32 nHeight, uint32 nDepth, const std::string &filename)
{
	if (Mem != NULL)
		return;

	Width = raiseToNextPowerOf2 (nWidth);
	Height = raiseToNextPowerOf2 (nHeight);
	Depth = raiseToNextPowerOf2 (nDepth);
	uint32 vdpo2 = getPowerOf2(Depth);
	NbW = 1 << (vdpo2 / 2);
	if ((vdpo2 & 1) != 0)
		NbH = 2 << (vdpo2 / 2);
	else
		NbH = 1 << (vdpo2 / 2);

	Mem = new uint8[NbW*Width*NbH*Height];
	uint32 i, j;

	if (filename.empty())
	{
		// No filename so init with default
		for (i = 0; i < NbW; ++i)
		{
			for (j = 0; j < NbH; ++j)
			{
				uint32 d = i+j*NbW;
				uint32 k, l;
				for (k = 0; k < Width; ++k)
				for (l = 0; l < Height; ++l)
				{
					float x = k+0.5f;
					float y = l+0.5f;
					float z = d+0.5f;
					float xc = Width/2.0f;
					float yc = Height/2.0f;
					float zc = Depth/2.0f;

					float r = (x-xc)*(x-xc)/(Width*Width/4.0f) + (y-yc)*(y-yc)/(Height*Height/4.0f)
							+ (z-zc)*(z-zc)/(Depth*Depth/4.0f);

					uint8 col = 255;
					if (r < 1.0f)
					{
						col = (uint8)((r)*223+32);
					}
					Mem[i*Width+k + (j*Height+l)*NbW*Width] = col;
				}
			}
		}
	}
	else
	{
		// Load file TODO !
	}

	Tex = new CTextureMem (Mem, NbW*Width*NbH*Height, true, false, NbW*Width, NbH*Height, CBitmap::Alpha);
	Tex->setWrapS (ITexture::Repeat);
	Tex->setWrapT (ITexture::Repeat);
	Tex->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);

	Tex->touch();
	Tex->setReleasable (false);
	Tex->generate();

	ToClamp.setTexture(0, Tex);

}


// ------------------------------------------------------------------------------------------------
// CCloudScape
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
CCloudScape::CCloudScape (NL3D::IDriver *pDriver) : _Noise3D (pDriver)
{
	_Driver = pDriver;
	// Misc purpose VB
	_VertexBuffer.setVertexFormat (CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag);
	_VertexBuffer.setNumVertices (4);
	_VertexBuffer.setName("CloudScape");

	// Material used for cleaning
	_MatClear.initUnlit();
	_MatClear.setDoubleSided (true);
	_MatClear.setZFunc (CMaterial::always);
	_MatClear.setZWrite (false);
	_MatClear.setBlend (false);

	_MatBill.initUnlit();
	_MatBill.setShader (CMaterial::Normal); // not needed

	_MatBill.texEnvOpRGB (0, CMaterial::Replace);
	_MatBill.texEnvArg0RGB (0, CMaterial::Texture, CMaterial::SrcColor);

	_MatBill.texEnvOpAlpha (0, CMaterial::Replace);
	_MatBill.texEnvArg0Alpha (0, CMaterial::Texture, CMaterial::SrcAlpha);

	_MatBill.texEnvOpRGB (1, CMaterial::InterpolateDiffuse);
	_MatBill.texEnvArg0RGB (1, CMaterial::Texture, CMaterial::SrcColor);
	_MatBill.texEnvArg1RGB (1, CMaterial::Previous, CMaterial::SrcColor);

	_MatBill.texEnvOpAlpha (1, CMaterial::InterpolateDiffuse);
	_MatBill.texEnvArg0Alpha (1, CMaterial::Texture, CMaterial::SrcAlpha);
	_MatBill.texEnvArg1Alpha (1, CMaterial::Previous, CMaterial::SrcAlpha);

	_MatBill.setBlend (true);
	_MatBill.setBlendFunc(CMaterial::one, CMaterial::invsrcalpha);
	_MatBill.setZFunc (CMaterial::always);
	_MatBill.setZWrite (false);
	_MatBill.setDoubleSided (true);
	_MatBill.setAlphaTest(true);
	_MatBill.setAlphaTestThreshold(2.0f/256.0f);

	_LODQualityThreshold = 160.0f;
	_IsIncomingCSS = false;
	_DebugQuad = false;
	_NbHalfCloudToUpdate = 1;
	_CurrentCloudInProcess = NULL;

	_LastAnimRenderTime = 0;
	_MaxDeltaTime = 0.1; // 100 ms
}

// ------------------------------------------------------------------------------------------------
CCloudScape::~CCloudScape ()
{
}

// ------------------------------------------------------------------------------------------------
void CCloudScape::init (SCloudScapeSetup *pCSS, NL3D::CCamera *pCamera)
{
	_ResetCounter = _Driver->getResetCounter();
	_ViewerCam = pCamera;

	_Noise3D.init();

	_AllClouds.resize (MAX_CLOUDS, CCloud(this));
	_CloudPower.resize (MAX_CLOUDS);
	_ShouldProcessCloud.resize (MAX_CLOUDS);

	// For the moment only one clamp texture (generated)
	Tex3DTemp.init (64, 32, 32);
	TexClamp.init (64, 32, 32,"");

	if (pCSS != NULL)
	{
		_CurrentCSS = *pCSS;
		_NewCSS = *pCSS;
		_OldCSS = *pCSS;
	}
	_IsIncomingCSS = false;
	_TimeNewCSS = 60.0*60.0;

	uint32 i;
	for (i = 0; i < MAX_CLOUDS; ++i)
	{
		float newX, newY, newZ, newSizeX, newSizeY, newSizeZ;

		CCloud &c = _AllClouds[i];

		c.setTex3DTemp (Tex3DTemp);
		c.setTexClamp (TexClamp);

		for(;;)
		{
			bool bRecalc = false;
			newX = MAX_DIST*(1.0f-2.0f*(((float)rand())/RAND_MAX));
			newY = MAX_DIST*(1.0f-2.0f*(((float)rand())/RAND_MAX));
			newZ = 85.0f+40.0f*(1.0f-2.0f*(((float)rand())/RAND_MAX));

			newSizeX = 60.0f+10.0f*(1.0f-2.0f*(((float)rand())/RAND_MAX));
			newSizeY = 30.0f+10.0f*(1.0f-2.0f*(((float)rand())/RAND_MAX));
			newSizeZ = 30.0f+10.0f*(1.0f-2.0f*(((float)rand())/RAND_MAX));
			float f = 0.7f+0.3f*((float)rand())/RAND_MAX;
			newSizeX *= 1.5f*f;
			newSizeY *= 1.5f*f;
			newSizeZ *= 1.5f*f;

			float d = sqrtf(SQR(newX)+SQR(newY));
			if (d > MAX_DIST) bRecalc = true;

			for (uint32 k = 0;k < i; ++k)
			{
				CCloud &c2 = _AllClouds[k];

				if ((fabs(newX-c2.getX()) < (newSizeX/2+c2.getSizeX()/2)) &&
					(fabs(newY-c2.getY()) < (newSizeY/2+c2.getSizeY()/2)) &&
					(fabs(newZ-c2.getZ()) < (newSizeZ/2+c2.getSizeZ()/2)))
					bRecalc = true;
			}
			if (!bRecalc) break;
		}

		c.init (64, 32, 32, 0.122f, 4);
		c.setX (newX-newSizeX/2);
		c.setY (newY-newSizeY/2);
		c.setZ (newZ-newSizeZ/2);

		c.setSizeX (newSizeX);
		c.setSizeY (newSizeY);
		c.setSizeZ (newSizeZ);

		c.Time = 0;
		c.FuturTime = _CurrentCSS.NbCloud * 2 * (0.04/_NbHalfCloudToUpdate);
		if (i < _CurrentCSS.NbCloud)
		{
			_CloudPower[i] = 255;
			_ShouldProcessCloud[i] = true;
		}
		else
		{
			_CloudPower[i] = 0;
			_ShouldProcessCloud[i] = false;
		}
	}

	_SortedClouds.resize (MAX_CLOUDS);

	_CloudSchedulerSize = _CurrentCSS.NbCloud;
	_CloudSchedulerLastAdded.resize (MAX_CLOUDS);
	_FrameCounter = 0;
	_CloudScheduler.clear();
	for (i = 0; i < MAX_CLOUDS; ++i)
		_CloudSchedulerLastAdded[i].ValidPos = false;

	if (_CurrentCSS.NbCloud == 0)
	{
		for (i = 0; i < QUEUE_SIZE; ++i)
		{
			SCloudSchedulerEntry cse;
			cse.CloudIndex = -1;
			cse.Frame = _FrameCounter;
			cse.Ambient = _CurrentCSS.Ambient;
			cse.Diffuse = _CurrentCSS.Diffuse;
			_CloudScheduler.insert(_CloudScheduler.end(), cse);
			++_FrameCounter;
		}
	}
	else
	{
		for (i = 0; i < QUEUE_SIZE; ++i)
		{
			sint32 nCloudNb;
			nCloudNb = i%_CurrentCSS.NbCloud;
			SCloudSchedulerEntry cse;
			cse.CloudIndex = nCloudNb;
			if (_CloudSchedulerLastAdded[nCloudNb].ValidPos == true)
			{
				SCloudSchedulerEntry &lastCSE = *_CloudSchedulerLastAdded[nCloudNb].Pos;
				sint32 delta = _FrameCounter - lastCSE.Frame;
				lastCSE.DeltaNextCalc = delta;
			}
			cse.Frame = _FrameCounter;
			cse.Ambient = _CurrentCSS.Ambient;
			cse.Diffuse = _CurrentCSS.Diffuse;
			cse.Power = _CloudPower[cse.CloudIndex];
			_CloudSchedulerLastAdded[cse.CloudIndex].Pos = _CloudScheduler.insert(_CloudScheduler.end(), cse);
			_CloudSchedulerLastAdded[cse.CloudIndex].ValidPos = true;
			//_CloudSchedulerLastAdded[cse.CloudIndex].Pos = _CloudScheduler.end()-1;
			++_FrameCounter;
		}
	}

	_GlobalTime = 0.0f;
	_DTRest = 0.0f;
	_Generate = true;
	_AverageFrameRate.init(16);
	for (i = 0; i < 16; ++i)
		_AverageFrameRate.addValue (40.0f/1000.0f);

	_ExtrapolatedPriorities.resize (MAX_CLOUDS);

	for (i = 0; i < QUEUE_SIZE; ++i)
		anim (41.0/1000.0, _ViewerCam);
}

// ------------------------------------------------------------------------------------------------
void CCloudScape::set (SCloudScapeSetup &css)
{
	_IncomingCSS = css;
	_IsIncomingCSS = true;
}

// ------------------------------------------------------------------------------------------------
void CCloudScape::anim (double dt, NL3D::CCamera *pCamera)
{
	double startDate = double(CTime::getLocalTime())/1000.0;
	sint32 i;

	// Disable fog
	bool fog = _Driver->fogEnabled();
	_Driver->enableFog (false);

	_ViewerCam = pCamera;

	// update the max delta time
	// If rendering was too slow and took too much time of the previous frame, we decrease the max delta time to give clouds less processing time
	// Otherwise a cycle occurs, and slow rendering propagate from frame to frame



	if (dt != 0 && _LastAnimRenderTime > MAX_TIME_FOR_CLOUD_ANIM && (_LastAnimRenderTime / dt) > MAX_FRAME_PERCENT_FOR_CLOUD_RENDERING)
	{
		// if cloud rendering take too much time of previous frame, allocate less time for clouds
		// NB : check is only done if clouds rendering is above a given thrheshold, because if only clouds are rendered, then they may take 100% of frame without
		// having slow rendering
		_MaxDeltaTime = std::max(MIN_CLOUDS_ANIM_DELTA_TIME, _MaxDeltaTime / 2);
	}
	else
	{
		// clouds didn't take too much time  -> allocate more time for them
		_MaxDeltaTime = std::min(MAX_CLOUDS_ANIM_DELTA_TIME , _MaxDeltaTime + 0.002);
	}



	// 10 fps -> 200 fps
	if (dt > _MaxDeltaTime) dt = _MaxDeltaTime;
	if (dt < MIN_CLOUDS_ANIM_DELTA_TIME) dt = MIN_CLOUDS_ANIM_DELTA_TIME;


	_DeltaTime = dt;
	_GlobalTime += _DeltaTime;
	_AverageFrameRate.addValue ((float)_DeltaTime);

	// Animate the CSS
	if (_TimeNewCSS > _NewCSS.TimeToChange)
	{
		_CurrentCSS = _NewCSS;
		_OldCSS = _NewCSS;
		if (_IsIncomingCSS)
		{
			_IsIncomingCSS = false;
			_NewCSS = _IncomingCSS;
			_TimeNewCSS = 0;
			if (_NewCSS.NbCloud > _OldCSS.NbCloud)
			for (i = 0; i < (sint32)(_NewCSS.NbCloud-_OldCSS.NbCloud); ++i)
			{
				CCloud &c = _AllClouds[_OldCSS.NbCloud+i];
				c.CloudPower = 0;
				_CloudPower[_OldCSS.NbCloud+i] = 0;
			}
		}
	}
	else
	{
		float inter = (float)(_TimeNewCSS / _NewCSS.TimeToChange);
		_CurrentCSS.WindSpeed = (_NewCSS.WindSpeed - _OldCSS.WindSpeed)*inter + _OldCSS.WindSpeed;
		_CurrentCSS.CloudSpeed = (_NewCSS.CloudSpeed - _OldCSS.CloudSpeed)*inter + _OldCSS.CloudSpeed;

		_CurrentCSS.Ambient.R = (uint8)((_NewCSS.Ambient.R - _OldCSS.Ambient.R)*inter + _OldCSS.Ambient.R);
		_CurrentCSS.Ambient.G = (uint8)((_NewCSS.Ambient.G - _OldCSS.Ambient.G)*inter + _OldCSS.Ambient.G);
		_CurrentCSS.Ambient.B = (uint8)((_NewCSS.Ambient.B - _OldCSS.Ambient.B)*inter + _OldCSS.Ambient.B);
		_CurrentCSS.Ambient.A = (uint8)((_NewCSS.Ambient.A - _OldCSS.Ambient.A)*inter + _OldCSS.Ambient.A);

		_CurrentCSS.Diffuse.R = (uint8)((_NewCSS.Diffuse.R - _OldCSS.Diffuse.R)*inter + _OldCSS.Diffuse.R);
		_CurrentCSS.Diffuse.G = (uint8)((_NewCSS.Diffuse.G - _OldCSS.Diffuse.G)*inter + _OldCSS.Diffuse.G);
		_CurrentCSS.Diffuse.B = (uint8)((_NewCSS.Diffuse.B - _OldCSS.Diffuse.B)*inter + _OldCSS.Diffuse.B);
		_CurrentCSS.Diffuse.A = (uint8)((_NewCSS.Diffuse.A - _OldCSS.Diffuse.A)*inter + _OldCSS.Diffuse.A);

		if (_NewCSS.NbCloud > _OldCSS.NbCloud)
		{
			// Add some clouds
			float slice = (_NewCSS.TimeToChange/4) / (_NewCSS.NbCloud-_OldCSS.NbCloud);
			sint32 diffCloud = _NewCSS.NbCloud-_OldCSS.NbCloud;

			_CurrentCSS.NbCloud = _OldCSS.NbCloud + (1+(uint32)(_TimeNewCSS/slice));
			if (_CurrentCSS.NbCloud > _NewCSS.NbCloud)
				_CurrentCSS.NbCloud = _NewCSS.NbCloud;

			for (i = 0; i < diffCloud; ++i)
			{
				_ShouldProcessCloud[_OldCSS.NbCloud+i] = true;
				if (_TimeNewCSS < i*slice)
					_CloudPower[_OldCSS.NbCloud+i] = 1;
				else if (_TimeNewCSS > (i*slice+3*_NewCSS.TimeToChange/4))
					_CloudPower[_OldCSS.NbCloud+i] = 255;
				else
					_CloudPower[_OldCSS.NbCloud+i] = (uint8)(255*(_TimeNewCSS-i*slice)/(3*_NewCSS.TimeToChange/4));
			}
		}
		else
		{
			// Remove some clouds
			sint32 diffCloud = _OldCSS.NbCloud-_NewCSS.NbCloud;
			if (diffCloud)
			{
				float slice = (_NewCSS.TimeToChange/4) / (float)diffCloud;
				_CurrentCSS.NbCloud = _OldCSS.NbCloud;

				for (i = 0; i < diffCloud; ++i)
				{
					if (_TimeNewCSS < i*slice)
						_CloudPower[_OldCSS.NbCloud-i-1] = 255;
					else if (_TimeNewCSS > (i*slice+3*_NewCSS.TimeToChange/4))
						_CloudPower[_OldCSS.NbCloud-i-1] = 0;
					else
						_CloudPower[_OldCSS.NbCloud-i-1] = (uint8)(255-255*(_TimeNewCSS-i*slice)/(3*_NewCSS.TimeToChange/4));
				}
			}
		}
	}

	// Make the right number of half cloud
	_DTRest += dt;

	while (_DTRest > (0.04/_NbHalfCloudToUpdate))
	{
		makeHalfCloud ();
		_DTRest -= 0.04/_NbHalfCloudToUpdate;

		for (i = 0; i < MAX_CLOUDS; ++i)
		{
			CCloud &c = _AllClouds[i];
			c.Time += 0.04/_NbHalfCloudToUpdate;
		}

		_TimeNewCSS += 0.04/_NbHalfCloudToUpdate;
	}

	// Backup fog
	_Driver->enableFog (fog);

	// Has the driver been reseted ?
	if (_ResetCounter != _Driver->getResetCounter())
	{
		/* Yes. Force the rebuild of all the clouds not setuped in VRAM */
		_ResetCounter = _Driver->getResetCounter();
		i = 0;
		while (i < MAX_CLOUDS)
		{
			while (_ShouldProcessCloud[i] &&
				(!_AllClouds[i]._TexBill->setupedIntoDriver() || !_AllClouds[i]._TexOldBill->setupedIntoDriver()))
			{
				// Force a cloudscape rebuild
				anim (41.0/1000.0, _ViewerCam);
			}
			i++;
		}
	}
	double endDate = double(CTime::getLocalTime())/1000.0;
	_LastAnimRenderTime = endDate - startDate;
}

// ------------------------------------------------------------------------------------------------
void CCloudScape::makeHalfCloud ()
{
	CVector Viewer = CVector(0,0,0); //_ViewerCam->getMatrix().getPos();

	if (_Generate)
	{
		// Find the next cloud in the list
		SCloudSchedulerEntry FrontCSE;

		FrontCSE = _CloudScheduler.front();

		// Is the cloud do not have another reference in the list add it now because it should be processed
		sint32 CloudIndexToAdd = -1;

		if ((FrontCSE.CloudIndex != -1) &&
			(_ShouldProcessCloud[FrontCSE.CloudIndex] == true) &&
			(	(_CloudSchedulerLastAdded[FrontCSE.CloudIndex].ValidPos == false) ||
				((_CloudSchedulerLastAdded[FrontCSE.CloudIndex].ValidPos == true) &&
				(_CloudSchedulerLastAdded[FrontCSE.CloudIndex].Pos == _CloudScheduler.begin()))
			))
		{
			// It should be added now !
			CloudIndexToAdd = FrontCSE.CloudIndex;
			FrontCSE.DeltaNextCalc = QUEUE_SIZE;
		}
		else if (_CurrentCSS.NbCloud != 0)
		{
			// Choose a Cloud Index To Add at the end of the list
			uint32 nPeriodeMax = _CurrentCSS.NbCloud+_CurrentCSS.NbCloud/10;
			sint32 Priority = -10000;
			uint32 i;

			float sumPrior = 0.0f;
			for (i = 0; i < MAX_CLOUDS; ++i)
			if (_ShouldProcessCloud[i])
			{
				CCloud &rC = _AllClouds[i];
				float ExtrapolatedTime = ((0.04f/_NbHalfCloudToUpdate) * QUEUE_SIZE * 2);
				float x = rC.getLastX () + ExtrapolatedTime * _CurrentCSS.WindSpeed;
				//float d = sqrtf(SQR(x+rC.getSizeX()/2-Viewer.x)+SQR(rC.getY()+rC.getSizeY()/2-Viewer.y)+
				//		SQR(rC.getZ()+rC.getSizeZ()/2-Viewer.z));
				float d = SQR(x+rC.getSizeX()/2-Viewer.x)+SQR(rC.getY()+rC.getSizeY()/2-Viewer.y)+
						SQR(rC.getZ()+rC.getSizeZ()/2-Viewer.z);
				float d05 = sqrtf(d);
				float d025 = sqrtf(d05);
				float d075 = d05*d025;

				_ExtrapolatedPriorities[i] = 1.0f / d075;
				sumPrior += _ExtrapolatedPriorities[i];
			}

			sint32 sumJeton = 0;
			for (i = 0; i < MAX_CLOUDS; ++i)
			if (_ShouldProcessCloud[i])
			{
				// Normalize priorities
				float factor = ((float)QUEUE_SIZE) / sumPrior;
				sint32 nbJeton = (sint32)(0.5f+(factor * _ExtrapolatedPriorities[i]));

				if (nbJeton < 1)
					nbJeton = 1;

				_ExtrapolatedPriorities[i] = (float)nbJeton;
				sumJeton += nbJeton;
			}

			if (sumJeton > QUEUE_SIZE)
			{
				do
				{
					for (i = 0; i < MAX_CLOUDS; ++i)
					if (_ShouldProcessCloud[i])
					{
						if (_ExtrapolatedPriorities[i] > 1)
						{
							_ExtrapolatedPriorities[i] -= 1;
							--sumJeton;
							if (sumJeton == QUEUE_SIZE) break;
						}
					}
				}
				while (sumJeton > QUEUE_SIZE);
			}

			for (i = 0; i < MAX_CLOUDS; ++i)
			if (_ShouldProcessCloud[i])
			{
				// Cloud Period
				sint32 newPriority = nPeriodeMax;
				// Is there a last entry in array ?
				if (_CloudSchedulerLastAdded[i].ValidPos == true)
				{
					SCloudSchedulerEntry &rLastCSE = *_CloudSchedulerLastAdded[i].Pos;
					newPriority = (sint32)(QUEUE_SIZE/_ExtrapolatedPriorities[i]);
					newPriority = (_FrameCounter -  rLastCSE.Frame) - newPriority;
				}
				else
				{
					newPriority = 10000;
				}
				if (newPriority > Priority)
				{
					Priority = newPriority;
					CloudIndexToAdd = i;
				}
			}
			nlassert (CloudIndexToAdd != -1);
		}

		// Ok now we have a good cloud index to add so make the new cloud entry
		SCloudSchedulerEntry newCSE;

		newCSE.CloudIndex = CloudIndexToAdd;
		newCSE.Frame = _FrameCounter;
		newCSE.Ambient = _CurrentCSS.Ambient;
		newCSE.Diffuse = _CurrentCSS.Diffuse;
		if (CloudIndexToAdd != -1)
		{
			newCSE.Power = _CloudPower[CloudIndexToAdd];

			// If the cloud where added previously to the list
			if (_CloudSchedulerLastAdded[CloudIndexToAdd].ValidPos == true)
			{
				// This means that the cloud were added from a long time ago
				SCloudSchedulerEntry &lastCSE = *_CloudSchedulerLastAdded[CloudIndexToAdd].Pos;
				sint32 delta = _FrameCounter - lastCSE.Frame;
				lastCSE.DeltaNextCalc = delta;

				// But the cloud can be removed (if so we have to not process it anymore)
				if (newCSE.Power == 0)
					_ShouldProcessCloud[CloudIndexToAdd] = false;
			}
			else
			{
				// No the cloud do not appear previously in the list... So its a new one
				_AllClouds[CloudIndexToAdd].reset (_ViewerCam);
			}

			// If the last cloud occurence of the cloud appear at beginning so no more occurence in list
			if (_CloudSchedulerLastAdded[FrontCSE.CloudIndex].Pos == _CloudScheduler.begin())
				_CloudSchedulerLastAdded[FrontCSE.CloudIndex].ValidPos = false;

			_CloudSchedulerLastAdded[CloudIndexToAdd].Pos = _CloudScheduler.insert(_CloudScheduler.end(), newCSE);
			_CloudSchedulerLastAdded[CloudIndexToAdd].ValidPos = true;
			//_CloudSchedulerLastAdded[CloudIndexToAdd].Pos = _CloudScheduler.end()-1;
		}
		else
		{
			_CloudScheduler.insert(_CloudScheduler.end(), newCSE);
		}
		_CloudScheduler.pop_front ();
		++_FrameCounter;
		// End of scheduling

		// Get the cloud to process (this must be the next occurence of front cloud)
		std::list<SCloudSchedulerEntry>::iterator it = _CloudScheduler.begin();
		while (it != _CloudScheduler.end())
		{
			SCloudSchedulerEntry &rCSE = *it;
			if (rCSE.CloudIndex == FrontCSE.CloudIndex)
				break;
			++it;
		}

		SCloudSchedulerEntry CSEToCalc;
		// The cloud is no more present in the list
		if (it == _CloudScheduler.end())
		{
			FrontCSE.DeltaNextCalc = 1;
			CSEToCalc = FrontCSE;
		}
		else
		{
			CSEToCalc = *it;
		}

		// Is the cloud to calc is a real cloud
		if (CSEToCalc.CloudIndex == -1)
		{
			_CurrentCloudInProcess = NULL;
		}
		else
		{
			_CurrentCloudInProcess = &_AllClouds[CSEToCalc.CloudIndex];
			CCloud &c = *_CurrentCloudInProcess;

			// To go from Front cloud to CSEToCalc cloud we should take the front DeltaNextCalc

			_CurrentCloudInProcessFuturTime = ((0.04/_NbHalfCloudToUpdate) * FrontCSE.DeltaNextCalc * 2);
			c.setX ((float)(c.getLastX() +  _CurrentCloudInProcessFuturTime * _CurrentCSS.WindSpeed));

			float d2D = sqrtf(SQR(c.getX()+c.getSizeX()/2-Viewer.x)+SQR(c.getY()+c.getSizeY()/2-Viewer.y));

			if (d2D > MAX_DIST)
				c.CloudDistAtt = 255;
			else if (d2D > (MAX_DIST-100.0f))
				c.CloudDistAtt = (uint8)(255*((d2D-(MAX_DIST-100.0f))/100.0f));
			else
				c.CloudDistAtt = 0;

			c.LastCloudPower = c.CloudPower;
			c.CloudPower = CSEToCalc.Power;
			c.CloudDiffuse = CSEToCalc.Diffuse;
			c.CloudAmbient = CSEToCalc.Ambient;

			c.anim (_CurrentCloudInProcessFuturTime*_CurrentCSS.CloudSpeed,
					_CurrentCloudInProcessFuturTime*_CurrentCSS.WindSpeed);

			c.generate (_Noise3D);
		}
	}
	else
	{
		if (_CurrentCloudInProcess != NULL)
		{
			CCloud &c = *_CurrentCloudInProcess;

			c.Time = 0;
			c.FuturTime = _CurrentCloudInProcessFuturTime;
			c.light();

			if (c.getX() > MAX_DIST)
			{
				c.setX (c.getX() - (2 * MAX_DIST));
				c.setLooping ();
			}

			float r = sqrtf(SQR(c.getSizeX()/2)+SQR(c.getSizeY()/2)+SQR(c.getSizeZ()/2));
			float d = sqrtf(SQR(c.getX()+c.getSizeX()/2-Viewer.x)+SQR(c.getY()+c.getSizeY()/2-Viewer.y)+
							SQR(c.getZ()+c.getSizeZ()/2-Viewer.z));
			uint32 lookAtSize = (uint32)(_LODQualityThreshold*r/d);
			lookAtSize = raiseToNextPowerOf2 (lookAtSize);
			if (lookAtSize > 128) lookAtSize = 128;

			c.genBill (_ViewerCam, lookAtSize);
		}
	}
	_Generate = !_Generate;
}

// ------------------------------------------------------------------------------------------------
void CCloudScape::render ()
{
	uint32 i, j;

	CVector Viewer = CVector (0,0,0);

	// Disable fog
	bool fog = _Driver->fogEnabled();
	_Driver->enableFog (false);

	CMatrix viewMat;
	viewMat = _ViewerCam->getMatrix ();
	viewMat.setPos(CVector(0,0,0));
	viewMat.invert ();
	CScissor s;
	s.initFullScreen ();
	_Driver->setupScissor (s);
	CViewport v;
	_Driver->setupViewport (v);
	CFrustum f = _ViewerCam->getFrustum();
	_Driver->setFrustum (f.Left, f.Right, f.Bottom, f.Top, f.Near, f.Far, f.Perspective);
	_Driver->setupViewMatrix (viewMat);
	_Driver->setupModelMatrix (CMatrix::Identity);

	uint32 nNbCloudToRender = 0;

	for (i = 0; i < MAX_CLOUDS; ++i)
	{
		CCloud &c = _AllClouds[i];
		SSortedCloudEntry &sce = _SortedClouds[nNbCloudToRender];
		sce.Cloud = &c;
		sce.Distance = sqrtf(SQR(c.getX()+c.getSizeX()/2-Viewer.x)+SQR(c.getY()+c.getSizeY()/2-Viewer.y)+
						SQR(c.getZ()+c.getSizeZ()/2-Viewer.z));
		nNbCloudToRender++;
	}

	for (i = 0; i < nNbCloudToRender-1; ++i)
	for (j = i+1; j < nNbCloudToRender; ++j)
	{
		if (_SortedClouds[i].Distance < _SortedClouds[j].Distance)
		{
			SSortedCloudEntry sceTmp = _SortedClouds[i];
			_SortedClouds[i] = _SortedClouds[j];
			_SortedClouds[j] = sceTmp;
		}
	}

	for (i = 0; i < nNbCloudToRender; ++i)
	{
		CCloud *pC = _SortedClouds[i].Cloud;
		if ((pC->CloudPower > 0) || (pC->LastCloudPower > 0))
			pC->dispBill (_ViewerCam);
	}

	// Backup fog
	_Driver->enableFog (fog);
}

// ------------------------------------------------------------------------------------------------
uint32 CCloudScape::getMemSize()
{
	uint32 nMemSize = 0;
	for (uint32 i = 0; i < MAX_CLOUDS; ++i)
	{
		CCloud &c = _AllClouds[i];
		nMemSize += c.getMemSize();
	}
	return nMemSize;
}

} // namespace NL3D

