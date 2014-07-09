// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "nel/gui/view_renderer.h"

#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/uv.h"
#include "nel/misc/hierarchical_timer.h"

using namespace NLMISC;
using namespace std;
using namespace NL3D;

namespace NLGUI
{

	CViewRenderer* CViewRenderer::instance            = NULL;
	NL3D::UDriver* CViewRenderer::driver              = NULL;
	NL3D::UTextContext* CViewRenderer::textcontext    = NULL;
	std::set< std::string >* CViewRenderer::hwCursors = NULL;
	float CViewRenderer::hwCursorScale                = 1.0f;

	CViewRenderer::CViewRenderer()
	{
		nlassert( CViewRenderer::driver != NULL );
		nlassert( CViewRenderer::textcontext != NULL );
		nlassert( CViewRenderer::hwCursors != NULL );
		setup();
	}

	CViewRenderer::~CViewRenderer()
	{
		for(uint i=0;i<VR_NUM_LAYER;i++)
		{
			delete _StringRBLayers[i];
			_StringRBLayers[i]= NULL;
			_EmptyLayer[i]= true;
		}
	}


	CViewRenderer* CViewRenderer::getInstance()
	{
		if( instance == NULL )
			instance = new CViewRenderer;
		return instance;
	}

	/*
	 * setClipWindow : set the current clipping window
	 * (this window do not inherit properties from parent or whatever)
	 */
	void CViewRenderer::setClipWindow (sint32 x, sint32 y, sint32 w, sint32 h)
	{
		_ClipX = x;
		_ClipY = y;
		_ClipW = w;
		_ClipH = h;

		_XMin = (float)_ClipX * _OneOverScreenW;
		_XMax = (float)(_ClipX+_ClipW) * _OneOverScreenW;
		_YMin = (float)_ClipY * _OneOverScreenH;
		_YMax = (float)(_ClipY+_ClipH) * _OneOverScreenH;
	}


	/*
	 * checkNewScreenSize
	 */
	void CViewRenderer::checkNewScreenSize ()
	{
		if (!driver) return;
		uint32 w, h;
		driver->getWindowSize (w, h);
		// not minimized? change coords
		if(w!=0 && h!=0)
		{
			_IsMinimized= false;
			_ScreenW = w;
			_ScreenH = h;
			if(_ScreenW>0)
				_OneOverScreenW = 1.0f / (float)_ScreenW;
			else
				_OneOverScreenW = 1000;
			if(_ScreenH>0)
				_OneOverScreenH = 1.0f / (float)_ScreenH;
			else
				_OneOverScreenH = 1000;
		}
		else
		{
			// Keep old coordinates (suppose resolution won't change, even if typically false wen we swithch from outgame to ingame)
			_IsMinimized= true;
		}
	}


	/*
	 * getScreenSize : get the screen window size
	 */
	void CViewRenderer::getScreenSize (uint32 &w, uint32 &h)
	{
		w = _ScreenW;
		h = _ScreenH;
	}

	/*
	 * get OOW / OOH
	 */
	void CViewRenderer::getScreenOOSize (float &oow, float &ooh)
	{
		oow= _OneOverScreenW;
		ooh= _OneOverScreenH;
	}

	void CViewRenderer::setup()
	{
		_ClipX = _ClipY = 0;
		_ClipW = 800;
		_ClipH = 600;
		_ScreenW = 800;
		_ScreenH = 600;
		_OneOverScreenW= 1.0f / (float)_ScreenW;
		_OneOverScreenH= 1.0f / (float)_ScreenH;
		_IsMinimized= false;
		_WFigurTexture= 0;
		_HFigurTexture= 0;
		_WFigurSeparatorTexture = 0;
		_FigurSeparatorTextureId = -1;
		_FigurBlankId = -1;
		_BlankId = -1;
		_WorldSpaceTransformation = true;
		_CurrentZ = 10;
		for(uint i=0;i<VR_NUM_LAYER;i++)
		{
			_StringRBLayers[i]= NULL;
			_EmptyLayer[i]= true;
		}
		_BlankGlobalTexture  = NULL;
	}


	/*
	 * init: init material and string buffer
	 */
	void CViewRenderer::init()
	{
		if (!driver) return;
		_Material = driver->createMaterial();

		setRenderStates();

		// Init all renderBuffer
		for(uint i=0;i<VR_NUM_LAYER;i++)
		{
			_StringRBLayers[i]= textcontext->createRenderBuffer();
		}
	}

	void CViewRenderer::setRenderStates()
	{
		_Material.setDoubleSided();
		_Material.setZWrite(false);
		_Material.setZFunc(UMaterial::always);
		_Material.setBlend (true);
		_Material.setBlendFunc (NL3D::UMaterial::srcalpha, NL3D::UMaterial::invsrcalpha);
		_Material.setColor(CRGBA::White);
		_Material.setTexture(0, NULL);
		_Material.setTexture(1, NULL);
		_Material.setTexture(2, NULL);
		_Material.setTexture(3, NULL);
		_Material.setZBias(0);
	}

	void CViewRenderer::release()
	{
		if( instance != NULL )
		{
			instance->reset();
			delete instance;
			instance = NULL;
		}
	}


	/*
	 *	reset: reset the whole view renderer
	 */
	void CViewRenderer::reset()
	{
		TGlobalTextureList::iterator ite = _GlobalTextures.begin();
		while (ite != _GlobalTextures.end())
		{
			UTextureFile *tf = dynamic_cast<NL3D::UTextureFile *>(ite->Texture);
			if (tf)
			{
				driver->deleteTextureFile (tf);
			}
			ite++;
		}

		_GlobalTextures.clear();
		_SImages.clear();
		_SImageIterators.clear();
		_TextureMap.clear();
		_IndexesToTextureIds.clear();
	}

	NL3D::UDriver* CViewRenderer::getDriver(){
		return driver;
	}

	void CViewRenderer::setTextContext(NL3D::UTextContext *textcontext)
	{
		CViewRenderer::textcontext = textcontext;
	}

	void CViewRenderer::setDriver( NL3D::UDriver *driver )
	{
		CViewRenderer::driver = driver;
	}

	// ***************************************************************************
	void CViewRenderer::SImage::setupQuadUV(bool flipv, uint8 rot, CQuadColorUV &dest)
	{
		nlassert(rot<=3);
		// Rotation is CW and flip is along x axis
		// Flip is vertical flip (this means we invert all y for a constant x)
		// The transforms are done in this order : first apply the flip (or not) and then rotate
		static const CUV UVTab[8][4] = {
			{ CUV(0, 0), CUV(1, 0), CUV(1, 1), CUV(0, 1) },	// rot 0, no flip
			{ CUV(1, 0), CUV(1, 1), CUV(0, 1), CUV(0, 0) }, // rot 1, no flip
			{ CUV(1, 1), CUV(0, 1), CUV(0, 0), CUV(1, 0) }, // rot 2, no flip
			{ CUV(0, 1), CUV(0, 0), CUV(1, 0), CUV(1, 1) }, // rot 3, no flip
			{ CUV(1, 0), CUV(0, 0), CUV(0, 1), CUV(1, 1) },	// rot 0, flipped
			{ CUV(0, 0), CUV(0, 1), CUV(1, 1), CUV(1, 0) }, // rot 1, flipped
			{ CUV(0, 1), CUV(1, 1), CUV(1, 0), CUV(0, 0) }, // rot 2, flipped
			{ CUV(1, 1), CUV(1, 0), CUV(0, 0), CUV(0, 1) }	// rot 3, flipped
		};

		// Take care that the origin in the texture is top left so to get the texture in bottom-up
		// we have to start at Max and go at Min. For left and right this is Min to Max.

		float du = UVMax.U - UVMin.U;
		float dv = UVMin.V - UVMax.V;

		uint idx = flipv*4 + rot;

		dest.Uv0 = CUV (UVMin.U + UVTab[idx][0].U * du, UVMax.V + UVTab[idx][0].V * dv);
		dest.Uv1 = CUV (UVMin.U + UVTab[idx][1].U * du, UVMax.V + UVTab[idx][1].V * dv);
		dest.Uv2 = CUV (UVMin.U + UVTab[idx][2].U * du, UVMax.V + UVTab[idx][2].V * dv);
		dest.Uv3 = CUV (UVMin.U + UVTab[idx][3].U * du, UVMax.V + UVTab[idx][3].V * dv);

		/* // TRAP : Unrolled Version (To be tested to know if it is faster than the previous one)
			if (flipv)
			{
				switch (rot)
				{
					case 0:
						qcoluv.Uv0.U = rI.UVMax.U;	qcoluv.Uv0.V = rI.UVMax.V;
						qcoluv.Uv1.U = rI.UVMin.U;	qcoluv.Uv1.V = rI.UVMax.V;
						qcoluv.Uv2.U = rI.UVMin.U;	qcoluv.Uv2.V = rI.UVMin.V;
						qcoluv.Uv3.U = rI.UVMax.U;	qcoluv.Uv3.V = rI.UVMin.V;
					break;
					case 1:
						qcoluv.Uv0.U = rI.UVMin.U;	qcoluv.Uv0.V = rI.UVMax.V;
						qcoluv.Uv1.U = rI.UVMin.U;	qcoluv.Uv1.V = rI.UVMin.V;
						qcoluv.Uv2.U = rI.UVMax.U;	qcoluv.Uv2.V = rI.UVMin.V;
						qcoluv.Uv3.U = rI.UVMax.U;	qcoluv.Uv3.V = rI.UVMax.V;
					break;
					case 2:
						qcoluv.Uv0.U = rI.UVMin.U;	qcoluv.Uv0.V = rI.UVMin.V;
						qcoluv.Uv1.U = rI.UVMax.U;	qcoluv.Uv1.V = rI.UVMin.V;
						qcoluv.Uv2.U = rI.UVMax.U;	qcoluv.Uv2.V = rI.UVMax.V;
						qcoluv.Uv3.U = rI.UVMin.U;	qcoluv.Uv3.V = rI.UVMax.V;
					break;
					case 3:
						qcoluv.Uv0.U = rI.UVMax.U;	qcoluv.Uv0.V = rI.UVMin.V;
						qcoluv.Uv1.U = rI.UVMax.U;	qcoluv.Uv1.V = rI.UVMax.V;
						qcoluv.Uv2.U = rI.UVMin.U;	qcoluv.Uv2.V = rI.UVMax.V;
						qcoluv.Uv3.U = rI.UVMin.U;	qcoluv.Uv3.V = rI.UVMin.V;
					break;
				}
			}
			else
			{
				switch (rot)
				{
					case 0:
						qcoluv.Uv0.U = rI.UVMin.U;	qcoluv.Uv0.V = rI.UVMax.V;
						qcoluv.Uv1.U = rI.UVMax.U;	qcoluv.Uv1.V = rI.UVMax.V;
						qcoluv.Uv2.U = rI.UVMax.U;	qcoluv.Uv2.V = rI.UVMin.V;
						qcoluv.Uv3.U = rI.UVMin.U;	qcoluv.Uv3.V = rI.UVMin.V;
					break;
					case 1:
						qcoluv.Uv0.U = rI.UVMin.U;	qcoluv.Uv0.V = rI.UVMin.V;
						qcoluv.Uv1.U = rI.UVMin.U;	qcoluv.Uv1.V = rI.UVMax.V;
						qcoluv.Uv2.U = rI.UVMax.U;	qcoluv.Uv2.V = rI.UVMax.V;
						qcoluv.Uv3.U = rI.UVMax.U;	qcoluv.Uv3.V = rI.UVMin.V;
					break;
					case 2:
						qcoluv.Uv0.U = rI.UVMax.U;	qcoluv.Uv0.V = rI.UVMin.V;
						qcoluv.Uv1.U = rI.UVMin.U;	qcoluv.Uv1.V = rI.UVMin.V;
						qcoluv.Uv2.U = rI.UVMin.U;	qcoluv.Uv2.V = rI.UVMax.V;
						qcoluv.Uv3.U = rI.UVMax.U;	qcoluv.Uv3.V = rI.UVMax.V;
					break;
					case 3:
						qcoluv.Uv0.U = rI.UVMax.U;	qcoluv.Uv0.V = rI.UVMax.V;
						qcoluv.Uv1.U = rI.UVMax.U;	qcoluv.Uv1.V = rI.UVMin.V;
						qcoluv.Uv2.U = rI.UVMin.U;	qcoluv.Uv2.V = rI.UVMin.V;
						qcoluv.Uv3.U = rI.UVMin.U;	qcoluv.Uv3.V = rI.UVMax.V;
					break;
				}
			}
		 */
	}
	// ***************************************************************************

	/*
	 * drawRotFlipBitmapTiled
	 */
	void CViewRenderer::drawRotFlipBitmapTiled (sint layerId, sint32 x, sint32 y, sint32 width, sint32 height, uint8 rot, bool flip,
												sint32 nTxId, uint tileOrigin, const CRGBA &col)
	{
		static volatile bool draw = true;
		if (!draw) return;
		if (width <= 0 || height <= 0) return;

		if (nTxId < 0) return;

		// Is totally clipped ?
		if ((x > (_ClipX+_ClipW)) || ((x+width) < _ClipX) ||
			(y > (_ClipY+_ClipH)) || ((y+height) < _ClipY))
			return;

		SImage &rI = *getSImage(nTxId);

		sint32 txw, txh;
		// start to draw at the reference corner
		getTextureSizeFromId (nTxId, txw, txh);

		// to avoid a division by zero crash later
		if (txw < 0 || txh < 0) return;

		if (rot > 3) rot = 3;

		sint32 startX = x, startY = y;
		sint32 stepX = txw, stepY = txh;

		if (rot & 1)
		{
			std::swap(txw, txh);
		}

		// choose new start pos & uvs depending on the reference corner
		// Along x axis
		if (tileOrigin & 1) // right or left ?
		{  // right
			startX  = x + width - txw;
			stepX   = -txw;
		}

		// Along y axis
		if (tileOrigin & 2) // bottom or top ?
		{  // top
			startY  = y + height - txh;
			stepY   = -txh;
		}

		// Fit screen coordinates

		float fStartX = (float) startX * _OneOverScreenW;
		float fStartY = (float) startY * _OneOverScreenH;
		float fStepX = (float) stepX * _OneOverScreenW;
		float fStepY = (float) stepY * _OneOverScreenH;
		float fTxW = (float) txw * _OneOverScreenW;
		float fTxH = (float) txh * _OneOverScreenH;

		CQuadColorUV qcoluv;

		qcoluv.Color0 = qcoluv.Color1 = qcoluv.Color2 = qcoluv.Color3 = col;
		qcoluv.V0.z = qcoluv.V1.z = qcoluv.V2.z = qcoluv.V3.z = 0;
		rI.setupQuadUV(flip,rot,qcoluv);

		uint numTileX = (uint32)((width - 1) / txw);
		uint numTileY = (uint32)((height- 1) / txh);

		float currY = fStartY;

		sint32 oldClipX = _ClipX;
		sint32 oldClipY = _ClipY;
		sint32 oldClipW = _ClipW;
		sint32 oldClipH = _ClipH;
		if (x < _ClipX) { width -= _ClipX - x; x = _ClipX; }
		if (y < _ClipY) { height -= _ClipY - y; y = _ClipY; }
		if ((x+width) > (_ClipX+_ClipW)) width -= (x+width) - (_ClipX+_ClipW);
		if ((y+height) > (_ClipY+_ClipH)) height -= (y+height) - (_ClipY+_ClipH);
		setClipWindow (x, y, width, height);

		// draw result let the clipper clip the quads
		for(uint py = 0; py <= numTileY; ++py)
		{
			float currX = fStartX;
			for(uint px = 0; px <= numTileX; ++px)
			{
				/// There is room for speedup there
				qcoluv.V0.x = currX;
				qcoluv.V1.x = currX + fTxW;
				qcoluv.V2.x = currX + fTxW;
				qcoluv.V3.x = currX;

				qcoluv.V0.y = currY;
				qcoluv.V1.y = currY;
				qcoluv.V2.y = currY + fTxH;
				qcoluv.V3.y = currY + fTxH;

				// Is NOT totally clipped ?
				if ( !(	(qcoluv.V0.x > _XMax) || (qcoluv.V2.x < _XMin) ||
						(qcoluv.V0.y > _YMax) || (qcoluv.V2.y < _YMin) ) )
					putQuadInLayer (*(rI.GlobalTexturePtr), layerId, qcoluv, rot);

				currX   += fStepX;
			}
			currY  += fStepY;
		}

		setClipWindow (oldClipX, oldClipY, oldClipW, oldClipH);
	}


	/*
	 * drawBitmap
	 */
	void CViewRenderer::drawRotFlipBitmap (sint layerId, sint32 x, sint32 y, sint32 width, sint32 height,
										   uint8 rot, bool flipv, sint32 nTxId, const CRGBA &col)
	{
		if (width <= 0 || height <= 0) return;

		if (nTxId < 0)
			return;



		float dstXmin, dstYmin, dstXmax, dstYmax;

		// Is totally clipped ?
		if ((x > (_ClipX+_ClipW)) || ((x+width) < _ClipX) ||
			(y > (_ClipY+_ClipH)) || ((y+height) < _ClipY))
			return;

		dstXmin = (float)(x) * _OneOverScreenW;
		dstYmin = (float)(y) * _OneOverScreenH;
		dstXmax = (float)(x + width)  * _OneOverScreenW;
		dstYmax = (float)(y + height) * _OneOverScreenH;

		CQuadColorUV qcoluv;
		qcoluv.V0.set (dstXmin,	dstYmin, 0);
		qcoluv.V1.set (dstXmax,	dstYmin, 0);
		qcoluv.V2.set (dstXmax,	dstYmax, 0);
		qcoluv.V3.set (dstXmin, dstYmax, 0);

		qcoluv.Color0 = qcoluv.Color1 = qcoluv.Color2 = qcoluv.Color3 = col;

		SImage &rI = *getSImage(nTxId);

		// Avoid switch in common case
		if (!flipv && !rot)
		{
			qcoluv.Uv0.U = rI.UVMin.U;	qcoluv.Uv0.V = rI.UVMax.V;
			qcoluv.Uv1.U = rI.UVMax.U;	qcoluv.Uv1.V = rI.UVMax.V;
			qcoluv.Uv2.U = rI.UVMax.U;	qcoluv.Uv2.V = rI.UVMin.V;
			qcoluv.Uv3.U = rI.UVMin.U;	qcoluv.Uv3.V = rI.UVMin.V;
		}
		// else standard case
		else
		{
			if (rot > 3)
				rot = 3;

			rI.setupQuadUV(flipv, rot, qcoluv);
		}

		static volatile bool doRot[4] = { true, true, true, true };
		if (doRot[rot])
		{
			putQuadInLayer (*(rI.GlobalTexturePtr), layerId, qcoluv, rot);
		}
	}


	/*
	 * draw11RotBitmap
	 * sTx must be lowered !!!
	 */
	void CViewRenderer::draw11RotFlipBitmap (sint layerId, sint32 x, sint32 y, uint8 rot, bool flipv, sint32 nTxId, const CRGBA &col)
	{
		if (nTxId < 0)
			return;

		sint32 txw, txh;
		SImage &rImage = *getSImage(nTxId);
		txw = (sint32)((rImage.UVMax.U - rImage.UVMin.U)*rImage.GlobalTexturePtr->Width+0.5f);
		txh = (sint32)((rImage.UVMax.V - rImage.UVMin.V)*rImage.GlobalTexturePtr->Height+0.5f);

		drawRotFlipBitmap (layerId, x, y, txw, txh, rot, flipv, nTxId, col);
	}


	inline void remapUV(CUV &dest, const CUV &src, const CUV &min, const CUV &max)
	{
		dest.set(src.U * (max.U  - min.U) + min.U, src.V * (max.V  - min.V) + min.V);
	}

	void CViewRenderer::drawQuad(sint layerId, const NLMISC::CQuadUV &quadUV, sint32 nTxId, NLMISC::CRGBA col /*=NLMISC::CRGBA(255,255,255,255)*/, bool additif, bool filtered)
	{
		nlassert(!(additif && !filtered)); // not implemented yet!
		if (nTxId < 0)
			return;
		CQuadColorUV normedQuad;
		//
		normedQuad.V0.set(quadUV.V0.x * _OneOverScreenW, 	quadUV.V0.y * _OneOverScreenH, 0.f);
		normedQuad.V1.set(quadUV.V1.x * _OneOverScreenW, 	quadUV.V1.y * _OneOverScreenH, 0.f);
		normedQuad.V2.set(quadUV.V2.x * _OneOverScreenW, 	quadUV.V2.y * _OneOverScreenH, 0.f);
		normedQuad.V3.set(quadUV.V3.x * _OneOverScreenW, 	quadUV.V3.y * _OneOverScreenH, 0.f);
		//
		float qXMin = minof(normedQuad.V0.x, normedQuad.V1.x, normedQuad.V2.x, normedQuad.V3.x);
		if (qXMin > _XMax) return;
		float qXMax = maxof(normedQuad.V0.x, normedQuad.V1.x, normedQuad.V2.x, normedQuad.V3.x);
		if (qXMax < _XMin) return;
		float qYMin = minof(normedQuad.V0.y, normedQuad.V1.y, normedQuad.V2.y, normedQuad.V3.y);
		if (qYMin > _YMax) return;
		float qYMax = maxof(normedQuad.V0.y, normedQuad.V1.y, normedQuad.V2.y, normedQuad.V3.y);
		if (qYMax < _YMin) return;
		//



		SImage &rImage = *getSImage(nTxId);
		SGlobalTexture &gt = *(rImage.GlobalTexturePtr);
		CUV deltaUV(1.f / (float) gt.Width, 1.f / (float) gt.Height);
		CUV cornerMin = rImage.UVMin + deltaUV;
		CUV cornerMax = rImage.UVMax - deltaUV;
		remapUV(normedQuad.Uv0, quadUV.Uv0, cornerMin, cornerMax);
		remapUV(normedQuad.Uv1, quadUV.Uv1, cornerMin, cornerMax);
		remapUV(normedQuad.Uv2, quadUV.Uv2, cornerMin, cornerMax);
		remapUV(normedQuad.Uv3, quadUV.Uv3, cornerMin, cornerMax);

		// test if clipping is required
		if (qXMin >= _XMin && qYMin >= _YMin && qXMax <= _XMax && qYMax <= _YMax)
		{
			// not clipped, easy case
			normedQuad.Color0 = normedQuad.Color1 = normedQuad.Color2 = normedQuad.Color3 = col;

			if (_WorldSpaceTransformation)
			{
				worldSpaceTransformation (normedQuad);
			}

			layerId+= VR_BIAS_LAYER;
			nlassert(layerId>=0 && layerId<VR_NUM_LAYER);
			CLayer	&layer = rImage.GlobalTexturePtr->Layers[layerId];
			if (!filtered)
			{
				if (layer.NbQuads == layer.Quads.size())
					layer.Quads.push_back (normedQuad);
				else
					layer.Quads[layer.NbQuads] = normedQuad;
				++ layer.NbQuads;
			}
			else if (additif) layer.FilteredAdditifQuads.push_back(normedQuad);
			else layer.FilteredAlphaBlendedQuads.push_back(normedQuad);
			_EmptyLayer[layerId]= false;
		}
		else
		{
			// Partially clipped (slowest case)
			// Must do the clip manually
			static const uint maxNumCorners = 8;
			//
			static CVector	outPos0[maxNumCorners];
			static CUV		outUV0[maxNumCorners];
			static CVector	outPos1[maxNumCorners];
			static CUV		outUV1[maxNumCorners];
			//
			outUV0[0] = normedQuad.Uv0;
			outUV0[1] = normedQuad.Uv1;
			outUV0[2] = normedQuad.Uv2;
			outUV0[3] = normedQuad.Uv3;
			//
			outPos0[0] = normedQuad.V0;
			outPos0[1] = normedQuad.V1;
			outPos0[2] = normedQuad.V2;
			outPos0[3] = normedQuad.V3;
			//
			CVector *pPos0 = outPos0;
			CVector *pPos1 = outPos1;
			CUV		*pUV0 = outUV0;
			CUV		*pUV1 = outUV1;
			//
			sint count = 4;
			//
			if (qXMin < _XMin)
			{
				// clip left
				CPlane clipper(-1.f, 0.f, 0.f, _XMin);
				count = clipper.clipPolygonBack(pPos0, pUV0, pPos1, pUV1, count);
				std::swap(pPos0, pPos1);
				std::swap(pUV0, pUV1);
			}
			if (qXMax > _XMax)
			{
				// clip right
				CPlane clipper(1.f, 0.f, 0.f, -_XMax);
				count = clipper.clipPolygonBack(pPos0, pUV0, pPos1, pUV1, count);
				std::swap(pPos0, pPos1);
				std::swap(pUV0, pUV1);
			}
			//
			if (qYMin < _YMin)
			{
				// clip bottom
				CPlane clipper(0.f, -1.f, 0.f, _YMin);
				count = clipper.clipPolygonBack(pPos0, pUV0, pPos1, pUV1, count);
				std::swap(pPos0, pPos1);
				std::swap(pUV0, pUV1);
			}
			if (qYMax > _YMax)
			{
				// clip top
				CPlane clipper(0.f, 1.f, 0.f, -_YMax);
				count = clipper.clipPolygonBack(pPos0, pUV0, pPos1, pUV1, count);
				std::swap(pPos0, pPos1);
				std::swap(pUV0, pUV1);
			}

			nlassert(count <= (sint)maxNumCorners);
			if (count >= 3)
			{
				count -= 2;
				layerId+= VR_BIAS_LAYER;
				nlassert(layerId>=0 && layerId<VR_NUM_LAYER);
				CLayer	&layer = rImage.GlobalTexturePtr->Layers[layerId];
				std::vector<NLMISC::CTriangleColorUV> *tris;
				if (!filtered)
				{
					tris = &layer.Tris;
				}
				else
				{
					tris = additif ? &layer.FilteredAdditifTris : &layer.FilteredAlphaBlendedTris;
				}
				tris->resize(tris->size() + count);
				CTriangleColorUV *lastTri = &tris->back() + 1;
				CTriangleColorUV *currTri = lastTri - count;
				const CVector *firstPos = pPos0++;
				const CUV *firstUV  = pUV0++;
				do
				{
					currTri->V0 = *firstPos;
					currTri->V1 = *pPos0;
					currTri->V2 = *(pPos0 + 1);
					currTri->Color0 = col;
					currTri->Color1 = col;
					currTri->Color2 = col;
					currTri->Uv0 = *firstUV;
					currTri->Uv1 = *pUV0;
					currTri->Uv2 = *(pUV0 + 1);

					pPos0 ++;
					pUV0 ++;
					++currTri;
				}
				while (currTri != lastTri);
				_EmptyLayer[layerId]= false;
			}
		}
	}


	void CViewRenderer::drawUnclippedTriangles(sint layerId, const std::vector<NLMISC::CTriangle> &tris, NLMISC::CRGBA col)
	{
		if (tris.empty()) return;
		if (!_BlankGlobalTexture) return;
		// primary goal here is batching, so we prefer to draw the triangle with a blank texture rather than
		// switching material and having to flush all primitives .
		layerId+= VR_BIAS_LAYER;
		nlassert(layerId>=0 && layerId<VR_NUM_LAYER);
		CLayer	&layer = _BlankGlobalTexture->Layers[layerId];
		uint startCount = (uint)layer.FilteredAlphaBlendedTris.size();
		layer.FilteredAlphaBlendedTris.resize(startCount + tris.size());
		const NLMISC::CTriangle *src =&tris[0];
		const NLMISC::CTriangle *last = src + tris.size();
		NLMISC::CTriangleColorUV *dest = &layer.FilteredAlphaBlendedTris[0] + startCount;
		_EmptyLayer[layerId]= false;
		do
		{
			dest->V0.set(src->V0.x * _OneOverScreenW, src->V0.y * _OneOverScreenH, 0.f);
			dest->V1.set(src->V1.x * _OneOverScreenW, src->V1.y * _OneOverScreenH, 0.f);
			dest->V2.set(src->V2.x * _OneOverScreenW, src->V2.y * _OneOverScreenH, 0.f);
			static volatile bool testOpaque = false;
			if (testOpaque)
			{
				dest->Color0 = CRGBA::White;
				dest->Color1 = CRGBA::White;
				dest->Color2 = CRGBA::White;
				dest->Uv0.set(0.f, 0.f);
				dest->Uv1.set(1.f, 0.f);
				dest->Uv2.set(1.f, 1.f);
			}
			else
			{
				dest->Color0 = col;
				dest->Color1 = col;
				dest->Color2 = col;
				dest->Uv0    = _BlankUV;
				dest->Uv1    = _BlankUV;
				dest->Uv2    = _BlankUV;
			}
			++ dest;
			++ src;
		}
		while (src != last);
	}

	/*
	 * loadTextures
	 */
	bool CViewRenderer::loadTextures (const std::string &textureFileName, const std::string &uvFileName, bool uploadDXTC)
	{
		SGlobalTexture gt;
		// Load texture file
		string filename = CPath::lookup (textureFileName, false);
		if (filename.empty() )
			return false;

		CIFile ifTmp;
		if (ifTmp.open(filename))
			CBitmap::loadSize (ifTmp, gt.Width, gt.Height);
		gt.Texture = driver->createTextureFile (filename);
		// Force to generate the texture now. This way we can extract the mouse bitmaps from it now without having to load it again.
		// Its why we don't release it at the end, because it is likely to be uploaded soon)
		CBitmap *texDatas = gt.Texture->generateDatas();
		//
		gt.Name = filename;
		gt.Texture->setFilterMode(UTexture::Nearest, UTexture::NearestMipMapOff);
		if(uploadDXTC)
			gt.Texture->setUploadFormat(UTexture::DXTC5);

		// Load uv file
		CIFile iFile;
		filename = CPath::lookup (uvFileName, false);
		if (filename.empty() )
			return false;
		if (!iFile.open(filename))
			return false;

		_GlobalTextures.push_back (gt);

		driver->setCursorScale( CViewRenderer::hwCursorScale );

		char bufTmp[256], tgaName[256];
		string sTGAname;
		float uvMinU, uvMinV, uvMaxU, uvMaxV;
		while (!iFile.eof())
		{
			iFile.getline (bufTmp, 256);
			sscanf (bufTmp, "%s %f %f %f %f", tgaName, &uvMinU, &uvMinV, &uvMaxU, &uvMaxV);
			SImage image;
			image.UVMin.U = uvMinU;
			image.UVMin.V = uvMinV;
			image.UVMax.U = uvMaxU;
			image.UVMax.V = uvMaxV;
			sTGAname = toLower(string(tgaName));

			string::size_type stripPng = sTGAname.find(".png");
			if (stripPng != string::npos)
			{
				sTGAname[stripPng + 1] = 't';
				sTGAname[stripPng + 2] = 'g';
				sTGAname[stripPng + 3] = 'a';
			}

			image.Name = sTGAname;
			image.GlobalTexturePtr = &(_GlobalTextures.back());
			if (getTextureIdFromName(sTGAname) != -1)
			{
				string tmp = string("duplicate texture name in ") + textureFileName + "(" + sTGAname + ")";
				nlwarning(tmp.c_str());
			}
			else
			{
				sint32 textureId = addSImage(image);
				//nlwarning("SIMAGE ADDED: id = %x, name = %s",  textureId, image.Name.c_str());
				// Insert in map.
				_TextureMap.insert( make_pair(image.Name, textureId) );
			}

			// if this is a cursor texture, extract it now (supported for rgba only now, because of the blit)
			if (texDatas && texDatas->getPixelFormat() == CBitmap::RGBA)
			{
				if( CViewRenderer::hwCursors->count( image.Name ) > 0 )
				{
					uint x0 = (uint) (image.UVMin.U * gt.Width);
					uint y0 = (uint) (image.UVMin.V * gt.Height);
					uint x1 = (uint) (image.UVMax.U * gt.Width);
					uint y1 = (uint) (image.UVMax.V * gt.Height);
					if (x1 != x0 && y1 != y0)
					{
						CBitmap curs;
						curs.resize(x1 - x0, y1 - y0);
						curs.blit(*texDatas, x0, y0, (x1 - x0), (y1 - y0), 0, 0);
						driver->addCursor(image.Name, curs);
					}
				}
			}
		}

		initIndexesToTextureIds ();
		initSystemTextures();
		initTypo();

		return true;
	}




	void CViewRenderer::setExternalTexture(const std::string &sGlobalTextureName,
										   NL3D::UTexture	*externalTexture,
										   uint32			 externalTexWidth,
										   uint32			 externalTexHeight,
										   uint32			 defaultTexWidth,
										   uint32			 defaultTexHeight
										  )
	{
		if (sGlobalTextureName.empty())
		{
			nlwarning("Can't create aglobal texture with an empty name");
			return;
		}
		// Look if already existing
		string sLwrGTName = strlwr(sGlobalTextureName);
		TGlobalTextureList::iterator ite = _GlobalTextures.begin();
		while (ite != _GlobalTextures.end())
		{
			std::string sText = strlwr(ite->Name);
			if (sText == sLwrGTName)
				break;
			ite++;
		}
		if (ite == _GlobalTextures.end())
		{
			SGlobalTexture gtTmp;
			gtTmp.FromGlobaleTexture = true;

			gtTmp.Name = sLwrGTName;
			_GlobalTextures.push_back(gtTmp);
			ite = _GlobalTextures.end();
			ite--;
		}
		ite->Width = externalTexWidth;
		ite->Height = externalTexHeight;
		ite->DefaultWidth = defaultTexWidth;
		ite->DefaultHeight = defaultTexHeight;
		ite->Texture = externalTexture;
	}

	/*
	 * createTexture
	 */
	sint32 CViewRenderer::createTexture (const std::string &sGlobalTextureName,
										 sint32 offsetX,
										 sint32 offsetY,
										 sint32 width,
										 sint32 height,
										 bool uploadDXTC,
										 bool bReleasable
										)
	{
		if (sGlobalTextureName.empty()) return -1;
		// Look if already existing
		string sLwrGTName = toLower(sGlobalTextureName);
		TGlobalTextureList::iterator ite = _GlobalTextures.begin();
		while (ite != _GlobalTextures.end())
		{
			std::string sText = toLower(ite->Name);
			if (sText == sLwrGTName)
				break;
			ite++;
		}

		// If global texture not exists create it
		if (ite == _GlobalTextures.end())
		{
			SGlobalTexture gtTmp;
			gtTmp.FromGlobaleTexture = false;
			string filename = CPath::lookup (sLwrGTName, false);
			if (filename.empty() ) return -1;
			CIFile ifTmp;
			if (ifTmp.open(filename))
			{
				CBitmap::loadSize (ifTmp, gtTmp.Width, gtTmp.Height);
				gtTmp.DefaultWidth = gtTmp.Width;
				gtTmp.DefaultHeight = gtTmp.Height;
				if (gtTmp.Width == 0 || gtTmp.Height == 0)
				{
					nlwarning("Failed to load the texture '%s', please check image format", filename.c_str());
				}
			}
			gtTmp.Texture = driver->createTextureFile (sLwrGTName);
			gtTmp.Name = sLwrGTName;
			gtTmp.Texture->setFilterMode(UTexture::Nearest, UTexture::NearestMipMapOff);
			if(uploadDXTC)
				gtTmp.Texture->setUploadFormat(UTexture::DXTC5);
			gtTmp.Texture->setReleasable(bReleasable);
			_GlobalTextures.push_back(gtTmp);
			ite = _GlobalTextures.end();
			ite--;
		}

		// Add a texture with reference to the i th global texture
		SImage iTmp;

		// Set default parameters
		if (width == -1)
			width = ite->DefaultWidth;
		if (height == -1)
			height = ite->DefaultHeight;

		iTmp.Name = sLwrGTName;
		iTmp.GlobalTexturePtr = &(*ite);
		iTmp.UVMin = CUV(((float)offsetX)/ite->Width , ((float)offsetY)/ite->Height);
		iTmp.UVMax = CUV(((float)offsetX+width)/ite->Width , ((float)offsetY+height)/ite->Height);
		sint32 TextID = addSImage(iTmp);
		//nlwarning("SIMAGE ADDED: id = %d, name = %s",  TextID, iTmp.Name.c_str());

		// Insert / replace in map.
		// TMP TMP FIX NICO
		//_TextureMap.insert( make_pair(iTmp.Name, TextID) );


		return TextID;
	}

	void CViewRenderer::updateTexturePos(const std::string &texturefileName, sint32 offsetX /*=0*/, sint32 offsetY /*=0*/, sint32 width /*=-1*/, sint32 height /*=-1*/)
	{
		sint32 id = getTextureIdFromName (texturefileName);
		if (id == -1)
		{
			nlwarning("Unknwown texture %s, can't update pos", texturefileName.c_str());
			return;
		}
		SImage *im = getSImage(id);
		nlassert(im);
		// Set default parameters
		sint32 gw = im->GlobalTexturePtr->Width;
		sint32 gh = im->GlobalTexturePtr->Height;
		if (width == -1)
			width = gw;
		if (height == -1)
			height = gh;
		im->UVMin = CUV(((float)offsetX)/gw , ((float)offsetY)/gh);
		im->UVMax = CUV(((float)offsetX+width)/gw, ((float)offsetY+height)/gh);
	}


	/*
	 * getGlobalTexture
	 */
	NL3D::UTexture *CViewRenderer::getGlobalTexture(const std::string &name)
	{
		string sLwrGTName = strlwr(name);
		TGlobalTextureList::iterator ite = _GlobalTextures.begin();
		while (ite != _GlobalTextures.end())
		{
			std::string sText = strlwr(ite->Name);
			if (sText == sLwrGTName)
				break;
			ite++;
		}
		if (ite != _GlobalTextures.end())
		{
			return ite->Texture;
		}
		return NULL;
	}

	/*
	 * deleteTexture
	 */
	void CViewRenderer::deleteTexture (sint32 textureId)
	{
		// Checks
		nlassert ((uint)textureId < _SImageIterators.size());
		if (_SImageIterators[textureId] == _SImages.end())
		{
			nlwarning("Can't delete texture with name %s", getTextureNameFromId(textureId).c_str());
			nlassert(0);
			return;
		}

		// Backup global texture pointer
		SGlobalTexture *gt = getSImage(textureId)->GlobalTexturePtr;

		// Erase only texture from global texture
		if (!(gt->FromGlobaleTexture))
		{
			//nlwarning("Removing texture with id %d", (int) textureId);
			// Erase the SImage
			//nlwarning("SIMAGE REMOVE : id = %x, name = %s", (int) textureId, getSImage(textureId)->Name.c_str());


			removeSImage (textureId);

			// Check if someone else use this global texture..
			TSImageList::iterator ite = _SImages.begin();
			while (ite != _SImages.end())
			{
				// Same global texture ?
				if (ite->GlobalTexturePtr == gt)
					break;

				ite++;
			}

			// Global texture still used ?
			if (ite == _SImages.end())
			{
				//nlwarning("REMOVE GLOBAL TEXTURE : id of simage = %x", (int) textureId);
				// No, remove the global texture
				for (TGlobalTextureList::iterator iteGT = _GlobalTextures.begin(); iteGT != _GlobalTextures.end(); iteGT++)
				{
					// This one ?
					if (&(*iteGT) == gt)
					{
						// Remove this global texture
						UTextureFile *tf = dynamic_cast<NL3D::UTextureFile *>(iteGT->Texture);
						if (tf)
						{
							driver->deleteTextureFile (tf);
						}
						_GlobalTextures.erase (iteGT);
						return;
					}
				}
				// Global texture has not been found
				nlstop;
			}
		}
	}

	bool CViewRenderer::getTexture( NLMISC::CBitmap &bm, const std::string &name )
	{
		TTextureMap::const_iterator itr = _TextureMap.find( name );
		if( itr == _TextureMap.end() )
			return false;

		sint32 id = itr->second;
		SImage *si = getSImage( id );
		NLMISC::CBitmap *src = si->GlobalTexturePtr->Texture->generateDatas();

		if( src->getPixelFormat() != NLMISC::CBitmap::RGBA )
			return false;

		uint x0 = (uint)( si->UVMin.U * si->GlobalTexturePtr->Width );
		uint y0 = (uint)( si->UVMin.V * si->GlobalTexturePtr->Height );
		uint x1 = (uint)( si->UVMax.U * si->GlobalTexturePtr->Width );
		uint y1 = (uint)( si->UVMax.V * si->GlobalTexturePtr->Height );

		if( x1 == x0 )
			return false;

		if( y1 == y0 )
			return false;

		bm.resize( x1 - x0, y1 - y0 );
		bm.blit( *src, x0, y0, ( x1 - x0 ), ( y1 - y0 ), 0, 0 );

		return true;
	}

	void CViewRenderer::getTextureNames( std::vector< std::string > &textures )
	{
		TTextureMap::const_iterator itr = _TextureMap.begin();
		while( itr != _TextureMap.end() )
		{
			textures.push_back( itr->first );
			++itr;
		}
	}

	/*
	 * getTextureIdFromName
	 */
	sint32 CViewRenderer::getTextureIdFromName (const string &sName) const
	{
		if(sName.empty())
			return -1;

		// convert to lowCase
		string nameLwr = toLower(sName);

		string::size_type stripPng = nameLwr.find(".png");
		if (stripPng != string::npos)
		{
			nameLwr[stripPng + 1] = 't';
			nameLwr[stripPng + 2] = 'g';
			nameLwr[stripPng + 3] = 'a';
		}

		// Search in map
		TTextureMap::const_iterator		it= _TextureMap.find(nameLwr);
		if( it==_TextureMap.end() )
			return -1;
		else
			return it->second;
	}

	/*
	 * getTextureNameFromId
	 */
	std::string CViewRenderer::getTextureNameFromId (sint32 TxID)
	{
		if ((TxID < 0) || (TxID >= (sint32)_SImageIterators.size()))
			return "";
		SImage *img = getSImage(TxID);
		return img->Name;
	}

	/*
	 * getTextureSizeFromName
	 */
	void CViewRenderer::getTextureSizeFromId (sint32 id, sint32 &width, sint32 &height)
	{
		if ((id < 0) || (id >= (sint32)_SImageIterators.size()))
		{
			width = height = 0;
		}
		else
		{
			SImage &rImage = *getSImage(id);
			width = (sint32)((rImage.UVMax.U - rImage.UVMin.U)*rImage.GlobalTexturePtr->Width+0.5f);
			height = (sint32)((rImage.UVMax.V - rImage.UVMin.V)*rImage.GlobalTexturePtr->Height+0.5f);
		}
	}
	/*
	 * getTextureColor
	 */
	CRGBA CViewRenderer::getTextureColor(sint32 id, sint32 x, sint32 y)
	{
		if ((id < 0) || (id >= (sint32)_SImageIterators.size()))
		{
			return CRGBA(255,255,255);
		}

		SImage &rImage = *getSImage(id);
		SGlobalTexture &rGT = *rImage.GlobalTexturePtr;
		sint32 width, height;
		width = (sint32)((rImage.UVMax.U - rImage.UVMin.U)*rGT.Width+0.5f);
		height = (sint32)((rImage.UVMax.V - rImage.UVMin.V)*rGT.Height+0.5f);
		float xRatio = ((float)x) / ((float)(width));
		float yRatio = ((float)y) / ((float)(height));
		UTexture *pTF = rGT.Texture;
		sint32 xConv = (sint32)((rImage.UVMin.U + xRatio * (rImage.UVMax.U - rImage.UVMin.U))*rGT.Width+0.5f);
		sint32 yConv = (rGT.Height-1)-(sint32)((rImage.UVMin.V + yRatio * (rImage.UVMax.V - rImage.UVMin.V))*rGT.Height+0.5f);
		return pTF->getPixelColor(xConv, yConv);
	}

	// ***************************************************************************
	sint32 CViewRenderer::getTypoTextureW(char c)
	{
		if ((c>=0) && (c<NumTypoChar))
			return _TypoCharWs[(uint)c];
		else
			return 1;
	}

	// ***************************************************************************
	sint32 CViewRenderer::getTypoTextureH(char /* c */)
	{
		return _TypoH;
	}

	// ***************************************************************************
	sint32 CViewRenderer::getTypoTextureId(char c)
	{
		if ((c>=0) && (c<NumTypoChar))
			return _TypoCharToTextureIds[(uint)c];
		else
			return -1;
	}

	/*
	 * flush
	 */
	void CViewRenderer::flush ()
	{
		H_AUTO ( RZ_Interface_ViewRenderer_flush )

		// Run All layers.
		for(uint layerId=0;layerId<VR_NUM_LAYER;layerId++)
		{
			if(_EmptyLayer[layerId])
				continue;

			// **** Run all Global Textures
			TGlobalTextureList::iterator ite = _GlobalTextures.begin();
			while (ite != _GlobalTextures.end())
			{
				// TMP TMP
	//			volatile SGlobalTexture *sg = &(*ite);
				CLayer	&layer= ite->Layers[layerId];
				if(layer.NbQuads>0 || !layer.Tris.empty())
				{
					// setup the global texture to material
					_Material.setTexture(0, ite->Texture);

					// Special Case if _WorldSpaceTransformation and _WorldSpaceScale, enable bilinear
					if(_WorldSpaceTransformation && _WorldSpaceScale)
						ite->Texture->setFilterMode(UTexture::Linear, UTexture::LinearMipMapOff);

					// draw quads and empty list
					if (layer.NbQuads != 0)
					{
						driver->drawQuads (&(layer.Quads[0]), layer.NbQuads, _Material);
						layer.NbQuads = 0;
					}

					if (!layer.Tris.empty())
					{
						driver->drawTriangles(layer.Tris, _Material);
						layer.Tris.clear();
					}

					// Special Case if _WorldSpaceTransformation and _WorldSpaceScale, reset
					if(_WorldSpaceTransformation && _WorldSpaceScale)
						ite->Texture->setFilterMode(UTexture::Nearest, UTexture::NearestMipMapOff);
				}
				if (!layer.FilteredAlphaBlendedQuads.empty() ||
					!layer.FilteredAlphaBlendedTris.empty() ||
					!layer.FilteredAdditifQuads.empty() ||
					!layer.FilteredAdditifTris.empty())
				{
					// setup the global texture to material
					_Material.setTexture(0, ite->Texture);

					// force filtering
					 ite->Texture->setFilterMode(UTexture::Linear, UTexture::LinearMipMapOff);
					// alpha blended
					if (!layer.FilteredAlphaBlendedQuads.empty())
					{
						driver->drawQuads (&(layer.FilteredAlphaBlendedQuads[0]), (uint32)layer.FilteredAlphaBlendedQuads.size(), _Material);
						layer.FilteredAlphaBlendedQuads.clear();
					}
					if (!layer.FilteredAlphaBlendedTris.empty())
					{
						driver->drawTriangles(layer.FilteredAlphaBlendedTris, _Material);
						layer.FilteredAlphaBlendedTris.clear();
					}
					// additif
					if (!layer.FilteredAdditifQuads.empty() ||
						!layer.FilteredAdditifTris.empty())
					{
						_Material.setBlendFunc (NL3D::UMaterial::one, NL3D::UMaterial::one);
						if (!layer.FilteredAdditifQuads.empty())
						{
							driver->drawQuads (&(layer.FilteredAdditifQuads[0]), (uint32)layer.FilteredAdditifQuads.size(), _Material);
							layer.FilteredAdditifQuads.clear();
						}
						if (!layer.FilteredAdditifTris.empty())
						{
							driver->drawTriangles(layer.FilteredAdditifTris, _Material);
							layer.FilteredAdditifTris.clear();
						}
						// restore alpha blend
						_Material.setBlendFunc (NL3D::UMaterial::srcalpha, NL3D::UMaterial::invsrcalpha);
					}
					ite->Texture->setFilterMode(UTexture::Nearest, UTexture::NearestMipMapOff);
				}
				ite++;
			}

			// **** Display Computed Strings of this layer
			if (_WorldSpaceTransformation)
				textcontext->flushRenderBufferUnProjected(_StringRBLayers[layerId], false);
			else
				textcontext->flushRenderBuffer(_StringRBLayers[layerId]);

			// flushed
			_EmptyLayer[layerId]= true;
		}
	}


	/**
	 * init the map _IndexesToTextures
	 */
	void CViewRenderer::initIndexesToTextureIds()
	{
		char buf[20];
		_IndexesToTextureIds.clear();
		for (uint i = 0; i < 10; i++)
		{
			sprintf (buf, "numbers_%d.tga", i);
			_IndexesToTextureIds.push_back (getTextureIdFromName(buf));
		}
		_FigurSeparatorTextureId = getTextureIdFromName("Numbers_sep.tga");
		_FigurBlankId = getTextureIdFromName("numbers_blank.tga");
		_BlankId = getTextureIdFromName("blank.tga");

		SImage	*blank = getSImage(_BlankId);
		if (blank)
		{
			_BlankGlobalTexture = blank->GlobalTexturePtr;
			_BlankUV = 0.5f * (blank->UVMin + blank->UVMax);
		}
		else
		{
			_BlankUV.set(0.f, 0.f);
		}


		// Init size
		if(_IndexesToTextureIds[0]!=-1)
		{
			getTextureSizeFromId (_IndexesToTextureIds[0], _WFigurTexture, _HFigurTexture);
		}
		if (_FigurSeparatorTextureId != -1)
		{
			getTextureSizeFromId (_FigurSeparatorTextureId, _WFigurSeparatorTexture, _HFigurSeparatorTexture);
		}

	}

	/**
	 *
	 */
	void CViewRenderer::initTypo()
	{
		_TypoH = 0;

		// since filename dose not support special char (?,. ....), specify a map from char to string.
		map<char, string>	specialCharMap;
		specialCharMap['?']= "question";


		char buf[256];
		// For all supported chars (if tga exist)
		for (uint i = 0; i < NumTypoChar; i++)
		{
			// Get the token string for this char.
			string	token;
			map<char, string>::iterator	it= specialCharMap.find(i);
			// General case
			if(it==specialCharMap.end())
				token= (char)i;
			else
				token= it->second;

			// get the fileName
			sprintf (buf, "typo_%s.tga", token.c_str());
			sint32 id = getTextureIdFromName(buf);
			if(id>=0)
			{
				_TypoCharToTextureIds[i]= id;
				sint32 w,h;
				getTextureSizeFromId (id, w, h);
				_TypoCharWs[i]= w;
				_TypoH = h;
			}
			else
			{
				_TypoCharToTextureIds[i]= -1;
				// simulate a space.
				_TypoCharWs[i]= 1;
			}
		}
	}


	/**
	 * needClipping
	 */
	bool CViewRenderer::needClipping (const CQuad &q)
	{
		if ((q.V0.x >= _XMin) && (q.V0.y >= _YMin) && (q.V2.x <= _XMax) && (q.V2.y <= _YMax))
			return false;
		else
			return true;
	}

	/**
	 * clip
	 */
	void CViewRenderer::clip (CQuadColorUV &qout, const CQuadColorUV &qin, uint rot)
	{
		float ratio;

		qout = qin;

		if (rot & 1)
		{
			// must reverse U & V during clipping
			if (qin.V0.x < _XMin)
			{
				ratio = ((float)(_XMin - qin.V0.x))/((float)(qin.V1.x - qin.V0.x));
				qout.V3.x = qout.V0.x = _XMin;
				qout.Uv0.V += ratio*(qin.Uv1.V-qin.Uv0.V);
				qout.Uv3.V += ratio*(qin.Uv2.V-qin.Uv3.V);
			}

			if (qin.V0.y < _YMin)
			{
				ratio = ((float)(_YMin - qin.V0.y))/((float)(qin.V3.y - qin.V0.y));
				qout.V1.y = qout.V0.y = _YMin;
				qout.Uv0.U += ratio*(qin.Uv3.U-qin.Uv0.U);
				qout.Uv1.U += ratio*(qin.Uv2.U-qin.Uv1.U);
			}

			if (qin.V2.x > _XMax)
			{
				ratio = ((float)(_XMax - qin.V2.x))/((float)(qin.V3.x - qin.V2.x));
				qout.V2.x = qout.V1.x = _XMax;
				qout.Uv2.V += ratio*(qin.Uv3.V-qin.Uv2.V);
				qout.Uv1.V += ratio*(qin.Uv0.V-qin.Uv1.V);
			}

			if (qin.V2.y > _YMax)
			{
				ratio = ((float)(_YMax - qin.V2.y))/((float)(qin.V1.y - qin.V2.y));
				qout.V2.y = qout.V3.y = _YMax;
				qout.Uv2.U += ratio*(qin.Uv1.U-qin.Uv2.U);
				qout.Uv3.U += ratio*(qin.Uv0.U-qin.Uv3.U);
			}
		}
		else
		{
			if (qin.V0.x < _XMin)
			{
				ratio = ((float)(_XMin - qin.V0.x))/((float)(qin.V1.x - qin.V0.x));
				qout.V3.x = qout.V0.x = _XMin;
				qout.Uv0.U += ratio*(qin.Uv1.U-qin.Uv0.U);
				qout.Uv3.U += ratio*(qin.Uv2.U-qin.Uv3.U);
			}

			if (qin.V0.y < _YMin)
			{
				ratio = ((float)(_YMin - qin.V0.y))/((float)(qin.V3.y - qin.V0.y));
				qout.V1.y = qout.V0.y = _YMin;
				qout.Uv0.V += ratio*(qin.Uv3.V-qin.Uv0.V);
				qout.Uv1.V += ratio*(qin.Uv2.V-qin.Uv1.V);
			}

			if (qin.V2.x > _XMax)
			{
				ratio = ((float)(_XMax - qin.V2.x))/((float)(qin.V3.x - qin.V2.x));
				qout.V2.x = qout.V1.x = _XMax;
				qout.Uv2.U += ratio*(qin.Uv3.U-qin.Uv2.U);
				qout.Uv1.U += ratio*(qin.Uv0.U-qin.Uv1.U);
			}

			if (qin.V2.y > _YMax)
			{
				ratio = ((float)(_YMax - qin.V2.y))/((float)(qin.V1.y - qin.V2.y));
				qout.V2.y = qout.V3.y = _YMax;
				qout.Uv2.V += ratio*(qin.Uv1.V-qin.Uv2.V);
				qout.Uv3.V += ratio*(qin.Uv0.V-qin.Uv3.V);
			}
		}
	}

	/**
	 * clip with uv2
	 */
	void CViewRenderer::clip (CQuadColorUV2 &qout, const CQuadColorUV2 &qin)
	{
		float ratio;

		qout = qin;

		if (qin.V0.x < _XMin)
		{
			ratio = ((float)(_XMin - qin.V0.x))/((float)(qin.V1.x - qin.V0.x));
			qout.V3.x = qout.V0.x = _XMin;
			qout.Uv0.U += ratio*(qin.Uv1.U-qin.Uv0.U);
			qout.Uv3.U += ratio*(qin.Uv2.U-qin.Uv3.U);
			qout.Uv02.U += ratio*(qin.Uv12.U-qin.Uv02.U);
			qout.Uv32.U += ratio*(qin.Uv22.U-qin.Uv32.U);
		}

		if (qin.V0.y < _YMin)
		{
			ratio = ((float)(_YMin - qin.V0.y))/((float)(qin.V3.y - qin.V0.y));
			qout.V1.y = qout.V0.y = _YMin;
			qout.Uv0.V += ratio*(qin.Uv3.V-qin.Uv0.V);
			qout.Uv1.V += ratio*(qin.Uv2.V-qin.Uv1.V);
			qout.Uv02.V += ratio*(qin.Uv32.V-qin.Uv02.V);
			qout.Uv12.V += ratio*(qin.Uv22.V-qin.Uv12.V);
		}

		if (qin.V2.x > _XMax)
		{
			ratio = ((float)(_XMax - qin.V2.x))/((float)(qin.V3.x - qin.V2.x));
			qout.V2.x = qout.V1.x = _XMax;
			qout.Uv2.U += ratio*(qin.Uv3.U-qin.Uv2.U);
			qout.Uv1.U += ratio*(qin.Uv0.U-qin.Uv1.U);
			qout.Uv22.U += ratio*(qin.Uv32.U-qin.Uv22.U);
			qout.Uv12.U += ratio*(qin.Uv02.U-qin.Uv12.U);
		}

		if (qin.V2.y > _YMax)
		{
			ratio = ((float)(_YMax - qin.V2.y))/((float)(qin.V1.y - qin.V2.y));
			qout.V2.y = qout.V3.y = _YMax;
			qout.Uv2.V += ratio*(qin.Uv1.V-qin.Uv2.V);
			qout.Uv3.V += ratio*(qin.Uv0.V-qin.Uv3.V);
			qout.Uv22.V += ratio*(qin.Uv12.V-qin.Uv22.V);
			qout.Uv32.V += ratio*(qin.Uv02.V-qin.Uv32.V);
		}
	}


	// ***************************************************************************
	/**
	 * putQuadInLayer : put a quad in a specific layer of a specific texture
	 */
	void CViewRenderer::putQuadInLayer (SGlobalTexture &gt, sint layerId, const NLMISC::CQuadColorUV &qcoluv, uint rot)
	{
		layerId+= VR_BIAS_LAYER;
		nlassert(layerId>=0 && layerId<VR_NUM_LAYER);
		CLayer	&layer= gt.Layers[layerId];

		// Clipping part
		if (!needClipping(qcoluv))
		{
			// World space transformation
			if (_WorldSpaceTransformation)
			{
				NLMISC::CQuadColorUV qcolor = qcoluv;
				worldSpaceTransformation (qcolor);

				// No need to clip the quad
				if (layer.NbQuads == layer.Quads.size())
					layer.Quads.push_back (qcolor);
				else
					layer.Quads[layer.NbQuads] = qcolor;
			}
			else
			{
				// No need to clip the quad
				if (layer.NbQuads == layer.Quads.size())
					layer.Quads.push_back (qcoluv);
				else
					layer.Quads[layer.NbQuads] = qcoluv;
			}

			++layer.NbQuads;
		}
		else
		{
			CQuadColorUV qclipped;
			clip (qclipped, qcoluv, rot);

			// World space transformation
			if (_WorldSpaceTransformation)
				worldSpaceTransformation (qclipped);

			if (layer.NbQuads == layer.Quads.size())
				layer.Quads.push_back (qclipped);
			else
				layer.Quads[layer.NbQuads] = qclipped;
			++layer.NbQuads;
		}

		// layer filled
		_EmptyLayer[layerId]= false;
	}


	// ***************************************************************************
	void	CViewRenderer::addSystemTexture(TSystemTexture e, const char *s)
	{
		_SystemTextures[e].Id= getTextureIdFromName(s);
		if(_SystemTextures[e].Id!=-1)
		{
			getTextureSizeFromId(_SystemTextures[e].Id, _SystemTextures[e].W, _SystemTextures[e].H);
		}
	}

	// ***************************************************************************
	void	CViewRenderer::initSystemTextures()
	{
		addSystemTexture(QuantityCrossTexture, "w_quantity.tga");
		addSystemTexture(DefaultBrickTexture, "brick_default.tga");
		addSystemTexture(DefaultItemTexture, "item_default.tga");
		addSystemTexture(ItemPlanTexture, "item_plan_over.tga");
		addSystemTexture(SkillTexture, "skill.tga");
		addSystemTexture(ItemEnchantedTexture, "sapload.tga");
		addSystemTexture(DragCopyTexture, "W_copy.tga");
		addSystemTexture(ItemWornedTexture, "ico_task_failed.tga");
		addSystemTexture(OutOfRangeTexture, "ico_out_of_range.tga");
		addSystemTexture(RegenTexture, "regen.tga");
		addSystemTexture(RegenBackTexture, "regen_back.tga");
		addSystemTexture(GlowStarTexture, "glow_star_24.tga");
		addSystemTexture(ItemLockedByOwnerTexture, "r2ed_toolbar_lock_small.tga");
	}


	// ***************************************************************************
	URenderStringBuffer		*CViewRenderer::getStringRenderBuffer(sint layerId)
	{
		layerId+= VR_BIAS_LAYER;
		nlassert(layerId>=0 && layerId<VR_NUM_LAYER);

		return _StringRBLayers[layerId];
	}

	// ***************************************************************************
	void CViewRenderer::drawWiredQuad(sint32 x, sint32 y, sint32 width, sint32 height, NLMISC::CRGBA col /*=NLMISC::CRGBA::White*/)
	{
		driver->drawWiredQuad(x * _OneOverScreenW, y * _OneOverScreenH, (x + width) * _OneOverScreenW, (y + height) * _OneOverScreenH, col);
	}

	// ***************************************************************************
	void CViewRenderer::drawFilledQuad(sint32 x, sint32 y, sint32 width, sint32 height, NLMISC::CRGBA col /*=NLMISC::CRGBA::White*/)
	{
		driver->drawQuad(x * _OneOverScreenW, y * _OneOverScreenH, (x + width) * _OneOverScreenW, (y + height) * _OneOverScreenH, col);
	}



	// ***************************************************************************
	void CViewRenderer::drawCustom (sint32 x, sint32 y, sint32 width, sint32 height, CRGBA col, UMaterial Mat)
	{
		float dstXmin, dstYmin, dstXmax, dstYmax;

		// Is totally clipped ?
		if ((x > (_ClipX+_ClipW)) || ((x+width) < _ClipX) ||
			(y > (_ClipY+_ClipH)) || ((y+height) < _ClipY))
			return;

		flush();

		// Initialize quad
		dstXmin = (float)(x) * _OneOverScreenW;
		dstYmin = (float)(y) * _OneOverScreenH;
		dstXmax = (float)(x + width)  * _OneOverScreenW;
		dstYmax = (float)(y + height) * _OneOverScreenH;

		CQuadColorUV2 qcoluv2;
		qcoluv2.V0.set (dstXmin,	dstYmin, 0);
		qcoluv2.V1.set (dstXmax,	dstYmin, 0);
		qcoluv2.V2.set (dstXmax,	dstYmax, 0);
		qcoluv2.V3.set (dstXmin, dstYmax, 0);

		qcoluv2.Color0 = qcoluv2.Color1 = qcoluv2.Color2 = qcoluv2.Color3 = col;

		qcoluv2.Uv0.U = 0;	qcoluv2.Uv0.V = 1;
		qcoluv2.Uv1.U = 1;	qcoluv2.Uv1.V = 1;
		qcoluv2.Uv2.U = 1;	qcoluv2.Uv2.V = 0;
		qcoluv2.Uv3.U = 0;	qcoluv2.Uv3.V = 0;

		qcoluv2.Uv02.U = 0;	qcoluv2.Uv02.V = 1;
		qcoluv2.Uv12.U = 1;	qcoluv2.Uv12.V = 1;
		qcoluv2.Uv22.U = 1;	qcoluv2.Uv22.V = 0;
		qcoluv2.Uv32.U = 0;	qcoluv2.Uv32.V = 0;

		// Clipping part
		CQuadColorUV2 qcoluv2_clipped;
		if (!needClipping(qcoluv2))
		{
			// No need to clip the quad
			qcoluv2_clipped = qcoluv2;
		}
		else
		{
			clip (qcoluv2_clipped, qcoluv2);
		}

		// World space transformation
		if (_WorldSpaceTransformation)
			worldSpaceTransformation (qcoluv2_clipped);

		// Draw clipped quad
		driver->drawQuads (&qcoluv2_clipped, 1, Mat);
	}

	// ***************************************************************************
	void CViewRenderer::drawCustom(sint32 x, sint32 y, sint32 width, sint32 height, const NLMISC::CUV &uv0Min, const NLMISC::CUV &uv0Max, NLMISC::CRGBA col, NL3D::UMaterial Mat)
	{
		float dstXmin, dstYmin, dstXmax, dstYmax;

		// Is totally clipped ?
		if ((x > (_ClipX+_ClipW)) || ((x+width) < _ClipX) ||
			(y > (_ClipY+_ClipH)) || ((y+height) < _ClipY))
			return;

		flush();

		// Initialize quad
		dstXmin = (float)(x) * _OneOverScreenW;
		dstYmin = (float)(y) * _OneOverScreenH;
		dstXmax = (float)(x + width)  * _OneOverScreenW;
		dstYmax = (float)(y + height) * _OneOverScreenH;

		CQuadColorUV qcoluv;
		qcoluv.V0.set (dstXmin,	dstYmin, 0);
		qcoluv.V1.set (dstXmax,	dstYmin, 0);
		qcoluv.V2.set (dstXmax,	dstYmax, 0);
		qcoluv.V3.set (dstXmin, dstYmax, 0);

		qcoluv.Color0 = qcoluv.Color1 = qcoluv.Color2 = qcoluv.Color3 = col;

		qcoluv.Uv0.U = uv0Min.U;	qcoluv.Uv0.V = uv0Max.V;
		qcoluv.Uv1.U = uv0Max.U;	qcoluv.Uv1.V = uv0Max.V;
		qcoluv.Uv2.U = uv0Max.U;	qcoluv.Uv2.V = uv0Min.V;
		qcoluv.Uv3.U = uv0Min.U;	qcoluv.Uv3.V = uv0Min.V;


		// Clipping part
		CQuadColorUV qcoluv_clipped;
		if (!needClipping(qcoluv))
		{
			// No need to clip the quad
			qcoluv_clipped = qcoluv;
		}
		else
		{
			clip (qcoluv_clipped, qcoluv, 0);
		}

		// Draw clipped quad
		driver->drawQuads (&qcoluv_clipped, 1, Mat);
	}

	// ***************************************************************************
	void CViewRenderer::drawCustom (sint32 x, sint32 y, sint32 width, sint32 height,
						const CUV &uv0Min, const CUV &uv0Max, const CUV &uv1Min, const CUV &uv1Max,
						NLMISC::CRGBA col, NL3D::UMaterial Mat)
	{
		float dstXmin, dstYmin, dstXmax, dstYmax;

		// Is totally clipped ?
		if ((x > (_ClipX+_ClipW)) || ((x+width) < _ClipX) ||
			(y > (_ClipY+_ClipH)) || ((y+height) < _ClipY))
			return;

		flush();

		// Initialize quad
		dstXmin = (float)(x) * _OneOverScreenW;
		dstYmin = (float)(y) * _OneOverScreenH;
		dstXmax = (float)(x + width)  * _OneOverScreenW;
		dstYmax = (float)(y + height) * _OneOverScreenH;

		CQuadColorUV2 qcoluv2;
		qcoluv2.V0.set (dstXmin,	dstYmin, 0);
		qcoluv2.V1.set (dstXmax,	dstYmin, 0);
		qcoluv2.V2.set (dstXmax,	dstYmax, 0);
		qcoluv2.V3.set (dstXmin, dstYmax, 0);

		qcoluv2.Color0 = qcoluv2.Color1 = qcoluv2.Color2 = qcoluv2.Color3 = col;

		qcoluv2.Uv0.U = uv0Min.U;	qcoluv2.Uv0.V = uv0Max.V;
		qcoluv2.Uv1.U = uv0Max.U;	qcoluv2.Uv1.V = uv0Max.V;
		qcoluv2.Uv2.U = uv0Max.U;	qcoluv2.Uv2.V = uv0Min.V;
		qcoluv2.Uv3.U = uv0Min.U;	qcoluv2.Uv3.V = uv0Min.V;

		qcoluv2.Uv02.U = uv1Min.U;	qcoluv2.Uv02.V = uv1Max.V;
		qcoluv2.Uv12.U = uv1Max.U;	qcoluv2.Uv12.V = uv1Max.V;
		qcoluv2.Uv22.U = uv1Max.U;	qcoluv2.Uv22.V = uv1Min.V;
		qcoluv2.Uv32.U = uv1Min.U;	qcoluv2.Uv32.V = uv1Min.V;

		// Clipping part
		CQuadColorUV2 qcoluv2_clipped;
		if (!needClipping(qcoluv2))
		{
			// No need to clip the quad
			qcoluv2_clipped = qcoluv2;
		}
		else
		{
			clip (qcoluv2_clipped, qcoluv2);
		}

		// World space transformation
		if (_WorldSpaceTransformation)
			worldSpaceTransformation (qcoluv2_clipped);

		// Draw clipped quad
		driver->drawQuads (&qcoluv2_clipped, 1, Mat);
	}

	// ***************************************************************************

	CViewRenderer::CTextureId::~CTextureId ()
	{
		if (_TextureId>=0)
			CViewRenderer::getInstance()->deleteTexture(_TextureId);
		_TextureId = -1;
	}

	// ***************************************************************************

	bool CViewRenderer::CTextureId::setTexture (const char *textureName, sint32 offsetX, sint32 offsetY, sint32 width, sint32 height,
												bool uploadDXTC, bool bReleasable)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		if (_TextureId>=0)
			rVR.deleteTexture(_TextureId);
		_TextureId = rVR.getTextureIdFromName(textureName);
		if (_TextureId<0)
			_TextureId = rVR.createTexture (textureName, offsetX, offsetY, width, height, uploadDXTC, bReleasable);

		return _TextureId >= 0;
	}

	// ***************************************************************************
	void CViewRenderer::CTextureId::serial(NLMISC::IStream &f)
	{
		std::string texName;
		if (f.isReading())
		{
			f.serial(texName);
			setTexture(texName.c_str());
		}
		else
		{
			CViewRenderer &rVR = *CViewRenderer::getInstance();
			texName = rVR.getTextureNameFromId(_TextureId);
			f.serial(texName);
		}
	}

	// ***************************************************************************

	void CViewRenderer::worldSpaceTransformation (NLMISC::CQuadColorUV &qcoluv)
	{
		// set the world Z
		qcoluv.V0.z = _CurrentZ;
		qcoluv.V1.z = _CurrentZ;
		qcoluv.V2.z = _CurrentZ;
		qcoluv.V3.z = _CurrentZ;

		// for scaled interface, apply the scale matrix
		qcoluv.V0= _WorldSpaceMatrix * qcoluv.V0;
		qcoluv.V1= _WorldSpaceMatrix * qcoluv.V1;
		qcoluv.V2= _WorldSpaceMatrix * qcoluv.V2;
		qcoluv.V3= _WorldSpaceMatrix * qcoluv.V3;

		// unproject
		qcoluv.V0 = _CameraFrustum.unProjectZ(qcoluv.V0);
		qcoluv.V1 = _CameraFrustum.unProjectZ(qcoluv.V1);
		qcoluv.V2 = _CameraFrustum.unProjectZ(qcoluv.V2);
		qcoluv.V3 = _CameraFrustum.unProjectZ(qcoluv.V3);
	}

	// ***************************************************************************

	void CViewRenderer::setWorldSpaceFrustum (const NL3D::CFrustum &cameraFrustum)
	{
		_CameraFrustum = cameraFrustum;
	}

	// ***************************************************************************

	void CViewRenderer::activateWorldSpaceMatrix (bool activate)
	{
		_WorldSpaceTransformation = activate;
		if (!_Material.empty())
			_Material.setZFunc(activate?UMaterial::lessequal:UMaterial::always);
	}

	// ***************************************************************************

	void CViewRenderer::drawText (sint layerId, float x, float y, uint wordIndex, float xmin, float ymin, float xmax, float ymax, UTextContext &textContext)
	{
		if (_WorldSpaceTransformation)
		{
			textContext.printClipAtUnProjected(*getStringRenderBuffer(layerId), _CameraFrustum, _WorldSpaceMatrix, x, y, _CurrentZ, wordIndex, xmin, ymin, xmax, ymax);
		}
		else
		{
			textContext.printClipAt(*getStringRenderBuffer(layerId), x, y, wordIndex, xmin, ymin, xmax, ymax);
		}

		// layer is no more empty
		_EmptyLayer[layerId + VR_BIAS_LAYER]= false;
	}

	// ***************************************************************************

	void CViewRenderer::setInterfaceDepth (const NLMISC::CVector &projCenter, float scale)
	{
		_CurrentZ = projCenter.z;

		// no scale? => identity matrix (faster)
		if(scale==1)
		{
			_WorldSpaceMatrix.identity();
			_WorldSpaceScale= false;
		}
		else
		{
			_WorldSpaceMatrix.identity();
			// must be in 0..1 coordinate here...
			CVector	pos= projCenter;
			pos.x*= _OneOverScreenW;
			pos.y*= _OneOverScreenH;
			// set a pivoted scale matrix
			_WorldSpaceMatrix.translate(pos);
			_WorldSpaceMatrix.scale(scale);
			_WorldSpaceMatrix.translate(-pos);
			_WorldSpaceScale= true;
		}
	}
}
