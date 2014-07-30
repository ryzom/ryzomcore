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

#ifndef NL_DRV_H
#define NL_DRV_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/matrix.h"
#include "nel/misc/stream.h"
#include "nel/misc/uv.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/texture.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/vertex_program.h"
#include "nel/3d/pixel_program.h"
#include "nel/3d/geometry_program.h"
#include "nel/3d/gpu_program_params.h"
#include "nel/3d/material.h"
#include "nel/misc/mutex.h"
#include "nel/3d/primitive_profile.h"

#include <vector>
#include <list>

namespace NLMISC
{
class IEventEmitter;
struct IMouseDevice;
struct IKeyboardDevice;
struct IInputDeviceManager;
class CRect;
class CLog;
}

namespace NL3D
{

using NLMISC::CRefPtr;
using NLMISC::CRefCount;
using NLMISC::CSmartPtr;
using NLMISC::CRGBA;
using NLMISC::CVector;
using NLMISC::CMatrix;
using NLMISC::CSynchronized;


class CMaterial;
class CIndexBuffer;
class CLight;
class CScissor;
class CViewport;
struct CMonitorColorProperties;
struct IOcclusionQuery;



// ****************************************************************************
/// A Graphic Mode descriptor.
struct GfxMode
{
	bool				OffScreen;
	bool				Windowed;
	uint16				Width;
	uint16				Height;
	uint8				Depth;
	uint				Frequency;	// In hz. Default is Windows selection
	sint8				AntiAlias;	// -1 = no AA, 0 = max, 2 = 2x sample, 4 = 4x sample, ...

	GfxMode()
	{
		OffScreen=false;
		Windowed=false;
		Width = 0;
		Height = 0;
		Depth = 0;
		Frequency = 0;
		AntiAlias = -1;
	}
	GfxMode(uint16 w, uint16 h, uint8 d, bool windowed = true, bool offscreen = false, uint frequency = 0, sint8 aa = -1);
};

// ****************************************************************************
// Exceptions.
struct EBadDisplay : public NLMISC::Exception
{
	EBadDisplay(const std::string &reason) : Exception(reason) { }
};

// ****************************************************************************
typedef void (*emptyProc)(void);

// ****************************************************************************
// *** IMPORTANT ********************
// *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
// **********************************
//
// * Driver implementation notes:
// *
// * Driver implementation must save monitor color parameters at initialization and restore it at release.
class IDriver : public NLMISC::CRefCount
{
public:
	/// Version of the driver interface. To increment when the interface change.
	static const uint32						InterfaceVersion;

public:
	enum TMessageBoxId { okId = 0, yesId, noId, abortId, retryId, cancelId, ignoreId, idCount };
	enum TMessageBoxType { okType = 0, okCancelType, yesNoType, abortRetryIgnoreType, yesNoCancelType, retryCancelType, typeCount };
	enum TMessageBoxIcon { noIcon = 0, handIcon, questionIcon, exclamationIcon, asteriskIcon, warningIcon, errorIcon, informationIcon, stopIcon, iconCount };
	enum TCullMode { CCW = 0, CW };
	enum TStencilOp { keep = 0, zero, replace, incr, decr, invert };
	enum TStencilFunc { never = 0, less, lessequal, equal, notequal, greaterequal, greater, always};

	/**
	  * Driver's polygon modes.
	  *
	  * \see setPolygonMode, getPolygonMode
	  */
	enum TPolygonMode { Filled = 0, Line, Point };

	/**
	  * Driver Max matrix count.
	  *	Kept for backward compatibility. Suppose any Hardware VertexProgram can handle only 16 matrix
	  *
	  */
	enum TMatrixCount { MaxModelMatrix = 16 };

	enum TMatrix
	{
		ModelView= 0,
		Projection,
		ModelViewProjection,
		NumMatrix
	};

	enum TTransform
	{
		Identity=0,
		Inverse,
		Transpose,
		InverseTranspose,
		NumTransform
	};

	enum TProgram
	{
		VertexProgram = 0,
		PixelProgram = 1,
		GeometryProgram = 2
	};

protected:
	CSynchronized<TTexDrvInfoPtrMap>	_SyncTexDrvInfos;
	TTexDrvSharePtrList					_TexDrvShares;
	TMatDrvInfoPtrList					_MatDrvInfos;
	TVBDrvInfoPtrList					_VBDrvInfos;
	TIBDrvInfoPtrList					_IBDrvInfos;
	TGPUPrgDrvInfoPtrList				_GPUPrgDrvInfos;

	TPolygonMode			_PolygonMode;

	uint					_ResetCounter;

public:
							IDriver();
	virtual					~IDriver();

	virtual bool			init(uintptr_t windowIcon = 0, emptyProc exitFunc = 0) = 0;

	/// Deriver should calls IDriver::release() first, to destroy all driver components (textures, shaders, VBuffers).
	virtual bool			release();

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate() = 0;

	// Test if the device is lost. Can only happen with D3D.
	// The calling application may skip some part of its rendering when it is the case (this is not a requirement, but may save cpu for other applications)
	virtual	bool			isLost() const = 0;

	/// Return true if driver is still active. Return false else. If he user close the window, must return false.
	virtual bool			isActive() = 0;



	/** Return the driver reset counter.
	 *  The reset counter is incremented at each driver reset.
	 */
	uint					getResetCounter() const { return _ResetCounter; }

	// get the number of call to swapBuffer since the driver was created
	virtual uint64			getSwapBufferCounter() const = 0;



	/// \name Disable Hardware Feature
	/**	Disable some Feature that may be supported by the Hardware
	 *	Call before setDisplay() to work properly
	 */
	// @{
	virtual void			disableHardwareVertexProgram() = 0;
	virtual void			disableHardwarePixelProgram() = 0;
	virtual void			disableHardwareVertexArrayAGP() = 0;
	virtual void			disableHardwareTextureShader() = 0;
	// @}



	/// \name Windowing
	// @{
	// first param is the associated window.
	// Must be a HWND for Windows (WIN32).
	virtual bool			setDisplay(nlWindow wnd, const GfxMode& mode, bool show = true, bool resizeable = true) throw(EBadDisplay) = 0;
	// Must be called after a setDisplay that initialize the mode
	virtual bool			setMode(const GfxMode &mode) = 0;
	virtual bool			getModes(std::vector<GfxMode> &modes) = 0;

	/// Set the title of the NeL window
	virtual void			setWindowTitle(const ucstring &title) = 0;
	/// Set icon(s) of the NeL window
	virtual void			setWindowIcon(const std::vector<NLMISC::CBitmap> &bitmaps) = 0;
	/// Set the position of the NeL window
	virtual void			setWindowPos(sint32 x, sint32 y) = 0;
	/// Show or hide the NeL window
	virtual void			showWindow(bool show) = 0;

	/// return the current screen mode (if we are in windowed, return the screen mode behind the window)
	virtual bool			getCurrentScreenMode(GfxMode &mode) = 0;

	/// enter/leave the dialog mode
	virtual void			beginDialogMode() = 0;
	virtual void			endDialogMode() = 0;

	// Return is the associated window information. (Implementation dependent)
	// Must be a HWND for Windows (WIN32).
	virtual nlWindow		getDisplay() = 0;

	/// Setup monitor color properties. Return false if setup failed
	virtual bool			setMonitorColorProperties(const CMonitorColorProperties &properties) = 0;

	// Return is the associated default window proc for the driver. (Implementation dependent)
	// Must be a void GlWndProc(IDriver *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) for Windows (WIN32).
	virtual emptyProc		getWindowProc() = 0;

	virtual NLMISC::IEventEmitter *getEventEmitter() = 0;
		
	/// Copy a string to system clipboard.
	virtual bool			copyTextToClipboard(const ucstring &text) = 0;

	/// Paste a string from system clipboard.
	virtual bool			pasteTextFromClipboard(ucstring &text) = 0;/// Return the depth of the driver after init().

	virtual uint8			getBitPerPixel() = 0;

	/** Output a system message box and print a message with an icon. This method can be call even if the driver is not initialized.
	  * This method is used to return internal driver problem when string can't be displayed in the driver window.
	  * If the driver can't open a messageBox, it should not override this method and let the IDriver class manage it with the ASCII console.
	  *
	  * \param message This is the message to display in the message box.
	  * \param title This is the title of the message box.
	  * \param type This is the type of the message box, ie number of button and label of buttons.
	  * \param icon This is the icon of the message box should use like warning, error etc...
	  */
	virtual TMessageBoxId	systemMessageBox(const char *message, const char *title, TMessageBoxType type = okType, TMessageBoxIcon icon = noIcon);

	/// Get the width and the height of the window
	virtual void			getWindowSize(uint32 &width, uint32 &height) = 0;

	/// Get the position of the window always (0,0) in fullscreen
	virtual void			getWindowPos(sint32 &x, sint32 &y) = 0;
	// @}



	/// \name Framebuffer operations
	// @{
	/// Clear the current target surface pixels. The function ignores the viewport settings but uses the scissor.
	virtual bool			clear2D(CRGBA rgba) = 0;

	/// Clear the current target surface zbuffer. The function ignores the viewport settings but uses the scissor.
	virtual bool			clearZBuffer(float zval=1) = 0;

	/// Clear the current target surface stencil buffer. The function ignores the viewport settings but uses the scissor.
	virtual bool			clearStencilBuffer(float stencilval=0) = 0;

	/// Set the color mask filter through where the operation done will pass
	virtual void			setColorMask(bool bRed, bool bGreen, bool bBlue, bool bAlpha) = 0;
	// @}



	/// \name Copy framebuffer to memory
	// @{
	/** get the RGBA back buffer. After swapBuffers(), the content of the back buffer is undefined.
	  *
	  * \param bitmap the buffer will be written in this bitmap
	  */
	virtual void			getBuffer(CBitmap &bitmap) = 0;

	/** get the ZBuffer (back buffer).
	  *
	  * \param zbuffer the returned array of Z. size of getWindowSize() .
	  */
	virtual void			getZBuffer(std::vector<float> &zbuffer) = 0;

	/** get a part of the RGBA back buffer. After swapBuffers(), the content of the back buffer is undefined.
	  * NB: 0,0 is the bottom left corner of the screen.
	  *
	  * \param bitmap the buffer will be written in this bitmap
	  * \param rect the in/out (wanted/clipped) part of Color buffer to retrieve.
	  */
	virtual void			getBufferPart(CBitmap &bitmap, NLMISC::CRect &rect) = 0;

	/** get a part of the ZBuffer (back buffer).
	  * NB: 0,0 is the bottom left corner of the screen.
	  *
	  * \param zbuffer the returned array of Z. size of rec.Width*rec.Height.
	  * \param rect the in/out (wanted/clipped) part of ZBuffer to retrieve.
	  */
	virtual void			getZBufferPart(std::vector<float> &zbuffer, NLMISC::CRect &rect) = 0;
	// @}



	/// \name Copy memory to framebuffer
	// @{
	/** fill the RGBA back buffer
	  *
	  * \param bitmap will be written in the buffer. no-op if bad size.
	  *	\return true if success
	  */
	virtual bool			fillBuffer(CBitmap &bitmap) = 0;
	// @}



	/// \name Viewport depth clipping
	// @{
	/** Set depth range. Depth range specify a linear mapping from device z coordinates (in the [-1, 1] range) to window coordinates (in the [0, 1] range)
	  * This mapping occurs after clipping of primitives and division by w of vertices coordinates.
	  * Default depth range is [0, 1].
	  * NB : znear should be different from zfar or an assertion is raised
	  */
	virtual void			setDepthRange(float znear, float zfar) = 0;
	// Get the current depth range
	virtual	void			getDepthRange(float &znear, float &zfar) const = 0;
	// @}

	

	/// \name Textures
	// @{
	/** is the texture is set up in the driver
	 *	NB: this method is thread safe.
	 */
	virtual bool			isTextureExist(const ITexture &tex) = 0;

	/** setup a texture, generate and upload if needed. same as setupTextureEx(tex, true, dummy);
	 */
	virtual bool			setupTexture(ITexture &tex) = 0;

	/** setup a texture in the driver.
	 *	\param bUpload if true the texture is created and uploaded to VRAM, if false the texture is only created
	 *  it is useful for the async upload texture to only create the texture and then make invalidate to upload
	 *  small piece each frame. There is ONE case where bUpload is forced to be true inside the method: if the texture
	 *	must be converted to RGBA. \see bAllUploaded
	 *	\param bAllUploaded true if any upload arise (texture invalid, must convert texture etc...).
	 *	\param bMustRecreateSharedTexture if true and if the texture supportSharing, then the texture is recreated
	 *	(and uploaded if bUpload==true) into the shared DrvInfo (if found). Default setup (false) imply that the DrvInfo
	 *	is only bound to tex (thus creating and uploading nothing)
	 *	NB: the texture must be at least touch()-ed for the recreate to work.
	 */
	virtual bool			setupTextureEx(	ITexture &tex, bool bUpload, bool &bAllUploaded,
											bool bMustRecreateSharedTexture = false) = 0;

	/** The texture must be created or uploadTexture do nothing.
	 *  These function can be used to upload piece by piece a texture. Use it in conjunction with setupTextureEx(..., false);
	 *  For compressed textures, the rect must aligned on pixel block. (a block of pixel size is 4x4 pixels).
	 */
	virtual bool			uploadTexture(ITexture &tex, NLMISC::CRect &rect, uint8 nNumMipMap) = 0;
	virtual bool			uploadTextureCube(ITexture &tex, NLMISC::CRect &rect, uint8 nNumMipMap, uint8 nNumFace) = 0;

	/**
	  * Invalidate shared texture
	  */
	bool					invalidateShareTexture(ITexture &);

	/**
	  * Get the driver share texture name
	  */
	static void				getTextureShareName(const ITexture &tex, std::string &output);

	/** if true force all the uncompressed RGBA 32 bits and RGBA 24 bits texture to be DXTC5 compressed.
	 *	Do this only during upload if ITexture::allowDegradation() is true and if ITexture::UploadFormat is "Automatic"
	 *	and if bitmap format is RGBA.
	 */
	virtual void			forceDXTCCompression(bool dxtcComp) = 0;

	/** if different from 0, enable anisotropic filter on textures. -1 enables max value.
	 *	Default is 0.
	 */
	virtual void			setAnisotropicFilter(sint filter) = 0;

	/** if !=1, force mostly all the textures (but TextureFonts lightmaps, interfaces  etc..)
	 *	to be divided by Divisor (2, 4, 8...)
	 *	Default is 1.
	 *	NB: this is done only on TextureFile
	 */
	virtual void			forceTextureResize(uint divisor) = 0;

	/** Get the number of texture stage available, for multi texturing (Normal material shaders). Valid only after setDisplay().
	 */
	virtual	uint			getNbTextureStages() const = 0;

	/** Get max number of per stage constant that can be used simultaneously.
	  * This will usually match the number of texture stages, but with a D3D driver, this feature is not available most of the time
	  * so it is emulated. If pixel shaders are available this will be fully supported.
	  * Under OpenGL this simply returns the maximum number of texture stages (getNbTextureStages) in both return values.
	  */
	virtual void			getNumPerStageConstant(uint &lightedMaterial, uint &unlightedMaterial) const = 0;
	
	// [DEPRECATED] Return if this texture is a rectangle texture that requires RECT sampler (OpenGL specific pre-NPOT functionality)
	virtual bool			isTextureRectangle(ITexture *tex) const = 0;

	// Return true if driver support non-power of two textures
	virtual	bool			supportNonPowerOfTwoTextures() const = 0;
	// @}

	

	/// \name Texture operations
	// @{
	// copy the first texture in a second one of different dimensions
	virtual bool			stretchRect(ITexture *srcText, NLMISC::CRect &srcRect, ITexture *destText, NLMISC::CRect &destRect) = 0;
	// @}



	/// \name Material
	// @{
	virtual bool			setupMaterial(CMaterial &mat) = 0;

	/** Special for Faster Specular Setup. Call this between lot of primitives rendered with Specular Materials.
	 *	Visual Errors may arise if you don't correctly call endSpecularBatch().
	 */
	virtual void			startSpecularBatch() = 0;
	virtual void			endSpecularBatch() = 0;

	/// \name Material multipass.
	/**	NB: setupMaterial() must be called before those methods.
	 *  NB: This is intended to be use with the rendering of simple primitives.
	 *  NB: Other render calls performs the needed setup automatically
	 */
	// @{
	/// init multipass for _CurrentMaterial. return number of pass required to render this material.
	virtual sint			beginMaterialMultiPass() = 0;
	/// active the ith pass of this material.
	virtual void			setupMaterialPass(uint pass) = 0;
	/// end multipass for this material.
	virtual void			endMaterialMultiPass() = 0;
	// @}

	// Does the driver support the per-pixel lighting shader ?
	virtual bool supportPerPixelLighting(bool specular) const = 0;
	// @}



	/// \name Camera
	// @{
	// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective = true) = 0;
	virtual	void			setFrustumMatrix(CMatrix &frust) = 0;
	virtual	CMatrix			getFrustumMatrix() = 0;

	virtual float			getClipSpaceZMin() const = 0;

	/** setup the view matrix (inverse of camera matrix).
	 *
	 * NB: you must setupViewMatrix() BEFORE setupModelMatrix(), or else undefined results.
	 */
	virtual void			setupViewMatrix(const CMatrix &mtx)=0;

	/** setup the view matrix (inverse of camera matrix).
	 *	Extended: give a cameraPos (mtx.Pos() is not taken into account but for getViewMatrix()),
	 *	so the driver use it to remove translation from all ModelMatrixes (and lights pos).
	 *	This approach improves greatly ZBuffer precision.
	 *
	 *	This is transparent to user, and getViewMatrix() return mtx (as in setupViewMatrix()).
	 *
	 * NB: you must setupViewMatrixEx() BEFORE setupModelMatrix(), or else undefined results.
	 *
	 * \param mtx the same view matrix (still with correct "inversed" camera position) as if passed in setupViewMatrix()
	 * \param cameraPos position of the camera (before inversion, ie mtx.getPos()!=cameraPos ).
	 */
	virtual void			setupViewMatrixEx(const CMatrix &mtx, const CVector &cameraPos) = 0;

	/** setup the model matrix.
	 *
	 * NB: you must setupModelMatrix() AFTER setupViewMatrix(), or else undefined results.
	 */
	virtual void			setupModelMatrix(const CMatrix &mtx) = 0;

	virtual CMatrix			getViewMatrix() const = 0;
	// @}



	/// \name Fixed pipeline vertex program
	// @{
	/** Force input normal to be normalized by the driver. default is false.
	 * NB: driver force the normalization himself if:
	 *		- current Model matrix has a scale.
	 */
	virtual	void			forceNormalize(bool normalize) = 0;


	/** return the forceNormalize() state.
	 */
	virtual	bool			isForceNormalize() const = 0;
	// @}

	

	/// \name Vertex Buffer Hard: Features
	// @{
	/** return true if driver support VertexBufferHard.
	 */
	virtual	bool			supportVertexBufferHard() const = 0;

	/** return true if volatile vertex buffer are supported. (e.g a vertex buffer which can be created with the flag CVertexBuffer::AGPVolatile or CVertexBuffer::RAMVolatile)
	 *  If these are not supported, a RAM vb is created instead (transparent to user)
     */
	virtual bool			supportVolatileVertexBuffer() const = 0;

	/** return true if driver support indices offset. That is, allow to specify a constant value that is added to each
	  * index in current active active index buffer when rendering indexed primitives
	  */
	virtual bool			supportIndexOffset() const = 0;

	/** return true if driver support VertexBufferHard, but vbHard->unlock() are slow (ATI-openGL).
	 */
	virtual	bool			slowUnlockVertexBufferHard() const = 0;
	// @}



	/// \name Vertex Buffer Hard: Settings
	// @{
	/* Returns true if static vertex and index buffers must by allocated in VRAM, false in AGP.
	 * Default is false.
	 */
	bool					getStaticMemoryToVRAM() const { return _StaticMemoryToVRAM; }

	/* Set to true if static vertex and index buffers must by allocated in VRAM, false in AGP.
	 * Default is false.
	 */
	void					setStaticMemoryToVRAM(bool staticMemoryToVRAM);

	/** return How many vertices VertexBufferHard support
	 */
	virtual	uint			getMaxVerticesByVertexBufferHard() const = 0;
	// @}



	/// \name Vertex Buffer Hard
	// @{
	/** Allocate the initial VertexArray Memory. (no-op if !supportVertexBufferHard()).
	 *	VertexArrayRange is first reseted, so any VBhard created before will be deleted.
	 *	NB: call it after setDisplay(). But setDisplay() by default call initVertexBufferHard(16Mo, 0);
	 *	so this is not necessary.
	 *	NB: If allocation fails, mem/=2, and retry, until mem < 500K.
	 *	\param agpMem amount of AGP Memory required. if 0, reseted.
	 *	\param vramMem amount of VRAM Memory required. if 0, reseted.
	 *	\return false if one the Buffer has not been allocated (at least at 500K).
	 */
	virtual	bool			initVertexBufferHard(uint agpMem, uint vramMem = 0) = 0;

	/** Return the amount of AGP memory allocated by initVertexBufferHard() to store vertices.
	*/
	virtual uint32			getAvailableVertexAGPMemory() = 0;

	/** Return the amount of video memory allocated by initVertexBufferHard() to store vertices.
	*/
	virtual uint32			getAvailableVertexVRAMMemory() = 0;
	// @}



	/// \name Vertex Buffer Objects
	// @{
	/** active a current VB, for future render().
	 * This method suppose that all vertices in the VB will be used.
	 *
	 * NB: please make sure you have setuped / unsetuped the current vertex program BEFORE activate the vertex buffer.
	 * Don't change the vertex buffer format/size after having activated it.
	 * Don't lock the vertex buffer after having activated it.
	 *
	 * \see activeVertexProgram
	 */
	virtual bool			activeVertexBuffer(CVertexBuffer &VB) = 0;

	/** active a current IB, for future render().
	 *
	 * Don't change the index buffer format/size after having activated it.
	 * Don't lock the index buffer after having activated it.
	 */
	virtual bool			activeIndexBuffer(CIndexBuffer &IB) = 0;
	// @}



	/// \name Rendering
	// @{
	/** Render a list of indexed lines with previously setuped VertexBuffer / IndexBuffer / Matrixes.
	 *  \param mat is the material to use during this rendering
	 *  \param firstIndex is the first index in the index buffer to use as first line.
	 *  \param nlines is the number of line to render.
	 */
	virtual bool			renderLines(CMaterial &mat, uint32 firstIndex, uint32 nlines) = 0;

	/** Render a list of indexed triangles with previously setuped VertexBuffer / IndexBuffer / Matrixes.
	 *  \param mat is the material to use during this rendering
	 *  \param firstIndex is the first index in the index buffer to use as first triangle.
	 *  \param ntris is the number of triangle to render.
	 */
	virtual bool			renderTriangles(CMaterial &mat, uint32 firstIndex, uint32 ntris) = 0;

	/** Render a list of triangles with previously setuped VertexBuffer / IndexBuffer / Matrixes, AND previously setuped MATERIAL!!
	 * This use the last material setuped. It should be a "Normal shader" material, because no multi-pass is allowed
	 * with this method.
	 * Actually, it is like a straight drawTriangles() in OpenGL.
	 * NB: nlassert() if ntris is 0!!!! this is unlike other render() call methods. For optimisation concern.
	 * NB: this is useful for landscape....
	 *  \param firstIndex is the first index in the index buffer to use as first triangle.
	 *  \param ntris is the number of triangle to render.
	 */
	virtual bool			renderSimpleTriangles(uint32 firstIndex, uint32 ntris) = 0;

	/** Render points with previously setuped VertexBuffer / Matrixes.
	 *  Points are stored as a sequence in the vertex buffer.
	 *  \param mat is the material to use during this rendering
	 *  \param startVertex is the first vertex to use during this rendering.
	 *  \param numPoints is the number of point to render.
	 */
	virtual bool			renderRawPoints(CMaterial &mat, uint32 startVertex, uint32 numPoints) = 0;

	/** Render lines with previously setuped VertexBuffer / Matrixes.
	 *  Lines are stored as a sequence in the vertex buffer.
	 *  \param mat is the material to use during this rendering
	 *  \param startVertex is the first vertex to use during this rendering.
	 *  \param numLine is the number of line to render.
	 */
	virtual bool			renderRawLines(CMaterial &mat, uint32 startVertex, uint32 numTri) = 0;

	/** Render triangles with previously setuped VertexBuffer / Matrixes.
	 *  Triangles are stored as a sequence in the vertex buffer.
	 *  \param mat is the material to use during this rendering
	 *  \param startVertex is the first vertex to use during this rendering.
	 *  \param numTri is the number of triangle to render.
	 */
	virtual bool			renderRawTriangles(CMaterial &mat, uint32 startVertex, uint32 numTri) = 0;

	/** If the driver support it, primitive can be rendered with an offset added to each index
      * These are the offseted version of the 'render' functions
	  * \see supportIndexOffset
	  */
	virtual bool			renderLinesWithIndexOffset(CMaterial &mat, uint32 firstIndex, uint32 nlines, uint indexOffset) = 0;
	virtual bool			renderTrianglesWithIndexOffset(CMaterial &mat, uint32 firstIndex, uint32 ntris, uint indexOffset) = 0;
	virtual bool			renderSimpleTrianglesWithIndexOffset(uint32 firstIndex, uint32 ntris, uint indexOffset) = 0;


	/** render quads with previously setuped VertexBuffer / Matrixes.
	 *  Quads are stored as a sequence in the vertex buffer.
	 * There's a guaranty for the orientation of its diagonal, which is drawn as follow :
     *
	 *  3----2
     *  |  / |
	 *  | /  |
	 *  |/   |
	 *  0----1
	 *
	 *  \param mat is the material to use during this rendering
	 *  \param startVertex is the first vertex to use during this rendering.
	 *  \param numQuad is the number of quad to render.
	 */
	virtual bool			renderRawQuads(CMaterial &mat, uint32 startVertex, uint32 numQuads) = 0;
	// @}



	/// \name Texture coordinates fixed pipeline
	// @{
	/** Say what Texture Stage use what UV coord.
	 *	by default activeVertexBuffer*() methods map all stage i to UV i. You can change this behavior,
	 *	after calling activeVertexBuffer*(), by using this method.
	 *
	 *	eg: mapTextureStageToUV(0,2) will force the 0th texture stage to use the 2th UV.
	 *
	 *	Warning! This DOESN'T work with VertexProgram enabled!! (assert)
	 *
	 *	Warning!: some CMaterial Shader may change automatically this behavior too when setupMaterial()
	 *	(and so render*()) is called. But Normal shader doesn't do it.
	 */
	virtual	void			mapTextureStageToUV(uint stage, uint uv) = 0;
	// @}



	/// \name Buffer swapping
	// @{
	/// Swap the back and front buffers.
	virtual bool			swapBuffers() = 0;

	/** set the number of VBL wait when a swapBuffers() is issued. 0 means no synchronisation to the VBL
	 *	Default is 1. Values >1 may be clamped to 1 by the driver.
	 */
	virtual void			setSwapVBLInterval(uint interval) = 0;
	/// get the number of VBL wait when a swapBuffers() is issued. 0 means no synchronisation to the VBL
	virtual uint			getSwapVBLInterval() = 0;
	// @}

	



	/// \name Profiling.
	// @{
	/** Get the number of primitives rendered from the last swapBuffers() call.
	 *	\param pIn the number of requested rendered primitive.
	 *	\param pOut the number of effective rendered primitive. pOut==pIn if no multi-pass material is used
	 *	(Lightmap, Specular ...).
	 */
	virtual	void			profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut) = 0;

	/** Return the amount of Texture memory requested. taking mipmap, compression, texture format, etc... into account.
	 *	NB: because of GeForce*, RGB888 is considered to be 32 bits. So it may be false for others cards :).
	 */
	virtual	uint32			profileAllocatedTextureMemory() = 0;

	/** Get the number of material setuped from the last swapBuffers() call.
	 */
	virtual	uint32			profileSetupedMaterials() const = 0;

	/** Get the number of matrix setuped from the last swapBuffers() call.
	 */
	virtual	uint32			profileSetupedModelMatrix() const = 0;

	/** Enable the sum of texture memory used since last swapBuffers() call. To retrieve the memory used call getUsedTextureMemory().
	 */
	virtual void			enableUsedTextureMemorySum (bool enable = true) = 0;

	/** Return the amount of texture video memory used since last swapBuffers() call. Before use this method, you should enable
	 *  the sum with enableUsedTextureMemorySum().
	 */
	virtual uint32			getUsedTextureMemory() const = 0;

	/** If the driver support it, enable profile VBHard locks.
	 *	No-Op if already profiling
	 */
	virtual	void			startProfileVBHardLock() = 0;

	/** If the driver support it, stop profile VBHard locks, and "print" result
	 *	No-Op if already profiling
	 *	NB: The results are the Locks in Chronological time (since last swapBuffers).
	 *	Since multiple frame are summed, an "*" is marked against the VBHard name to show if it was not
	 *	always this one (ptr test and not name test) in the chronological order.
	 *	NB: if the driver does not support VBHard or VBHard profiling (like ATI VBHard), result is empty.
	 *	NB: ???? string is displayed if the VBHard has no name or if was just deleted.
	 */
	virtual	void			endProfileVBHardLock(std::vector<std::string> &result) = 0;

	/** display VBhards allocated
	 */
	virtual	void			profileVBHardAllocation(std::vector<std::string> &result) = 0;

	// Index buffer profiling, same use than with vertex buffers
	virtual	void			startProfileIBLock() = 0;
	virtual	void			endProfileIBLock(std::vector<std::string> &result) = 0;
	virtual	void			profileIBAllocation(std::vector<std::string> &result) = 0;

	/** For each texture setuped in the driver, "print" result: type, shareName, format and size (mipmap included)
	 */
	void					profileTextureUsage(std::vector<std::string> &result);
	// @}



	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled() = 0;
	virtual	void			enableFog(bool enable = true) = 0;
	/// setup fog parameters. fog must enabled to see result. start and end are distance values.
	virtual	void			setupFog(float start, float end, NLMISC::CRGBA color) = 0;
	/// Get.
	virtual	float			getFogStart() const = 0;
	virtual	float			getFogEnd() const = 0;
	virtual	NLMISC::CRGBA	getFogColor() const = 0;
	// @}



	/// \name Viewport
	// @{
	/** Set the current viewport
	  *
	  * \param viewport is a viewport to setup as current viewport.
	  */
	virtual void			setupViewport(const class CViewport &viewport) = 0;

	/** Get the current viewport
	  */
	virtual	void			getViewport(CViewport &viewport) = 0;

	/** Set the current Scissor.
	  * \param scissor is a scissor to setup the current Scissor, in Window relative coordinate (0,1).
	  */
	virtual void			setupScissor(const class CScissor &scissor) = 0;
	// @}



	/// \name Driver information
	// @{
	/**
	  * Get the driver version. Not the same than interface version. Incremented at each implementation change.
	  *
	  * \see InterfaceVersion
	  */
	virtual uint32			getImplementationVersion() const = 0;

	/**
	  * Get driver information.
	  * get the nel name of the driver (ex: "Opengl 1.2 NeL Driver")
	  */
	virtual const char		*getDriverInformation() = 0;

	/**
	  * Get videocard information.
	  * get the official name of the driver
	  */
	virtual const char		*getVideocardInformation () = 0;
	// @}



	/// \name Mouse / Keyboard / Game devices
	// @{
	/// show cursor if b is true, or hide it if b is false
	virtual void			showCursor(bool b) = 0;

	/// x and y must be between 0.0 and 1.0
	virtual void			setMousePos(float x, float y) = 0;

	/** Enable / disable  low level mouse. This allow to take advantage of some options (speed of the mouse, automatic wrapping)
	  * It returns a interface to these parameters when it is supported, or NULL otherwise
	  * The interface pointer is valid as long as the low level mouse is enabled.
	  * A call to disable the mouse returns NULL, and restore the default mouse behavior
	  * NB : - In this mode the mouse cursor isn't drawn.
      *      - Calls to showCursor have no effects
	  *      - Calls to setCapture have no effects
	  */
	virtual NLMISC::IMouseDevice			*enableLowLevelMouse(bool enable, bool exclusive) = 0;

	/** Enable / disable  a low level keyboard.
	  * Such a keyboard can only send KeyDown and KeyUp event. It just consider the keyboard as a
	  * gamepad with lots of buttons...
	  * This returns a interface to some parameters when it is supported, or NULL otherwise.
	  * The interface pointer is valid as long as the low level keyboard is enabled.
	  * A call to disable the keyboard returns NULL, and restore the default keyboard behavior
	  */
	virtual NLMISC::IKeyboardDevice			*enableLowLevelKeyboard(bool enable) = 0;

	/** Get the delay in ms for mouse double clicks.
	  */
	virtual uint			getDoubleClickDelay(bool hardwareMouse) = 0;

	/** If true, capture the mouse to force it to stay under the window.
	  * NB : this has no effects if a low level mouse is used
	  */
	virtual void			setCapture(bool b) = 0;

	// see if system cursor is currently captured
	virtual bool			isSystemCursorCaptured() = 0;

	// Add a new cursor (name is case unsensitive)
	virtual void			addCursor(const std::string &name, const NLMISC::CBitmap &bitmap) = 0;

	// Display a cursor from its name (case unsensitive)
	virtual void			setCursor(const std::string &name, NLMISC::CRGBA col, uint8 rot, sint hotSpotX, sint hotSpotY, bool forceRebuild = false) = 0;

	// Change default scale for all cursors
	virtual void			setCursorScale(float scale) = 0;

	/** Check whether there is a low level device manager available, and get its interface. Return NULL if not available
	  * From this interface you can deal with mouse and keyboard as above, but you can also manage game device (joysticks, joypads ...)
	  */
	virtual NLMISC::IInputDeviceManager		*getLowLevelInputDeviceManager() = 0;
	// @}



	/// \name Render target // TODO: Handle Color/ZBuffer/Stencil consistently
	// @{
	/** Set the current render target.
	  *
	  * The render target can be a texture (tex pointer) or the back buffer (tex = NULL).
	  * The texture must have been right sized before the call.
	  * This mark the texture as valid, but doesn't copy data to system memory.
	  * This also mean that regenerating texture data will erase what
	  * has been copied before in the device memory.
	  * This doesn't work with compressed textures.
	  * Ideally, the FrameBuffer should have the same format than the texture.
	  *
	  * When direct render to texture is not available (openGl), it uses the frame buffer for the rendering and copy the frame buffer
	  * content into the texture when setRenderTarget(NULL) is called.
	  *
	  * The x, y, width and height parameters are only used in this case to optimize the copy from the framebuffer
	  * to the texture.
	  *
	  * If a texture is set as target, the viewport and the scissor are now relative to the texture sizes,
	  * and not to the x, y, width and height parameters.
	  *
	  * The texture content can be lost after the first setRenderTarget().
	  *
	  * The texture must have the render target abilities enabled. (ITexture::setRenderTarget ())
	  *
	  * \param tex					the texture to render into.
	  * \param x					x position within the destination texture of the renderable area.
	  * \param y					y position within the destination texture of the renderable area.
	  * \param width				width of the renderable area, if 0, use the whole size.
	  * \param height				height of the renderable area, if 0, use the whole size.
	  * \param mipmapLevel			the mipmap to copy texture to.
	  * \param cubaFace				the face of the cube to copy texture to.
	  * \return true if the render target has been changed
	  */
	virtual bool			setRenderTarget(	ITexture *tex,
												uint32 x = 0,
												uint32 y = 0,
												uint32 width = 0,
												uint32 height = 0,
												uint32 mipmapLevel = 0,
												uint32 cubeFace = 0
												) = 0;

	virtual ITexture		*getRenderTarget() const = 0;

	/** Retrieve the render target size.
	  * If the render target is the frame buffer, it returns the size of the frame buffer.
	  * It the render target is a texture, it returns the size of the texture mipmap selected as render target.
	  */
	virtual bool			getRenderTargetSize (uint32 &width, uint32 &height) = 0;

	/** Trick method : copy the current texture target into another texture without updating the current texture.
	  *
	  * This method copies the current texture into another texture.
	  * WARNING : at the next setRenderTarget () call, the current texture target WILL NOT BE UPDATED.
	  *
	  * When direct render to texture is not available, this method can save a texture copy :
	  *
	  * Use this method to copy a temporary texture target into a destination texture.
	  * Then, resets the rendering target with setRenderTarget().
	  *
	  * The temporary texture is copied into the final texture direct from the frame buffer. The temporary texture is not filled in VRAM when
	  * the framebuffer is set back as render target.
	  *
	  * Works only if a texture is used as render target.
	  *
	  * This method invalidates the vertex buffer, the view and model matrices, the viewport and the frustum.
	  *
	  * \param tex					the texture to render into.
	  * \param offsetx				x position within the destination texture.
	  * \param y					y position within the destination texture.
	  * \param x					x position within the current texture target.
	  * \param y					y position within the current texture target.
	  * \param width				width of the renderable area to copy, if 0, use the whole size.
	  * \param height				height of the renderable area  to copy, if 0, use the whole size.
	  * \param mipmapLevel			the mipmap to copy texture to.
	  */
	virtual bool			copyTargetToTexture(	ITexture *tex,
													uint32 offsetx = 0,
													uint32 offsety = 0,
													uint32 x = 0,
													uint32 y = 0,
													uint32 width = 0,
													uint32 height = 0,
		                                            uint32 mipmapLevel = 0
													) = 0;
	// @}



	/// \name Render state: Polygon mode
	// @{
	/** Set the global polygon mode. Can be filled, line or point. The implementation driver must
	  * call IDriver::setPolygonMode and active this mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see getPolygonMode(), TPolygonMode
	  */
	virtual void			setPolygonMode (TPolygonMode mode)
	{
		_PolygonMode=mode;
	}

	/** Get the global polygon mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see setPolygonMode(), TPolygonMode
	  */
	TPolygonMode 	getPolygonMode ()
	{
		return _PolygonMode;
	}
	// @}



	/// \name Fixed pipeline lights
	// @{
	/**
	  * return the number of light supported by driver. typically 8.
	  *
	  * \see enableLight() setLight()
	  */
	virtual uint			getMaxLight() const = 0;

	/**
	  * Setup a light.
	  *
	  * You must call enableLight() to active the light.
	  *
	  * \param num is the number of the light to set.
	  * \param light is a light to set in this slot.
	  * \see enableLight()
	  */
	virtual void			setLight(uint8 num, const CLight &light) = 0;

	/**
	  * Enable / disable light.
	  *
	  * You must call setLight() if you active the light.
	  *
	  * \param num is the number of the light to enable / disable.
	  * \param enable is true to enable the light, false to disable it.
	  * \see setLight()
	  */
	virtual void			enableLight(uint8 num, bool enable = true) = 0;

	/**
	  * Set ambient.
	  *
	  * \param color is the new global ambient color for the scene.
	  * \see setLight(), enableLight()
	  */
	virtual void			setAmbientColor(NLMISC::CRGBA color) = 0;

	/** Setup the light used for per pixel lighting. The given values should have been modulated by the material diffuse and specular.
	  * This is only useful for material that have their shader set as 'PerPixelLighting'
	  * \param the light used for per pixel lighting
	  */
	virtual void			setPerPixelLightingLight(NLMISC::CRGBA diffuse, NLMISC::CRGBA specular, float shininess) = 0;

	/** Setup the unique light used for Lightmap Shader.
	  *	Lightmaped primitives are lit per vertex with this light (should be local attenuated for maximum efficiency)
	  * This is only useful for material that have their shader set as 'LightMap'
	  * \param the light used for per pixel lighting
	  */
	virtual void			setLightMapDynamicLight(bool enable, const CLight &light) = 0;
	// @}



	/// \name Vertex Program
	// @{

	// Order of preference
	// - activeVertexProgram
	// - CMaterial pass[n] VP (uses activeVertexProgram, but does not override if one already set by code)
	// - default generic VP that mimics fixed pipeline / no VP with fixed pipeline

	/**
	  * Does the driver supports vertex program, but emulated by CPU ?
	  */
	virtual bool			isVertexProgramEmulated() const = 0;

	/** Return true if the driver supports the specified vertex program profile.
	  */
	virtual bool			supportVertexProgram(CVertexProgram::TProfile profile) const = 0;

	/** Compile the given vertex program, return if successful.
	  * If a vertex program was set active before compilation, 
	  * the state of the active vertex program is undefined behaviour afterwards.
	  */
	virtual bool			compileVertexProgram(CVertexProgram *program) = 0;

	/** Set the active vertex program. This will override vertex programs specified in CMaterial render calls.
	  * Also used internally by setupMaterial(CMaterial) when getVertexProgram returns NULL.
	  * The vertex program is activated immediately.
	  */
	virtual bool			activeVertexProgram(CVertexProgram *program) = 0;
	// @}



	/// \name Pixel Program
	// @{

	// Order of preference
	// - activePixelProgram
	// - CMaterial pass[n] PP (uses activePixelProgram, but does not override if one already set by code)
	// - PP generated from CMaterial (uses activePixelProgram, but does not override if one already set by code)

	/** Return true if the driver supports the specified pixel program profile.
	  */
	virtual bool			supportPixelProgram(CPixelProgram::TProfile profile) const = 0;

	/** Compile the given pixel program, return if successful.
	  * If a pixel program was set active before compilation, 
	  * the state of the active pixel program is undefined behaviour afterwards.
	  */
	virtual bool			compilePixelProgram(CPixelProgram *program) = 0;

	/** Set the active pixel program. This will override pixel programs specified in CMaterial render calls.
	  * Also used internally by setupMaterial(CMaterial) when getPixelProgram returns NULL.
	  * The pixel program is activated immediately.
	  */
	virtual bool			activePixelProgram(CPixelProgram *program) = 0;
	// @}



	/// \name Geometry Program
	// @{

	// Order of preference
	// - activeGeometryProgram
	// - CMaterial pass[n] PP (uses activeGeometryProgram, but does not override if one already set by code)
	// - none

	/** Return true if the driver supports the specified pixel program profile.
	  */
	virtual bool			supportGeometryProgram(CGeometryProgram::TProfile profile) const = 0;

	/** Compile the given pixel program, return if successful.
	  * If a pixel program was set active before compilation, 
	  * the state of the active pixel program is undefined behaviour afterwards.
	  */
	virtual bool			compileGeometryProgram(CGeometryProgram *program) = 0;

	/** Set the active pixel program. This will override pixel programs specified in CMaterial render calls.
	  * Also used internally by setupMaterial(CMaterial) when getGeometryProgram returns NULL.
	  * The pixel program is activated immediately.
	  */
	virtual bool			activeGeometryProgram(CGeometryProgram *program) = 0;
	// @}



	/// \name Program parameters
	// @{
	// Set parameters
	virtual void			setUniform1f(TProgram program, uint index, float f0) = 0;
	virtual void			setUniform2f(TProgram program, uint index, float f0, float f1) = 0;
	virtual void			setUniform3f(TProgram program, uint index, float f0, float f1, float f2) = 0;
	virtual void			setUniform4f(TProgram program, uint index, float f0, float f1, float f2, float f3) = 0;
	virtual void			setUniform1i(TProgram program, uint index, sint32 i0) = 0;
	virtual void			setUniform2i(TProgram program, uint index, sint32 i0, sint32 i1) = 0;
	virtual void			setUniform3i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2) = 0;
	virtual void			setUniform4i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2, sint32 i3) = 0;
	virtual void			setUniform1ui(TProgram program, uint index, uint32 ui0) = 0;
	virtual void			setUniform2ui(TProgram program, uint index, uint32 ui0, uint32 ui1) = 0;
	virtual void			setUniform3ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2) = 0;
	virtual void			setUniform4ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2, uint32 ui3) = 0;
	virtual void			setUniform3f(TProgram program, uint index, const NLMISC::CVector& v) = 0;
	virtual void			setUniform4f(TProgram program, uint index, const NLMISC::CVector& v, float f3) = 0;
	virtual void			setUniform4f(TProgram program, uint index, const NLMISC::CRGBAF& rgba) = 0;
	virtual void			setUniform4x4f(TProgram program, uint index, const NLMISC::CMatrix& m) = 0;
	virtual void			setUniform4fv(TProgram program, uint index, size_t num, const float *src) = 0;
	virtual void			setUniform4iv(TProgram program, uint index, size_t num, const sint32 *src) = 0;
	virtual void			setUniform4uiv(TProgram program, uint index, size_t num, const uint32 *src) = 0;
	// Set builtin parameters
	/**
	  * Setup uniforms with a current matrix.
	  *
	  *	This call must be done after setFrustum(), setupViewMatrix() or setupModelMatrix() to get correct
	  *	results.
	  *
	  * \param index is the base constant index where to store the matrix. This index must be a multiple of 4.
	  * \param matrix is the matrix id to store in the constants
	  * \param transform is the transformation to apply to the matrix before store it in the constants.
	  *
	  */
	virtual void			setUniformMatrix(TProgram program, uint index, TMatrix matrix, TTransform transform) = 0;
	/**
	  * Setup the uniform with the fog vector. This vector must be used to get the final fog value in a vertex shader.
	  * You must use it like this:
	  * DP4 o[FOGC].x, c[4], R4;
	  * With c[4] the constant used for the fog vector and R4 the vertex local position.
	  *
	  *	This call must be done after setFrustum(), setupViewMatrix(), setupModelMatrix() and setupFog() to get correct
	  *	results.
	  *
	  * \param index is the index where to store the vector.
	  *
	  */
	virtual void			setUniformFog(TProgram program, uint index) = 0;
    // Set feature parameters
	virtual bool			isUniformProgramState() = 0;
	// @}



	/// \name Legacy effects
	// @{
	// test if support for cloud render in a single pass
	virtual	bool			supportCloudRenderSinglePass() const = 0;

	// [FIXME] Return true if driver support Bloom effect // FIXME: This is terrible
	virtual	bool			supportBloomEffect() const = 0;
	// @}



	/// \name Backface color
	// @{
	/// Check if the driver support double sided colors vertex programs
	virtual bool		    supportVertexProgramDoubleSidedColor() const = 0;
	/**
	  * Activate VertexProgram 2Sided Color mode. In 2Sided mode, the BackFace (if material 2Sided enabled) read the
	  *	result from o[BFC0], and not o[COL0].
	  *	default is false. you should reset to false after use.
	  * NB: no-op if not supported by driver
	  */
	virtual	void			enableVertexProgramDoubleSidedColor(bool doubleSided) =0;
	// @}



	/// \name Texture addressing modes aka textures/pixels shaders
	// @{
	/// test whether the device supports some form of texture shader. (could be limited to DX6 EMBM for example)
	virtual bool			supportTextureShaders() const = 0;
	// Is the shader water supported ? If not, the driver caller should implement its own version
	virtual bool			supportWaterShader() const = 0;
	//
	/// test whether a texture addressing mode is supported
	virtual bool			supportTextureAddrMode(CMaterial::TTexAddressingMode mode) const = 0;
	/** setup the 2D matrix for the OffsetTexture, OffsetTextureScale and OffsetTexture addressing mode
	  * It should be stored as the following
	  * [a0 a1]
	  * [a2 a3]
	  */
	virtual void			setMatrix2DForTextureOffsetAddrMode(const uint stage, const float mat[4]) = 0;
	//@}



	/** \name EMBM support. If texture shaders are present, this is not available, must use them instead.
	  * EMBM is a color op of CMaterial.
	  * NB : EMBM is the equivalent of the CMaterial::OffsetTexture addressing mode. However, it is both a texture
	  * addressing mode and a color op.
	  * NB : EMBM may not be supported by all stages.
	  *
	  * if embm unit is at last last stage, it operates on texture at first stage
	  * otherwise it operates on texture at next stage
	  */

	// @{
	// Test if EMBM is supported.
	virtual bool			supportEMBM() const = 0;
	// Test if EMBM is supported for the given stage
	virtual bool			isEMBMSupportedAtStage(uint stage) const = 0;
	// set the matrix used for EMBM addressing
	virtual void			setEMBMMatrix(const uint stage, const float mat[4]) = 0;
	// @}



	/// \name Misc
	// @{
	/**	Does the driver support Blend Constant Color ??? If yes CMaterial::blendConstant* enum can be used
	 *	for blend Src ord Dst factor. If no, using these enum will have undefined results.
	 */
	virtual	bool			supportBlendConstantColor() const = 0;

	/**	see supportBlendConstantColor(). Set the current Blend Constant Color.
	 */
	virtual	void			setBlendConstantColor(NLMISC::CRGBA col) = 0;

	/**	see supportBlendConstantColor(). Get the current Blend Constant Color.
	 */
	virtual	NLMISC::CRGBA	getBlendConstantColor() const = 0;

	/** force the driver to flush all command. glFinish() in opengl.
	 *	Interesting only for debug and profiling purpose.
	 */
	virtual	void			finish() = 0;

	// Flush command queue an immediately returns
	virtual void            flush() = 0;

	/** Use AntiAliasing For polygons (GL_POLYGON_SMOOTH like, not the FSAA).
	 *	See GL_POLYGON_SMOOTH help, and GL_SRC_ALPHA_SATURATE OpenGL doc (not yet implemented now since
	 *	used only for alpha part in ShadowMap gen)
	 */
	virtual	void			enablePolygonSmoothing(bool smooth) = 0;

	/// see enablePolygonSmoothing()
	virtual	bool			isPolygonSmoothingEnabled() const = 0;
	// @}



	/**	Special method to internally swap the Driver handle of 2 textures.
	 *	USE IT WITH CARE (eg: may have Size problems, mipmap problems, format problems ...)
	 *	Actually, it is used only by CAsyncTextureManager, to manage Lods of DXTC CTextureFile.
	 *	NB: internally, all textures slots are disabled.
	 */
	virtual void			swapTextureHandle(ITexture &tex0, ITexture &tex1) = 0;

	/** Advanced usage. Get the texture Handle.Useful for texture sorting for instance
	 *	NB: if the texture is not setuped in the driver, 0 is returned.
	 *	NB: if implementation does not support it, 0 may be returned. OpenGL ones return the Texture ID.
	 *	NB: unlike isTextureExist(), this method is not thread safe.
	 */
	virtual	uintptr_t		getTextureHandle(const ITexture&tex) = 0;

	// see if the Multiply-Add Tex Env operator is supported (see CMaterial::Mad)
	virtual	bool			supportMADOperator() const = 0;

	// Adapter class
	class CAdapter
	{
	public:
		std::string			Driver;
		std::string			Description;
		std::string			DeviceName;
		std::string			Vendor;
		sint64				DriverVersion;
		uint32				VendorId;
		uint32				DeviceId;
		uint32				SubSysId;
		uint32				Revision;
	};

	// Get the number of hardware renderer available on the client platform.
	virtual uint			getNumAdapter() const = 0;

	// Get a hardware renderer description.
	virtual bool			getAdapter(uint adapter, CAdapter &desc) const = 0;

	/** Choose the hardware renderer.
	 *  Call it before the setDisplay and enumModes methods
	 *	Choose adapter = 0xffffffff for the default one.
	 */
	virtual bool			setAdapter(uint adapter) = 0;

	/** Tell if the vertex color memory format is RGBA (openGL) or BGRA (directx)
	  * BGRA :
   	  *			*****************************************************************
	  *	Offset:	*    0          *      1        *     2         *     3         *
   	  *			*****************************************************************
	  *	RGBA	*    red        *      green    *     blue      *     alpha     *
	  *			*****************************************************************
	  *	BGRA	*    blue       *      green    *     red       *     alpha     *
	  *			*****************************************************************
	  */
	virtual CVertexBuffer::TVertexColorType getVertexColorFormat() const =0;



	/// \name Bench
	// @{
	// Start the bench. See CHTimer::startBench();
	virtual void			startBench(bool wantStandardDeviation = false, bool quick = false, bool reset = true) =0;

	// End the bench. See CHTimer::endBench();
	virtual void			endBench () =0;

	// Display the bench result
	virtual void			displayBench (class NLMISC::CLog *log) =0;
	// @}



	/// \name Occlusion query mechanism
	// @{
	// Test whether this device supports the occlusion query mechanism
	virtual bool			supportOcclusionQuery() const = 0;
	/** Create an occlusion query object.
	  * \return NULL is not enough resources or if not supported
	  */
	virtual IOcclusionQuery *createOcclusionQuery() = 0;
	// Delete an occlusion query object previously obtained by a call to createOcclusionQuery
	virtual void			deleteOcclusionQuery(IOcclusionQuery *oq) = 0;
	// @}




	/** Set cull mode
	  * Useful for mirrors / cube map rendering or when the scene must be rendered upside down
	  */
	virtual void			setCullMode(TCullMode cullMode) = 0;
	virtual	TCullMode       getCullMode() const = 0;

	/** Set stencil support
	  */
	virtual void			enableStencilTest(bool enable) = 0;
	virtual bool			isStencilTestEnabled() const = 0;
	virtual void			stencilFunc(TStencilFunc stencilFunc, int ref, uint mask) = 0;
	virtual void			stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass) = 0;
	virtual void			stencilMask(uint mask) = 0;

protected:
	friend	class			IVBDrvInfos;
	friend	class			IIBDrvInfos;
	friend	class			CTextureDrvShare;
	friend	class			ITextureDrvInfos;
	friend	class			IMaterialDrvInfos;
	friend	class			IProgramDrvInfos;
	friend	class			IProgramParamsDrvInfos;

	/// remove ptr from the lists in the driver.
	void					removeVBDrvInfoPtr(ItVBDrvInfoPtrList vbDrvInfoIt);
	void					removeIBDrvInfoPtr(ItIBDrvInfoPtrList ibDrvInfoIt);
	void					removeTextureDrvInfoPtr(ItTexDrvInfoPtrMap texDrvInfoIt);
	void					removeTextureDrvSharePtr(ItTexDrvSharePtrList texDrvShareIt);
	void					removeMatDrvInfoPtr(ItMatDrvInfoPtrList shaderIt);
	void					removeGPUPrgDrvInfoPtr(ItGPUPrgDrvInfoPtrList gpuPrgDrvInfoIt);

private:
	bool					_StaticMemoryToVRAM;

};

// --------------------------------------------------

}

#endif // NL_DRV_H

