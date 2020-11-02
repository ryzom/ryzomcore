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

#include "nel/3d/texture_file.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/debug.h"
#include "nel/misc/hierarchical_timer.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

bool CTextureFile::_SupportNonPowerOfTwoTextures = false;

///==================================================================
void CTextureFile::buildBitmapFromFile(NLMISC::CBitmap &dest, const std::string &fileName, bool asyncload, uint8 mipMapSkip, bool enlargeCanvasNonPOW2Tex)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	H_AUTO( NL3D_buildBitmapFromFile )

	NLMISC::CIFile f;

	string file;
	{
		H_AUTO( NL3D_buildBitmapPathLookup )
		file = CPath::lookup(fileName, false);
	}
	if( file.empty() )
	{
		H_AUTO( NL3D_makeDummyBitmap )
		// Not found...
		dest.makeDummy();
		nlwarning("Missing textureFile: %s", fileName.c_str());
		return;
	}
	{
		H_AUTO( NL3D_buildBitmapFileCache )

		f.setAsyncLoading (asyncload);
		f.setCacheFileOnOpen(false);	//asyncload); //AJM: significant performance loss for caching when loading textures
		f.allowBNPCacheFileOnOpen(false);	//asyncload);
	}

	{
		H_AUTO( NL3D_openBitmapFile)
		// Load bitmap.
		if (f.open(file))
		{
			H_AUTO( NL3D_loadBitmap )
			// skip DDS mipmap if wanted
			dest.load (f, mipMapSkip);
		}
		else
		{
			H_AUTO( NL3D_makeDummyBitmap )
				// Not found...
			dest.makeDummy();
			nlwarning("Missing textureFile: %s", fileName.c_str());
			return;
		}
	}
	// *** Need usercolor computing ?

	// Texture not compressed ?
	if (dest.PixelFormat == RGBA)
	{
		H_AUTO( NL3D_buildBitmapUserColor )
		// Make a filename
		string path = CFile::getFilename(fileName);
		string ext = strrchr (fileName.c_str(), '.');
		path.resize (path.size () - ext.size());
		path += "_usercolor" + ext;

		// Loopup the texture
		string file2 = CPath::lookup( path, false, false);
		if (!file2.empty())
		{
			// The file2 exist, load and compute it
			CBitmap bitmap;
			bitmap.loadGrayscaleAsAlpha (true);

			// Open and read the file2
			NLMISC::CIFile f2;
			f2.setAsyncLoading (asyncload);
			f2.setCacheFileOnOpen (asyncload); // Same as async loading
			if (f2.open(file2))
			{
				bitmap.load(f2);
			}
			else
			{
				// Not found...
				dest.makeDummy();
				nlwarning("Missing textureFile: %s", file2.c_str());
				return;
			}

			// Texture are the same size ?
			if ((dest.getWidth() == bitmap.getWidth()) && (dest.getHeight() == bitmap.getHeight()))
			{
				// Convert in Alpha
				if (bitmap.convertToType (CBitmap::Alpha))
				{
					// Compute it
					uint8 *userColor = (uint8 *)&(bitmap.getPixels ()[0]);
					CRGBA *color = (CRGBA *)&(dest.getPixels ()[0]);

					// For each pixel
					uint pixelCount = dest.getWidth()*dest.getHeight();
					uint pixel;
					for (pixel = 0; pixel<pixelCount; pixel++)
					{
						if (userColor[pixel]==0)
						{
							// New code: use new restrictions from IDriver.
							float	Rt, Gt, Bt, At;
							float	Lt;
							float	Rtm, Gtm, Btm, Atm;

							// read 0-1 RGB pixel.
							Rt= (float)color[pixel].R/255;
							Gt= (float)color[pixel].G/255;
							Bt= (float)color[pixel].B/255;
							Lt= Rt*0.3f + Gt*0.56f + Bt*0.14f;

							// take Alpha from userColor src.
							At= (float)userColor[pixel]/255;
							Atm= 1-Lt*(1-At);

							// If normal case.
							if(Atm>0)
							{
								Rtm= Rt*At / Atm;
								Gtm= Gt*At / Atm;
								Btm= Bt*At / Atm;
							}
							// Else special case: At==0, and Lt==1.
							else
							{
								Rtm= Gtm= Btm= 0;
							}

							// copy to buffer.
							sint	r,g,b,a;
							r= (sint)(Rtm*255+0.5f);
							g= (sint)(Gtm*255+0.5f);
							b= (sint)(Btm*255+0.5f);
							a= (sint)(Atm*255+0.5f);
							clamp(r, 0,255);
							clamp(g, 0,255);
							clamp(b, 0,255);
							clamp(a, 0,255);
							color[pixel].R = (uint8)r;
							color[pixel].G = (uint8)g;
							color[pixel].B = (uint8)b;
							color[pixel].A = (uint8)a;
						}
					}
				}
				else
				{
					nlinfo ("Can't convert the usercolor texture %s in alpha mode", file2.c_str());
				}
			}
			else
			{
				// Error
				nlinfo ("User color texture is not the same size than the texture. (Tex : %s, Usercolor : %s)", file.c_str(), file2.c_str());
			}
		}
	}
	if(!isPowerOf2(dest.getWidth()) || !isPowerOf2(dest.getHeight()) )
	{
		H_AUTO( NL3D_buildBitmapPowerOf2 )
		// If the user want to correct those texture so that their canvas is enlarged
		if (enlargeCanvasNonPOW2Tex)
		{
			uint pow2w = NLMISC::raiseToNextPowerOf2(dest.getWidth());
			uint pow2h = NLMISC::raiseToNextPowerOf2(dest.getHeight());
			CBitmap enlargedBitmap;
			enlargedBitmap.resize(pow2w, pow2h, dest.PixelFormat);
			// blit src bitmap
			enlargedBitmap.blit(&dest, 0, 0);
			// swap them
			dest.swap(enlargedBitmap);
		}
		else if (!_SupportNonPowerOfTwoTextures || dest.getMipMapCount() > 1)
		{
			// Bug...
			nlwarning("TextureFile: %s is not a Power Of 2: %d,%d", fileName.c_str(), dest.getWidth(), dest.getHeight());
			dest.makeNonPowerOf2Dummy();
		}
	}
}


/*==================================================================*\
							CTEXTUREFILE
\*==================================================================*/

/*------------------------------------------------------------------*\
							doGenerate()
\*------------------------------------------------------------------*/
void CTextureFile::doGenerate(bool async)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	H_AUTO( NL3D_TextureFileDoGenerate )

	buildBitmapFromFile(*this, _FileName, async, _MipMapSkipAtLoad, _EnlargeCanvasNonPOW2Tex);
}


// ***************************************************************************
void	CTextureFile::serial(NLMISC::IStream &f)
{
	/*
	Version 1:
		- AllowDegradation.
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(1);

	// serial the base part of ITexture.
	ITexture::serial(f);

	f.serial(_FileName);
	if(ver>=1)
		f.serial(_AllowDegradation);
	else if(f.isReading())
		_AllowDegradation= true;

	if(f.isReading())
		touch();
}


// ***************************************************************************
void	CTextureFile::setAllowDegradation(bool allow)
{
	_AllowDegradation= allow;
}

// ***************************************************************************
CTextureFile::CTextureFile(const CTextureFile &other) : ITexture(other)
{
	dupInfo(other);
}

// ***************************************************************************
CTextureFile &CTextureFile::operator = (const CTextureFile &other)
{
	// copy base infos
	(ITexture &) *this = (ITexture &) other;
	dupInfo(other);
	return *this;
}

// ***************************************************************************
void CTextureFile::dupInfo(const CTextureFile &other)
{
	_FileName         = other._FileName;
	_AllowDegradation = other._AllowDegradation;
	_SupportSharing	  = other._SupportSharing;
	_MipMapSkipAtLoad = other._MipMapSkipAtLoad;
	_EnlargeCanvasNonPOW2Tex   = other._EnlargeCanvasNonPOW2Tex;
}


// ***************************************************************************
void			CTextureFile::enableSharing(bool enable)
{
	_SupportSharing = enable;
}

// ***************************************************************************
void			CTextureFile::setMipMapSkipAtLoad(uint8 level)
{
	_MipMapSkipAtLoad= level;
}

// ***************************************************************************
std::string		CTextureFile::getShareName() const
{
	return toLowerAscii(_FileName);
}


} // NL3D
