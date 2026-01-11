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

#ifndef NL_U_WATER_ENVMAP_H
#define NL_U_WATER_ENVMAP_H

#include "u_scene.h"
#include "u_camera.h"

namespace NL3D
{

class UScene;
class UDriver;
class CWaterEnvMap;


// Interface to render water environment map
struct IWaterEnvMapRender
{
	enum TFace { positive_x=0, negative_x, positive_y, negative_y, positive_z, negative_z };
	/** viewport / scissor / render target are set by the caller. The user can begin to render the required env map directly.
      * All faces are rendered in order (rendering may be spread accross several frames, so it's up to the user to provide coherent datas for all faces)
	  * The time parameter will be the same for all faces until the cube map is completed
	  */
	virtual void render(TFace face, TGlobalAnimationTime time, UDriver &drv) = 0;

	virtual ~IWaterEnvMapRender() {}
};

/** An environment map that can be rendered by user.
  * Such a map should be created and deleted from a UDriver interface.
  * The map must then be set in a UScene for the water objects of that scene to use it.
  * A water envmap can be shared accross several scenes
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004
  */
class UWaterEnvMap
{
public:
	virtual ~UWaterEnvMap() {}

	/** Init the envmap
	  * \param cubeMapSize Size of environment cube map
	  * \param projection2DSize Depending on the shader being used, the cube map may need to be projected in 2D before use
	  *        This give the size of the 2D envmap used in this case
	  * \param updateTime The time for envmap update will be spread accros the given time interval (in seconds). 0 means an update each frame
	  */
	virtual void				init(uint cubeMapSize, uint projection2DSize, TGlobalAnimationTime updateTime = 0) = 0;
	// Set an external renderer that will update the envmap used for water rendering. The renderer will be called during the update as needed
	virtual	void			    setWaterEnvMapRenderCallback(IWaterEnvMapRender *rdr) = 0;
	virtual IWaterEnvMapRender *getWaterEnvMapRenderCallback() const = 0;
	// Invalidate current envmap, causing the whole map to  be recomputed at the next update.
	virtual void				invalidate() = 0;
	// set constant alpha of envmap
	virtual void				setAlpha(uint8 alpha) = 0;
	virtual uint8				getAlpha() const = 0;
	// Advanced : direct access to internal class
	virtual CWaterEnvMap		*getWaterEnvMap() = 0;
};



/** Helper class to render faces of a water env cubemap
  * This helps setting the right camera matrix to compute the cubemap faces
  */
class CWaterEnvMapRenderHelper : public IWaterEnvMapRender
{
public:
	// Should be defined by derivers. Render the scene at the given time. The same time will be given
	// for each faces of the cubemap
	virtual void doRender(const CMatrix &camMatrix, TGlobalAnimationTime time, UDriver &drv) = 0;
private:
	// from IWaterEnvMapRender
	virtual void render(TFace face, TGlobalAnimationTime time, UDriver &drv);
};


/** Helper class to render faces of a water env cubemap from a UScene at the given position
  * Deriver may redefine the preRender method for scene animation & framebuffer setup
  */
class CWaterEnvMapRenderFromUScene : public CWaterEnvMapRenderHelper
{
public:
	// ctor
	CWaterEnvMapRenderFromUScene();
	virtual ~CWaterEnvMapRenderFromUScene() {}
	// Set the scene and camera to be used for render, and create a camera for that purpose
	void				  setScene(UScene *scene, UCamera cam);
	void				  setCamPos(const NLMISC::CVector &pos) { _CamPos = pos; }
	const NLMISC::CVector  &getCamPos() const { return _CamPos; }
	void				  setZRange(float znear, float zfar) { _ZNear = znear; _ZFar = zfar; }
	UScene				  *getScene() { return _Scene; }
	float				  getZNear() const { return _ZNear; }
	float				  getZFar() const { return _ZFar; }
	// set the parts of the scene to be rendered
	void				  setRenderPart(UScene::TRenderPart renderPart) { _RenderPart = renderPart; }
	UScene::TRenderPart	  getRenderPart() const { return _RenderPart; }
	/**
	  * This is the place to do scene animation before rendering, and to prepare z-buffer / color buffer
	  * The passed date will be the same for all 6 faces until the cubemap is completed
	  * Please note that if the scene is used for something else than water envmap rendering, there may be 2 concurrent animation dates being set.
	  * For time independant animation this is not a problem, but for incremental animation such as particle systems or lens flare that cannot be stepped back this may
	  * cause incoherent content for neightbouring faces.
	  * Solutions include hidding such objects or making sure that they don't overlap several faces of the cube, using replacement shapes etc.
	  * Default implementation does nothing.
	  */
	virtual void preRender(TGlobalAnimationTime /* time */, UDriver &/* drv */) {}
	virtual void postRender(TGlobalAnimationTime /* time */, UDriver &/* drv */) {}
private:
	UCamera				_Cam;
	UScene				*_Scene;
	float				_ZNear, _ZFar;
	UScene::TRenderPart _RenderPart;
	NLMISC::CVector	_CamPos;
private:
	virtual void doRender(const CMatrix &camMatrix, TGlobalAnimationTime time, UDriver &drv);
};

}


#endif
