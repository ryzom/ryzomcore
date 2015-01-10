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

#include "stdopengl.h"

#include "driver_opengl.h"
#include "nel/3d/texture_cube.h"
#include "nel/3d/texture_bloom.h"
#include "nel/misc/rect.h"
#include "nel/misc/file.h" // temp

// Define this to force nearest filter (debug)
// #define NEL_FORCE_NEAREST

using	namespace NLMISC;
using	namespace std;


//#define NEL_DUMP_UPLOAD_TIME


#ifdef NEL_DUMP_UPLOAD_TIME
	#define NEL_MEASURE_UPLOAD_TIME_START NLMISC::TTicks startTick = CTime::getPerformanceTime();
	#define NEL_MEASURE_UPLOAD_TIME_END NLMISC::TTicks endTick = CTime::getPerformanceTime(); \
		nlinfo("3D: upload time = %.2f ms", (float) (1000 * (CTime::ticksToSecond(endTick) - CTime::ticksToSecond(startTick))));
#else
	#define NEL_MEASURE_UPLOAD_TIME_START
	#define NEL_MEASURE_UPLOAD_TIME_END
#endif

namespace NL3D {

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

// ***************************************************************************
CTextureDrvInfosGL::CTextureDrvInfosGL(IDriver *drv, ItTexDrvInfoPtrMap it, CDriverGL *drvGl, bool isRectangleTexture) : ITextureDrvInfos(drv, it)
{
	H_AUTO_OGL(CTextureDrvInfosGL_CTextureDrvInfosGL)
	//nldebug("3D: CTextureDrvInfosGL::ctor()");
	// The id is auto created here.
	glGenTextures(1,&ID);

	Compressed = false;
	MipMap = false;
	TextureMemory = 0;

	// Nb: at Driver dtor, all tex infos are deleted, so _Driver is always valid.
	_Driver= drvGl;

#ifdef USE_OPENGLES
	TextureMode = GL_TEXTURE_2D;
#else
	TextureMode = isRectangleTexture?GL_TEXTURE_RECTANGLE_NV:GL_TEXTURE_2D;
#endif

	FBOId = 0;
	InitFBO = false;
	AttachDepthStencil = true;

	TextureUsedIdx = 0;
}
// ***************************************************************************
CTextureDrvInfosGL::~CTextureDrvInfosGL()
{
	H_AUTO_OGL(CTextureDrvInfosGL_CTextureDrvInfosGLDtor)
	// The id is auto deleted here.
	glDeleteTextures(1,&ID);

	// release profiling texture mem.
	_Driver->_AllocatedTextureMemory-= TextureMemory;

	// release in TextureUsed.
	if (TextureUsedIdx < _Driver->_TextureUsed.size() && _Driver->_TextureUsed[TextureUsedIdx] == this)
	{
		_Driver->_TextureUsed[TextureUsedIdx] = NULL;
	}

#ifdef USE_OPENGLES
	if (InitFBO)
	{
		nglDeleteFramebuffersOES(1, &FBOId);
		if(AttachDepthStencil)
		{
			nglDeleteRenderbuffersOES(1, &DepthFBOId);
			if(!UsePackedDepthStencil)
				nglDeleteRenderbuffersOES(1, &StencilFBOId);
		}
	}
#endif
}

CDepthStencilFBO::CDepthStencilFBO(CDriverGL *driver, uint width, uint height)
{
	nldebug("3D: Init shared FBO");

	m_Driver = driver;
	Width = width;
	Height = height;

	bool packedDepthStencil = driver->supportPackedDepthStencil();
	nglGenRenderbuffersEXT(1, &DepthFBOId);
	if (packedDepthStencil)
		StencilFBOId = DepthFBOId;
	else
		nglGenRenderbuffersEXT(1, &StencilFBOId);

	if (packedDepthStencil)
	{
		//nldebug("3D: using packed depth stencil");
		nglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, StencilFBOId);
		nglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT, width, height);
	}
	else
	{
		nglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, DepthFBOId);
		nglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
		/*
		nglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, StencilFBOId);
		nglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX8_EXT, width, height);
		*/
	}

	nlassert(DepthFBOId);
	nlassert(StencilFBOId);

	driver->_DepthStencilFBOs.push_back(this);
}

CDepthStencilFBO::~CDepthStencilFBO()
{
	// driver remove
	m_Driver->_DepthStencilFBOs.erase(std::find(m_Driver->_DepthStencilFBOs.begin(), m_Driver->_DepthStencilFBOs.end(), this));

	if (DepthFBOId)
	{
		nldebug("3D: Release shared FBO");
		nglDeleteRenderbuffersEXT(1, &DepthFBOId);
		if (StencilFBOId == DepthFBOId)
			StencilFBOId = 0;
		DepthFBOId = 0;
	}
	if (StencilFBOId)
	{
		nglDeleteRenderbuffersEXT(1, &StencilFBOId);
		StencilFBOId = 0;
	}
}

// ***************************************************************************
bool CTextureDrvInfosGL::initFrameBufferObject(ITexture * tex)
{
	if (!InitFBO)
	{
		if (tex->isBloomTexture())
		{
			AttachDepthStencil = !((CTextureBloom*)tex)->isMode2D();
		}

#ifdef USE_OPENGLES
		// generate IDs
		nglGenFramebuffersOES(1, &FBOId);
		if(AttachDepthStencil)
		{
			nglGenRenderbuffersOES(1, &DepthFBOId);
			if(UsePackedDepthStencil)
				StencilFBOId = DepthFBOId;
			else
				nglGenRenderbuffersOES(1, &StencilFBOId);
		}

		//nldebug("3D: using depth %d and stencil %d", DepthFBOId, StencilFBOId);

		// initialize FBO
		nglBindFramebufferOES(GL_FRAMEBUFFER_OES, FBOId);
		nglFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, TextureMode, ID, 0);

		// attach depth/stencil render to FBO
		// note: for some still unkown reason it's impossible to add
		// a stencil buffer as shown in the respective docs (see
		// opengl.org extension registry). Until a safe approach to add
		// them is found, there will be no attached stencil for the time
		// being, aside of using packed depth+stencil buffers.
		if(AttachDepthStencil)
		{
			if(UsePackedDepthStencil)
			{
				//nldebug("3D: using packed depth stencil");
				nglBindRenderbufferOES(GL_RENDERBUFFER_OES, StencilFBOId);
				nglRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH24_STENCIL8_OES, tex->getWidth(), tex->getHeight());
			}
			else
			{
				nglBindRenderbufferOES(GL_RENDERBUFFER_OES, DepthFBOId);
				nglRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT24_OES, tex->getWidth(), tex->getHeight());
				/*
				nglBindRenderbufferEXT(GL_RENDERBUFFER_OES, StencilFBOId);
				nglRenderbufferStorageEXT(GL_RENDERBUFFER_OES, GL_STENCIL_INDEX8_EXT, tex->getWidth(), tex->getHeight());
				*/
			}
			nglFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, DepthFBOId);
			nldebug("3D: glFramebufferRenderbufferExt(depth:24) = %X", nglCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
			nglFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, StencilFBOId);
			nldebug("3D: glFramebufferRenderbufferExt(stencil:8) = %X", nglCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
		}
#else
		// generate IDs
		nglGenFramebuffersEXT(1, &FBOId);
		
		//nldebug("3D: using depth %d and stencil %d", DepthFBOId, StencilFBOId);

		// initialize FBO
		nglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBOId);
		nglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TextureMode, ID, 0);

		// attach depth stencil
		if (AttachDepthStencil)
		{
			for (std::vector<CDepthStencilFBO *>::iterator it(_Driver->_DepthStencilFBOs.begin()), end(_Driver->_DepthStencilFBOs.end()); it != end; ++it)
			{
				if ((*it)->Width == tex->getWidth() && (*it)->Height == tex->getHeight())
				{
					DepthStencilFBO = (*it);
					break;
				}
			}
			if (!DepthStencilFBO)
			{
				DepthStencilFBO = new CDepthStencilFBO(_Driver, tex->getWidth(), tex->getHeight());
			}
			nglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
										 GL_RENDERBUFFER_EXT, DepthStencilFBO->DepthFBOId);
			nldebug("3D: glFramebufferRenderbufferExt(depth:24) = %X", nglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
			nglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
										 GL_RENDERBUFFER_EXT, DepthStencilFBO->StencilFBOId);
			nldebug("3D: glFramebufferRenderbufferExt(stencil:8) = %X", nglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
		}
#endif

		// check status
		GLenum status = (GLenum) nglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

		switch(status) {
#ifdef GL_FRAMEBUFFER_COMPLETE_EXT
			case GL_FRAMEBUFFER_COMPLETE_EXT:
				InitFBO = true;
				break;
#endif
#ifdef GL_FRAMEBUFFER_COMPLETE_OES
			case GL_FRAMEBUFFER_COMPLETE_OES:
				InitFBO = true;
				break;
#endif
#ifdef GL_FRAMEBUFFER_UNSUPPORTED_EXT
			case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
				nlwarning("Unsupported framebuffer format");
				break;
#endif
#ifdef GL_FRAMEBUFFER_UNSUPPORTED_OES
			case GL_FRAMEBUFFER_UNSUPPORTED_OES:
				nlwarning("Unsupported framebuffer format");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
				nlwarning("Framebuffer incomplete attachment");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES:
				nlwarning("Framebuffer incomplete attachment");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
				nlwarning("Framebuffer incomplete, missing attachment");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES:
				nlwarning("Framebuffer incomplete, missing attachment");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT
			case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
				nlwarning("Framebuffer incomplete, duplicate attachment");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
				nlwarning("Framebuffer incomplete, attached images must have same dimensions");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES:
				nlwarning("Framebuffer incomplete, attached images must have same dimensions");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
				nlwarning("Framebuffer incomplete, attached images must have same format");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_FORMATS_OES
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_OES:
				nlwarning("Framebuffer incomplete, attached images must have same format");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
				nlwarning("Framebuffer incomplete, missing draw buffer");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
				nlwarning("Framebuffer incomplete, missing read buffer");
				break;
#endif
#ifdef GL_FRAMEBUFFER_BINDING_EXT
			case GL_FRAMEBUFFER_BINDING_EXT:
				nlwarning("Framebuffer BINDING_EXT");
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				nlwarning("Framebuffer incomplete multisample");
				break;
#endif
#ifdef GL_FRAMEBUFFER_BINDING_OES
			case GL_FRAMEBUFFER_BINDING_OES:
				nlwarning("Framebuffer BINDING_EXT");
				break;
#endif
			default:
				nlwarning("Framebuffer incomplete status %d", (sint)status);
				//nlassert(0);
		}

		// clean up resources if allocation failed
		if (!InitFBO)
		{
			nglDeleteFramebuffersEXT(1, &FBOId);

			if (AttachDepthStencil)
			{
				DepthStencilFBO = NULL;
			}
		}

	}

	return InitFBO;
}

// ***************************************************************************
bool CTextureDrvInfosGL::activeFrameBufferObject(ITexture * tex)
{
	if(tex)
	{
		if(initFrameBufferObject(tex))
		{
			glBindTexture(TextureMode, 0);
			nglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBOId);
		}
		else
			return false;
	}
	else
	{
		nglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	return true;
}

// ***************************************************************************
// Get the glText mirror of an existing setuped texture.
static inline CTextureDrvInfosGL* getTextureGl(ITexture& tex)
{
	H_AUTO_OGL(getTextureGl)
	CTextureDrvInfosGL*	gltex;
	gltex= (CTextureDrvInfosGL*)(ITextureDrvInfos*)(tex.TextureDrvShare->DrvTexture);
	return gltex;
}

// ***************************************************************************
// Translation of TexFmt mode.
GLint	CDriverGL::getGlTextureFormat(ITexture& tex, bool &compressed)
{
	H_AUTO_OGL(CDriverGL_getGlTextureFormat)
	ITexture::TUploadFormat		texfmt= tex.getUploadFormat();

	// If auto, retrieve the pixel format of the bitmap.
	if(texfmt== ITexture::Auto)
	{
		switch(tex.getPixelFormat())
		{
			case CBitmap::RGBA:
				if(_ForceDXTCCompression && tex.allowDegradation() )
					texfmt= ITexture::DXTC5;
				else
					texfmt= ITexture::RGBA8888;
				break;
			case CBitmap::DXTC1: texfmt= ITexture::DXTC1; break;
			case CBitmap::DXTC1Alpha: texfmt= ITexture::DXTC1Alpha; break;
			case CBitmap::DXTC3: texfmt= ITexture::DXTC3; break;
			case CBitmap::DXTC5: texfmt= ITexture::DXTC5; break;
			case CBitmap::Luminance: texfmt= ITexture::Luminance; break;
			case CBitmap::Alpha: texfmt= ITexture::Alpha; break;
			case CBitmap::AlphaLuminance: texfmt= ITexture::AlphaLuminance; break;
			case CBitmap::DsDt: texfmt= ITexture::DsDt; break;
			default: texfmt= ITexture::RGBA8888; break;
		}
	}

	// Get gl tex format, try S3TC compressed ones.
	if(_Extensions.EXTTextureCompressionS3TC)
	{
		compressed= true;
		// Try Compressed ones.
		switch(texfmt)
		{
#ifdef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
			case ITexture::DXTC1:		return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
			case ITexture::DXTC1Alpha:	return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
			case ITexture::DXTC3:		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
			case ITexture::DXTC5:		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
#endif
			default: break;
		}
	}

	// Get standard gl tex format.
	compressed= false;
	switch(texfmt)
	{
#ifdef USE_OPENGLES
		case ITexture::RGBA8888: return GL_RGBA;
//		case ITexture::RGBA4444: return GL_RGBA4_OES;
//		case ITexture::RGBA5551: return GL_RGB5_A1_OES;
		case ITexture::RGB888: return GL_RGB;
//		case ITexture::RGB565: return GL_RGB5_OES;
		case ITexture::Luminance: return GL_LUMINANCE;
		case ITexture::Alpha: return GL_ALPHA;
		case ITexture::AlphaLuminance: return GL_LUMINANCE_ALPHA;
#else
		case ITexture::RGBA8888: return GL_RGBA8;
		case ITexture::RGBA4444: return GL_RGBA4;
		case ITexture::RGBA5551: return GL_RGB5_A1;
		case ITexture::RGB888: return GL_RGB8;
		case ITexture::RGB565: return GL_RGB5;
		case ITexture::Luminance: return GL_LUMINANCE8;
		case ITexture::Alpha: return GL_ALPHA8;
		case ITexture::AlphaLuminance: return GL_LUMINANCE8_ALPHA8;
		case ITexture::DsDt:
			if (_Extensions.NVTextureShader) return GL_DSDT_NV;
			else if (_Extensions.ATIEnvMapBumpMap || _Extensions.ATIFragmentShader)
			{
				return GL_DU8DV8_ATI;
			}
			else
			{
				nlassert(0);
				return 0;
			}
		break;
#endif
		default:
		break;
	}

	return GL_RGBA8;
}

// ***************************************************************************
static GLint	getGlSrcTextureFormat(ITexture &tex, GLint glfmt)
{
	H_AUTO_OGL(getGlSrcTextureFormat)

	// Is destination format is alpha or lumiance ?
	if ((glfmt==GL_ALPHA8)||(glfmt==GL_LUMINANCE8_ALPHA8)||(glfmt==GL_LUMINANCE8))
	{
		switch(tex.getPixelFormat())
		{
		case CBitmap::Alpha:	return GL_ALPHA;
		case CBitmap::AlphaLuminance:	return GL_LUMINANCE_ALPHA;
		case CBitmap::Luminance:	return GL_LUMINANCE;
		default: break;
		}
	}

#ifndef USE_OPENGLES
	if (glfmt == GL_DSDT_NV)
	{
		return GL_DSDT_NV;
	}

	if (glfmt == GL_DU8DV8_ATI)
	{
		return GL_DUDV_ATI;
	}
#endif

	// Else, not a Src format for upload, or RGBA.
	return GL_RGBA;
}

// ***************************************************************************
static GLenum getGlSrcTextureComponentType(GLint texSrcFormat)
{
	H_AUTO_OGL(getGlSrcTextureComponentType);

#ifndef USE_OPENGLES
	switch (texSrcFormat)
	{
		case GL_DSDT_NV:
		case GL_DUDV_ATI:
			return GL_BYTE; // these are signed format
		break;
		default:
		break;
	}
#endif

	return GL_UNSIGNED_BYTE;
}

// ***************************************************************************
uint				CDriverGL::computeMipMapMemoryUsage(uint w, uint h, GLint glfmt) const
{
	H_AUTO_OGL(CDriverGL_computeMipMapMemoryUsage)
	switch(glfmt)
	{
	case GL_RGBA8:		return w*h* 4;
	// Well this is ugly, but simple :). GeForce 888 is stored as 32 bits.
	case GL_RGB8:		return w*h* 4;
#ifdef GL_RGBA4
	case GL_RGBA4:		return w*h* 2;
#endif
#ifdef GL_RGB5_A1
	case GL_RGB5_A1:	return w*h* 2;
#endif
#ifdef GL_RGB5
	case GL_RGB5:		return w*h* 2;
#endif
	case GL_LUMINANCE8:	return w*h* 1;
	case GL_ALPHA8:		return w*h* 1;
	case GL_LUMINANCE8_ALPHA8:	return w*h* 2;
#ifdef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:	return w*h /2;
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:	return w*h /2;
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:	return w*h* 1;
#endif
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:	return w*h* 1;
#endif
#ifdef GL_DU8DV8_ATI
	case GL_DU8DV8_ATI:
#endif
#ifdef GL_DSDT_NV
	case GL_DSDT_NV:						return w*h* 2;
#endif
	}

	// One format has not been coded.
	nlstop;

	/// ???
	return w*h* 4;
}

// ***************************************************************************
// Translation of Wrap mode.
static inline GLenum	translateWrapToGl(ITexture::TWrapMode mode, const CGlExtensions	&extensions)
{
	H_AUTO_OGL(translateWrapToGl)
	if(mode== ITexture::Repeat)
		return GL_REPEAT;

#ifdef USE_OPENGLES
	return GL_CLAMP_TO_EDGE;
#else
	if(extensions.Version1_2)
		return GL_CLAMP_TO_EDGE;

	return GL_CLAMP;
#endif
}

// ***************************************************************************
static inline GLenum	translateMagFilterToGl(CTextureDrvInfosGL *glText)
{
	H_AUTO_OGL(translateMagFilterToGl)
#ifdef NEL_FORCE_NEAREST
	return GL_NEAREST;
#else // NEL_FORCE_NEAREST
	switch(glText->MagFilter)
	{
		case ITexture::Linear: return GL_LINEAR;
		case ITexture::Nearest: return GL_NEAREST;
		default: break;
	}

	nlstop;
	return GL_LINEAR;
#endif // NEL_FORCE_NEAREST
}


// ***************************************************************************
static inline GLenum	translateMinFilterToGl(CTextureDrvInfosGL *glText)
{
	H_AUTO_OGL(translateMinFilterToGl)
#ifdef NEL_FORCE_NEAREST
	return GL_NEAREST;
#else // NEL_FORCE_NEAREST

	if(glText->MipMap)
	{
		switch(glText->MinFilter)
		{
			case ITexture::NearestMipMapOff: return GL_NEAREST;
			case ITexture::NearestMipMapNearest: return GL_NEAREST_MIPMAP_NEAREST;
			case ITexture::NearestMipMapLinear: return GL_NEAREST_MIPMAP_LINEAR;
			case ITexture::LinearMipMapOff: return GL_LINEAR;
			case ITexture::LinearMipMapNearest: return GL_LINEAR_MIPMAP_NEAREST;
			case ITexture::LinearMipMapLinear: return GL_LINEAR_MIPMAP_LINEAR;
			default: break;
		}
	}
	else
	{
		switch(glText->MinFilter)
		{
			case ITexture::NearestMipMapOff:
			case ITexture::NearestMipMapNearest:
			case ITexture::NearestMipMapLinear:
				 return GL_NEAREST;
			case ITexture::LinearMipMapOff:
			case ITexture::LinearMipMapNearest:
			case ITexture::LinearMipMapLinear:
				 return GL_LINEAR;
			default: break;
		}
	}

	nlstop;
	return GL_LINEAR;
#endif // NEL_FORCE_NEAREST
}

// ***************************************************************************
static inline bool		sameDXTCFormat(ITexture &tex, GLint glfmt)
{
	H_AUTO_OGL(sameDXTCFormat);

#ifdef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
	if(glfmt==GL_COMPRESSED_RGB_S3TC_DXT1_EXT && tex.PixelFormat==CBitmap::DXTC1)
		return true;
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT1_EXT && tex.PixelFormat==CBitmap::DXTC1Alpha)
		return true;
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT3_EXT && tex.PixelFormat==CBitmap::DXTC3)
		return true;
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT5_EXT && tex.PixelFormat==CBitmap::DXTC5)
		return true;
#endif

	return false;
}

// ***************************************************************************
static inline bool		isDXTCFormat(GLint glfmt)
{
	H_AUTO_OGL(isDXTCFormat);

#ifdef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
	if(glfmt==GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
		return true;
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
		return true;
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
		return true;
#endif

#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
		return true;
#endif

	return false;
}

// ***************************************************************************
bool CDriverGL::setupTexture (ITexture& tex)
{
	H_AUTO_OGL(setupTexture)
	bool nTmp;
	return setupTextureEx (tex, true, nTmp);
}

// ***************************************************************************
#ifndef NL_DEBUG
	inline
#endif
void CDriverGL::bindTextureWithMode(ITexture &tex)
{
	CTextureDrvInfosGL*	gltext;
	gltext= getTextureGl(tex);
	// system of "backup the previous binded texture" seems to not work with some drivers....
	_DriverGLStates.activeTextureARB(0);
	if(tex.isTextureCube())
	{
		if (_Extensions.ARBTextureCubeMap)
		{
			_DriverGLStates.setTextureMode(CDriverGLStates::TextureCubeMap);
			// Bind this texture
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, gltext->ID);
		}
	}
	else
	{
		CDriverGLStates::TTextureMode textureMode= CDriverGLStates::Texture2D;
#ifndef USE_OPENGLES
		if(gltext->TextureMode == GL_TEXTURE_RECTANGLE_NV)
			textureMode = CDriverGLStates::TextureRect;
#endif

		_DriverGLStates.setTextureMode(textureMode);
		// Bind this texture
		glBindTexture(gltext->TextureMode, gltext->ID);
	}
}

// ***************************************************************************
#ifndef NL_DEBUG
	inline
#endif
void CDriverGL::setupTextureBasicParameters(ITexture &tex)
{
	CTextureDrvInfosGL*	gltext;
	gltext= getTextureGl(tex);
	// TODO: possible cache here, but beware, this is called just after texture creation as well, so these fields
	// haven't ever been filled.
	gltext->WrapS= tex.getWrapS();
	gltext->WrapT= tex.getWrapT();
	gltext->MagFilter= tex.getMagFilter();
	gltext->MinFilter= tex.getMinFilter();
	if(tex.isTextureCube())
	{
		if (_Extensions.ARBTextureCubeMap)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_WRAP_S, translateWrapToGl(ITexture::Clamp, _Extensions));
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_WRAP_T, translateWrapToGl(ITexture::Clamp, _Extensions));
#ifndef USE_OPENGLES
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_WRAP_R, translateWrapToGl(ITexture::Clamp, _Extensions));
#endif
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_MAG_FILTER, translateMagFilterToGl(gltext));
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_MIN_FILTER, translateMinFilterToGl(gltext));

			if (_AnisotropicFilter > 1.f && gltext->MinFilter > ITexture::NearestMipMapLinear)
				glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAX_ANISOTROPY_EXT, _AnisotropicFilter);
		}
	}
	else
	{
		glTexParameteri(gltext->TextureMode,GL_TEXTURE_WRAP_S, translateWrapToGl(gltext->WrapS, _Extensions));
		glTexParameteri(gltext->TextureMode,GL_TEXTURE_WRAP_T, translateWrapToGl(gltext->WrapT, _Extensions));
		glTexParameteri(gltext->TextureMode,GL_TEXTURE_MAG_FILTER, translateMagFilterToGl(gltext));
		glTexParameteri(gltext->TextureMode,GL_TEXTURE_MIN_FILTER, translateMinFilterToGl(gltext));

		if (_AnisotropicFilter > 1.f && gltext->MinFilter > ITexture::NearestMipMapLinear)
			glTexParameteri(gltext->TextureMode, GL_TEXTURE_MAX_ANISOTROPY_EXT, _AnisotropicFilter);
	}
	//
	tex.clearFilterOrWrapModeTouched();
}

// ***************************************************************************
bool CDriverGL::setupTextureEx (ITexture& tex, bool bUpload, bool &bAllUploaded, bool bMustRecreateSharedTexture)
{
	H_AUTO_OGL(setupTextureEx)
	//nldebug("3D: CDriverGL::setupTextureEx(%016p, %d, %d, %d)", &tex, bUpload, bAllUploaded, bMustRecreateSharedTexture);
	bAllUploaded = false;

	if(tex.isTextureCube() && (!_Extensions.ARBTextureCubeMap))
		return true;

	// 0. Create/Retrieve the driver texture.
	//=======================================
	bool mustCreate = false;
	if ( !tex.TextureDrvShare )
	{
		//nldebug("3D:   creating CTextureDrvShare()");
		// insert into driver list. (so it is deleted when driver is deleted).
		ItTexDrvSharePtrList	it= _TexDrvShares.insert(_TexDrvShares.end(), (NL3D::CTextureDrvShare*)NULL);
		// create and set iterator, for future deletion.
		*it= tex.TextureDrvShare= new CTextureDrvShare(this, it, &tex);

		// Must (re)-create the texture.
		mustCreate = true;
	}

	// Does the texture has been touched ?
	if ( (!tex.touched()) && (!mustCreate) )
	{
		// if wrap mode or filter mode is touched, update it here
		if (tex.filterOrWrapModeTouched())
		{
			activateTexture(0, NULL); // unbind any previous texture
			bindTextureWithMode(tex);
			//
			setupTextureBasicParameters(tex); // setup what has changed (will reset the touch flag)

			// Disable texture 0
			_CurrentTexture[0]= NULL;
			_CurrentTextureInfoGL[0]= NULL;
			_DriverGLStates.setTextureMode(CDriverGLStates::TextureDisabled);
			//
		}
		//
		return true; // Do not do anything
	}

	// 1. If modified, may (re)load texture part or all of the texture.
	//=================================================================

	bool	mustLoadAll= false;
	bool	mustLoadPart= false;

	// To avoid any delete/new ptr problem, disable all texturing.
	/* If an old texture is deleted, _CurrentTexture[*] and _CurrentTextureInfoGL[*] are invalid.
		But this is grave only if a new texture is created, with the same pointer (bad luck).
		Since an newly allocated texture always pass here before use, we are sure to avoid any problems.
	*/
	for(uint stage = 0; stage < inlGetNumTextStages(); stage++)
	{
		activateTexture(stage, NULL);
	}

	// A. Share mgt.
	//==============
	if(tex.supportSharing())
	{
		// Try to get the shared texture.

		// Create the shared Name.
		std::string	name;
		getTextureShareName (tex, name);

		// insert or get the texture.
		{
			CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
			TTexDrvInfoPtrMap &rTexDrvInfos = access.value();

			ItTexDrvInfoPtrMap	itTex;
			itTex= rTexDrvInfos.find(name);

			// texture not found?
			if( itTex==rTexDrvInfos.end() )
			{
				// insert into driver map. (so it is deleted when driver is deleted).
				itTex= (rTexDrvInfos.insert(make_pair(name, (ITextureDrvInfos*)NULL))).first;
				// create and set iterator, for future deletion.
				itTex->second= tex.TextureDrvShare->DrvTexture = new CTextureDrvInfosGL(this, itTex, this, isTextureRectangle(&tex));

				// need to load ALL this texture.
				mustLoadAll= true;
			}
			else
			{
				tex.TextureDrvShare->DrvTexture= itTex->second;

				if(bMustRecreateSharedTexture)
					// reload this shared texture (user request)
					mustLoadAll= true;
				else
					// Do not need to reload this texture, even if the format/mipmap has changed, since we found this
					// couple in the map.
					mustLoadAll= false;
			}
		}
		// Do not test if part of texture may need to be computed, because Rect invalidation is incompatible
		// with texture sharing.
	}
	else
	{
		// If texture not already created.
		if(!tex.TextureDrvShare->DrvTexture)
		{
			// Must create it. Create auto a GL id (in constructor).
			// Do not insert into the map. This un-shared texture will be deleted at deletion of the texture.
			// Inform ITextureDrvInfos by passing NULL _Driver.
			tex.TextureDrvShare->DrvTexture = new CTextureDrvInfosGL(NULL, ItTexDrvInfoPtrMap(), this, isTextureRectangle(&tex));

			// need to load ALL this texture.
			mustLoadAll= true;
		}
		else if(tex.isAllInvalidated())
			mustLoadAll= true;
		else if(tex.touched())
			mustLoadPart= true;
	}

	// B. Setup texture.
	//==================
	if(mustLoadAll || mustLoadPart)
	{
		// system of "backup the previous binded texture" seems to not work with some drivers....
		bindTextureWithMode(tex);

		CTextureDrvInfosGL*	gltext;
		gltext= getTextureGl(tex);

		glPixelStorei(GL_UNPACK_ALIGNMENT,1);

		// a. Load All the texture case.
		//==============================
		if (mustLoadAll)
		{
			// profiling. sub old textre memory usage, and reset.
			_AllocatedTextureMemory-= gltext->TextureMemory;
			gltext->TextureMemory= 0;


			if(tex.isTextureCube())
			{
				CTextureCube *pTC = NLMISC::safe_cast<CTextureCube *>(&tex);

				// Regenerate all the texture.
				tex.generate();

				for(uint nText = 0; nText < 6; ++nText)
				if(pTC->getTexture((CTextureCube::TFace)nText) != NULL)
				{
					ITexture *pTInTC = pTC->getTexture((CTextureCube::TFace)nText);

					// In open GL, we have to flip the faces of the cube map
					if( ((CTextureCube::TFace)nText) == CTextureCube::positive_x )
						pTInTC->flipH();
					if( ((CTextureCube::TFace)nText) == CTextureCube::negative_x )
						pTInTC->flipH();
					if( ((CTextureCube::TFace)nText) == CTextureCube::positive_y )
						pTInTC->flipH();
					if( ((CTextureCube::TFace)nText) == CTextureCube::negative_y )
						pTInTC->flipH();
					if( ((CTextureCube::TFace)nText) == CTextureCube::positive_z )
						pTInTC->flipV();
					if( ((CTextureCube::TFace)nText) == CTextureCube::negative_z )
						pTInTC->flipV();

					// Get the correct texture format from texture...
					GLint	glfmt= getGlTextureFormat(*pTInTC, gltext->Compressed);
					GLint	glSrcFmt= getGlSrcTextureFormat(*pTInTC, glfmt);
					GLenum  glSrcType= getGlSrcTextureComponentType(glSrcFmt);

					sint	nMipMaps;
					if(glSrcFmt==GL_RGBA && pTInTC->getPixelFormat()!=CBitmap::RGBA )
						pTInTC->convertToType(CBitmap::RGBA);
					if(tex.mipMapOn())
					{
						pTInTC->buildMipMaps();
						nMipMaps= pTInTC->getMipMapCount();
					}
					else
						nMipMaps= 1;

					// MipMap upload?
					gltext->MipMap= nMipMaps>1;

					// Fill mipmaps.
					for(sint i=0;i<nMipMaps;i++)
					{
						void	*ptr= pTInTC->getPixels(i).getPtr();
						uint	w= pTInTC->getWidth(i);
						uint	h= pTInTC->getHeight(i);
						if (bUpload)
						{
							NEL_MEASURE_UPLOAD_TIME_START
							glTexImage2D (NLCubeFaceToGLCubeFace[nText], i, glfmt, w, h, 0, glSrcFmt, glSrcType, ptr);
							bAllUploaded = true;
							NEL_MEASURE_UPLOAD_TIME_END
						}
						else
						{
							NEL_MEASURE_UPLOAD_TIME_START
							glTexImage2D (NLCubeFaceToGLCubeFace[nText], i, glfmt, w, h, 0, glSrcFmt, glSrcType, NULL);
							NEL_MEASURE_UPLOAD_TIME_END
						}
						// profiling: count TextureMemory usage.
						gltext->TextureMemory+= computeMipMapMemoryUsage(w, h, glfmt);
					}
				}
			}
			else
			{
				// Regenerate all the texture.
				tex.generate();

				if(tex.getSize()>0)
				{
					// Get the correct texture format from texture...
					GLint	glfmt= getGlTextureFormat(tex, gltext->Compressed);
					GLint	glSrcFmt= getGlSrcTextureFormat(tex, glfmt);
					GLenum  glSrcType= getGlSrcTextureComponentType(glSrcFmt);

					// DXTC: if same format, and same mipmapOn/Off, use glTexCompressedImage*.
					// We cannot build the mipmaps if they are not here.
					if(_Extensions.EXTTextureCompressionS3TC && sameDXTCFormat(tex, glfmt))
					{
						sint	nMipMaps = 1;

						if(tex.mipMapOn())
							nMipMaps= tex.getMipMapCount();

						// MipMap upload?
						gltext->MipMap= nMipMaps>1;

						// Degradation in Size allowed only if DXTC texture are provided with mipmaps.
						// Because use them to resize !!!
						uint	decalMipMapResize= 0;
						if(_ForceTextureResizePower>0 && tex.allowDegradation() && nMipMaps>1)
						{
							decalMipMapResize= min(_ForceTextureResizePower, (uint)(nMipMaps-1));
						}

						// Fill mipmaps.
						for(sint i=decalMipMapResize;i<nMipMaps;i++)
						{
							void	*ptr= tex.getPixels(i).getPtr();
							sint	size= tex.getPixels(i).size();
							if (bUpload)
							{
#ifdef USE_OPENGLES
								glCompressedTexImage2D (GL_TEXTURE_2D, i-decalMipMapResize, glfmt,
															tex.getWidth(i),tex.getHeight(i), 0, size, ptr);
#else
								nglCompressedTexImage2DARB (GL_TEXTURE_2D, i-decalMipMapResize, glfmt,
															tex.getWidth(i),tex.getHeight(i), 0, size, ptr);
#endif
								bAllUploaded = true;
							}
							else
							{
								//glCompressedTexImage2DARB (GL_TEXTURE_2D, i-decalMipMapResize, glfmt,
								//							tex.getWidth(i),tex.getHeight(i), 0, size, NULL);
								NEL_MEASURE_UPLOAD_TIME_START

								glTexImage2D (gltext->TextureMode, i-decalMipMapResize, glfmt, tex.getWidth(i), tex.getHeight(i),
												0, glSrcFmt, glSrcType, NULL);
								NEL_MEASURE_UPLOAD_TIME_END
							}

							// profiling: count TextureMemory usage.
							gltext->TextureMemory+= tex.getPixels(i).size();
						}
					}
					else
					{
						sint	nMipMaps;
						if(glSrcFmt==GL_RGBA && tex.getPixelFormat()!=CBitmap::RGBA )
						{
							bUpload = true; // Force all upload
							tex.convertToType(CBitmap::RGBA);
						}

						// Degradation in Size.
						if(_ForceTextureResizePower>0 && tex.allowDegradation())
						{
							uint	w= tex.getWidth(0) >> _ForceTextureResizePower;
							uint	h= tex.getHeight(0) >> _ForceTextureResizePower;
							w= max(1U, w);
							h= max(1U, h);
							tex.resample(w, h);
						}

						if(tex.mipMapOn())
						{
							tex.buildMipMaps();
							nMipMaps= tex.getMipMapCount();
						}
						else
							nMipMaps= 1;

						// MipMap upload?
						gltext->MipMap= nMipMaps>1;

						// Fill mipmaps.
						for(sint i=0;i<nMipMaps;i++)
						{
							void	*ptr= tex.getPixels(i).getPtr();
							uint	w= tex.getWidth(i);
							uint	h= tex.getHeight(i);

							if (bUpload)
							{
								NEL_MEASURE_UPLOAD_TIME_START
								glTexImage2D (gltext->TextureMode, i, glfmt, w, h, 0,glSrcFmt, glSrcType, ptr);
								NEL_MEASURE_UPLOAD_TIME_END
								bAllUploaded = true;
							}
							else
							{
								NEL_MEASURE_UPLOAD_TIME_START
								glTexImage2D (gltext->TextureMode, i, glfmt, w, h, 0,glSrcFmt, glSrcType, NULL);
								NEL_MEASURE_UPLOAD_TIME_END
							}
							// profiling: count TextureMemory usage.
							gltext->TextureMemory += computeMipMapMemoryUsage (w, h, glfmt);
						}
					}
				}
			}
			//printf("%d,%d,%d\n", tex.getMipMapCount(), tex.getWidth(0), tex.getHeight(0));

			// profiling. add new TextureMemory usage.
			_AllocatedTextureMemory+= gltext->TextureMemory;
		}
		// b. Load part of the texture case.
		//==================================
		// Replace parts of a compressed image. Maybe don't work with the actual system of invalidateRect()...
		else if (mustLoadPart && !gltext->Compressed)
		{
			// Regenerate wanted part of the texture.
			tex.generate();

			if(tex.getSize()>0)
			{
				// Get the correct texture format from texture...
				//===============================================
				bool	dummy;
				GLint	glfmt= getGlTextureFormat(tex, dummy);
				GLint	glSrcFmt= getGlSrcTextureFormat(tex, glfmt);
				GLenum  glSrcType= getGlSrcTextureComponentType(glSrcFmt);

				sint	nMipMaps;
				if(glSrcFmt==GL_RGBA && tex.getPixelFormat()!=CBitmap::RGBA )
					tex.convertToType(CBitmap::RGBA);
				if(tex.mipMapOn())
				{
					bool	hadMipMap= tex.getMipMapCount()>1;
					tex.buildMipMaps();
					nMipMaps= tex.getMipMapCount();
					// If the texture had no mipmap before, release them.
					if(!hadMipMap)
					{
						tex.releaseMipMaps();
					}
				}
				else
					nMipMaps= 1;

				// For all rect, update the texture/mipmap.
				//===============================================
				list<NLMISC::CRect>::iterator	itRect;
				for(itRect=tex._ListInvalidRect.begin(); itRect!=tex._ListInvalidRect.end(); itRect++)
				{
					CRect	&rect= *itRect;
					sint	x0= rect.X;
					sint	y0= rect.Y;
					sint	x1= rect.X+rect.Width;
					sint	y1= rect.Y+rect.Height;

					// Fill mipmaps.
					for(sint i=0;i<nMipMaps;i++)
					{
						void	*ptr= tex.getPixels(i).getPtr();
						sint	w= tex.getWidth(i);
						sint	h= tex.getHeight(i);
						clamp(x0, 0, w);
						clamp(y0, 0, h);
						clamp(x1, x0, w);
						clamp(y1, y0, h);

#ifdef USE_OPENGLES
						glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

						if (!bUpload)
							ptr = NULL;

						sint pixelSize = 4;

						if (glSrcFmt == GL_ALPHA)
							pixelSize = 1;

						for(sint yy = 0; yy < (y1-y0); yy++)
						{
							char *row = (char*)ptr + ((yy + y0)*w + x0) * pixelSize;
							glTexSubImage2D (GL_TEXTURE_2D, i, x0, y0+yy, x1-x0, 1, glSrcFmt, glSrcType, row );
						}
#else
						glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
						glPixelStorei(GL_UNPACK_SKIP_ROWS, y0);
						glPixelStorei(GL_UNPACK_SKIP_PIXELS, x0);
						if (bUpload)
							glTexSubImage2D (GL_TEXTURE_2D, i, x0, y0, x1-x0, y1-y0, glSrcFmt,glSrcType, ptr);
						else
							glTexSubImage2D (GL_TEXTURE_2D, i, x0, y0, x1-x0, y1-y0, glSrcFmt,glSrcType, NULL);
#endif

						// Next mipmap!!
						// floor .
						x0= x0/2;
						y0= y0/2;
						// ceil.
						x1= (x1+1)/2;
						y1= (y1+1)/2;
					}
				}

				// Reset the transfer mode...
#ifdef USE_OPENGLES
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#else
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
				glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
#endif
			}
		}

		// Release, if wanted.
		if(tex.getReleasable())
			tex.release();

		// Basic parameters.
		//==================
		setupTextureBasicParameters(tex);

		// Disable texture 0
		_CurrentTexture[0]= NULL;
		_CurrentTextureInfoGL[0]= NULL;
		_DriverGLStates.setTextureMode(CDriverGLStates::TextureDisabled);
	}

	// The texture is correctly setuped.
	tex.clearTouched();
	return true;
}

// ***************************************************************************
bool CDriverGL::uploadTexture (ITexture& tex, CRect& rect, uint8 nNumMipMap)
{
	H_AUTO_OGL(uploadTexture)
	if (tex.TextureDrvShare == NULL)
		return false; // Texture not created
	if (tex.TextureDrvShare->DrvTexture == NULL)
		return false; // Texture not created
	if (tex.isTextureCube())
		return false;

	nlassert(nNumMipMap<(uint8)tex.getMipMapCount());

	// validate rect.
	sint x0 = rect.X;
	sint y0 = rect.Y;
	sint x1 = rect.X+rect.Width;
	sint y1 = rect.Y+rect.Height;
	sint w = tex.getWidth (nNumMipMap);
	sint h = tex.getHeight (nNumMipMap);
	clamp (x0, 0, w);
	clamp (y0, 0, h);
	clamp (x1, x0, w);
	clamp (y1, y0, h);

	// bind the texture to upload
	CTextureDrvInfosGL*	gltext;
	gltext = getTextureGl (tex);

	// system of "backup the previous binded texture" seems to not work with some drivers....
	_DriverGLStates.activeTextureARB (0);
	CDriverGLStates::TTextureMode textureMode= CDriverGLStates::Texture2D;

#ifndef USE_OPENGLES
	if(gltext->TextureMode == GL_TEXTURE_RECTANGLE_NV)
		textureMode = CDriverGLStates::TextureRect;
#endif

	_DriverGLStates.setTextureMode (textureMode);
	// Bind this texture, for reload...
	glBindTexture (gltext->TextureMode, gltext->ID);

	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

	bool dummy;
	GLint glfmt = getGlTextureFormat (tex, dummy);
	GLint glSrcFmt = getGlSrcTextureFormat (tex, glfmt);
	GLenum  glSrcType= getGlSrcTextureComponentType(glSrcFmt);

	// If DXTC format
	if (_Extensions.EXTTextureCompressionS3TC && sameDXTCFormat(tex, glfmt))
	{

		sint nUploadMipMaps;
		if (tex.mipMapOn())
			nUploadMipMaps = tex.getMipMapCount();
		else
			nUploadMipMaps = 1;

		uint decalMipMapResize = 0;
		if (_ForceTextureResizePower>0 && tex.allowDegradation() && nUploadMipMaps>1)
		{
			decalMipMapResize = min(_ForceTextureResizePower, (uint)(nUploadMipMaps-1));
		}

		// Compute src compressed size and location
		sint imageSize = (x1-x0)*(y1-y0);
		void *ptr = tex.getPixels(nNumMipMap).getPtr();

		// If DXTC1 or DXTC1A, then 4 bits/texel else 8 bits/texel
		if (glfmt == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || glfmt == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
		{
			imageSize /= 2;
			ptr = (uint8*)ptr + y0*w/2 + x0/2;
		}
		else
		{
			ptr = (uint8*)ptr + y0*w + x0;
		}

		// Upload

		if (decalMipMapResize > nNumMipMap)
		{
			_CurrentTexture[0]= NULL;
			_CurrentTextureInfoGL[0]= NULL;
			_DriverGLStates.setTextureMode (CDriverGLStates::TextureDisabled);
			return false;
		}

		nlassert (((x0&3) == 0) && ((y0&3) == 0));
		if ((w>=4) && (h>=4))
		{
#ifdef USE_OPENGLES
			glCompressedTexSubImage2D (
#else
			nglCompressedTexSubImage2DARB (
#endif
				GL_TEXTURE_2D, nNumMipMap-decalMipMapResize,
				x0, y0, (x1-x0), (y1-y0), glfmt, imageSize, ptr );
		}
		else
		{
			// The CompressedTexSubImage2DARB function do not work properly if width or height
			// of the mipmap is less than 4 pixel so we use the other form. (its not really time critical
			// to upload 16 bytes so we can do it twice if texture is cut)
			imageSize = tex.getPixels(nNumMipMap).size();
#ifdef USE_OPENGLES
			glCompressedTexImage2D (
#else
			nglCompressedTexImage2DARB (
#endif
				GL_TEXTURE_2D, nNumMipMap-decalMipMapResize,
										glfmt, w, h, 0, imageSize, ptr);
		}
	}
	else
	{
		// glSrcFmt and ITexture format must be identical
		nlassert (glSrcFmt!=GL_RGBA || tex.getPixelFormat()==CBitmap::RGBA);

		void	*ptr= tex.getPixels(nNumMipMap).getPtr();

#ifdef USE_OPENGLES
		sint pixelSize = 4;

		if (glSrcFmt == GL_ALPHA)
			pixelSize = 1;

		for(sint yy = 0; yy < (y1-y0); yy++)
		{
			char *row = (char*)ptr + ((yy + y0)*w + x0) * pixelSize;
			glTexSubImage2D (GL_TEXTURE_2D, nNumMipMap, x0, y0+yy, x1-x0, 1, glSrcFmt, glSrcType, row );
		}

		// Reset the transfer mode...
//		glPixelStorei (GL_UNPACK_ALIGNMENT, 0);
#else
		glPixelStorei (GL_UNPACK_ROW_LENGTH, w);
		glPixelStorei (GL_UNPACK_SKIP_ROWS, y0);
		glPixelStorei (GL_UNPACK_SKIP_PIXELS, x0);
		glTexSubImage2D (GL_TEXTURE_2D, nNumMipMap, x0, y0, x1-x0, y1-y0, glSrcFmt,glSrcType, ptr);

		// Reset the transfer mode...
		glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei (GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei (GL_UNPACK_SKIP_PIXELS, 0);
#endif
	}

	// Disable texture 0
	_CurrentTexture[0]= NULL;
	_CurrentTextureInfoGL[0]= NULL;
	_DriverGLStates.setTextureMode (CDriverGLStates::TextureDisabled);

	return true;
}

// ***************************************************************************
bool CDriverGL::uploadTextureCube (ITexture& tex, CRect& /* rect */, uint8 /* nNumMipMap */, uint8 /* nNumFace */)
{
	H_AUTO_OGL(uploadTextureCube)
	if (tex.TextureDrvShare == NULL)
		return false; // Texture not created
	if (!tex.isTextureCube())
		return false;

	return true;
}

// ***************************************************************************
bool CDriverGL::activateTexture(uint stage, ITexture *tex)
{
	H_AUTO_OGL(activateTexture)
	if (this->_CurrentTexture[stage]!=tex)
	{
		_DriverGLStates.activeTextureARB(stage);
		if(tex && tex->TextureDrvShare)
		{
			// get the drv info. should be not NULL.
			CTextureDrvInfosGL*	gltext;
			gltext= getTextureGl(*tex);

			// Profile, log the use of this texture
			//=========================================
			if (_SumTextureMemoryUsed)
			{
				// Insert the pointer of this texture
				if (gltext->TextureUsedIdx >= _TextureUsed.size() || _TextureUsed[gltext->TextureUsedIdx] != gltext)
				{
					gltext->TextureUsedIdx = _TextureUsed.size();
					_TextureUsed.push_back(gltext);
				}
			}

			if(tex->isTextureCube())
			{
				// setup texture mode, after activeTextureARB()
				_DriverGLStates.setTextureMode(CDriverGLStates::TextureCubeMap);

				if(_Extensions.ARBTextureCubeMap)
				{
					// Activate texturing...
					//======================

					// If the shared texture is the same than before, no op.
					if(_CurrentTextureInfoGL[stage] != gltext)
					{
						// Cache setup.
						_CurrentTextureInfoGL[stage]= gltext;

						// setup this texture
						glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, gltext->ID);

						// Change parameters of texture, if necessary.
						//============================================
						if(gltext->MagFilter!= tex->getMagFilter())
						{
							gltext->MagFilter= tex->getMagFilter();
							glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_MAG_FILTER, translateMagFilterToGl(gltext));
						}
						if(gltext->MinFilter!= tex->getMinFilter())
						{
							gltext->MinFilter= tex->getMinFilter();
							glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_MIN_FILTER, translateMinFilterToGl(gltext));
						}
					}
				}
			}
			else
			{
				// setup texture mode, after activeTextureARB()
				CDriverGLStates::TTextureMode textureMode= CDriverGLStates::Texture2D;
#ifndef USE_OPENGLES
				if(gltext->TextureMode == GL_TEXTURE_RECTANGLE_NV)
					textureMode = CDriverGLStates::TextureRect;
#endif
				_DriverGLStates.setTextureMode(/*CDriverGLStates::Texture2D*/textureMode);

				// Activate texture...
				//======================

				// If the shared texture is the same than before, no op.
				if(_CurrentTextureInfoGL[stage] != gltext)
				{
					// Cache setup.
					_CurrentTextureInfoGL[stage]= gltext;

					// setup this texture
					glBindTexture(gltext->TextureMode, gltext->ID);


					// Change parameters of texture, if necessary.
					//============================================
					if(gltext->WrapS!= tex->getWrapS())
					{
						gltext->WrapS= tex->getWrapS();
						glTexParameteri(gltext->TextureMode,GL_TEXTURE_WRAP_S, translateWrapToGl(gltext->WrapS, _Extensions));
					}
					if(gltext->WrapT!= tex->getWrapT())
					{
						gltext->WrapT= tex->getWrapT();
						glTexParameteri(gltext->TextureMode,GL_TEXTURE_WRAP_T, translateWrapToGl(gltext->WrapT, _Extensions));
					}
					if(gltext->MagFilter!= tex->getMagFilter())
					{
						gltext->MagFilter= tex->getMagFilter();
						glTexParameteri(gltext->TextureMode,GL_TEXTURE_MAG_FILTER, translateMagFilterToGl(gltext));
					}
					if(gltext->MinFilter!= tex->getMinFilter())
					{
						gltext->MinFilter= tex->getMinFilter();
						glTexParameteri(gltext->TextureMode,GL_TEXTURE_MIN_FILTER, translateMinFilterToGl(gltext));
					}
				}
			}
		}
		else
		{
			// Force no texturing for this stage.
			_CurrentTextureInfoGL[stage]= NULL;
			// setup texture mode, after activeTextureARB()
			_DriverGLStates.setTextureMode(CDriverGLStates::TextureDisabled);

#ifndef USE_OPENGLES
			if (_Extensions.ATITextureEnvCombine3)
			{
				// very strange bug with ATI cards : when a texture is set to NULL at a stage, the stage is still active sometimes...
				activateTexEnvMode(stage, _TexEnvReplace); // set the whole stage to replace fix the problem
			}
#endif
		}

		this->_CurrentTexture[stage]= tex;
	}

	return true;
}


// This maps the CMaterial::TTexOperator
static	const	GLenum	OperatorLUT[9]= { GL_REPLACE, GL_MODULATE, GL_ADD, GL_ADD_SIGNED_EXT, GL_INTERPOLATE_EXT, GL_INTERPOLATE_EXT, GL_INTERPOLATE_EXT, GL_INTERPOLATE_EXT, GL_BUMP_ENVMAP_ATI };

// This maps the CMaterial::TTexSource
static	const	GLenum	SourceLUT[4]= { GL_TEXTURE, GL_PREVIOUS_EXT, GL_PRIMARY_COLOR_EXT, GL_CONSTANT_EXT };

// This maps the CMaterial::TTexOperand
static	const	GLenum	OperandLUT[4]= { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA };

// This maps the CMaterial::TTexOperator, used for openGL Arg2 setup.
static	const	GLenum	InterpolateSrcLUT[8]= { GL_TEXTURE, GL_TEXTURE, GL_TEXTURE, GL_TEXTURE, GL_TEXTURE, GL_PREVIOUS_EXT, GL_PRIMARY_COLOR_EXT, GL_CONSTANT_EXT };

#ifndef USE_OPENGLES

// ***************************************************************************
// Set general tex env using ENV_COMBINE4 for the current setupped stage (used by forceActivateTexEnvMode)
static void	forceActivateTexEnvModeEnvCombine4(const CMaterial::CTexEnv  &env)
{
	H_AUTO_OGL(forceActivateTexEnvModeEnvCombine4);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);

	//== RGB ==
	switch(env.Env.OpRGB)
	{
		case CMaterial::Replace:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_COLOR);
			// Arg2 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			// Arg3 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_COLOR);
		break;
		case CMaterial::Add:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_COLOR);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
			// Arg3 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);
		break;
		case CMaterial::AddSigned:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD_SIGNED_EXT);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_COLOR);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
			// Arg3 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);
		break;
		case CMaterial::InterpolateTexture:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1 = (1 - texture)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
			// Arg3 = texture
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_ALPHA);
		break;
		case CMaterial::InterpolatePrevious:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1 = (1 - previous)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
			// Arg3 = previous
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_ALPHA);
		break;
		case CMaterial::InterpolateDiffuse:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1 = (1 - diffuse)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
			// Arg3 = diffuse
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_PRIMARY_COLOR_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_ALPHA);
		break;
		case CMaterial::InterpolateConstant:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1 = (1 - constant)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
			// Arg3 = constant
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_ALPHA);
		break;
		case CMaterial::Mad:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
			// Arg2 = env arg2
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, SourceLUT[env.Env.SrcArg2RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, OperandLUT[env.Env.OpArg2RGB]);
			// Arg3 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);
		break;
		default:
			// default is modulate
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
			// Arg2 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
			// Arg3 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_COLOR);
		break;
	}
	//== Alpha part ==
	switch(env.Env.OpAlpha)
	{
		case CMaterial::Replace:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
			// Arg1 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg3 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_SRC_ALPHA);
		break;
		case CMaterial::Add:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
			// Arg1 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, SourceLUT[env.Env.SrcArg1Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, OperandLUT[env.Env.OpArg1Alpha]);
			// Arg3 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_ONE_MINUS_SRC_ALPHA);
		break;
		case CMaterial::AddSigned:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD_SIGNED_EXT);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
			// Arg1 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, SourceLUT[env.Env.SrcArg1Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, OperandLUT[env.Env.OpArg1Alpha]);
			// Arg3 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_ONE_MINUS_SRC_ALPHA);
		break;
		case CMaterial::InterpolateTexture:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
			// Arg1 = (1 - texture)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, SourceLUT[env.Env.SrcArg1Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, OperandLUT[env.Env.OpArg1Alpha]);
			// Arg3 = texture
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_SRC_ALPHA);
		break;
		case CMaterial::InterpolatePrevious:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
			// Arg1 = (1 - previous)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, SourceLUT[env.Env.SrcArg1Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, OperandLUT[env.Env.OpArg1Alpha]);
			// Arg3 = previous
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_SRC_ALPHA);
		break;
		case CMaterial::InterpolateDiffuse:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
			// Arg1 = (1 - diffuse)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_PRIMARY_COLOR_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, SourceLUT[env.Env.SrcArg1Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, OperandLUT[env.Env.OpArg1Alpha]);
			// Arg3 = diffuse
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_PRIMARY_COLOR_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_SRC_ALPHA);
		break;
		case CMaterial::InterpolateConstant:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
			// Arg1 = (1 - constant)
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
			// Arg2 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, SourceLUT[env.Env.SrcArg1Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, OperandLUT[env.Env.OpArg1Alpha]);
			// Arg3 = constant
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_CONSTANT_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_SRC_ALPHA);
		break;
		case CMaterial::Mad:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
			// Arg1 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, SourceLUT[env.Env.SrcArg1Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, OperandLUT[env.Env.OpArg1Alpha]);
			// Arg2 = env arg2
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, SourceLUT[env.Env.SrcArg2Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, OperandLUT[env.Env.OpArg2Alpha]);
			// Arg3 = 1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_ONE_MINUS_SRC_ALPHA);
		break;
		default:
			// default is modulate
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
			// Arg0 = env arg0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
			// Arg1 = env arg1
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, SourceLUT[env.Env.SrcArg1Alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, OperandLUT[env.Env.OpArg1Alpha]);
			// Arg2 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
			// Arg3 = 0
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_ZERO);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_SRC_ALPHA);
		break;
	}
}

#endif

// ***************************************************************************
void		CDriverGL::forceActivateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env)
{
	H_AUTO_OGL(forceActivateTexEnvMode)
	// cache mgt.
	_CurrentTexEnv[stage].EnvPacked= env.EnvPacked;
	// Disable Special tex env f().
	_CurrentTexEnvSpecial[stage]= TexEnvSpecialDisabled;


	// Setup the gl env mode.
	_DriverGLStates.activeTextureARB(stage);

#ifdef USE_OPENGLES
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	// RGB.
	//=====
	if (env.Env.OpRGB == CMaterial::Mad)
	{
		//
		if (false)
		{
			// GL_MODULATE_ADD_ATI
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
			// Arg0.
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, SourceLUT[env.Env.SrcArg0RGB] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1.
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, SourceLUT[env.Env.SrcArg1RGB] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, OperandLUT[env.Env.OpArg1RGB]);
			// Arg2.
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, SourceLUT[env.Env.SrcArg2RGB] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, OperandLUT[env.Env.OpArg2RGB]);
		}
		else
		{
			// fallback to modulate ..
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			//
			// Arg0.
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, SourceLUT[env.Env.SrcArg0RGB] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1.
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, SourceLUT[env.Env.SrcArg1RGB] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, OperandLUT[env.Env.OpArg1RGB]);
		}
	}
	else
	{
		// Operator.
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, OperatorLUT[env.Env.OpRGB] );
		// Arg0.
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, SourceLUT[env.Env.SrcArg0RGB] );
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, OperandLUT[env.Env.OpArg0RGB]);
		// Arg1.
		if(env.Env.OpRGB > CMaterial::Replace)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, SourceLUT[env.Env.SrcArg1RGB] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, OperandLUT[env.Env.OpArg1RGB]);
			// Arg2.
			if(env.Env.OpRGB >= CMaterial::InterpolateTexture )
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, InterpolateSrcLUT[env.Env.OpRGB] );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
			}
		}
	}
	// Alpha.
	//=====
	if (env.Env.OpAlpha == CMaterial::Mad)
	{
		if (true)
		{
			// GL_MODULATE_ADD_ATI
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
			// Arg0.
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, SourceLUT[env.Env.SrcArg0Alpha] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, OperandLUT[env.Env.OpArg0Alpha]);
			// Arg1.
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, SourceLUT[env.Env.SrcArg1Alpha] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, OperandLUT[env.Env.OpArg1Alpha]);
			// Arg2.
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, SourceLUT[env.Env.SrcArg2Alpha] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, OperandLUT[env.Env.OpArg2Alpha]);
		}
		else
		{
			// fallback to modulate ..
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			//
			// Arg0.
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, SourceLUT[env.Env.SrcArg0RGB] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, OperandLUT[env.Env.OpArg0RGB]);
			// Arg1.
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, SourceLUT[env.Env.SrcArg1RGB] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, OperandLUT[env.Env.OpArg1RGB]);
		}
	}
	else
	{
		// Operator.
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, OperatorLUT[env.Env.OpAlpha] );
		// Arg0.
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, SourceLUT[env.Env.SrcArg0Alpha] );
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, OperandLUT[env.Env.OpArg0Alpha]);
		// Arg1.
		if(env.Env.OpAlpha > CMaterial::Replace)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, SourceLUT[env.Env.SrcArg1Alpha] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, OperandLUT[env.Env.OpArg1Alpha]);
			// Arg2.
			if(env.Env.OpAlpha >= CMaterial::InterpolateTexture )
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, InterpolateSrcLUT[env.Env.OpAlpha] );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
			}
		}
	}
#else
	// if the Mad operator is used, then
	// "Normal drivers", setup EnvCombine.
	if(_Extensions.EXTTextureEnvCombine)
	{
		// if Mad operator is used, special setup
		if ((env.Env.OpAlpha == CMaterial::Mad || env.Env.OpRGB == CMaterial::Mad) && _Extensions.NVTextureEnvCombine4)
		{
			forceActivateTexEnvModeEnvCombine4(env);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			// RGB.
			//=====
			if (env.Env.OpRGB == CMaterial::Mad)
			{
				//
				if (_Extensions.ATITextureEnvCombine3)
				{
					glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE_ADD_ATI);
					// Arg0.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
					// Arg1.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
					// Arg2.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, SourceLUT[env.Env.SrcArg2RGB] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, OperandLUT[env.Env.OpArg2RGB]);
				}
				else
				{
					// fallback to modulate ..
					glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
					//
					// Arg0.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
					// Arg1.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
				}
			}
			else
			{
				// Operator.
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, OperatorLUT[env.Env.OpRGB] );
				// Arg0.
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, SourceLUT[env.Env.SrcArg0RGB] );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, OperandLUT[env.Env.OpArg0RGB]);
				// Arg1.
				if(env.Env.OpRGB > CMaterial::Replace)
				{
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, SourceLUT[env.Env.SrcArg1RGB] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, OperandLUT[env.Env.OpArg1RGB]);
					// Arg2.
					if(env.Env.OpRGB >= CMaterial::InterpolateTexture )
					{
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, InterpolateSrcLUT[env.Env.OpRGB] );
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_ALPHA);
					}
				}
			}
			// Alpha.
			//=====
			if (env.Env.OpAlpha == CMaterial::Mad)
			{
				if (_Extensions.ATITextureEnvCombine3)
				{
					glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE_ADD_ATI);
					// Arg0.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
					// Arg1.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, SourceLUT[env.Env.SrcArg1Alpha] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, OperandLUT[env.Env.OpArg1Alpha]);
					// Arg2.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, SourceLUT[env.Env.SrcArg2Alpha] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, OperandLUT[env.Env.OpArg2Alpha]);
				}
				else
				{
					// fallback to modulate ..
					glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
					//
					// Arg0.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0RGB] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0RGB]);
					// Arg1.
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, SourceLUT[env.Env.SrcArg1RGB] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, OperandLUT[env.Env.OpArg1RGB]);
				}
			}
			else
			{
				// Operator.
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, OperatorLUT[env.Env.OpAlpha] );
				// Arg0.
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, SourceLUT[env.Env.SrcArg0Alpha] );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, OperandLUT[env.Env.OpArg0Alpha]);
				// Arg1.
				if(env.Env.OpAlpha > CMaterial::Replace)
				{
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, SourceLUT[env.Env.SrcArg1Alpha] );
					glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, OperandLUT[env.Env.OpArg1Alpha]);
					// Arg2.
					if(env.Env.OpAlpha >= CMaterial::InterpolateTexture )
					{
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, InterpolateSrcLUT[env.Env.OpAlpha] );
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
					}
				}
			}
		}
	}
	// Very Bad drivers.
	else
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
#endif
}

// ***************************************************************************
void		CDriverGL::activateTexEnvColor(uint stage, NLMISC::CRGBA col)
{
	H_AUTO_OGL(CDriverGL_activateTexEnvColor)
	if (col != _CurrentTexEnv[stage].ConstantColor)
	{
		forceActivateTexEnvColor(stage, col);
	}
}

// ***************************************************************************
void		CDriverGL::activateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env)
{
	H_AUTO_OGL(CDriverGL_activateTexEnvMode)
	// If a special Texture environnement is setuped, or if not the same normal texture environnement,
	// must setup a new normal Texture environnement.
	if(_CurrentTexEnvSpecial[stage] != TexEnvSpecialDisabled || _CurrentTexEnv[stage].EnvPacked!= env.EnvPacked)
	{
		forceActivateTexEnvMode(stage, env);
	}
}


// ***************************************************************************
void		CDriverGL::activateTexEnvColor(uint stage, const CMaterial::CTexEnv  &env)
{
	H_AUTO_OGL(CDriverGL_activateTexEnvColor)
	if(_CurrentTexEnv[stage].ConstantColor!= env.ConstantColor)
	{
		forceActivateTexEnvColor(stage, env);
	}
}


// ***************************************************************************
void		CDriverGL::forceDXTCCompression(bool dxtcComp)
{
	H_AUTO_OGL(CDriverGL_forceDXTCCompression)
	_ForceDXTCCompression= dxtcComp;
}

// ***************************************************************************
void		CDriverGL::setAnisotropicFilter(sint filtering)
{
	H_AUTO_OGL(CDriverGL_setAnisotropicFiltering);

	if (!_Extensions.EXTTextureFilterAnisotropic) return;

	if (filtering < 0 || filtering > _Extensions.EXTTextureFilterAnisotropicMaximum)
	{
		// set maximum value for anisotropic filter
		_AnisotropicFilter = _Extensions.EXTTextureFilterAnisotropicMaximum;
	}
	else
	{
		// set specified value for anisotropic filter
		_AnisotropicFilter = filtering;
	}
}

// ***************************************************************************
void		CDriverGL::forceTextureResize(uint divisor)
{
	H_AUTO_OGL(CDriverGL_forceTextureResize)
	clamp(divisor, 1U, 256U);

	// 16 -> 4.
	_ForceTextureResizePower= getPowerOf2(divisor);
}


// ***************************************************************************
void		CDriverGL::swapTextureHandle(ITexture &tex0, ITexture &tex1)
{
	H_AUTO_OGL(CDriverGL_swapTextureHandle)
	// ensure creation of both texture
	setupTexture(tex0);
	setupTexture(tex1);

	// avoid any problem, disable all textures
	for(uint stage = 0; stage < inlGetNumTextStages(); stage++)
	{
		activateTexture(stage, NULL);
	}

	// get the handle.
	CTextureDrvInfosGL	*t0= getTextureGl(tex0);
	CTextureDrvInfosGL	*t1= getTextureGl(tex1);

	/* Swap contents. Can't swap directly the pointers cause would have to change all CTextureDrvShare which point on
		Can't do swap(*t0, *t1), because must keep the correct _DriverIterator
	*/
	swap(t0->ID, t1->ID);
	swap(t0->MipMap, t1->MipMap);
	swap(t0->Compressed, t1->Compressed);
	swap(t0->TextureMemory, t1->TextureMemory);
	swap(t0->WrapS, t1->WrapS);
	swap(t0->WrapT, t1->WrapT);
	swap(t0->MagFilter, t1->MagFilter);
	swap(t0->MinFilter, t1->MinFilter);
	swap(t0->TextureMode, t1->TextureMode);
	swap(t0->FBOId, t1->FBOId);
	swap(t0->DepthStencilFBO, t1->DepthStencilFBO);
	swap(t0->InitFBO, t1->InitFBO);
}


// ***************************************************************************
uintptr_t CDriverGL::getTextureHandle(const ITexture &tex)
{
	H_AUTO_OGL(CDriverGL_getTextureHandle)
	// If DrvShare not setuped
	if(!tex.TextureDrvShare)
		return 0;

	// If DrvInfo not setuped
	const CTextureDrvInfosGL	*t0= (const CTextureDrvInfosGL*)(const ITextureDrvInfos*)(tex.TextureDrvShare->DrvTexture);
	if(!t0)
		return 0;

	return t0->ID;
}

// ***************************************************************************

/*
	Under opengl, "render to texture" uses the frame buffer. The scene is rendered into the current frame buffer and the result
	is copied into the texture.

	setRenderTarget (tex) does nothing but backup the framebuffer area used and updates the viewport and scissor
	setRenderTarget (NULL) copies the modified framebuffer area into "tex" and then, updates the viewport and scissor
 */

bool CDriverGL::setRenderTarget (ITexture *tex, uint32 x, uint32 y, uint32 width, uint32 height, uint32 mipmapLevel, uint32 cubeFace)
{
	H_AUTO_OGL(CDriverGL_setRenderTarget )

	// make backup of offscreen buffer to old texture if not using FBOs
	if (!_RenderTargetFBO && _TextureTarget && _TextureTargetUpload && (_TextureTarget != tex || _TextureTargetCubeFace != cubeFace))
	{
		// Flush it
		copyFrameBufferToTexture (_TextureTarget, _TextureTargetLevel, _TextureTargetX, _TextureTargetY, 0,
			0, _TextureTargetWidth, _TextureTargetHeight, _TextureTargetCubeFace);
	}

	// Set a new texture as render target ?
	if (tex)
	{
		// Check the texture is a render target
		nlassertex (tex->getRenderTarget(), ("The texture must be a render target. Call ITexture::setRenderTarget(true)."));

		if(tex->isBloomTexture() && supportBloomEffect())
		{
			// NOTE: No support for mip map level here!

			uint32 w, h;
			getWindowSize(w, h);

			getViewport(_OldViewport);

			if (!width) width = tex->getWidth();
			if (!height) height = tex->getHeight();

			CViewport newVP;
			newVP.init(0, 0, ((float)width/(float)w), ((float)height/(float)h));
			setupViewport(newVP);

			_RenderTargetFBO = tex;

			return activeFrameBufferObject(tex);
		}

		// Backup the parameters
		_TextureTargetLevel = mipmapLevel;
		_TextureTargetX = x;
		_TextureTargetY = y;
		_TextureTargetWidth = width;
		_TextureTargetHeight = height;
		_TextureTargetUpload = true;
		_TextureTargetCubeFace = cubeFace;
	}
	else if (_RenderTargetFBO)
	{
		activeFrameBufferObject(NULL);
		setupViewport(_OldViewport);
		_OldViewport = _CurrViewport;

		_RenderTargetFBO = NULL;
		return false;
	}

	// Backup the texture
	_TextureTarget = tex;

	// Update the viewport
	setupViewport (_CurrViewport);

	// Update the scissor
	setupScissor (_CurrScissor);

	_RenderTargetFBO = NULL;
	_OldViewport = _CurrViewport;

	return true;
}

ITexture *CDriverGL::getRenderTarget() const
{
	return _RenderTargetFBO ? _RenderTargetFBO : _TextureTarget;
}

// ***************************************************************************

bool CDriverGL::copyTargetToTexture (ITexture *tex,
												 uint32 offsetx,
												 uint32 offsety,
												 uint32 x,
												 uint32 y,
												 uint32 width,
												 uint32 height,
		                                         uint32 mipmapLevel)
{
	H_AUTO_OGL(CDriverGL_copyTargetToTexture)
	if (!_TextureTarget)
		return false;
	_TextureTargetUpload = false;
	if ((width == 0) || (height == 0))
	{
		uint32 _width;
		uint32 _height;
		getRenderTargetSize (_width, _height);
		if (width == 0)
			width = _width;
		if (height == 0)
			height = _height;
	}
	copyFrameBufferToTexture(tex, mipmapLevel, offsetx, offsety, x, y, width, height);
	return true;
}

// ***************************************************************************

bool CDriverGL::getRenderTargetSize (uint32 &width, uint32 &height)
{
	H_AUTO_OGL(CDriverGL_getRenderTargetSize)
	NLMISC::CSmartPtr<ITexture> tex = _RenderTargetFBO ? _RenderTargetFBO : (_TextureTarget ? _TextureTarget : NULL);
	if (tex)
	{
		width = tex->getWidth();
		height = tex->getHeight();
	}
	else
	{
		getWindowSize(width, height);
	}

	return false;
}

// ***************************************************************************

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
