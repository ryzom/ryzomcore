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



#ifndef CL_MOVIE_SHOOTER_H
#define CL_MOVIE_SHOOTER_H


/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/bitmap.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"


///////////
// USING //
///////////
using	NLMISC::CRGBA;
using	NL3D::UDriver;
using	NL3D::UTextContext;

// ***************************************************************************
/**	Shoot frames into RAM, then write them to disc.
 *
 */
class	CMovieShooter
{
public:
	CMovieShooter();
	~CMovieShooter();

	/// Fail if can't allocate Movie Memory, or if maxMemory==0
	bool				init(uint maxMemory);

	/// Clear the container.
	void				clearMemory();

	bool				enabled() const {return _MemorySize>0;}

	/// Add a frame from driver shoot.
	bool				addFrame(double time, UDriver *pDriver);

	/// Add a frame. return false if error. NB: if the movie has looped, erase some images from begin, but still return true
	bool				addFrame(double time, CRGBA	*pImage, uint w, uint h);

	/// return the number of frames currenty added.
	uint				getNumFrames();

	/** replay the movie in a driver, until end of movie
	 *	NB: stop if ESC is pressed.
	 */
	void				replayMovie(UDriver *drv, UTextContext *textContext);

	/** Save the movie to a directory: shoot_00000.tga, .... Throw Exception if error.
	 *	Frames are saved at a constant rate. if linearInterp is true, frames are interpolated to give
	 *	the best view result.
	 *	NB: stop if ESC is pressed
	 */
	void				saveMovie(UDriver *drv, UTextContext *textContext, const char *path, float framePeriod, bool allowLinearInterp= true, const char *prefix = "shot_");

	/// Reset all frames. ready to addFrame() from 0.
	void				resetMovie();

	/// Set the frame skipping (in nb frame) set to 0 to not skip frames
	void				setFrameSkip (uint32 nNbFrameToSkip);


// **************
private:

	struct	CFrameHeader
	{
		double			Time;
		uint32			Width, Height;
		uint8			*Data;
		// NULL if end of list.
		CFrameHeader	*Next;
	};


	uint8				*_MemoryBlock;
	uint32				_MemorySize;
	uint32				_CurrentIndex;
	uint				_NumFrames;
	// The first frame of the movie. != from _MemoryBlock if looped
	CFrameHeader		*_FirstFrame;
	CFrameHeader		*_LastFrame;

	uint32				_FrameSkip;
	uint32				_CurrentFrameSkip;

	// Resize and Fill a bitmap with a frame.
	void				getFrameData(CFrameHeader *frame, NLMISC::CBitmap &bmp);
};


// ***************************************************************************
// The default movie_shooter
extern	CMovieShooter		MovieShooter;


#endif // CL_DEMO_H

/* End of demo.h */
