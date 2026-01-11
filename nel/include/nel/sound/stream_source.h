// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2010-2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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

#ifndef NLSOUND_STREAM_SOURCE_H
#define NLSOUND_STREAM_SOURCE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/mutex.h>

// Project includes
#include <nel/sound/u_stream_source.h>
#include "nel/sound/source_common.h"
#include "nel/sound/mixing_track.h"
#include "nel/sound/stream_sound.h"

namespace NLSOUND {
	class IBuffer;

/**
 * CStreamSource
 * \brief CStreamSource
 * \date 2010-01-28 07:09GMT
 * \author Jan Boon (Kaetemi)
 */
class CStreamSource : public CSourceCommon
{
public:
	CStreamSource(CStreamSound *streamSound = NULL, bool spawn = false, TSpawnEndCallback cb = 0, void *cbUserParam = 0, NL3D::CCluster *cluster = 0, CGroupController *groupController = NULL);
	virtual ~CStreamSource();
	
	/// Return the sound binded to the source (or NULL if there is no sound)
	virtual TSoundId				getSound()									{ return m_StreamSound; }
	/// Return the sound binded to the source (or NULL if there is no sound)
	virtual CStreamSound			*getStreamSound()							{ return m_StreamSound; }
	
	/// \name Playback control
	//@{
	/// Set looping on/off for future playbacks (default: off)
	virtual void					setLooping(bool l);
	/// Play
	virtual void					play();
protected:
	void							stopInt();
public:
	/// Stop playing
	virtual void					stop();
	/// Get playing state. Return false even if the source has stopped on its own.
	virtual bool					isPlaying();
	/// Returns the number of milliseconds the source has been playing
	virtual uint32					getTime();
	//@}
		
	/// \name Source properties
	//@{
	/** Set the position vector (default: (0,0,0)).
	 * 3D mode -> 3D position
	 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
	 */
	virtual void					setPos(const NLMISC::CVector& pos);
	/// Get the position vector (3D mode only)
	
	/** Get the virtual source position.
	 *	This method compute the virtual source position with cluster system.
	 */
	NLMISC::CVector					getVirtualPos() const;

	virtual void					setVelocity(const NLMISC::CVector& vel);
	/// Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
	virtual void					setDirection(const NLMISC::CVector& dir);
	virtual void					updateFinalGain();
	/** Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
	 * of one octave. 0 is not a legal value.
	 */
	virtual void					setPitch(float pitch);
	/// Set the source relative mode. If true, positions are interpreted relative to the listener position (default: false)
	virtual void					setSourceRelativeMode(bool mode);
	//@}
	
	/// \name Streaming source controls
	//@{
	/// Set the sample format. (channels = 1, 2, ...; bitsPerSample = 8, 16; frequency = samples per second, 44100, ...)
	virtual void					setFormat(uint8 channels, uint8 bitsPerSample, uint32 frequency);	
	/// Return the sample format information.
	virtual void					getFormat(uint8 &channels, uint8 &bitsPerSample, uint32 &frequency) const;	
	/// Get a writable pointer to the buffer of specified size. Use capacity to specify the required bytes. Returns NULL when all the buffer space is already filled. Call setFormat() first.
	virtual uint8					*lock(uint capacity);	
	/// Notify that you are done writing to the locked buffer, so it can be copied over to hardware if needed. Set size to the number of bytes actually written to the buffer. Returns true if ok.
	virtual bool					unlock(uint size);
	/// Get the recommended buffer size to use with lock()/unlock()
	virtual void					getRecommendedBufferSize(uint &samples, uint &bytes) const;
	/// Get the recommended sleep time based on the size of the last submitted buffer and the available buffer space
	virtual uint32					getRecommendedSleepTime() const;
	/// Return if there are still buffers available for playback.
	virtual bool					hasFilledBuffersAvailable() const;
	//@}
	
	/// Prepare the buffers in this stream for the given maximum capacity. (TODO: Move this into UStreamSource)
	void preAllocate(uint capacity);
	
	/// Return the track
	CTrack							*getTrack()									{ return m_Track; }

private:
	CStreamSource(const CStreamSource &);
	CStreamSource &operator=(const CStreamSource &);
	
protected:
	/// Return the source type
	TSOURCE_TYPE					getType() const								{ return SOURCE_STREAM; }

	/// Returns if this logical source has a physical source attached to it.
	inline bool						hasPhysicalSource() const					{ return m_Track != NULL; }

	/// Get the physical source of the track that is in use by this source.
	inline ISource					*getPhysicalSource()						{ return m_Track->getPhysicalSource(); }
	
	/// Try to capture a physical source for this logical source. Use hasPhysicalSource to verify. Called when source needs to start playing.
	void							initPhysicalSource();
	
	/// Free the physical source for re-use by another logical source. Called when source stops playing.
	void							releasePhysicalSource();

	/// Update available buffers
	void							updateAvailableBuffers();
	
	/// The simple sound of this source.
	CStreamSound					*m_StreamSound;

	/// The volume falloff factor.
	float							m_Alpha;

	/// Corresponding track (if selected for playing)
	CTrack							*m_Track;

	/// True when the sound is played muted and until the mixer event notifying the end.
	// bool							m_PlayMuted;
	
	/// Number of free buffers
	uint							m_FreeBuffers;
	
	/// Next buffer to use
	uint							m_NextBuffer;
	
	/// The last buffer size submitted.
	uint							m_LastSize;
	
	/// Two buffers which are swapped.
	IBuffer							*m_Buffers[3]; // an array of two pointers

	/// Mutex for buffer ops.
	NLMISC::CMutex					m_BufferMutex;

	/// The bytes per second according to the buffer format
	uint							m_BytesPerSecond;

	/// Waiting for play for high priority sources
	bool							m_WaitingForPlay;

	/// Inverse pitch
	float							m_PitchInv;

}; /* class CStreamSource */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_STREAM_SOURCE_H */

/* end of file */
