// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
#include "movie_shooter.h"
#include "time_client.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/path.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/common.h"
#include "nel/misc/file.h"
#include "nel/misc/debug.h"
#include "nel/misc/system_info.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace std;


// ***************************************************************************
CMovieShooter		MovieShooter;


// ***************************************************************************
CMovieShooter::CMovieShooter()
{
	_MemoryBlock= NULL;
	_MemorySize= 0;
	_CurrentIndex= 0;
	_NumFrames= 0;
	_FirstFrame= NULL;
	_LastFrame= NULL;
	_FrameSkip = _CurrentFrameSkip = 0;
}

// ***************************************************************************
CMovieShooter::~CMovieShooter()
{
	clearMemory();
}

// ***************************************************************************
bool				CMovieShooter::init(uint maxMemory)
{
	clearMemory();

	if(maxMemory==0)
		return false;

	_MemoryBlock= new uint8[maxMemory];
	if(!_MemoryBlock)
	{
		nlwarning("Failed to Allocate %d bytes for MovieShooter", maxMemory);
		return false;
	}
	// Must fill memory with 0, just to ensure the system allocate the virtual pages.
	memset(_MemoryBlock, 0, maxMemory);
	_MemorySize= maxMemory;
	_CurrentIndex= 0;
	_NumFrames= 0;
	_FirstFrame= NULL;
	_LastFrame= NULL;

	return true;
}

// ***************************************************************************
void				CMovieShooter::clearMemory()
{
	if(_MemoryBlock)
		delete [] (_MemoryBlock);
	_MemoryBlock= NULL;
	_MemorySize= 0;
	_CurrentIndex= 0;
	_NumFrames= 0;
	_FirstFrame= NULL;
	_LastFrame= NULL;
}

// ***************************************************************************
bool				CMovieShooter::addFrame(double time, UDriver	*pDriver)
{
	if(!enabled())
		return false;

	// Get the buffer from driver. static to avoid reallocation
	static	CBitmap		bitmap;
	pDriver->getBuffer(bitmap);
	nlassert(bitmap.getPixelFormat()==CBitmap::RGBA);

	// add the frame.
	if(bitmap.getPixels().size()==0)
		return false;
	return addFrame(time, (CRGBA*)(&bitmap.getPixels()[0]), bitmap.getWidth(), bitmap.getHeight());
}

// ***************************************************************************
bool				CMovieShooter::addFrame(double time, CRGBA	*pImage, uint w, uint h)
{
	if(!enabled() || w==0 || h==0)
		return false;

	++_CurrentFrameSkip;
	if (_CurrentFrameSkip <= _FrameSkip)
		return true;
	_CurrentFrameSkip = 0;

	// _MemorySize must contain at least ONE frame.
	uint	dataSize= w*h*sizeof(uint16);
	uint	totalSize= dataSize+sizeof(CFrameHeader);
	nlassert(totalSize<=_MemorySize);

	// If too big to fit in memory malloc, loop
	if( totalSize + _CurrentIndex > _MemorySize)
	{
		_CurrentIndex= 0;
	}

	// prepare a new frame.
	CFrameHeader	*newFrame= (CFrameHeader*)(_MemoryBlock+_CurrentIndex);

	// while this frame erase first one.
	while( _FirstFrame!=NULL && (uint8*)newFrame<=(uint8*)_FirstFrame && ((uint8*)newFrame+totalSize)>(uint8*)_FirstFrame )
	{
		// skip to the next frame.
		_FirstFrame= _FirstFrame->Next;
		_NumFrames--;
		// if empty, clean all.
		if(_FirstFrame==NULL)
		{
			nlassert(_NumFrames==0);
			_LastFrame=NULL;
			_CurrentIndex= 0;
			newFrame= (CFrameHeader*)_MemoryBlock;
		}
	}

	// Build the frame.
	newFrame->Time= time;
	newFrame->Width= w;
	newFrame->Height= h;
	newFrame->Data= _MemoryBlock+_CurrentIndex+sizeof(CFrameHeader);
	newFrame->Next= NULL;

	// Compress and Fill Data. As fast as possible
	uint16	*dst= (uint16*)newFrame->Data;
	CRGBA	*src= pImage;
	for(uint y=h;y>0;y--)
	{
		// Precache all the line. NB: correct for 800, since 800*4==3200, ie under the 4K cache size.
		CFastMem::precache(src, w*sizeof(CRGBA));
		// For all pixels; compress, and store.
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
		__asm
		{
			mov	ecx, w
			mov	esi, src
			mov	edi, dst
		myLoop2:
			mov eax, [esi]

			mov	ebx, eax
			mov	edx, eax

			and	eax, 0x000000F8
			and ebx, 0x0000FC00
			shl eax, 8
			shr ebx, 5

			or	eax, ebx
			and edx, 0x00F80000

			shr edx, 19
			add esi, 4

			or	eax, edx

			mov [edi],eax

			add edi, 2

			dec ecx
			jnz myLoop2
		}
		src+= w;
		dst+= w;
#else
		for(uint x=w;x>0;x--, src++, dst++)
		{
			*dst= src->get565();
		}
#endif
	}

	// inc index.
	_CurrentIndex+= totalSize;

	// Link to the list.
	if(_FirstFrame==NULL)
	{
		_FirstFrame= _LastFrame= newFrame;
	}
	else
	{
		_LastFrame->Next= newFrame;
		_LastFrame= newFrame;
	}

	// Ok!
	_NumFrames++;
	return true;
}

// ***************************************************************************
uint				CMovieShooter::getNumFrames()
{
	return _NumFrames;
}

// ***************************************************************************
void				CMovieShooter::saveMovie(UDriver *drv, UTextContext *textContext, const char *path, float framePeriod, bool allowLinearInterp, const char *savePrefix /*= "shot_"*/)
{
	if(!CFile::isDirectory(path))
		throw Exception("SaveMovie: %s is not a directory", path);

	if(getNumFrames()<2)
		throw Exception("SaveMovie: no Frames to save");

	if(framePeriod<=0)
		throw Exception("SaveMovie: bad Frame Period");


	// Initialize Pen
	textContext->setFontSize(10);
	textContext->setColor(CRGBA(255,255,255));
	textContext->setHotSpot(UTextContext::TopLeft);


	// prepape save
	string	fileName= string(path) + "/" + string(savePrefix);

	// first frame
	CFrameHeader	*precFrame= _FirstFrame;
	double	time= precFrame->Time;

	// Save all frames
	uint	fileIndex= 0;
	sint	n= getNumFrames();
	// Write interpolation of frames => must have 2 frames.
	while(n>=2)
	{
		// Grab Inputs.
		drv->EventServer.pump(true);
		// Stop to save ??
		if(drv->AsyncListener.isKeyDown(KeyESCAPE))
		{
			break;
		}

		// nextFrame is...
		CFrameHeader	*nextFrame= precFrame->Next;

		// Skip any frame. Get first end frame with Time>time
		while(n>=2 && nextFrame->Time<=time)
		{
			// skip the Frame
			precFrame= nextFrame;
			nextFrame= nextFrame->Next;
			n--;
		}

		// If a frame exist, get it.
		if(n>=2)
		{
			// get the interpolation factor
			double	interpValue= (time-precFrame->Time)/(nextFrame->Time-precFrame->Time);

			// interpolation is possible only if 2 frames are of same size, and if wanted
			bool	interpOk;
			uint	w, h;
			w= precFrame->Width;
			h= precFrame->Height;
			interpOk= allowLinearInterp && (w==nextFrame->Width && h==nextFrame->Height);

			// get the first frame
			static	CBitmap		bmp0;
			getFrameData(precFrame, bmp0);

			// if interp ok
			if(interpOk)
			{
				static	CBitmap		bmp1;
				getFrameData(nextFrame, bmp1);


				uint	coef= (uint)floor(256*interpValue+0.5f);
				coef= min(256U, coef);

				// blend
				CRGBA	*dst= (CRGBA*)&bmp0.getPixels()[0];
				CRGBA	*src= (CRGBA*)&bmp1.getPixels()[0];
				for(uint nPix= w*h;nPix>0;nPix--, src++, dst++)
				{
					dst->blendFromuiRGBOnly(*dst, *src, coef);
				}
			}

			// write the bitmap
			char	fname[500];
			smprintf(fname, 500, "%s%05d.tga", fileName.c_str(), fileIndex);
			COFile	file(fname);
			bmp0.writeTGA(file,24,false);

			// Copy frame to buffer, and swap
			drv->fillBuffer(bmp0);
			textContext->printfAt(0.05f,0.80f, "Movie Saving: %d%%", 100-n*100/getNumFrames());
			drv->swapBuffers();
		}

		// next frame to write.
		fileIndex++;
		time+= framePeriod;
	}
}

// ***************************************************************************
void				CMovieShooter::replayMovie(UDriver *drv, UTextContext *textContext)
{
	nlassert(drv);

	if(getNumFrames()<2)
		throw Exception("ReplayMovie: no Frames to save");

	// Initialize Pen
	textContext->setFontSize(10);
	textContext->setColor(CRGBA(255,255,255));
	textContext->setHotSpot(UTextContext::TopLeft);

	// first frame
	CFrameHeader	*frame= _FirstFrame;
	uint			n= getNumFrames();

	// replay all frames
	double	tPrec= ryzomGetLocalTime ()*0.001;
	double	lastFrameTime= frame->Time;
	while(frame)
	{
		// Grab Inputs.
		drv->EventServer.pump(true);
		// Stop to save ??
		if(drv->AsyncListener.isKeyDown(KeyESCAPE))
		{
			break;
		}

		// get the frame
		static	CBitmap		bmp0;
		getFrameData(frame, bmp0);

		// Copy frame to buffer
		drv->fillBuffer(bmp0);
		textContext->printfAt(0.05f,0.80f, "Movie Replay: %d%%", 100-n*100/getNumFrames());

		// Wait frame
		double	tNext;
		do
		{
			tNext= ryzomGetLocalTime ()*0.001;
		}
		while( tNext-tPrec < frame->Time-lastFrameTime );

		lastFrameTime= frame->Time;
		tPrec= tNext;

		// swap.
		drv->swapBuffers();

		// nextFrame
		frame= frame->Next;
		n--;
	}
}


// ***************************************************************************
void				CMovieShooter::getFrameData(CFrameHeader *frame, NLMISC::CBitmap &bmp)
{
	uint	w= frame->Width;
	uint	h= frame->Height;
	// resize
	if(bmp.getWidth()!=w || bmp.getHeight()!=h)
		bmp.resize(w, h);
	// unpack.
	uint16	*src= (uint16*)frame->Data;
	CRGBA	*dst= (CRGBA*)&bmp.getPixels()[0];
	for(uint npix= w*h; npix>0; npix--, src++, dst++)
	{
		dst->set565(*src);
	}
}

// ***************************************************************************
void				CMovieShooter::resetMovie()
{
	_CurrentIndex= 0;
	_NumFrames= 0;
	_FirstFrame= NULL;
	_LastFrame= NULL;
}

// ***************************************************************************
void				CMovieShooter::setFrameSkip (uint32 nNbFrameToSkip)
{
	_FrameSkip = nNbFrameToSkip;
	_CurrentFrameSkip = 0;
}
