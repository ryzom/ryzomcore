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

#ifndef NL_SOURCE_DSOUND_H
#define NL_SOURCE_DSOUND_H

#include "nel/sound/driver/source.h"
#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/buffer.h"

namespace NLSOUND {

class CBufferDSound;

/**  Keep trace of following states of the DirectSound buffer:
 *   - the buffer is being filled with samples (filling),
 *   - the buffer still contains samples but silence is being written (silencing),
 *   - the buffer contains no samples but only silence (silenced)
 */
/*enum TSourceDSoundBufferState
{
	/// The buffer is being filled with samples (filling),
	NL_DSOUND_FILLING,
	/// The buffer still contains samples but silence is being written (silencing),
	NL_DSOUND_SILENCING,
	/// The buffer contains no samples but only silence (silenced)
	NL_DSOUND_SILENCED
} ;
/*

/** The state of the source as experienced by the user: playing, paused, and stopped. */
/*
enum TSourceDSoundUserState
{
	/// The buffer is playing.
	NL_DSOUND_PLAYING,
	/// The buffer is paused.
	NL_DSOUND_PAUSED,
	/// The buffer is stopped.
	NL_DSOUND_STOPPED
};
*/

/** To figger out whether the sound device has played all the samples in the buffer,
 *  the position of the play cursor is traced relatively to the position of the last
 *  sample in the buffer.
 */
/*
enum TSourceDSoundEndState
{
	NL_DSOUND_TAIL1,
	NL_DSOUND_TAIL2,
	NL_DSOUND_ENDED
};
*/

/**
 * DirectSound sound source
 *

 * For arguments as 3D vectors, use the NeL vector coordinate system
 *
 * \author Peter Hanappe
 * \author Nevrax France
 * \date 2002
 */
class CSourceDSound : public ISource
{
	friend class CSoundDriverDSound;
	
public:
	/// Constructor
	CSourceDSound(uint sourcename = 0);
    /// Destructor
	virtual ~CSourceDSound();
	
    /// Initialize the DirectSound buffers. Called by the sound driver only.
	void init(LPDIRECTSOUND directSound, bool useEax);
	
	/// \name Initialization
	//@{
	/// Enable or disable streaming mode. Source must be stopped to call this.
	virtual void setStreaming(bool streaming);
	/** Set the buffer that will be played (no streaming)
	 * If the buffer is stereo, the source mode becomes stereo and the source relative mode is on,
	 * otherwise the source is considered as a 3D source. Use submitStreamingBuffer for streaming.
	 */
	virtual void setStaticBuffer(IBuffer *buffer);
	/// Return the buffer, or NULL if streaming is used. Not available for streaming.
	virtual IBuffer *getStaticBuffer();
	/// Add a buffer to the streaming queue.  A buffer of 100ms length is optimal for streaming.
	/// Should be called by a thread which checks countStreamingBuffers every 100ms.
	virtual void submitStreamingBuffer(IBuffer *buffer);
	/// Return the amount of buffers in the queue (playing and waiting). 3 buffers is optimal.
	virtual uint countStreamingBuffers() const;
	//@}
	
	/// \name Playback control
	//@{
	/// Set looping on/off for future playbacks (default: off), not available for streaming
	virtual void setLooping(bool l);
	/// Return the looping state
	virtual bool getLooping() const;
	
	/** Play the static buffer (or stream in and play).
	 *	This method can return false if the sample for this sound is unloaded.
	 */
	virtual bool play();
	/// Stop playing
	virtual void stop();
	/// Pause. Call play() to resume.
	virtual void pause();
	/// Return true if play() or pause(), false if stop().
	virtual bool isPlaying() const;
	/// Return true if playing is finished or stop() has been called.
	virtual bool isStopped() const;
	/// Return true if the playing source is paused
	virtual bool isPaused() const;
	/// Returns the number of milliseconds the source has been playing
	virtual uint32 getTime();
	//@}
	
	/// \name Source properties
	//@{
	/** Set the position vector (default: (0,0,0)).
	 * 3D mode -> 3D position
	 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
	 */
	virtual void setPos(const NLMISC::CVector& pos, bool deffered = true);
	/** Get the position vector.
	 * See setPos() for details.
	 */
	virtual const NLMISC::CVector &getPos() const;
	/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
	virtual void setVelocity(const NLMISC::CVector& vel, bool deferred = true);
	/// Get the velocity vector
	virtual void getVelocity(NLMISC::CVector& vel) const;
	/// Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
	virtual void setDirection(const NLMISC::CVector& dir);
	/// Get the direction vector
	virtual void getDirection(NLMISC::CVector& dir) const;
	/** Set the gain (volume value inside [0 , 1]). (default: 1)
	 * 0.0 -> silence
	 * 0.5 -> -6dB
	 * 1.0 -> no attenuation
	 * values > 1 (amplification) not supported by most drivers
	 */
	virtual void setGain(float gain);
	/// Get the gain
	virtual float getGain() const;
	/** Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
	 * of one octave. 0 is not a legal value.
	 */
	virtual void setPitch(float pitch);
	/// Get the pitch
	virtual float getPitch() const;
	/// Set the source relative mode. If true, positions are interpreted relative to the listener position
	virtual void setSourceRelativeMode(bool mode);
	/// Get the source relative mode
	virtual bool getSourceRelativeMode() const;
	/// Set the min and max distances (default: 1, MAX_FLOAT) (3D mode only)
	virtual void setMinMaxDistances(float mindist, float maxdist, bool deferred = true);
	/// Get the min and max distances
	virtual void getMinMaxDistances(float& mindist, float& maxdist) const;
	/// Set the cone angles (in radian) and gain (in [0 , 1]) (default: 2PI, 2PI, 0)
	virtual void setCone(float innerAngle, float outerAngle, float outerGain);
	/// Get the cone angles (in radian)
	virtual void getCone(float& innerAngle, float& outerAngle, float& outerGain) const;
	/** Set the alpha value for the volume-distance curve
	 *
	 *	Useful only with OptionManualRolloff. value from -1 to 1 (default 0)
	 *
	 *  alpha.0: the volume will decrease linearly between 0dB and -100 dB
	 *  alpha = 1.0: the volume will decrease linearly between 1.0 and 0.0 (linear scale)
	 *  alpha = -1.0: the volume will decrease inversely with the distance (1/dist). This
	 *                is the default used by DirectSound/OpenAL
	 *
	 *  For any other value of alpha, an interpolation is be done between the two
	 *  adjacent curves. For example, if alpha equals 0.5, the volume will be halfway between
	 *  the linear dB curve and the linear amplitude curve.
	 */
	virtual void setAlpha(double a);
	//@}
	
	/// \name Direct output
	//@{
	/// Enable or disable direct output [true/false], default: true
	virtual void setDirect(bool enable = true);
	/// Return if the direct output is enabled
	virtual bool getDirect() const;
	/// Set the gain for the direct path
	virtual void setDirectGain(float gain);
	/// Get the gain for the direct path
	virtual float getDirectGain() const;
	
	/// Enable or disable the filter for the direct channel
	virtual void enableDirectFilter(bool enable = true);
	/// Check if the filter on the direct channel is enabled
	virtual bool isDirectFilterEnabled() const;
	/// Set the filter parameters for the direct channel
	virtual void setDirectFilter(TFilter filter, float lowFrequency, float highFrequency, float passGain);
	/// Get the filter parameters for the direct channel
	virtual void getDirectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const;
	/// Set the direct filter gain
	virtual void setDirectFilterPassGain(float passGain);
	/// Get the direct filter gain
	virtual float getDirectFilterPassGain() const;
	//@}
	
	/// \name Effect output
	//@{
	/// Set the effect send for this source, NULL to disable. [IEffect], default: NULL
	virtual void setEffect(IReverbEffect *reverbEffect);
	/// Get the effect send for this source
	virtual IEffect *getEffect() const;
	/// Set the gain for the effect path
	virtual void setEffectGain(float gain);
	/// Get the gain for the effect path
	virtual float getEffectGain() const;
	
	/// Enable or disable the filter for the effect channel
	virtual void enableEffectFilter(bool enable = true);
	/// Check if the filter on the effect channel is enabled
	virtual bool isEffectFilterEnabled() const;
	/// Set the filter parameters for the effect channel
	virtual void setEffectFilter(TFilter filter, float lowFrequency, float highFrequency, float passGain);
	/// Get the filter parameters for the effect channel
	virtual void getEffectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const;
	/// Set the effect filter gain
	virtual void setEffectFilterPassGain(float passGain);
	/// Get the effect filter gain
	virtual float getEffectFilterPassGain() const;
	//@}
	
	/// Return the OpenAL source name
	uint sourceName() { return _SourceName; }
	
    /// Returns the buffer associated with this source.
    IBuffer *getBuffer();
	
	/// Reset the source before reuse
	void reset();

	/// Update the source (e.g. continue to stream the data in)
	bool update();
	
	/// Update the source's volume according to its distance and fade out curve.
	/// It takes the current position of the listener as argument.
	void updateVolume(const NLMISC::CVector& listener);
	
private:
	
	void copySampleTo16BitsTrack(void *dst, void *src, uint nbSample, TSampleFormat sourceFormat);
	
	enum TSourceState
	{
		source_stopped,
		source_playing,
		source_silencing,
		source_swap_pending
	};
	
	
	/// Release all DirectSound resources
	void					release();
	
    // The minimum size of available space in the DS buffer for update
    static const uint32			_UpdateCopySize;
    // The size of the samples that are copied when buffers are swapped
    static const uint32			_SwapCopySize;
    // The number of channels
    static const uint			_DefaultChannels;
    // The default sample rate
    static const uint			_DefaultSampleRate;
    // The default sample size
    static const uint			_DefaultSampleSize;
    // The length of the crossfade, in samples
    static const uint32			_XFadeSize;
	
	/// The play and write cursors
	struct TCursors
	{
		uint32		PlayCursor;
		uint32		WriteCursor;
		uint32		WriteSize;
	};
	
	/// A locked buffer info.
	struct TLockedBufferInfo
	{
		// First locked part.
		sint16		*Ptr1;
		uint32		Size1;

		// second locked part (or 0 if none)
		sint16		*Ptr2;
		uint32		Size2;
	};
	
	// Utility function that locks the DirectSound buffer and restores it if it was lost.
//	bool					lock(uint32 writePos, uint32 size, uint8* &ptr1, DWORD &bytes1, uint8* &ptr2, DWORD &bytes2);
	bool					lock(uint32 writePos, uint32 size, TLockedBufferInfo &lockedInfo);
	
	// Utility function that unlocks the DirectSound buffer
//	bool					unlock(uint8* ptr1, DWORD bytes1, uint8* ptr2, DWORD bytes2);
	bool					unlock(const TLockedBufferInfo &lockedInfo);
	
	void					getCursors(TCursors &cursors);
	uint32					checkFillCursor();
	
	
	void					fillData(const TLockedBufferInfo &lbi, int nbSample);
	void					fillData(sint16 *dst, uint nbSample);
	void					fillSilence(const TLockedBufferInfo &lbi, int nbSample);
	
	void					xfade(const TLockedBufferInfo &lbi, sint16 *src);
	void					fadeOut(const TLockedBufferInfo &lbi);
	void					fadeIn(const TLockedBufferInfo &lbi);
	
	void					advanceFill(TLockedBufferInfo &lbi, uint nbSample);
	
	
	
	// Replace the current buffer with the swap buffer
	void					swap();
	// getFadeOutSize() calculates how many samples have been written after the
	// write cursor in the DirectSound buffer. This value is returned in the
	// writtenTooMuch variable. The xfadeSize contains the number of samples over
	// which to do a xfade or fade out, and in1 points to the sample in the sample
	// buffer where to start the fade.
//	void					getFadeOutSize(uint32 writePos, uint32& xfadeSize, sint16* &in1, uint32 &writtenTooMuch);
	
	// Fill the buffer with fresh samples. Should be called inside the critical zone.
	bool					fill();
	// Fill the buffer with sparkling silence. Should be called inside the critical zone.
	bool					silence();
	// Do a cross fade between the current buffer and the buffer stored in the _SwapBuffer
	// variable. Call inside the critical zone.
	void					crossFade();
	// Fade out the current buffer. Call inside the critical zone.
	void					fadeOut();
	// Fade in the current buffer. Call inside the critical zone.
	void					fadeIn();
	/// Check whether the play position has advanced enough to require an update
	bool					needsUpdate();
	
	
	// Source name
	uint					_SourceName;
	
	
	TSourceState			_State;
//	uint32					_FillCursor;
	
    // The size of the sound buffer, in bytes
//    uint32					_BufferSize;
	
	IBuffer					*_Sample;
	// Size of the buffer in sample
	uint					_SampleSize;
	// Position in the buffer in sample.
	uint					_SampleOffset;
	// The number of sample realy played (depend on play cursor).
	uint32					_PlayOffset;
	TSampleFormat			_Format;
	uint					_SampleFreq;
	
	
	// The frequency of the source [0,10], i.e. slowed down or accelerated
	float					_Freq;
	// The sample rate of the source (= _Freq * _Buffer sample rate)
	uint32					_SampleRate;
	
	
	IBuffer					*_NextSample;
	
	uint32					_LastPlayPos;
	uint32					_FillOffset;
	uint32					_SilenceWriten;
	
    // The next sound buffer
//    IBuffer					*_SwapBuffer;
	
    // To loop or not to loop
    bool					_Loop;
	
	// The state of the source (playing, paused, stopped)
//	TSourceDSoundUserState	_UserState;
	
    // DirectSound secondary buffer
    LPDIRECTSOUNDBUFFER		_SecondaryBuffer;
    // The byte size of the DirectSound secondary buffers
    static const uint32			_SecondaryBufferSize;
	// The mask for the buffer size
    static const uint32			_SizeMask;
    // 3D interface of the secondary buffer.
    LPDIRECTSOUND3DBUFFER	_3DBuffer;
    // The critial section object to protect the swap and update functions
    CRITICAL_SECTION		_CriticalSection;
	
	// The state for ADPCM decompression.
	IBuffer::TADPCMState	_ADPCMState;
	
	
	// The state of the DirectSound buffer (filling, silencing, silenced)
//	TSourceDSoundBufferState _SecondaryBufferState;
	
	
	
    // The next position in the DirectSound buffer where we should write next
//    uint32					_NextWritePos;
	
    // The total number of bytes written from the current sound buffer
//    uint32					_BytesWritten;
	
    // The amount of silence written (in bytes) after the end of the current sound buffer
//    uint32					_SilenceWritten;
	
    // The position of the last audio sample written.
//    uint32					_EndPosition;
	
	
	// The state of the buffer to reach the end of the audio samples. To flag the buffer as
	// STOPPED, we have to make sure all the samples in the buffer are played. The play
	// cursor in the DirectSound buffer is inspected and when it crosses the position of
	// the last sample written (_EndPosition) the buffer is flagged as stopped. The _EndState
	// field is used to keep a trace of the whereabouts of the play cursor.
//	TSourceDSoundEndState	_EndState;
	
	
	// Has this source been handed out.
/*	bool					_IsUsed;
	
	// Set the 'used' state of the source. Managed by the driver.
	void					setUsed(bool v) { _IsUsed = v; }
	
	// Return the 'used' state of the source
	bool					isUsed() { return _IsUsed; }
*/
	sint32					_Volume;
	float					_Gain;
	double					_Alpha;
	NLMISC::CVector			_Pos;
	bool					_PosRelative;

#if EAX_AVAILABLE == 1
	LPKSPROPERTYSET			_EAXSource;
#endif


#if NLSOUND_PROFILE

public:

	static double _LastSwapTime;
	static double _TotalSwapTime;
	static double _MaxSwapTime;
	static double _MinSwapTime;
	static uint32 _SwapCount;

	static double _TotalUpdateTime;
	static double _MaxUpdateTime;
	static double _MinUpdateTime;
	static uint32 _UpdateCount;
	static uint32 _TotalUpdateSize;

	static double _PosTime;
	static double _LockTime;
	static double _CopyTime;
	static double _UnlockTime;
	static uint32 _CopyCount;

public:

	static double getTestLast()    { return 1000.0f * _LastSwapTime; };
	static double getTestMax()     { return 1000.0f * _MaxSwapTime; };
	static double getTestMin()     { return 1000.0f * _MinSwapTime; };
	static double getTestAverage() { return (_SwapCount > 0) ? 1000.0f * _TotalSwapTime / _SwapCount : 0.0; };

	static double getAveragePosTime()    { return (_CopyCount > 0) ? 1000.0f * _PosTime / _CopyCount : 0.0; };
	static double getAverageLockTime()   { return (_CopyCount > 0) ? 1000.0f * _LockTime / _CopyCount : 0.0; };
	static double getAverageCopyTime()   { return (_CopyCount > 0) ? 1000.0f * _CopyTime / _CopyCount : 0.0; };
	static double getAverageUnlockTime() { return (_CopyCount > 0) ? 1000.0f * _UnlockTime / _CopyCount : 0.0; };
	static double getAverageCumulTime()  { return (_CopyCount > 0) ? 1000.0f * (_PosTime + _LockTime + _CopyTime +  _UnlockTime) / _CopyCount : 0.0; };
	static uint   getAverageUpdateSize() { return (_CopyCount > 0) ? (uint) (_TotalUpdateSize / _CopyCount) : 0; };

	static double getMaxUpdateTime() { return 1000.0f * _MaxUpdateTime; };
	static double getMinUpdateTime() { return 1000.0f * _MinUpdateTime; };
	static double getAverageUpdateTime() { return (_UpdateCount > 0) ? 1000.0f * _TotalUpdateTime / _UpdateCount : 0.0; };

	static double getTotalUpdateTime() { return 1000.0f * _TotalUpdateTime; };
	static double getUpdateBytesPerMsec() { return (_UpdateCount > 0) ? _TotalUpdateSize / _TotalUpdateTime / 1000.0 : 0.0; }

#endif
	
};


} // NLSOUND


#endif // NL_SOURCE_DSOUND_H

/* End of source_al.h */
