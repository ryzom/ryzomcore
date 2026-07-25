// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2010  Robert TIMM (rti) <mail@rtti.de>
// Copyright (C) 2013-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
	virtual	~CDriverUser() NL_OVERRIDE;
	// @}

	virtual	bool			isLost() const NL_OVERRIDE;

	/// \name Window / driver management.
	// @{

	virtual void			disableHardwareVertexProgram() NL_OVERRIDE;
	virtual void			disableHardwarePixelProgram() NL_OVERRIDE;
	virtual void			disableHardwareVertexArrayAGP() NL_OVERRIDE;
	virtual void			disableHardwareTextureShader() NL_OVERRIDE;

	/// create the window.
	virtual	bool			setDisplay(const CMode &mode, bool show, bool resizeable) NL_OVERRIDE;
	virtual	bool			setDisplay(nlWindow wnd, const CMode &mode, bool show, bool resizeable) NL_OVERRIDE;
	virtual bool			setMode(const CMode& mode) NL_OVERRIDE;
	virtual bool			getModes(std::vector<CMode> &modes) NL_OVERRIDE;
	virtual bool			getCurrentScreenMode(CMode &mode) NL_OVERRIDE;
	virtual void			beginDialogMode() NL_OVERRIDE;
	virtual void			endDialogMode() NL_OVERRIDE;

	/// Set the title of the NeL window
	virtual void			setWindowTitle(const ucstring &title) NL_OVERRIDE;

	/// Set icon(s) of the NeL window
	virtual void			setWindowIcon(const std::vector<NLMISC::CBitmap> &bitmaps) NL_OVERRIDE;

	/// Set the position of the NeL window
	virtual void			setWindowPos(sint32 x, sint32 y) NL_OVERRIDE;

	/// Show or hide the NeL window
	virtual void			showWindow(bool show) NL_OVERRIDE;

	/// Release the window.
	virtual	void			release() NL_OVERRIDE;

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate(void) NL_OVERRIDE;
	/// Return true if driver is still active. Return false else. If he user close the window, must return false.
	virtual bool			isActive() NL_OVERRIDE;
	/// Return an OS dependent window handle. Under Win32, it is a HWND.
	virtual nlWindow		getDisplay () NL_OVERRIDE;

	// @}


	/// \name Buffers.
	// @{
	/// This clear only the RGBA back buffer
	virtual	void			clearRGBABuffer(CRGBA col= CRGBA(255,255,255,255)) NL_OVERRIDE;
	/// This clear only the RGBA back buffer
	virtual	void			clearZBuffer() NL_OVERRIDE;
	/// This clear the buffers (ALL the buffer :) )
	virtual	void			clearBuffers(CRGBA col= CRGBA(255,255,255,255)) NL_OVERRIDE;
	/// This swap the back and front buffer (ALL the buffer :) ).
	virtual	void			swapBuffers() NL_OVERRIDE;
	virtual bool			isFrameReady() NL_OVERRIDE;
	virtual void            finish() NL_OVERRIDE;
	virtual void            flush() NL_OVERRIDE;

	virtual void			setSwapVBLInterval(uint interval) NL_OVERRIDE;
	virtual uint			getSwapVBLInterval() NL_OVERRIDE;

	// @}



	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled() NL_OVERRIDE;
	virtual	void			enableFog(bool enable) NL_OVERRIDE;
	/// setup fog parameters. fog must enabled to see result. start and end are in [0,1] range.
	virtual	void			setupFog(float start, float end, CRGBA color) NL_OVERRIDE;
	/// setup fog mode and density. mode/density are orthogonal to start/end/color.
	virtual	void			setupFogMode(uint mode = 0, float density = 1.f) NL_OVERRIDE;
	// @}

	/// \name Light support.
	// @{
	virtual void			setLight (uint8 num, const ULight& light) NL_OVERRIDE;
	virtual void			enableLight (uint8 num, bool enable=true) NL_OVERRIDE;
	virtual void			setAmbientColor (CRGBA color) NL_OVERRIDE;
	// @}

	/// \name Cull mode
	// @{
	virtual void			setCullMode(TCullMode cullMode) NL_OVERRIDE;
	virtual	TCullMode       getCullMode() const NL_OVERRIDE;
	// @}

	/// \name Stencil support
	// @{
	virtual void			enableStencilTest(bool enable) NL_OVERRIDE;
	virtual bool			isStencilTestEnabled() const NL_OVERRIDE;
	virtual void			stencilFunc(TStencilFunc stencilFunc, int ref, uint mask) NL_OVERRIDE;
	virtual void			stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass) NL_OVERRIDE;
	virtual void			stencilMask(uint mask) NL_OVERRIDE;
	// @}

	/// \name Clip planes
	// @{
	virtual void			enableClipPlane(uint index, bool enable) NL_OVERRIDE;
	virtual void			setClipPlane(uint index, const NLMISC::CPlane &plane) NL_OVERRIDE;
	// @}

	/// \name Scene gestion.
	// @{
	/// Create a new scene.
	virtual	UScene			*createScene(bool bSmallScene) NL_OVERRIDE;
	/// Delete a scene.
	virtual	void			deleteScene(UScene	*scene) NL_OVERRIDE;
	// @}


	/// \name AnimationSet gestion.
	// @{
	/// Create an empty AnimationSet.
	virtual	UAnimationSet	*createAnimationSet(bool headerOptim = true) NL_OVERRIDE;
	/// Create a new AnimationSet, load it from a file. Use CPath to search the animation set. exception EPathNotFound if not found.
	virtual	UAnimationSet	*createAnimationSet(const std::string &animationSetFile) NL_OVERRIDE;
	/// Delete a AnimationSet.
	virtual	void			deleteAnimationSet(UAnimationSet *animationSet) NL_OVERRIDE;
	// @}


	/// Get the render target manager
	virtual CRenderTargetManager	&getRenderTargetManager() NL_OVERRIDE { return _RenderTargetManager; }

	/// Set a texture the size of the window as render target
	virtual void					beginDefaultRenderTarget(uint32 width = 0, uint32 height = 0) NL_OVERRIDE;

	/// Draw the render target to the back buffer
	virtual void					endDefaultRenderTarget(UScene *scene) NL_OVERRIDE;


	/// \name Components gestion for Interface 2D/3D.
	// @{

	/// create a new TextContext, for a given font.
	virtual	UTextContext	*createTextContext(const std::string fontFileName, const std::string fontExFileName = "") NL_OVERRIDE;
	/// delete a TextContext.
	virtual	void			deleteTextContext(UTextContext	*textContext) NL_OVERRIDE;
	/// Set the maxMemory used for the FontManager
	virtual	void			setFontManagerMaxMemory(uint maxMem) NL_OVERRIDE;
	/// get cahce information.
	virtual		std::string getFontManagerCacheInformation() const NL_OVERRIDE ;


	/** Create a new texture file, searching in CPath.
	 * \param file filename, local to CPath paths.
	 */
	virtual	UTextureFile	*createTextureFile(const std::string &file) NL_OVERRIDE;
	/// Delete a texture file. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureFile(UTextureFile *textfile) NL_OVERRIDE;
	/// Create a new Raw texture, to be filled by user.
	virtual	UTextureMem		*createTextureMem(uint width, uint height, CBitmap::TType texType = CBitmap::RGBA) NL_OVERRIDE;
	/// Delete a Raw texture. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureMem(UTextureMem *textraw) NL_OVERRIDE;
	/// Create a new Material, to be filled by user.
	virtual	UMaterial		createMaterial() NL_OVERRIDE;
	/// Delete a Material.
	virtual	void			deleteMaterial(UMaterial &mat) NL_OVERRIDE;
	// @}


	/// \name Matrix context for Interface 2D/3D.
	/** UScene ignore those function (use camera parameters instead), and do not disturb this active Matrix context.
	 * (after a scene rendering, the Matrix context for this interface is restored).
	 */
	// @{

	/** Set the active scissor for rendering. Default to fullscreen.
	 */
	virtual	void			setScissor(const CScissor &) NL_OVERRIDE;
	virtual	CScissor		getScissor() NL_OVERRIDE;
	/** Set the active viewport for rendering. Default to fullscreen.
	 */
	virtual	void			setViewport(const CViewport &) NL_OVERRIDE;
	virtual	CViewport		getViewport() NL_OVERRIDE;
	/** Set the active Frustum for rendering.
	 */
	virtual	void			setFrustum(const CFrustum &frust) NL_OVERRIDE;
	virtual	CFrustum		getFrustum() NL_OVERRIDE;
	virtual	void			setFrustumMatrix(CMatrix &frust) NL_OVERRIDE;
	virtual	CMatrix			getFrustumMatrix() NL_OVERRIDE;

	virtual float			getClipSpaceZMin() const NL_OVERRIDE;
	/** Set the active ViewMatrix for rendering.
	 * NB: this is the view matrix, which is the inverse of camera matrix.
	 */
	virtual	void			setViewMatrix(const CMatrix &mat) NL_OVERRIDE;
	virtual	CMatrix			getViewMatrix() NL_OVERRIDE;
	/** Set the active ModelMatrix for rendering. NB: UScene ignore this function (use camera parameters instead).
	 */
	virtual	void			setModelMatrix(const CMatrix &mat) NL_OVERRIDE;
	virtual	CMatrix			getModelMatrix() NL_OVERRIDE;


	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 2D.
	 * ModelMatrix is setup to identity. ViewMatrix is setup so that (x,y) of vectors maps to x,y screen!!!
	 */
	virtual	void			setMatrixMode2D(const CFrustum &frust) NL_OVERRIDE;
	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 3D, using parameters of a UCamera.
	 * ModelMatrix setuped to identity. ViewMatrix setuped to the inverse of camera 's LocalMatrix.
	 * Frustum setuped to UCamera frustum.
	 */
	virtual	void			setMatrixMode3D(UCamera &camera) NL_OVERRIDE;
	virtual void			setDepthRange(float znear, float zfar) NL_OVERRIDE;
	virtual void			getDepthRange(float & znear, float & zfar) NL_OVERRIDE;

	/// Set the color mask filter through where the operation done will pass
	virtual void			setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha) NL_OVERRIDE;


	// @}


	/// \name Interface 2D/3D.
	/** All of those render primitives are unlit! You must use UScene to render lighted meshes.
	 * NB: If you set a texture to your material, the primitives are textured, even if no Uvs are provided. \n
	 * NB: All rendering are done in current viewport / current matrix context.
	 */
	// @{

	/// Draw the Line, taking color from material.
	virtual	void			drawLine(const NLMISC::CLine &tri, UMaterial &mat) NL_OVERRIDE;
	/// Draw the Line, taking color from primitive.
	virtual	void			drawLine(const NLMISC::CLineColor &tri, UMaterial &mat) NL_OVERRIDE;
	/// Draw the Line, taking color from material. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineUV &tri, UMaterial &mat) NL_OVERRIDE;
	/// Draw the Line, taking color from primitive. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineColorUV &tri, UMaterial &mat) NL_OVERRIDE;

	/// Draw the Triangle, taking color from material.
	virtual	void			drawTriangle(const NLMISC::CTriangle &tri, UMaterial &mat) NL_OVERRIDE;
	/// Draw the Triangle, taking color from primitive.
	virtual	void			drawTriangle(const NLMISC::CTriangleColor &tri, UMaterial &mat) NL_OVERRIDE;
	/// Draw the Triangle, taking color from material. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleUV &tri, UMaterial &mat) NL_OVERRIDE;
	/// Draw the Triangle, taking color from primitive. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleColorUV &tri, UMaterial &mat) NL_OVERRIDE;

	/// Draw the Quad, taking color from material.
	virtual	void			drawQuad(const NLMISC::CQuad &tri, UMaterial &mat) NL_OVERRIDE;
	/// Draw the Quad, taking color from primitive.
	virtual	void			drawQuad(const NLMISC::CQuadColor &tri, UMaterial &mat) NL_OVERRIDE;
	/// Draw the Quad, taking color from material. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadUV &tri, UMaterial &mat) NL_OVERRIDE;
	/// Draw the Quad, taking color from primitive. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadColorUV &tri, UMaterial &mat) NL_OVERRIDE;

	virtual	void			drawQuads(const std::vector<NLMISC::CQuadColorUV> &quad, UMaterial &mat) NL_OVERRIDE;
	virtual	void			drawQuads(const std::vector<NLMISC::CQuadColorUV2> &quad, UMaterial &mat) NL_OVERRIDE;
	virtual	void			drawTriangles(const std::vector<NLMISC::CTriangleColorUV> &tris, UMaterial &mat) NL_OVERRIDE;
	virtual	void			drawQuads(const NLMISC::CQuadColorUV *quads, uint32 nbQuads, UMaterial &mat) NL_OVERRIDE;
	virtual	void			drawQuads(const NLMISC::CQuadColorUV2 *quads, uint32 nbQuads, UMaterial &mat) NL_OVERRIDE;
	virtual	void			drawTriangles(const NLMISC::CTriangleColorUV *tris, uint32 nbTris, UMaterial &mat) NL_OVERRIDE;

	// @}


	/// \name Tools for Interface 2D.
	/** For all those function, setMatrixMode2D*() should have been called (else strange results!!).
	 */
	// @{

	/// Draw a bitmap 2D. Warning: this is slow...
	virtual	void			drawBitmap (float x, float y, float width, float height, class UTexture& texture, bool blend=true, CRGBA col= CRGBA(255,255,255,255)) NL_OVERRIDE;
	/// Draw a line in 2D. Warning: this is slow...
	virtual	void			drawLine (float x0, float y0, float x1, float y1, CRGBA col= CRGBA(255,255,255,255)) NL_OVERRIDE;
	/// Draw a Triangle in 2D. Warning: this is slow...
	virtual	void			drawTriangle (float x0, float y0, float x1, float y1, float x2, float y2, CRGBA col) NL_OVERRIDE;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float x0, float y0, float x1, float y1, CRGBA col) NL_OVERRIDE;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float xcenter, float ycenter, float radius, CRGBA col) NL_OVERRIDE;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float x0, float y0, float x1, float y1, CRGBA col) NL_OVERRIDE;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float xcenter, float ycenter, float radius, CRGBA col) NL_OVERRIDE;

	// @}



	/// \name Driver information/Queries
	// @{
	virtual uint32			getImplementationVersion () const NL_OVERRIDE;
	virtual const char*		getDriverInformation () NL_OVERRIDE;
	virtual const char*		getVideocardInformation () NL_OVERRIDE;
	virtual sint			getTotalVideoMemory () const NL_OVERRIDE;
	virtual	uint			getNbTextureStages() NL_OVERRIDE;
	virtual void			getWindowSize (uint32 &width, uint32 &height) NL_OVERRIDE;
	virtual uint			getWindowWidth () NL_OVERRIDE;
	virtual uint			getWindowHeight () NL_OVERRIDE;
	virtual void			getWindowPos (sint32 &x, sint32 &y) NL_OVERRIDE;
	virtual uint32			getAvailableVertexAGPMemory () NL_OVERRIDE;
	virtual uint32			getAvailableVertexVRAMMemory () NL_OVERRIDE;
	virtual void			getBuffer (CBitmap &bitmap) NL_OVERRIDE;
	virtual void			getZBuffer (std::vector<float>  &zbuffer) NL_OVERRIDE;
	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect) NL_OVERRIDE;
	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect) NL_OVERRIDE;
	virtual bool			fillBuffer (CBitmap &bitmap) NL_OVERRIDE;
	// @}

	/// show cursor if b is true, or hide it if b is false
	virtual void			showCursor (bool b) NL_OVERRIDE;
	/// x and y must be between 0.0 and 1.0
	virtual void			setMousePos (float x, float y) NL_OVERRIDE;
	/// If true, capture the mouse to force it to stay under the window.
	virtual void			setCapture (bool b) NL_OVERRIDE;

	// see if system cursor is currently captured
	virtual bool			isSystemCursorCaptured() NL_OVERRIDE;

	// Add a new cursor (name is case unsensitive)
	virtual void			addCursor(const std::string &name, const NLMISC::CBitmap &bitmap) NL_OVERRIDE;

	// Display a cursor from its name (case unsensitive)
	virtual void			setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild = false) NL_OVERRIDE;

	// Change default scale for all cursors
	virtual void			setCursorScale(float scale) NL_OVERRIDE;
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
	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon) NL_OVERRIDE;


	/** Set the global polygon mode. Can be filled, line or point. The implementation driver must
	  * call IDriver::setPolygonMode and active this mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see getPolygonMode(), TPolygonMode
	  */
	virtual void			setPolygonMode (TPolygonMode mode) NL_OVERRIDE;
	virtual U3dMouseListener*	create3dMouseListener () NL_OVERRIDE;
	virtual void delete3dMouseListener (U3dMouseListener *listener) NL_OVERRIDE;
	virtual TPolygonMode 	getPolygonMode () NL_OVERRIDE;
	virtual void			forceDXTCCompression(bool dxtcComp) NL_OVERRIDE;
	virtual void			setAnisotropicFilter(sint filter) NL_OVERRIDE;
	virtual uint			getAnisotropicFilter() const NL_OVERRIDE;
	virtual uint			getAnisotropicFilterMaximum() const NL_OVERRIDE;
	virtual void			forceTextureResize(uint divisor) NL_OVERRIDE;
	virtual bool			supportMonitorColorProperties () const NL_OVERRIDE;
	virtual bool			setMonitorColorProperties (const CMonitorColorProperties &properties) NL_OVERRIDE;
	// @}

	/// \name Shape Bank
	// @{
	///
	virtual	UShapeBank*		getShapeBank() NL_OVERRIDE
	{
		return &_ShapeBank;
	}
	// @}


	/// \name Profiling.
	// @{

	virtual	void			profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut) NL_OVERRIDE;

	virtual	uint32			profileAllocatedTextureMemory() NL_OVERRIDE;

	virtual	uint32			profileSetupedMaterials() const NL_OVERRIDE;

	virtual	uint32			profileSetupedModelMatrix() const NL_OVERRIDE;

	virtual void			enableUsedTextureMemorySum (bool enable) NL_OVERRIDE;

	virtual uint32			getUsedTextureMemory() const NL_OVERRIDE;

	virtual	void			startProfileVBHardLock() NL_OVERRIDE;

	virtual	void			endProfileVBHardLock(std::vector<std::string> &result) NL_OVERRIDE;

	virtual	void			profileVBHardAllocation(std::vector<std::string> &result) NL_OVERRIDE;

	virtual	void			startProfileIBLock() NL_OVERRIDE;

	virtual	void			endProfileIBLock(std::vector<std::string> &result) NL_OVERRIDE;

	virtual	void			profileIBAllocation(std::vector<std::string> &result) NL_OVERRIDE;

	virtual	void			profileTextureUsage(std::vector<std::string> &result) NL_OVERRIDE;

	// @}


	/// \name Async Texture loading mgt
	// @{
	virtual void				setupAsyncTextureLod(uint baseLevel, uint maxLevel) NL_OVERRIDE;
	virtual void				setupAsyncTextureMaxUploadPerFrame(uint maxup) NL_OVERRIDE;
	virtual void				setupMaxTotalAsyncTextureSize(uint maxText) NL_OVERRIDE;
	virtual void				setupMaxHLSColoringPerFrame(uint maxCol) NL_OVERRIDE;
	virtual void				updateAsyncTexture() NL_OVERRIDE;
	virtual	uint				getTotalAsyncTextureSizeAsked() const NL_OVERRIDE;
	virtual	uint				getLastAsyncTextureSizeGot() const NL_OVERRIDE;
	virtual void				loadHLSBank(const std::string &fileName) NL_OVERRIDE;
	// @}

	virtual	bool				supportMADOperator() const NL_OVERRIDE;

	virtual	bool				supportBloomEffect() const NL_OVERRIDE;

	virtual	bool				supportGPUSkinning() const NL_OVERRIDE;

	virtual	bool				supportLargeUBOArrays() const NL_OVERRIDE;

	/// \name Bench
	// @{
	virtual void startBench (bool wantStandardDeviation = false, bool quick = false, bool reset = true) NL_OVERRIDE;
	virtual void endBench () NL_OVERRIDE;
	virtual void displayBench (class NLMISC::CLog *log) NL_OVERRIDE;
	// @}

	/// \name Water envmap
	// @{
	virtual UWaterEnvMap *createWaterEnvMap() NL_OVERRIDE;
	virtual void		  deleteWaterEnvMap(UWaterEnvMap *map) NL_OVERRIDE;
	// @}

	// Copy a string to system clipboard.
	virtual bool copyTextToClipboard(const std::string &text) NL_OVERRIDE;

	// Paste a string from system clipboard.
	virtual bool pasteTextFromClipboard(std::string &text) NL_OVERRIDE;

	virtual uint64	getSwapBufferCounter() NL_OVERRIDE;

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





















