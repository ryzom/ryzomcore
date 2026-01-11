/**
* UStreamSource
* \file u_stream_source.h
* \brief UStreamSource
* \date 2010-01-28 12:58GMT
* \author Jan Boon (Kaetemi)
*/

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2010  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NLSOUND_U_STREAM_SOURCE_H
#define NLSOUND_U_STREAM_SOURCE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include <nel/sound/u_source.h>

namespace NLSOUND {

/**
 * UStreamSource
 * \brief UStreamSource
 * \date 2010-01-28 12:58GMT
 * \author Jan Boon (Kaetemi)
 */
class UStreamSource : public USource
{
public:
	virtual ~UStreamSource() { }

	/// Cast this to a USource
	inline USource					*asUSource() { return static_cast<USource *>(this); }

	/// \name Streaming source controls
	//@{
	/// Set the sample format. (channels = 1, 2, ...; bitsPerSample = 8, 16; frequency = samples per second, 44100, ...)
	virtual void					setFormat(uint8 channels, uint8 bitsPerSample, uint32 frequency) = 0;	
	/// Return the sample format information.
	virtual void					getFormat(uint8 &channels, uint8 &bitsPerSample, uint32 &frequency) const = 0;	
	/// Get a writable pointer to the buffer of specified size. Use capacity to specify the required bytes. Returns NULL when all the buffer space is already filled. Call setFormat() first.
	virtual uint8					*lock(uint capacity) = 0;	
	/// Notify that you are done writing to the locked buffer, so it can be copied over to hardware if needed. Set size to the number of bytes actually written to the buffer. Returns true if ok.
	virtual bool					unlock(uint size) = 0;
	/// Get the recommended buffer size to use with lock()/unlock()
	virtual void					getRecommendedBufferSize(uint &samples, uint &bytes) const = 0;
	/// Get the recommended sleep time based on the size of the last submitted buffer and the available buffer space
	virtual uint32					getRecommendedSleepTime() const = 0;
	/// Return if there are still buffers available for playback.
	virtual bool					hasFilledBuffersAvailable() const = 0;
	//@}

protected:
	UStreamSource() { }

private:
	UStreamSource(const UStreamSource &);
	UStreamSource &operator=(const UStreamSource &);
	
}; /* class UStreamSource */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_U_STREAM_SOURCE_H */

/* end of file */
