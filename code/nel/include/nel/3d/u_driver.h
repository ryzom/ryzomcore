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

#ifndef NL_U_DRIVER_H
#define NL_U_DRIVER_H

#include "nel/misc/types_nl.h"
#include "viewport.h"
#include "scissor.h"
#include "frustum.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/matrix.h"
#include "nel/misc/rgba.h"
#include "nel/misc/rect.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/event_server.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/hierarchical_timer.h"
#include "primitive_profile.h"



namespace NLMISC
{
	class CLog;
}


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CMatrix;
using NLMISC::CRGBA;
using NLMISC::CBitmap;


class UScene;
class UCamera;
class UTextureFile;
class UTextureMem;
class UMaterial;
class UTextContext;
class UShapeBank;
class U3dMouseListener;
class ULight;
class UAnimationSet;
class UWaterEnvMap;
class CRenderTargetManager;

typedef void (*emptyProc)(void);

// ****************************************************************************
/// Monitor color properties
struct CMonitorColorProperties
{
	float		Contrast[3];		// Contrast parameters, RGB,		[-1.f ~ 1.f]
	float		Luminosity[3];		// Luminosity parameters, RGB,		[-1.f ~ 1.f]
	float		Gamma[3];			// Gamma parameters, RGB,			[-1.f ~ 1.f]
};

// ***************************************************************************
/**
 * Game Interface for window Driver, first object to create.
 * From UDriver, you can create Scene, and render basic primitives (slow!!)
 *
 * All function calls are invalid before init() is called!!
 *
 * NB: there is ONE FontManager per UDriver.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UDriver
{
public:
	/// A Graphic Mode descriptor.
	struct CMode
	{
		std::string			DisplayDevice;
		bool				Windowed;
		uint16				Width;
		uint16				Height;
		uint8				Depth;
		uint				Frequency;	// In hz. Default is Windows selection
		sint8				AntiAlias;	// -1 = no AA, 0 = max, 2 = 2x sample, 4 = 4x sample, ...

		CMode()
		{
			Windowed = false;
			Width = 0;
			Height = 0;
			Depth = 0;
			Frequency = 0;
			AntiAlias = -1;
		}
		CMode(uint16 w, uint16 h, uint8 d, bool windowed= true, uint frequency = 0, sint8 aa = -1, const std::string &displayDevice = std::string())
		{
			DisplayDevice = displayDevice;
			Windowed = windowed;
			Width = w;
			Height = h;
			Depth = d;
			Frequency = frequency;
			AntiAlias = aa;
		}
	};

	typedef std::vector<CMode> TModeList;


	/// Message Box enums.
	enum TMessageBoxId { okId=0, yesId, noId, abortId, retryId, cancelId, ignoreId, idCount };
	enum TMessageBoxType { okType=0, okCancelType, yesNoType, abortRetryIgnoreType, yesNoCancelType, retryCancelType, typeCount };
	enum TMessageBoxIcon { noIcon=0, handIcon, questionIcon, exclamationIcon, asteriskIcon, warningIcon, errorIcon, informationIcon, stopIcon, iconCount };

	/// Polygon Mode.
	enum TPolygonMode { Filled=0, Line, Point };

	// Cull mode
	enum TCullMode { CCW = 0, CW };

	// Stencil enums
	enum TStencilOp { keep = 0, zero, replace, incr, decr, invert };
	enum TStencilFunc { never = 0, less, lessequal, equal, notequal, greaterequal, greater, always};

	// Existing drivers
	enum TDriver { Direct3d = 0, OpenGl, OpenGlEs };

public:
	/// The EventServer of this driver. Init after setDisplay()!!
	NLMISC::CEventServer			EventServer;
	/// The AsyncListener of this driver. Init after setDisplay()!!
	NLMISC::CEventListenerAsync		AsyncListener;


public:

	/// \name Object
	// @{
	UDriver();
	virtual	~UDriver();
	// @}

	// Test if the device is lost. Can only happen with D3D.
	// The calling application may skip some part of its rendering when it is the case (this is not a requirement, but may save cpu for other applications)
	virtual	bool			isLost() const = 0;


	/// \name Disable Hardware Feature
	/**	Disable some Feature that may be supported by the Hardware
	 *	Call before setDisplay() to work properly
	 */
	// @{
	virtual void			disableHardwareVertexProgram()=0;
	virtual void			disableHardwarePixelProgram()=0;
	virtual void			disableHardwareVertexArrayAGP()=0;
	virtual void			disableHardwareTextureShader()=0;
	// @}


	/// \name Window / driver management.
	// @{

	/**
	  * create the window. call activate(). Return true if mode activated, false if it failed.
	  * \param show show or hide the window in window mode.
	  */
	virtual	bool			setDisplay(const CMode &mode, bool show = true, bool resizeable = true) =0;
	virtual	bool			setDisplay(nlWindow wnd, const CMode &mode, bool show = true, bool resizeable = true) =0;
	virtual bool			setMode(const CMode& mode)=0;
	virtual bool			getModes(std::vector<CMode> &modes)=0;
	virtual bool			getCurrentScreenMode(CMode &mode)=0;

	/// Set the title of the NeL window
	virtual void			setWindowTitle(const ucstring &title)=0;

	/// Set icon(s) of the NeL window
	virtual void			setWindowIcon(const std::vector<NLMISC::CBitmap> &bitmaps)=0;

	/// Set the position of the NeL window
	virtual void			setWindowPos(sint32 x, sint32 y)=0;

	/// Show or hide the NeL window
	virtual void			showWindow(bool show = true)=0;

	/* Pass in dialog box mode. After having called this method, you can use normal GUI.
	 * In fullscreen under direct3d, the main 3d window is minimized.
	 *
	 * \code
	 * Driver->beginDialogMode();
	 * CFileDialog fileDialog(TRUE);
	 * if (fileDialog.DoModal() == IDOK)
	 * {
	 *	...
	 * }
	 * Driver->endDialogMode();
	 * \endcode
	 */
	virtual void			beginDialogMode() =0;

	/* Leave the dialog box mode. After having called this method, you can't use normal GUI anymore.
	 * In fullscreen under direct3d, the main 3d window is maximized.
	 */
	virtual void			endDialogMode() =0;

	/// Release the window. All components are released (Texture, materials, scene, textcontexts).
	virtual	void			release() =0;

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate(void)=0;
	/// Return true if driver is still active. Return false else. If he user close the window, must return false.
	virtual bool			isActive()=0;

	/// Return an OS dependent window handle. Under Win32, it is a HWND.
	virtual nlWindow		getDisplay () = 0;
	// @}



	/// \name Buffers.
	// @{
	/// This clear only the RGBA back buffer
	virtual	void			clearRGBABuffer(CRGBA col= CRGBA(255,255,255,255)) =0;
	/// This clear only the Zbuffer
	virtual	void			clearZBuffer() =0;
	/// This clear the buffers (ALL the buffer :) )
	virtual	void			clearBuffers(CRGBA col= CRGBA(255,255,255,255)) =0;
	/// This swap the back and front buffer (ALL the buffer :) ).
	virtual	void			swapBuffers() =0;
	// Finish all commands
	virtual void            finish() = 0;
	// Flush the command buffer then immediately returns
	virtual void            flush() = 0;

	/** set the number of VBL wait when a swapBuffers() is issued. 0 means no synchronisation to the VBL
	 *	Default is 1. Values >1 may be clamped to 1 by the driver.
	 */
	virtual void			setSwapVBLInterval(uint interval) =0;
	/// get the number of VBL wait when a swapBuffers() is issued. 0 means no synchronisation to the VBL
	virtual uint			getSwapVBLInterval() =0;

	// @}



	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled()=0;
	virtual	void			enableFog(bool enable)=0;
	/// $ fog parameters. fog must enabled to see result. start and end are in [0,1] range.
	virtual	void			setupFog(float start, float end, CRGBA color)=0;
	// @}

	/// \name Light support.
	// @{
	virtual void			setLight (uint8 num, const ULight& light) = 0;
	virtual void			enableLight (uint8 num, bool enable=true) = 0;
	virtual void			setAmbientColor (CRGBA color) = 0;
	// @}

	/// \name Cull mode
	// @{
		virtual void			setCullMode(TCullMode cullMode) = 0;
		virtual	TCullMode       getCullMode() const = 0;
	// @}

	/// \name Stencil support
	// @{
	virtual void			enableStencilTest(bool enable) = 0;
	virtual bool			isStencilTestEnabled() const = 0;
	virtual void			stencilFunc(TStencilFunc stencilFunc, int ref, uint mask) = 0;
	virtual void			stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass) = 0;
	virtual void			stencilMask(uint mask) = 0;
	// @}

	/// \name Scene gestion.
	// @{
	/// Create a new scene.
	virtual	UScene			*createScene(bool bSmallScene) =0;
	/// Delete a scene.
	virtual	void			deleteScene(UScene	*scene) =0;
	// @}


	/// \name AnimationSet gestion.
	// @{
	/// Create an empty AnimationSet.
	virtual	UAnimationSet		*createAnimationSet(bool headerOptim = true) =0;
	/// Create a new AnimationSet, load it from a file. Use CPath to search the animation set.  exception EPathNotFound if not found.
	virtual	UAnimationSet		*createAnimationSet(const std::string &animationSetFile) =0;
	/// Delete a AnimationSet. NB: actually, this animation set is internally deleted only when no more UPlayList use it.
	virtual	void				deleteAnimationSet(UAnimationSet *animationSet) =0;
	// @}


	/// Get the render target manager
	virtual CRenderTargetManager	&getRenderTargetManager() =0;

	/// Set a texture of specified size or of the size of the window as render target
	virtual void					beginDefaultRenderTarget(uint32 width = 0, uint32 height = 0) =0;

	/// Draw the render target to the back buffer
	virtual void					endDefaultRenderTarget(UScene *scene) =0;


	/// \name Components gestion for Interface 2D/3D.
	// @{


	/// create a new TextContext, for a given font.
	virtual	UTextContext	*createTextContext(const std::string fontFileName, const std::string fontExFileName = "") =0;
	/// delete a TextContext.
	virtual	void			deleteTextContext(UTextContext	*textContext) =0;
	/// Set the maxMemory used for the FontManager
	virtual	void			setFontManagerMaxMemory(uint maxMem)=0;
	/// get cahce information.
	virtual		std::string getFontManagerCacheInformation() const =0;


	/** Create a new texture file, searching in CPath. NB: by default a textureFile created with this
	 *	method has a setAllowDegradation() at false.
	 * \param file filename, local to CPath paths.
	 */
	virtual	UTextureFile	*createTextureFile(const std::string &file) =0;
	/// Delete a texture file. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureFile(UTextureFile *textfile) =0;
	/// Create a new Raw texture, to be filled by user.
	virtual	UTextureMem		*createTextureMem(uint width, uint height, CBitmap::TType texType = CBitmap::RGBA) = 0;
	/// Delete a Raw texture. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureMem(UTextureMem *textraw) =0;
	/// Create a new Material, to be filled by user.
	virtual	UMaterial		createMaterial() =0;
	/// Delete a Material.
	virtual	void			deleteMaterial(UMaterial &mat) =0;
	// @}


	/// \name Matrix context for Interface 2D/3D.
	/** UScene ignore those function (use camera parameters instead, and UScene viewport), and do not disturb
	 * this active Matrix context. (after a scene rendering, the Matrix context for this interface is restored).
	 * Remarks are nearly same for UTextContext, except for UTextContext::render3D() (see doc):
	 *		- UTextContext use the setuped viewport of UDriver Matrix context.
	 *		- UTextContext use its own Matrix2D setup (own Frustum and own ViewAMtrix/ ModelMatrix).
	 *		- UTextContext restore ALL the matrix context, after any rendering function.
	 */
	// @{

	/** Set the active scissor for rendering. Default to fullscreen.
	 */
	virtual	void			setScissor(const CScissor &)=0;
	virtual	CScissor		getScissor()=0;
	/** Set the active viewport for rendering. Default to fullscreen.
	 */
	virtual	void			setViewport(const CViewport &)=0;
	virtual	CViewport		getViewport()=0;
	/** Set the active Frustum for rendering.
	 */
	virtual	void			setFrustum(const CFrustum &frust) =0;
	virtual	CFrustum		getFrustum() =0;
	virtual	void			setFrustumMatrix(CMatrix &frust) =0;
	virtual	CMatrix			getFrustumMatrix() =0;

	virtual float			getClipSpaceZMin() const = 0;

	/** Set the active ViewMatrix for rendering.
	 * NB: this is the view matrix, which is the inverse of camera matrix.
	 */
	virtual	void			setViewMatrix(const CMatrix &mat) =0;
	virtual	CMatrix			getViewMatrix() =0;
	/** Set the active ModelMatrix for rendering. NB: UScene ignore this function (use camera parameters instead).
	 */
	virtual	void			setModelMatrix(const CMatrix &mat) =0;
	virtual	CMatrix			getModelMatrix() =0;


	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 2D.
	 * ModelMatrix is setup to identity. ViewMatrix is setup so that (x,y) of vectors maps to x,y screen.
	 */
	virtual	void			setMatrixMode2D(const CFrustum &frust) =0;
	/// Tool function: same as setMatrixMode2D(), using a CFrustum(0,1,0,1,-1,1,false).
	void					setMatrixMode2D11();
	/// Tool function: same as setMatrixMode2D(), using a CFrustum(0,4/3,0,1,-1,1,false).
	void					setMatrixMode2D43();
	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 3D, using parameters of a UCamera.
	 * ModelMatrix setuped to identity. ViewMatrix setuped to the inverse of camera 's LocalMatrix.
	 * Frustum setuped to UCamera frustum.
	 */
	virtual	void			setMatrixMode3D(UCamera &camera) =0;
	/** Set depth range. Depth range specify a linear mapping from device z coordinates (in the [-1, 1] range) to window coordinates (in the [0, 1] range)
	  * This mapping occurs after clipping of primitives and division by w of vertices coordinates.
	  * Default range is [0, 1].
	  * NB : znear should be different from zfar or an assertion is raised
	  */
	virtual void			setDepthRange(float znear, float zfar) = 0;
	virtual void			getDepthRange(float & znear, float & zfar) = 0;

	/// Set the color mask filter through where the operation done will pass
	virtual void			setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha)=0;




	/// \name Interface 2D/3D.
	/** All of those render primitives are unlit! You must use UScene to render lighted meshes.
	 * NB: If you set a texture to your material, the primitives are textured, even if no Uvs are provided. \n
	 * NB: All rendering are done in current viewport / current matrix context.
	 */
	// @{

	/// Draw the Line, taking color from material.
	virtual	void			drawLine(const NLMISC::CLine &tri, UMaterial &mat) =0;
	/// Draw the Line, taking color from primitive.
	virtual	void			drawLine(const NLMISC::CLineColor &tri, UMaterial &mat) =0;
	/// Draw the Line, taking color from material. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineUV &tri, UMaterial &mat) =0;
	/// Draw the Line, taking color from primitive. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineColorUV &tri, UMaterial &mat) =0;

	/// Draw the Triangle, taking color from material.
	virtual	void			drawTriangle(const NLMISC::CTriangle &tri, UMaterial &mat) =0;
	/// Draw the Triangle, taking color from primitive.
	virtual	void			drawTriangle(const NLMISC::CTriangleColor &tri, UMaterial &mat) =0;
	/// Draw the Triangle, taking color from material. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleUV &tri, UMaterial &mat) =0;
	/// Draw the Triangle, taking color from primitive. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleColorUV &tri, UMaterial &mat) =0;

	/// Draw the Quad, taking color from material.
	virtual	void			drawQuad(const NLMISC::CQuad &tri, UMaterial &mat) =0;
	/// Draw the Quad, taking color from primitive.
	virtual	void			drawQuad(const NLMISC::CQuadColor &tri, UMaterial &mat) =0;
	/// Draw the Quad, taking color from material. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadUV &tri, UMaterial &mat) =0;
	/// Draw the Quad, taking color from primitive. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadColorUV &tri, UMaterial &mat) =0;

	virtual	void			drawQuads(const std::vector<NLMISC::CQuadColorUV> &quad, UMaterial &mat) =0;
	virtual	void			drawQuads(const std::vector<NLMISC::CQuadColorUV2> &quad, UMaterial &mat) =0;
	virtual	void			drawTriangles(const std::vector<NLMISC::CTriangleColorUV> &tris, UMaterial &mat) = 0;
	virtual	void			drawQuads(const NLMISC::CQuadColorUV *qs, uint32 nbq, UMaterial &mat) =0;
	virtual	void			drawQuads(const NLMISC::CQuadColorUV2 *quads, uint32 nbQuads, UMaterial &mat) =0;
	virtual	void			drawTriangles(const NLMISC::CTriangleColorUV *tris, uint32 nbTris, UMaterial &mat) = 0;

	// @}


	/// \name Tools for Interface 2D.
	/** For all those function, setMatrixMode2D*() should have been called (else strange results!!).
	 */
	// @{

	/// Draw a bitmap 2D. Warning: this is slow...
	virtual	void			drawBitmap (float x, float y, float width, float height, class UTexture& texture, bool blend=true, CRGBA col= CRGBA(255,255,255,255)) =0;
	/// Draw a line in 2D. Warning: this is slow...
	virtual	void			drawLine (float x0, float y0, float x1, float y1, CRGBA col= CRGBA(255,255,255,255)) =0;
	/// Draw a Triangle in 2D. Warning: this is slow...
	virtual	void			drawTriangle (float x0, float y0, float x1, float y1, float x2, float y2, CRGBA col) =0;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float x0, float y0, float x1, float y1, CRGBA col) =0;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float xcenter, float ycenter, float radius, CRGBA col) =0;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float x0, float y0, float x1, float y1, CRGBA col) =0;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float xcenter, float ycenter, float radius, CRGBA col) =0;

	// @}



	/// \name Driver information/Queries
	// @{
	/**
	  * Get the driver version. Not the same than interface version. Incremented at each implementation change.
	  *
	  * \see InterfaceVersion
	  */
	virtual uint32			getImplementationVersion () const = 0;

	/**
	  * Get driver information.
	  * get the nel name of the driver (ex: "Opengl 1.2 NeL Driver")
	  */
	virtual const char*		getDriverInformation () = 0;

	/**
	  * Get videocard information.
	  * get the official name of the driver
	  */
	virtual const char*		getVideocardInformation () = 0;

	/// Get the number of texture stage available, for multitexturing (Normal material shaders). Valid only after setDisplay().
	virtual	uint			getNbTextureStages() = 0;

	/// Get the width and the height of the window
	virtual void			getWindowSize (uint32 &width, uint32 &height) = 0;
	/// Get the width of the window
	virtual uint			getWindowWidth () =0;
	/// Get the height of the window
	virtual uint			getWindowHeight () =0;

	/// Get the x and y coord of the windows always (0,0) in fullscreen
	virtual void			getWindowPos (sint32 &x, sint32 &y) = 0;

	/** Return the amount of AGP memory allocated by initVertexArrayRange() to store vertices.
	*/
	virtual uint32			getAvailableVertexAGPMemory () =0;

	/** Return the amount of video memory allocated by initVertexArrayRange() to store vertices.
	*/
	virtual uint32			getAvailableVertexVRAMMemory () =0;

	/** get the RGBA back buffer. After swapBuffers(), the content of the back buffer is undefined.
	  *
	  * \param bitmap the buffer will be written in this bitmap
	  */
	virtual void			getBuffer (CBitmap &bitmap) = 0;

	/** get the ZBuffer (back buffer).
	  *
	  * \param zbuffer the returned array of Z. size of getWindowSize() .
	  */
	virtual void			getZBuffer (std::vector<float>  &zbuffer) = 0;

	/** get a part of the RGBA back buffer. After swapBuffers(), the content of the back buffer is undefined.
	  * NB: 0,0 is the bottom left corner of the screen.
	  *
	  * \param bitmap the buffer will be written in this bitmap
	  * \param rect the in/out (wanted/clipped) part of Color buffer to retrieve.
	  */
	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect) = 0;

	/** get a part of the ZBuffer (back buffer).
	  * NB: 0,0 is the bottom left corner of the screen.
	  *
	  * \param zbuffer the returned array of Z. size of rec.Width*rec.Height.
	  * \param rect the in/out (wanted/clipped) part of ZBuffer to retrieve.
	  */
	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect) = 0;

	/** fill the RGBA back buffer
	  *
	  * \param bitmap will be written in the buffer. no-op if bad size.
	  *	\return true if success
	  */
	virtual bool			fillBuffer (CBitmap &bitmap) = 0;

	// @}


	/// \name Mouse / Keyboard / Gamedevices
	// @{
		/** show cursor if b is true, or hide it if b is false
		  * NB: This has no effects if a low level mouse is used.
		  */
		virtual void			showCursor (bool b) = 0;

		/// x and y must be between 0.0 and 1.0
		virtual void			setMousePos (float x, float y) = 0;

		/** If true, capture the mouse to force it to stay under the window.
		  * NB : If a low level mouse is used, it does nothing
		  */
		virtual void			setCapture (bool b) = 0;

		// see if system cursor is currently captured
		virtual bool			isSystemCursorCaptured() = 0;

		// Add a new cursor (name is case unsensitive)
		virtual void			addCursor(const std::string &name, const NLMISC::CBitmap &bitmap) = 0;

		// Display a cursor from its name (case unsensitive)
		virtual void			setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild = false) = 0;

		// Change default scale for all cursors
		virtual void			setCursorScale(float scale) = 0;

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
	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon) =0;


	/** Set the global polygon mode. Can be filled, line or point. The implementation driver must
	  * call IDriver::setPolygonMode and active this mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see getPolygonMode(), TPolygonMode
	  */
	virtual void			setPolygonMode (TPolygonMode mode) =0;

	/** Get the global polygon mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see setPolygonMode(), TPolygonMode
	  */
	virtual TPolygonMode 	getPolygonMode () =0;

	/** Create a 3d mouse listener
	  *
	  * \return a 3d mouse listener.
	  */
	virtual U3dMouseListener*	create3dMouseListener () =0;

	/** Delete a 3d mouse listener
	  *
	  * \param listener a 3d mouse listener.
	  */
	virtual void delete3dMouseListener (U3dMouseListener *listener) =0;

	/** if true force all the uncompressed RGBA 32 bits and RGBA 24 bits texture to be DXTC5 compressed.
	 *	Default is false.
	 *	NB: this is done only on TextureFile, with format Automatic
	 */
	virtual void			forceDXTCCompression(bool dxtcComp)=0;

	/** if different from 0, enable anisotropic filter on textures. -1 enables max value.
	 *	Default is 0.
	 */
	virtual void			setAnisotropicFilter(sint filter)=0;

	/** if !=1, force mostly all the textures (but TextureFonts lightmaps, interfaces  etc..)
	 *	to be divided by Divisor (2, 4, 8...)
	 *	Default is 1.
	 *	NB: this is done only on TextureFile
	 */
	virtual void			forceTextureResize(uint divisor)=0;

	/** Setup monitor color properties.
	  *
	  * Return false if setup failed.
	  */
	virtual bool			setMonitorColorProperties (const CMonitorColorProperties &properties) = 0;

	// @}

	/// \name Shape Bank
	// @{
	/**
	  * Get the global shape bank. The shape bank handles all the shape caches.
	  * \see UShapeBank
	  */
	virtual	UShapeBank*		getShapeBank() = 0;
	// @}


	/// \name Profiling.
	// @{

	/** Get the number of primitives rendered from the last swapBuffers() call.
	 *	\param pIn the number of requested rendered primitive.
	 *	\param pOut the number of effective rendered primitive. pOut==pIn if no multi-pass material is used
	 *	(Lightmap, Specular ...).
	 */
	virtual	void			profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut) =0;

	/** Return the amount of Texture memory requested. taking mipmap, compression, texture format, etc... into account.
	 *	NB: because of GeForce*, RGB888 is considered to be 32 bits. So it may be false for others cards :).
	 */
	virtual	uint32			profileAllocatedTextureMemory() =0;

	/** Get the number of material setuped from the last swapBuffers() call.
	 */
	virtual	uint32			profileSetupedMaterials() const =0;

	/** Get the number of matrix setuped from the last swapBuffers() call.
	 */
	virtual	uint32			profileSetupedModelMatrix() const =0;

	/** Enable the sum of texture memory used since last swapBuffers() call. To retrieve the memory used call getUsedTextureMemory().
	 */
	virtual void			enableUsedTextureMemorySum (bool enable=true) =0;

	/** Return the amount of texture video memory used since last swapBuffers() call. Before use this method, you should enable
	 *  the sum with enableUsedTextureMemorySum().
	 */
	virtual uint32			getUsedTextureMemory() const =0;

	/** If the driver support it, enable profile VBHard locks.
	 *	No-Op if already profiling
	 */
	virtual	void			startProfileVBHardLock() = 0;

	/** If the driver support it, stop profile VBHard locks, and "print" result
	 *	No-Op if already profiling
	 *	NB: The results are the Locks in Chronogical time (since last swapBuffers).
	 *	Since multiple frame are summed, an "*" is marked againts the VBHard name to show if it was not
	 *	always this one (ptr test and not name test) in the chronogical order.
	 *	NB: if the driver does not support VBHard or VBHard profiling (like ATI VBHard), result is empty.
	 *	NB: ???? string is displayed if the VBHard has no name or if was just deleted.
	 */
	virtual	void			endProfileVBHardLock(std::vector<std::string> &result) = 0;

	/** display VBhards allocated
	 */
	virtual	void			profileVBHardAllocation(std::vector<std::string> &result) = 0;

	/** If the driver support it, enable profile index buffers
	 *	No-Op if already profiling
	 */
	virtual	void			startProfileIBLock() = 0;

	/** If the driver support it, stop profile index buffer locks, and "print" result
	 *	No-Op if already profiling
	 *	NB: The results are the Locks in Chronogical time (since last swapBuffers).
	 *	Since multiple frame are summed, an "*" is marked againts the index buffer name to show if it was not
	 *	always this one (ptr test and not name test) in the chronogical order.
	 *	NB: if the driver does not support index buffer profiling, result is empty.
	 *	NB: ???? string is displayed if the index buffer has no name or if was just deleted.
	 */
	virtual	void			endProfileIBLock(std::vector<std::string> &result) = 0;

	/** display index buffer allocated
	 */
	virtual	void			profileIBAllocation(std::vector<std::string> &result) = 0;

	/** For each texture setuped in the driver, "print" result: type, shareName, format and size (mipmap included)
	 */
	virtual	void			profileTextureUsage(std::vector<std::string> &result) =0;

	// @}


	/// \name Async Texture loading mgt (see UInstance)
	// @{
	/** setup the mipMap levels.
	 *	\baseLevel When the texture is first added, it is loaded skipping the baseLevel
	 *	first mipmap
	 *	\maxLevel During time, furhter mipmap are loaded, according to instance position etc... maxLevel
	 *	tells where to stop. If 0, the texture will finally be entirely uploaded.
	 *	Default is 3,1.
	 */
	virtual void				setupAsyncTextureLod(uint baseLevel, uint maxLevel) =0;
	/// Setup max texture upload in driver per updateAsyncTexture() call.
	virtual void				setupAsyncTextureMaxUploadPerFrame(uint maxup) =0;
	/// Setup max total texture size allowed. Default is 10Mo
	virtual void				setupMaxTotalAsyncTextureSize(uint maxText) =0;
	/// Setup max texture HLS Coloring per update() call (in bytes). Default to 20K.
	virtual void				setupMaxHLSColoringPerFrame(uint maxCol) =0;
	/** update the manager. New loaded texture are uploaded. Instances are updated to know if all their
	 *	pending textures have been uploaded.
	 */
	virtual void				updateAsyncTexture() =0;

	/// get the async texture Size asked (ie maybe bigger than MaxTotalTextureSize).
	virtual	uint				getTotalAsyncTextureSizeAsked() const =0;
	/// get what the system really allows
	virtual	uint				getLastAsyncTextureSizeGot() const =0;

	/** Load a .hlsBank, add it to the HLSManager of the AsyncTextureManager.
	 *	Use CPath::lookup. throw EPathNotFound if error
	 */
	virtual void				loadHLSBank(const std::string &fileName) =0;

	// @}

	// see if the tex env operator CMaterial::Mad is supported
	virtual	bool				supportMADOperator() const = 0;

	// check if bloom effect is supported
	virtual bool				supportBloomEffect() const = 0;

	/// \name Bench
	// @{

	// Start the bench. See CHTimer::startBench();
	virtual void startBench (bool wantStandardDeviation = false, bool quick = false, bool reset = true) =0;

	// End the bench. See CHTimer::endBench();
	virtual void endBench () =0;

	// Display the bench result
	virtual void displayBench (class NLMISC::CLog *log) =0;

	// @}

	/// \name Water envmap
	// @{
		// Create a new water envmap. Such an envmap must then be set in a UScene for the water surfaces of that scene to use it
		virtual UWaterEnvMap *createWaterEnvMap() = 0;
		// Delete a water envmap previously created with 'createWaterEnvMap'
		virtual void		  deleteWaterEnvMap(UWaterEnvMap *) = 0;
	// @}

	virtual uint64	getSwapBufferCounter() = 0;

	/// \name Clipboard management
	// @{
		// Copy a string to system clipboard.
		virtual bool copyTextToClipboard(const ucstring &text) =0;

		// Paste a string from system clipboard.
		virtual bool pasteTextFromClipboard(ucstring &text) =0;
	// @}

public:

	/**
	 *	This is the static function which build a UDriver, the root for all 3D functions.
	 */
	static	UDriver			*createDriver(uintptr_t windowIcon = 0, bool direct3d = false, emptyProc exitFunc = 0);
	static	UDriver			*createDriver(uintptr_t windowIcon, TDriver driver, emptyProc exitFunc = 0);

	/**
	 *	Purge static memory
	 */
	static	void			purgeMemory();

};


} // NL3D


#endif // NL_U_DRIVER_H

/* End of u_driver.h */
