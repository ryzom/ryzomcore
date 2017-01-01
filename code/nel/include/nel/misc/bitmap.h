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


#ifndef NL_BITMAP_H
#define NL_BITMAP_H


#include "types_nl.h"
#include "rgba.h"
#include "debug.h"
#include <vector>
#include "object_vector.h"


namespace NLMISC
{


class IStream;

//------------------ DDS STUFFS --------------------

#ifndef NL_MAKEFOURCC
    #define NL_MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |   \
                ((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))
#endif

const uint32	DDS_HEADER = NL_MAKEFOURCC('D', 'D', 'S', ' ');
const uint32	DXT_HEADER = NL_MAKEFOURCC('D', 'X', 'T', '\0');
const uint32	PNG_HEADER = NL_MAKEFOURCC(0x89, 'P', 'N', 'G');
const uint32	JPG_HEADER = NL_MAKEFOURCC(0xff, 0xd8, 0xff, 0xe0);
const uint32	GIF_HEADER = NL_MAKEFOURCC('G', 'I', 'F', '8');


// dwLinearSize is valid
#define DDSD_LINEARSIZE         0x00080000l


//---------------- END OF DDS STUFFS ------------------


const uint8	MAX_MIPMAP = 16;




/**
 * Class Bitmap
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class CBitmap
{
protected :
	CObjectVector<uint8> _Data[MAX_MIPMAP];

	// The number of mipmaps. base image IS a mipmap. 1 means a base image with no mipmaping.
	uint8	_MipMapCount;
	bool	_LoadGrayscaleAsAlpha;
	uint32	_Width;
	uint32	_Height;

	// don't forget to update operator=() and swap() if adding a data member

private :


	/**
	 * blend 2 integers between 0 and 255 .
	 * \param n0 first integer
	 * \param n1 second integer
	 * \param coef coefficient for the first integer (must be in [0,256])
	 */
	uint32 blend(uint32 &n0, uint32 &n1, uint32 coef0);


	/**
	 * Read a DDS from an IStream.
	 * The bitmap is readen as a set of bytes and stocked compressed.
	 * Width and Height are multiple of 4.
	 * \param IStream The stream must be in reading mode.
	 * \return image depth
	 * \throw EDDSBadHeader : surface is header is not valid.
	 */
	uint8 readDDS(NLMISC::IStream &f, uint mipMapSkip);


	/**
	 * Read a TGA from an IStream.
	 * TGA pictures can be in 24 or 32 bits, RLE or uncompressed
	 * \param f IStream (must be a reading stream)
	 * \return image depth if succeed, 0 else
	 */
	uint8 readTGA(	NLMISC::IStream &f);


	/**
	 * Read a PNG from an IStream.
	 * PNG pictures can be in 24 or 32 bits
	 * \param f IStream (must be a reading stream)
	 * \return image depth if succeed, 0 else
	 */
	uint8 readPNG( NLMISC::IStream &f );


	/**
	 * Read a JPG from an IStream.
	 * JPG pictures can be in 24
	 * \param f IStream (must be a reading stream)
	 * \return image depth if succeed, 0 else
	 */
	uint8 readJPG( NLMISC::IStream &f );


	/**
	 * Read a GIF from an IStream.
	 * GIF pictures are all converted to 32bit
	 * \param f IStream (must be a reading stream)
	 * \return image depth if succeed, 0 else
	 */
	uint8 readGIF( NLMISC::IStream &f );


	/**
	 * Change bitmap format
	 *
	 * about DXTC1 to DXTC5 :
	 * Does nothing if the format is not DXTC1
	 * about alpha encoding :
	 *		alpha0 == alpha1
	 *		code(x,y) == 7 for every (x,y)
	 *
	 * about luminance to alpha and alpha to luminance :
	 *      the buffer keeps unchanged
	 *
	 */
	///@{
	bool convertToDXTC5();

	bool convertToRGBA();
	bool luminanceToRGBA();
	bool alphaToRGBA();
	bool alphaLuminanceToRGBA();

	bool convertToLuminance();
	bool rgbaToLuminance();
	bool alphaToLuminance();
	bool alphaLuminanceToLuminance();

	bool convertToAlpha();
	bool rgbaToAlpha();
	bool luminanceToAlpha();
	bool alphaLuminanceToAlpha();

	bool convertToAlphaLuminance();
	bool rgbaToAlphaLuminance();
	bool luminanceToAlphaLuminance();
	bool alphaToAlphaLuminance();

	///@}

	/**
	 * Decompress bitmap compressed with S3TC DXT1 algorithm.
	 * \param alpha if alpha is true there's alpha.
	 */
	bool decompressDXT1(bool alpha);

	/**
	 * Decompress bitmap compressed with S3TC DXT3 algorithm.
	 * \throw EAllocationFailure : can't allocate memory.
	 */
	bool decompressDXT3();


	/**
	 * Decompress bitmap compressed with S3TC DXT3 algorithm.
	 * \throw EAllocationFailure : can't allocate memory.
	 */
	bool decompressDXT5();


	/**
	 * Extracting RGBA infos from a 16bits word. (used by S3TC decompression)
	 * \param color a 16bits integer
	 * \param r a CRGBA
	 */
	static void uncompress(uint16 color, NLMISC::CRGBA &);


	/**
	 * The resample function
	 * \param pSrc CRGBA array
	 * \param pDest CRGBA array for storing resampled texture
	 * \param nSrcWidth original width
	 * \param nSrcHeight original height
	 * \param nDestWidth width after resample
	 * \param nDestHeight height after resample
	 */
	void resamplePicture32 (const NLMISC::CRGBA *pSrc, NLMISC::CRGBA *pDest,
							 sint32 nSrcWidth, sint32 nSrcHeight,
							 sint32 nDestWidth, sint32 nDestHeight);

	/**
	 * The FAST resample function : works only when reducing the size by two
	 * and when the image is square
	 * \param pSrc CRGBA array
	 * \param pDest CRGBA array for storing resampled texture
	 * \param nSrcWidth original width
	 * \param nSrcHeight original height
	 * \param nDestWidth width after resample
	 * \param nDestHeight height after resample
	 */
	void resamplePicture32Fast (const NLMISC::CRGBA *pSrc, NLMISC::CRGBA *pDest,
								sint32 nSrcWidth, sint32 nSrcHeight,
								sint32 nDestWidth, sint32 nDestHeight);


	/**
	 * The grayscale resample function
	 * \param pSrc grayscale 8-bits array
	 * \param pDest grayscale 8-bits array for storing resampled texture
	 * \param nSrcWidth original width
	 * \param nSrcHeight original height
	 * \param nDestWidth width after resample
	 * \param nDestHeight height after resample
	 */
	void resamplePicture8 (const uint8 *pSrc, uint8 *pDest,
							 sint32 nSrcWidth, sint32 nSrcHeight,
							 sint32 nDestWidth, sint32 nDestHeight);

	/**
	 * The FAST resample function : works only when reducing the size by two
	 * and when the image is square
	 * \param pSrc grayscale 8-bits array
	 * \param pDest grayscale 8-bits array for storing resampled texture
	 * \param nSrcWidth original width
	 * \param nSrcHeight original height
	 * \param nDestWidth width after resample
	 * \param nDestHeight height after resample
	 */
	void resamplePicture8Fast (const uint8 *pSrc, uint8 *pDest,
								sint32 nSrcWidth, sint32 nSrcHeight,
								sint32 nDestWidth, sint32 nDestHeight);


	/**
	 * Quadratic interpolator
	 * \return the interpolation in (x,y) of the values (xy**)
	 */
	float getColorInterp (float x, float y, float xy00, float xy01, float xy10, float xy11) const;


	/// name  DXTC single texel read
	//@{
		static CRGBA getDXTCColorFromBlock(const uint8 *block, sint x, sint y);
		CRGBA getDXTC1Texel(sint x, sint y, uint32 numMipMap) const;
		CRGBA getDXTC3Texel(sint x, sint y, uint32 numMipMap) const;
		CRGBA getDXTC5Texel(sint x, sint y, uint32 numMipMap) const;
	//@}


	CRGBA getRGBAPixel(sint x, sint y, uint32 numMipMap /*=0*/) const;


	// Make a dummy from a bitfield
	void	makeDummyFromBitField(const uint8	bitmap[1024]);

	// Flip DXTC
	void	flipHDXTCBlockColor(uint8 *bitColor, uint32 w);
	void	flipVDXTCBlockColor(uint8 *bitColor, uint32 h);
	void	flipHDXTCBlockAlpha3(uint8 *bitColor, uint32 w);
	void	flipVDXTCBlockAlpha3(uint8 *bitColor, uint32 h);
	void	flipHDXTCBlockAlpha5(uint8 *bitColor, uint32 w);
	void	flipVDXTCBlockAlpha5(uint8 *bitColor, uint32 h);
	void	flipDXTC(bool vertical);
	void	flipDXTCMipMap(bool vertical, uint mm, uint type);

public:

	enum TType
	{
		RGBA=0,
		Luminance,
		Alpha,
		AlphaLuminance,
		DXTC1,
		DXTC1Alpha,
		DXTC3,
		DXTC5,
		DsDt,
		ModeCount,
		DonTKnow=0xffffffff
	} PixelFormat;

	static const uint32 bitPerPixels[ModeCount];
	static const uint32 DXTC1HEADER;
	static const uint32 DXTC3HEADER;
	static const uint32 DXTC5HEADER;

	// don't forget to update operator=() and swap() if adding a data member

	CBitmap();
	virtual ~CBitmap();

	// swap 2 bitmaps contents
	void	swap(CBitmap &other);

	/**
	 * Read a bitmap(TGA, JPEG, PNG or DDS) from an IStream.
	 * Bitmap supported are DDS (DXTC1, DXTC1 with Alpha, DXTC3, DXTC5), PNG, JPEG and
	 * uncompressed TGA (24 and 32 bits).
	 * \param IStream The stream must be in reading mode.
	 * \param mipMapSkip if the file is a DDS with mipMap. N=mipMapSkip mipmaps are skipped.
	 * \return image depth (24 or 32), or 0 if load failed
	 * \throw ESeekFailed : seek has failed
	 */
	uint8	load(NLMISC::IStream &f, uint mipMapSkip=0);


	/**
	 * Determinate the bitmap size from a bitmap(TGA or DDS) from an IStream. load just header of the file.
	 * Bitmap supported are DDS (DXTC1, DXTC1 with Alpha, DXTC3, DXTC5), PNG, JPEG and
	 * uncompressed TGA (24 and 32 bits).
	 * NB: at the end, f is seeked to begin.
	 * \param IStream The stream must be in reading mode.
	 * \param width the width of the image. 0 if fails.
	 * \param height the height of the image. 0 if fails.
	 * \throw ESeekFailed : seek has failed
	 */
	static void		loadSize(NLMISC::IStream &f, uint32 &width, uint32 &height);


	/** same than other loadSize(), but with a pathName.
	 * \see loadSize()
	 */
	static void		loadSize(const std::string &path, uint32 &retWidth, uint32 &retHeight);


	/**
	 * Make a dummy "?" texture. Useful for file not found. Mode is rgba.
	 */
	void	makeDummy();


	/**
	 * Make a dummy "2" texture. Useful for file not power of 2. Mode is rgba.
	 */
	void	makeNonPowerOf2Dummy();


	/**
	 * Make a bitmap fully opaque (set alpha to 255).
	 */
	void	makeOpaque();


	/**
	* Make fully transparent pixels (alpha 0) black.
	*/
	void	makeTransparentPixelsBlack();


	/**
	 * Return if the bitmap has uniform alpha values for all pixels.
	 * \param alpha return the uniform value if return is true
	 * \return uniform or not
	 */
	bool	isAlphaUniform(uint8 *alpha = NULL) const;


	/**
	 * Return if the bitmap is a real grayscale.
	 * \return grayscale or not
	 */
	bool	isGrayscale() const;

	/**
	 * Return the pixels buffer of the image, or of one of its mipmap.
	 * Return a reference of an array in pixel format get with getPixelFormat().
	 * \return CObjectVector<uint8>& RGBA pixels
	 */
	///@{
	CObjectVector<uint8>& getPixels(uint32 numMipMap = 0)
	{
		//nlassert (numMipMap<=_MipMapCount);
		return _Data[numMipMap];
	}
	const CObjectVector<uint8>& getPixels(uint32 numMipMap = 0) const
	{
		//nlassert (numMipMap<=_MipMapCount);
		return _Data[numMipMap];
	}
	/** Gain ownership of this texture datas. As a result, the bitmap is reseted (put in the same state than when ctor is called, e.g a single mipmap with null size)
	  * The CObjectVector objects that contains the bitmap (one per mipmap) datas are 'swapped' with those in the array  provided by the caller.
	  * NB : The user must provide at least min(getMipMapCount(), maxMipMapCount) entries in the array
	  * \param mipmapArray Array of mipmap that receive the bitmap datas
	  * \param maxMipMapCount Max number of mipmap to be copied in the destination array.
	  */
	void unattachPixels(CObjectVector<uint8> *mipmapDestArray, uint maxMipMapCount = MAX_MIPMAP);

	///@}


	/**
	 * Convert bitmap to another type
	 * conversion to rgba always work. No-op if already rgba.
	 * \param type new type for the bitmap
	 * \return true if conversion succeeded, false else
	 */
	bool convertToType (TType type);



	/**
	 * Return the format of pixels stored at the present time in the object buffer.
	 * \return Pixel format (rgba luminance alpha alphaLuminance dxtc1 dxtc1Alpha dxtc3 dxtc5)
	 */
	TType getPixelFormat() const
	{
		return PixelFormat;
	}


	/**
	 * Return the image width, or a mipmap width.
	 * \param mipMap mipmap level
	 * \return image width (0 if mipmap not found)
	 */
	virtual uint32 getWidth(uint32 numMipMap = 0) const;


	/**
	 * Return the image height, or a mipmap height.
	 * \param mipMap mipmap level
	 * \return image height (0 if mipmap not found)
	 */
	virtual uint32 getHeight(uint32 numMipMap = 0) const;


	/**
	 * Return the size (in pixels) of the image: <=> getHeight()*getWidth().
	 * \param mipMap mipmap level
	 * \return image size (0 if mipmap not found)
	 */
	uint32 getSize(uint32 numMipMap = 0) const;


	/**
	 * Return the number of mipmaps. Level0 is a mipmap...
	 * \return number of mipmaps. 0 if no image at all. 1 if no mipmaping (for the base level).
	 */
	uint32 getMipMapCount() const
	{
		return _MipMapCount;
	}

	// Compute the number of mipmap needed for that bitmap (independently from the current number of mipmaps that have been set)
	uint32 computeNeededMipMapCount() const;

	/**
	 * Rotate a bitmap in CCW mode.
	 *
	 * \see releaseMipMaps().
	 */
	void rotateCCW();

	/**
	 * Build the mipmaps of the bitmap if they don't exist.
	 * Work only in RGBA mode...
	 * \see releaseMipMaps().
	 */
	void buildMipMaps();

	/**
	 * Release the mipmaps of the bitmap if they exist.
	 * Work for any mode.
	 * \see buildMipMaps().
	 */
	void releaseMipMaps();

	/**
	 * Reset the buffer. Mipmaps are deleted and bitmap is not valid anymore.
	 *
	 * \param type is the new type used for this texture
	 */
	void reset(TType type=RGBA);


	/**
	 * Resample the bitmap. If mipmaps exist they are deleted, then rebuilt
	 * after resampling.
	 * \param nNewWidth width after resample
	 * \param nNewHeight height after resample
	 */
	void resample (sint32 nNewWidth, sint32 nNewHeight);


	/**
	 * Resize the bitmap. If mipmaps exist they are deleted and not rebuilt.
	 * This is not a crop. Pixels are lost after resize.
	 *
	 * \param nNewWidth width after resize
	 * \param nNewHeight height after resize
	 * \param newType is the new type of the bitmap. If don_t_know, keep the same pixel format that before.
	 * \param resetTo0 by default the vector are filled by 0. set false to gain performances.
	 */
	void resize (sint32 nNewWidth, sint32 nNewHeight, TType newType=DonTKnow, bool resetTo0= true);


	/**  ADVANCED USE
	 * Resize a single mipmap level. resize() should have been called before.
	 * This is not a crop. Pixels are lost after resize.
	 *	No validity check is made. It is the user responsabitility fo setup correct mipmap size.
	 *
	 * \param numMipMap id of the mipmap
	 * \param nNewWidth width after resize
	 * \param nNewHeight height after resize
	 * \param resetTo0 by default the vector are filled by 0. set false to gain performances.
	 */
	void resizeMipMap (uint32 numMipMap, sint32 nNewWidth, sint32 nNewHeight, bool resetTo0= true);


	/**  ADVANCED USE
	 *	To use in conjunction with resizeMipMap. Setup the correct total number of mipmap
	 *	No validity check is made. It is the user responsabitility fo setup correct mipmap count.
	 */
	void setMipMapCount(uint32 mmc);


	/**
	 * Write a TGA (24 or 32 bits) from the object pixels buffer.
	 * If the current pixel format is not rgba then the method does nothing
	 * If the pixel format is Alpha then we save in 8 bpp
	 * \param f IStream (must be a reading stream)
	 * \param d depth : 8 or 16 or 24 or 32 (0 for automatic)
	 * \param upsideDown if true, the bitmap will be saved with the upside down
	 * \return true if succeed, false else
	 */
	bool writeTGA(NLMISC::IStream &f, uint32 d=0, bool upsideDown = false);

	/**
	 * Write a PNG (24 or 32 bits) from the object pixels buffer.
	 * If the current pixel format is not rgba then the method does nothing
	 * If the pixel format is Alpha then we save in 8 bpp
	 * \param f IStream (must be a reading stream)
	 * \param d depth : 8 or 16 or 24 or 32 (0 for automatic)
	 * \return true if succeed, false else
	 */
	bool writePNG(NLMISC::IStream &f, uint32 d=0);

	/**
	 * Write a JPG from the object pixels buffer.
	 * If the current pixel format is not rgba then the method does nothing
	 * If the pixel format is Alpha then we save in 8 bpp
	 * \param f IStream (must be a reading stream)
	 * \param quality 0=very bad quality 100=best quality
	 * \return true if succeed, false else
	 */
	bool writeJPG(NLMISC::IStream &f, uint8 quality = 80);

	/**
	 * Tell the bitmap to load grayscale bitmap as alpha or luminance format.
	 *
	 * \param loadAsAlpha is true to load grayscale bitmaps as alpha. false to load grayscale bitmaps as luminance.
	 * default value is true.
	 */
	void loadGrayscaleAsAlpha (bool loadAsAlpha)
	{
		_LoadGrayscaleAsAlpha=loadAsAlpha;
	}


	/**
	 * Tell if the bitmap loads grayscale bitmap as alpha or luminance format.
	 *
	 * \return true if the bitmap loads grayscale bitmaps as alpha, false if it loads grayscale bitmaps as luminance.
	 */
	bool isGrayscaleAsAlpha () const
	{
		return _LoadGrayscaleAsAlpha;
	}


	/**
	 * Perform a simple blit from the source to this bitmap at the (x, y) pos
	 * The dimension of the original bitmap are preserved
	 * For now, this texture and the source must have the same format
	 * With DXTC format, the dest coordinates must be a multiple of 4
	 * mipmap are not rebuild when present
	 * IMPORTANT : copy to self is not handled correctly
	 * \return true if the params were corrects and if the blit occurs. In debug build there's an assertion
	 */
	bool blit(const CBitmap *src, sint32 x, sint32 y) ;
	/**
	 * Extended version of blit. The destinaion of the blit is 'this' bitmap
	 * Source and dest rect are clamped as necessary.
	 * For now, only RGBA is supported (an asertion occurs otherwise)
	 * mipmap are not updated.
	 * IMPORTANT : copy to self is not handled correctly
	 */
	void blit(const CBitmap &src, sint srcX, sint srcY, sint srcWidth, sint srcHeight, sint destX, sint destY);


	/**
	 * Get the color in the bitmap for the first mipmap
	 * The mipmaps must be built. If not just return the bilinear at the given point.
	 * NB: coordinates are clamped.
	 * \param tiled If false coordinate are clamped, else the bitmap is considered to tile
	 */
	CRGBAF getColor (float x, float y) const;
	// Get Color with optional tiling on axis
	CRGBAF getColor (float x, float y, bool tileU, bool tileV) const;





	/** Get the pixel at the given coordinate.
	  * Works in RGBA and DXTC modes.
	  * Outside of the bitmap it returns Black (or if mipmap is not found)
	  */
	CRGBA  getPixelColor(sint x, sint y, uint32 numMipMap = 0) const;

	/** Set the pixel at the given coordinate.
	* Works in RGBA mode only.
	*/
	void  setPixelColor(sint x, sint y, CRGBA c, uint32 numMipMap = 0);

	/**
	 * Horizontal flip (all the columns are flipped)
	 * Works only with RGBA, and DXTC formats (only if w/h is a power of 2)
	 */
	void flipH();


	/**
	 * Vertical flip (all the rows are flipped)
	 * Works only with RGBA, and DXTC formats (only if w/h is a power of 2)
	 */
	void flipV();

	/**
	 * Rotation of the bitmap of 90 degree in clockwise
	 */
	void rot90CW();

	/**
	 * Rotation of the bitmap of 90 degree in counter clockwise
	 */
	void rot90CCW();

	/** Set this bitmap as the result of the blend bewteen 2 bitmap
	  * REQUIRE : - Bm0 and Bm1 should have the same size.
	  *           - Both bitmap should be convertible to RGBA pixel format.
	  * The result is a RGBA bitmap.
	  * NB: this just works with the first mipmaps
	  * \param factor The blend factor. 0 means the result is equal to Bm0, 256 means the result is equal to Bm1
	  * \param inputBitmapIsMutable when true, bitmap can be converted in place when needed (no copy done)
	  */
	void blend(CBitmap &Bm0, CBitmap &Bm1, uint16 factor, bool inputBitmapIsMutable = false);

	void getData(uint8*& extractData);

	void getDibData(uint8*& extractData);

	CBitmap& operator= (const CBitmap& from)
	{
		if (&from == this)
			return *this;
		for (uint i=0; i!=MAX_MIPMAP; ++i)
		{
			_Data[i] = from._Data[i]; // requires more than a surface copy
		}
		_MipMapCount = from._MipMapCount;
		_LoadGrayscaleAsAlpha = from._LoadGrayscaleAsAlpha;
		_Width = from._Width;
		_Height = from._Height;
		PixelFormat = from.PixelFormat;
		return *this;
	}
};

} // NLMISC


#endif // NL_BITMAP_H

/* End of bitmap.h */
