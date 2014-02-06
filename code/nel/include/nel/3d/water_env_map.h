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

#ifndef NL_WATER_ENV_MAP_H
#define NL_WATER_ENV_MAP_H

#include "nel/misc/smart_ptr.h"
//
#include "nel/3d/texture_cube.h"
#include "nel/3d/texture_blank.h"


namespace NL3D
{

class IDriver;
class CVertexBuffer;
class CIndexBuffer;
class CMaterial;
struct IWaterEnvMapRender;

/**
  * User computed environment map for water reflection
  * Derivers should define the 'render' method to draw the envmap.
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004
  */
class CWaterEnvMap
{
public:
	// ctor
	CWaterEnvMap();
	virtual ~CWaterEnvMap() { }
	/** Init the envmap
	  * \param CubeMapSize Size of environment cube map.
	  * \param Projection2DSize Depending on the shader being used, the cube map may need to be projected in 2D before use.
	  *        This give the size of the 2D envmap used in this case.
	  * \param UpdateTime time in seconds needed to update all 6 faces of the cube. 0 means the update is done at each frame
	  */
	void				init(uint cubeMapSize, uint projection2DSize, TGlobalAnimationTime updateTime, IDriver &driver);
	// Update content
	void				update(TGlobalAnimationTime time, IDriver &driver);
	// Called when one face of the envmap must be computed. Should be defined by derivers
	virtual void		render(CTextureCube::TFace face, TGlobalAnimationTime time) = 0;
	// Invalidate content (force to recompute a complete new view at next update)
	void				invalidate();
	// Get envmap 2D texture (after projection of cube map)
	ITexture			*getEnvMap2D() const { return _Env2D; }
	// tmp for debug : render test mesh with current model / view matrixs
	// void				renderTestMesh(IDriver &driver);
	// set constant alpha of envmap
	void				setAlpha(uint8 alpha) { _Alpha = alpha; }
	uint8				getAlpha() const { return _Alpha; }
private:
	NLMISC::CSmartPtr<CTextureCube>  _EnvCubic;
	NLMISC::CSmartPtr<CTextureBlank> _Env2D;
	uint64							 _LastRenderTick;
	TGlobalAnimationTime			 _LastRenderTime;
	TGlobalAnimationTime			 _UpdateTime;
	TGlobalAnimationTime			 _StartRenderTime;
	uint							 _NumRenderedFaces;
	static CVertexBuffer			 _TestVB;  // tmp a simple sphere to test result of mapping
	static CIndexBuffer				 _TestIB;
	static CVertexBuffer			 _FlattenVB; // vb to map cube map top hemisphere to a 2D map
	static CIndexBuffer				 _FlattenIB;
	static CMaterial				 _MaterialPassThru;
	static CMaterial				 _MaterialPassThruZTest;
	static bool						 _FlattenVBInitialized;
	uint							 _EnvCubicSize;
	uint							 _Env2DSize;
	uint8							 _Alpha;
private:
	void initFlattenVB();
	void initTestVB(); // tmp
	void doInit();
};

} // NL3D

#endif
