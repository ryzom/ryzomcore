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

#include "hls_bank_texture_info.h"
#include "../s3tc_compressor_lib/s3tc_compressor.h"
#include "nel/misc/mem_stream.h"


using namespace NLMISC;


// ***************************************************************************
CHLSBankTextureInfo::CHLSBankTextureInfo()
{
	DividedBy2= false;
}


// ***************************************************************************
void			CHLSBankTextureInfo::serial(NLMISC::IStream &f)
{
	sint ver= f.serialVersion(0);
	f.serial(DividedBy2);
	f.serial(SrcBitmap);
	f.serialCont(Masks);
	f.serialCont(Instances);
}


// ***************************************************************************
void		CHLSBankTextureInfo::CMaskBitmap::build(const NLMISC::CBitmap &src)
{
	nlassert(src.getPixelFormat()==CBitmap::Luminance);
	Width= src.getWidth();
	Height= src.getHeight();
	Pixels.resize(Width*Height);
	if(!Pixels.empty())
	{
		uint8	*pSrc = (uint8*)src.getPixels().getPtr();
		uint8	*pDst = &Pixels[0];
		memcpy(pDst, pSrc, Pixels.size());
	}
}


// ***************************************************************************
void		CHLSBankTextureInfo::CMaskBitmap::buildBitmap(NLMISC::CBitmap &dst)
{
	dst.resize(Width, Height, CBitmap::RGBA);
	if(!Pixels.empty())
	{
		uint8	*pSrc= &Pixels[0];
		CRGBA	*pDst= (CRGBA*)(&dst.getPixels()[0]);
		for(uint i=0; i<Pixels.size(); i++)
		{
			pDst[i].R= pSrc[i];
			pDst[i].G= pSrc[i];
			pDst[i].B= pSrc[i];
			pDst[i].A= pSrc[i];
		}
	}
}


// ***************************************************************************
void		CHLSBankTextureInfo::CDXTCBitmap::build(const NLMISC::CBitmap &src)
{
	nlassert(src.getPixelFormat()==CBitmap::RGBA);
	CMemStream		memStream(false);
	// Compress the bitmap in a stream
	CS3TCCompressor		comp;
	comp.compress(src, true, DXT5, memStream);
	// sotre the stream.
	_Data.resize(memStream.length());
	memcpy(&_Data[0], memStream.buffer(), _Data.size());
}


// ***************************************************************************
void		CHLSBankTextureInfo::CDXTCBitmap::buildBitmap(NLMISC::CBitmap &dst)
{
	CMemStream		memStream(true);
	memStream.fill(&_Data[0], (uint32)_Data.size());
	// load the DXTC5 from memStream
	dst.reset();
	dst.load(memStream);
}


// ***************************************************************************
void		CHLSBankTextureInfo::CDXTCBitmap::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(0);

	f.serialCont(_Data);
}
