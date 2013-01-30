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

#ifndef NL_SOURCE_FMOD_H
#define NL_SOURCE_FMOD_H

#include "nel/sound/driver/source.h"
#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/buffer.h"

namespace NLSOUND {
	class CBufferFMod;

// TODO: Doc comments in FMod driver are wrong.
// Bad copy pastes from DSound & OpenAL drivers.
// Fix drivername, author and date (2004?)!

/**
 * DirectSound sound source
 *

 * For arguments as 3D vectors, use the NeL vector coordinate system
 *
 * \author Peter Hanappe
 * \author Nevrax France
 * \date 2002
 */
class CSourceFMod : public ISource
{
	friend class CSoundDriverFMod;
	
public:
	/// Constructor
	CSourceFMod( uint sourcename = 0 );
	/// Destructor
	virtual ~CSourceFMod();
	
    /// Initialize the DirectSound buffers. Called by the sound driver only.
	void init();
	
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
	
	/// Reset the source before reuse
	void reset();
	
	/// Update the source (e.g. continue to stream the data in)
	bool update();
	
	/** Update the source's volume according to its distance and fade out curve.
	 *  It takes the current position of the listener as argument.
	 *	Called only with OptionManualRolloff
	 */
	void updateVolume(const NLMISC::CVector& listener);
	
	bool needsUpdate();
	
	void updateFModPosIfRelative();
	
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
	void release();
	
	// Source name
	uint					_SourceName;
	
	TSourceState			_State;
	
	IBuffer					*_Sample;
	IBuffer					*_NextSample;
	sint					_FModChannel;
	
    // States
	bool					_PosRelative;
    bool					_Loop;
	float					_Gain;
	float					_Pitch;
	double					_Alpha;
	NLMISC::CVector			_Pos;
	NLMISC::CVector			_Vel;
	NLMISC::CVector			_Front;
	float					_MinDist, _MaxDist;
	
	void updateFModPos();
	
};


} // NLSOUND


#endif // NL_SOURCE_FMOD_H

/* End of source_al.h */
