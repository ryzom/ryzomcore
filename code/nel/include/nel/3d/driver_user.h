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

#ifndef NL_DRIVER_USER_H
#define NL_DRIVER_USER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_material.h"
#include "nel/3d/event_mouse_listener.h"
#include "nel/3d/driver.h"
#include "nel/3d/register_3d.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/font_manager.h"
#include "nel/3d/ptr_set.h"
#include "nel/3d/shape_bank_user.h"
#include "nel/3d/light_user.h"
#include "nel/3d/vertex_stream_manager.h"
#include "nel/3d/async_texture_manager.h"
#include "nel/3d/lod_character_manager.h"
#include "nel/3d/render_target_manager.h"

namespace NL3D
{


class	CTextureUser;
class	CTextContextUser;
class	CSceneUser;
class	CAnimationSetUser;


// ***************************************************************************
/** UDriver implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CDriverUser : public UDriver
{
// **********************
protected:
	struct	CMatrixContext
	{
		CScissor	Scissor;		// Init to fullscreen.
		CViewport	Viewport;		// Init to fullscreen.
		CFrustum	Frustum;
		CMatrix		ViewMatrix;
		CMatrix		ModelMatrix;
	};


protected:
	IDriver					*_Driver;
	bool					_WindowInit;
	CMatrixContext			_CurrentMatrixContext;
	CFontManager			_FontManager;
	CRenderTargetManager	_RenderTargetManager;
	// Components List.
	typedef	CPtrSet<CTextureUser>		TTextureSet;
	typedef	CPtrSet<CTextContextUser>	TTextContextSet;
	typedef	CPtrSet<CSceneUser>			TSceneSet;
	typedef	CPtrSet<CAnimationSetUser>	TAnimationSetSet;
	TTextureSet				_Textures;
	TTextContextSet			_TextContexts;
	TSceneSet				_Scenes;
	TAnimationSetSet		_AnimationSets;
	CShapeBankUser			_ShapeBank;
	// There is one MeshSkin Vertex Stream per driver, and for all scenes.
	CVertexStreamManager	_MeshSkinManager;
	// Special MeshSkin Vertex Stream for shadow generation
	CVertexStreamManager	_ShadowMeshSkinManager;
	// There is one AsyncTextureManager per driver, and for all scenes
	CAsyncTextureManager	_AsyncTextureManager;
	// There is one LodCharacterManager per driver, and for all scenes
	CLodCharacterManager	_LodCharacterManager;

	// For 2D/3D Interface.
	CVertexBuffer			_VBFlat;
	CVertexBuffer			_VBColor;
	CVertexBuffer			_VBUv;
	CVertexBuffer			_VBColorUv;
	CIndexBuffer			_PBLine, _PBTri;

	CVertexBuffer			_VBQuadsColUv;
	CVertexBuffer			_VBQuadsColUv2;
	CVertexBuffer			_VBTrisColUv;
	// For security, texture are initUnlit() at init()/release().
	UMaterial				_MatFlat;
	UMaterial				_MatText;
	UMaterial				_MatStretchText;
	CMaterial				_MatFlatInternal;
	CMaterial				_MatTextInternal;
	CMaterial				_MatTextStretchInternal;

	// Default render target for effect pipeline
	CTextureUser			*_EffectRenderTarget;
	UMaterial				_MatRenderTarget;
	CMaterial				_MatRenderTargetInt;
	NLMISC::CQuadUV			_RenderTargetQuad;

	// StaticInit
	static	bool			_StaticInit;

protected:
	void			setupMatrixContext();
	CMaterial		&convMat(UMaterial &mat);


// **********************
public:


	/// \name Object
	// @{
	CDriverUser (uintptr_t windowIcon, UDriver::TDriver driver, emptyProc exitFunc = 0);
	virtual	~CDriverUser();
	// @}

	virtual	bool			isLost() const;

	/// \name Window / driver management.
	// @{

	virtual void			disableHardwareVertexProgram();
	virtual void			disableHardwarePixelProgram();
	virtual void			disableHardwareVertexArrayAGP();
	virtual void			disableHardwareTextureShader();

	/// create the window.
	virtual	bool			setDisplay(const CMode &mode, bool show, bool resizeable);
	virtual	bool			setDisplay(nlWindow wnd, const CMode &mode, bool show, bool resizeable);
	virtual bool			setMode(const CMode& mode);
	virtual bool			getModes(std::vector<CMode> &modes);
	virtual bool			getCurrentScreenMode(CMode &mode);
	virtual void			beginDialogMode();
	virtual void			endDialogMode();

	/// Set the title of the NeL window
	virtual void			setWindowTitle(const ucstring &title);

	/// Set icon(s) of the NeL window
	virtual void			setWindowIcon(const std::vector<NLMISC::CBitmap> &bitmaps);

	/// Set the position of the NeL window
	virtual void			setWindowPos(sint32 x, sint32 y);

	/// Show or hide the NeL window
	virtual void			showWindow(bool show);

	/// Release the window.
	virtual	void			release();

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate(void);
	/// Return true if driver is still active. Return false else. If he user close the window, must return false.
	virtual bool			isActive();
	/// Return an OS dependent window handle. Under Win32, it is a HWND.
	virtual nlWindow		getDisplay ();

	// @}


	/// \name Buffers.
	// @{
	/// This clear only the RGBA back buffer
	virtual	void			clearRGBABuffer(CRGBA col= CRGBA(255,255,255,255));
	/// This clear only the RGBA back buffer
	virtual	void			clearZBuffer();
	/// This clear the buffers (ALL the buffer :) )
	virtual	void			clearBuffers(CRGBA col= CRGBA(255,255,255,255));
	/// This swap the back and front buffer (ALL the buffer :) ).
	virtual	void			swapBuffers();
	virtual void            finish();
	virtual void            flush();

	virtual void			setSwapVBLInterval(uint interval);
	virtual uint			getSwapVBLInterval();

	// @}



	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled();
	virtual	void			enableFog(bool enable);
	/// setup fog parameters. fog must enabled to see result. start and end are in [0,1] range.
	virtual	void			setupFog(float start, float end, CRGBA color);
	// @}

	/// \name Light support.
	// @{
	virtual void			setLight (uint8 num, const ULight& light);
	virtual void			enableLight (uint8 num, bool enable=true);
	virtual void			setAmbientColor (CRGBA color);
	// @}

	/// \name Cull mode
	// @{
	virtual void			setCullMode(TCullMode cullMode);
	virtual	TCullMode       getCullMode() const;
	// @}

	/// \name Stencil support
	// @{
	virtual void			enableStencilTest(bool enable);
	virtual bool			isStencilTestEnabled() const;
	virtual void			stencilFunc(TStencilFunc stencilFunc, int ref, uint mask);
	virtual void			stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass);
	virtual void			stencilMask(uint mask);
	// @}

	/// \name Scene gestion.
	// @{
	/// Create a new scene.
	virtual	UScene			*createScene(bool bSmallScene);
	/// Delete a scene.
	virtual	void			deleteScene(UScene	*scene);
	// @}


	/// \name AnimationSet gestion.
	// @{
	/// Create an empty AnimationSet.
	virtual	UAnimationSet	*createAnimationSet(bool headerOptim = true);
	/// Create a new AnimationSet, load it from a file. Use CPath to search the animation set. exception EPathNotFound if not found.
	virtual	UAnimationSet	*createAnimationSet(const std::string &animationSetFile);
	/// Delete a AnimationSet.
	virtual	void			deleteAnimationSet(UAnimationSet *animationSet);
	// @}


	/// Get the render target manager
	virtual CRenderTargetManager	&getRenderTargetManager() { return _RenderTargetManager; }

	/// Set a texture the size of the window as render target
	virtual void					beginDefaultRenderTarget(uint32 width = 0, uint32 height = 0);

	/// Draw the render target to the back buffer
	virtual void					endDefaultRenderTarget(UScene *scene);


	/// \name Components gestion for Interface 2D/3D.
	// @{

	/// create a new TextContext, for a given font.
	virtual	UTextContext	*createTextContext(const std::string fontFileName, const std::string fontExFileName = "");
	/// delete a TextContext.
	virtual	void			deleteTextContext(UTextContext	*textContext);
	/// Set the maxMemory used for the FontManager
	virtual	void			setFontManagerMaxMemory(uint maxMem);
	/// get cahce information.
	virtual		std::string getFontManagerCacheInformation() const ;


	/** Create a new texture file, searching in CPath.
	 * \param file filename, local to CPath paths.
	 */
	virtual	UTextureFile	*createTextureFile(const std::string &file);
	/// Delete a texture file. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureFile(UTextureFile *textfile);
	/// Create a new Raw texture, to be filled by user.
	virtual	UTextureMem		*createTextureMem(uint width, uint height, CBitmap::TType texType = CBitmap::RGBA);
	/// Delete a Raw texture. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureMem(UTextureMem *textraw);
	/// Create a new Material, to be filled by user.
	virtual	UMaterial		createMaterial();
	/// Delete a Material.
	virtual	void			deleteMaterial(UMaterial &mat);
	// @}


	/// \name Matrix context for Interface 2D/3D.
	/** UScene ignore those function (use camera parameters instead), and do not disturb this active Matrix context.
	 * (after a scene rendering, the Matrix context for this interface is restored).
	 */
	// @{

	/** Set the active scissor for rendering. Default to fullscreen.
	 */
	virtual	void			setScissor(const CScissor &);
	virtual	CScissor		getScissor();
	/** Set the active viewport for rendering. Default to fullscreen.
	 */
	virtual	void			setViewport(const CViewport &);
	virtual	CViewport		getViewport();
	/** Set the active Frustum for rendering.
	 */
	virtual	void			setFrustum(const CFrustum &frust);
	virtual	CFrustum		getFrustum();
	virtual	void			setFrustumMatrix(CMatrix &frust);
	virtual	CMatrix			getFrustumMatrix();

	virtual float			getClipSpaceZMin() const;
	/** Set the active ViewMatrix for rendering.
	 * NB: this is the view matrix, which is the inverse of camera matrix.
	 */
	virtual	void			setViewMatrix(const CMatrix &mat);
	virtual	CMatrix			getViewMatrix();
	/** Set the active ModelMatrix for rendering. NB: UScene ignore this function (use camera parameters instead).
	 */
	virtual	void			setModelMatrix(const CMatrix &mat);
	virtual	CMatrix			getModelMatrix();


	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 2D.
	 * ModelMatrix is setup to identity. ViewMatrix is setup so that (x,y) of vectors maps to x,y screen!!!
	 */
	virtual	void			setMatrixMode2D(const CFrustum &frust);
	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 3D, using parameters of a UCamera.
	 * ModelMatrix setuped to identity. ViewMatrix setuped to the inverse of camera 's LocalMatrix.
	 * Frustum setuped to UCamera frustum.
	 */
	virtual	void			setMatrixMode3D(UCamera &camera);
	virtual void			setDepthRange(float znear, float zfar);
	virtual void			getDepthRange(float & znear, float & zfar);

	/// Set the color mask filter through where the operation done will pass
	virtual void			setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha);


	// @}


	/// \name Interface 2D/3D.
	/** All of those render primitives are unlit! You must use UScene to render lighted meshes.
	 * NB: If you set a texture to your material, the primitives are textured, even if no Uvs are provided. \n
	 * NB: All rendering are done in current viewport / current matrix context.
	 */
	// @{

	/// Draw the Line, taking color from material.
	virtual	void			drawLine(const NLMISC::CLine &tri, UMaterial &mat);
	/// Draw the Line, taking color from primitive.
	virtual	void			drawLine(const NLMISC::CLineColor &tri, UMaterial &mat);
	/// Draw the Line, taking color from material. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineUV &tri, UMaterial &mat);
	/// Draw the Line, taking color from primitive. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineColorUV &tri, UMaterial &mat);

	/// Draw the Triangle, taking color from material.
	virtual	void			drawTriangle(const NLMISC::CTriangle &tri, UMaterial &mat);
	/// Draw the Triangle, taking color from primitive.
	virtual	void			drawTriangle(const NLMISC::CTriangleColor &tri, UMaterial &mat);
	/// Draw the Triangle, taking color from material. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleUV &tri, UMaterial &mat);
	/// Draw the Triangle, taking color from primitive. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleColorUV &tri, UMaterial &mat);

	/// Draw the Quad, taking color from material.
	virtual	void			drawQuad(const NLMISC::CQuad &tri, UMaterial &mat);
	/// Draw the Quad, taking color from primitive.
	virtual	void			drawQuad(const NLMISC::CQuadColor &tri, UMaterial &mat);
	/// Draw the Quad, taking color from material. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadUV &tri, UMaterial &mat);
	/// Draw the Quad, taking color from primitive. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadColorUV &tri, UMaterial &mat);

	virtual	void			drawQuads(const std::vector<NLMISC::CQuadColorUV> &quad, UMaterial &mat);
	virtual	void			drawQuads(const std::vector<NLMISC::CQuadColorUV2> &quad, UMaterial &mat);
	virtual	void			drawTriangles(const std::vector<NLMISC::CTriangleColorUV> &tris, UMaterial &mat);
	virtual	void			drawQuads(const NLMISC::CQuadColorUV *quads, uint32 nbQuads, UMaterial &mat);
	virtual	void			drawQuads(const NLMISC::CQuadColorUV2 *quads, uint32 nbQuads, UMaterial &mat);
	virtual	void			drawTriangles(const NLMISC::CTriangleColorUV *tris, uint32 nbTris, UMaterial &mat);

	// @}


	/// \name Tools for Interface 2D.
	/** For all those function, setMatrixMode2D*() should have been called (else strange results!!).
	 */
	// @{

	/// Draw a bitmap 2D. Warning: this is slow...
	virtual	void			drawBitmap (float x, float y, float width, float height, class UTexture& texture, bool blend=true, CRGBA col= CRGBA(255,255,255,255));
	/// Draw a line in 2D. Warning: this is slow...
	virtual	void			drawLine (float x0, float y0, float x1, float y1, CRGBA col= CRGBA(255,255,255,255));
	/// Draw a Triangle in 2D. Warning: this is slow...
	virtual	void			drawTriangle (float x0, float y0, float x1, float y1, float x2, float y2, CRGBA col);
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float x0, float y0, float x1, float y1, CRGBA col);
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float xcenter, float ycenter, float radius, CRGBA col);
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float x0, float y0, float x1, float y1, CRGBA col);
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float xcenter, float ycenter, float radius, CRGBA col);

	// @}



	/// \name Driver information/Queries
	// @{
	virtual uint32			getImplementationVersion () const;
	virtual const char*		getDriverInformation ();
	virtual const char*		getVideocardInformation ();
	virtual sint			getTotalVideoMemory () const;
	virtual	uint			getNbTextureStages();
	virtual void			getWindowSize (uint32 &width, uint32 &height);
	virtual uint			getWindowWidth ();
	virtual uint			getWindowHeight ();
	virtual void			getWindowPos (sint32 &x, sint32 &y);
	virtual uint32			getAvailableVertexAGPMemory ();
	virtual uint32			getAvailableVertexVRAMMemory ();
	virtual void			getBuffer (CBitmap &bitmap);
	virtual void			getZBuffer (std::vector<float>  &zbuffer);
	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect);
	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect);
	virtual bool			fillBuffer (CBitmap &bitmap);
	// @}

	/// show cursor if b is true, or hide it if b is false
	virtual void			showCursor (bool b);
	/// x and y must be between 0.0 and 1.0
	virtual void			setMousePos (float x, float y);
	/// If true, capture the mouse to force it to stay under the window.
	virtual void			setCapture (bool b);

	// see if system cursor is currently captured
	virtual bool			isSystemCursorCaptured();

	// Add a new cursor (name is case unsensitive)
	virtual void			addCursor(const std::string &name, const NLMISC::CBitmap &bitmap);

	// Display a cursor from its name (case unsensitive)
	virtual void			setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild = false);

	// Change default scale for all cursors
	virtual void			setCursorScale(float scale);
	// @}


	/// \name Misc.
	// @{

	/** Output a system message box and print a message with an icon. This method can be call even if the driver is not initialized.
	  * This method is used to return internal driver problem when string can't be displayed in the driver window.
	  * If the driver can't open a messageBox, it should not override this method and let the IDriver class manage it with the ASCII console.
	  *
	  * \param message This is the message to display in the message box.
	  * \param title This is the title of the message box.
	  * \param type This is the type of the message box, ie number of button and label of buttons.
	  * \param icon This is the icon of the message box should use like warning, error etc...
	  */
	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);


	/** Set the global polygon mode. Can be filled, line or point. The implementation driver must
	  * call IDriver::setPolygonMode and active this mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see getPolygonMode(), TPolygonMode
	  */
	virtual void			setPolygonMode (TPolygonMode mode);
	virtual U3dMouseListener*	create3dMouseListener ();
	virtual void delete3dMouseListener (U3dMouseListener *listener);
	virtual TPolygonMode 	getPolygonMode ();
	virtual void			forceDXTCCompression(bool dxtcComp);
	virtual void			setAnisotropicFilter(sint filter);
	virtual uint			getAnisotropicFilter() const;
	virtual uint			getAnisotropicFilterMaximum() const;
	virtual void			forceTextureResize(uint divisor);
	virtual bool			setMonitorColorProperties (const CMonitorColorProperties &properties);
	// @}

	/// \name Shape Bank
	// @{
	///
	virtual	UShapeBank*		getShapeBank()
	{
		return &_ShapeBank;
	}
	// @}


	/// \name Profiling.
	// @{

	virtual	void			profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut);

	virtual	uint32			profileAllocatedTextureMemory();

	virtual	uint32			profileSetupedMaterials() const;

	virtual	uint32			profileSetupedModelMatrix() const;

	virtual void			enableUsedTextureMemorySum (bool enable);

	virtual uint32			getUsedTextureMemory() const;

	virtual	void			startProfileVBHardLock();

	virtual	void			endProfileVBHardLock(std::vector<std::string> &result);

	virtual	void			profileVBHardAllocation(std::vector<std::string> &result);

	virtual	void			startProfileIBLock();

	virtual	void			endProfileIBLock(std::vector<std::string> &result);

	virtual	void			profileIBAllocation(std::vector<std::string> &result);

	virtual	void			profileTextureUsage(std::vector<std::string> &result);

	// @}


	/// \name Async Texture loading mgt
	// @{
	virtual void				setupAsyncTextureLod(uint baseLevel, uint maxLevel);
	virtual void				setupAsyncTextureMaxUploadPerFrame(uint maxup);
	virtual void				setupMaxTotalAsyncTextureSize(uint maxText);
	virtual void				setupMaxHLSColoringPerFrame(uint maxCol);
	virtual void				updateAsyncTexture();
	virtual	uint				getTotalAsyncTextureSizeAsked() const;
	virtual	uint				getLastAsyncTextureSizeGot() const;
	virtual void				loadHLSBank(const std::string &fileName);
	// @}

	virtual	bool				supportMADOperator() const;

	virtual	bool				supportBloomEffect() const;

	/// \name Bench
	// @{
	virtual void startBench (bool wantStandardDeviation = false, bool quick = false, bool reset = true);
	virtual void endBench ();
	virtual void displayBench (class NLMISC::CLog *log);
	// @}

	/// \name Water envmap
	// @{
	virtual UWaterEnvMap *createWaterEnvMap();
	virtual void		  deleteWaterEnvMap(UWaterEnvMap *map);
	// @}

	// Copy a string to system clipboard.
	virtual bool copyTextToClipboard(const ucstring &text);

	// Paste a string from system clipboard.
	virtual bool pasteTextFromClipboard(ucstring &text);

	virtual uint64	getSwapBufferCounter();

	// copy the first texture in a second one of different dimensions
	virtual bool stretchRect(UScene * scene, class UTexture & srcUText, NLMISC::CRect &srcRect,
		class UTexture & destUText, NLMISC::CRect &destRect);

	virtual bool setRenderTarget(class UTexture & uTex,
		uint32 x = 0,
		uint32 y = 0,
		uint32 width = 0,
		uint32 height = 0,
		uint32 mipmapLevel = 0,
		uint32 cubeFace = 0);


public:

	/// \name Accessor for CSeneUser.
	// @{
	IDriver		*getDriver()
	{
		return _Driver;
	}
	void		restoreMatrixContext()
	{
		setupMatrixContext();
	}
	// same as restoreMatrixContext(), but don't reset Viewport/Scissor
	void		restoreMatrixContextMatrixOnly();

	// @}

};


} // NL3D


#endif // NL_DRIVER_USER_H

/* End of driver_user.h */





















