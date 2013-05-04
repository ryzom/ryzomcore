/**
 * Contains the ISource interface, as well as several '#define's that
 * specify the limits of parameters that can be passed to the source.
 * \file source.h
 * \brief Interface for physical sound sources
 * \author Olivier Cado
 * \author Nevrax France
 * \author Jan Boon (Kaetemi)
 * \date 2001-2009
 */

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

#ifndef NL_SOURCE_H
#define NL_SOURCE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

#include "sound_driver.h"

/** 
 * The minimum allowed gain is specified as 0.0f.
 * Inverting the wave by specifying a negative gain is not allowed.
 * \brief Minimum allowed gain (volume).
 */
#define NLSOUND_MIN_GAIN 0.0f
/**
 * The maximum allowed gain is specified as 1.0f.
 * OpenAL implementations may or may not clamp the gain to a maximum 
 * of 1.0f, so this maximum is forced on all implementations.
 * If you pass a value outside the minimum and maximum bounds,
 * it will automatically be clamped between them.
 * \brief Maximum allowed gain (volume).
 */
#define NLSOUND_MAX_GAIN 1.0f

/**
 * The minimum allowed pitch is specified as 0.0f.
 * Going backwards in time by specifying a negative pitch is illegal.
 * Setting the pitch to 0.0f is the same as pausing the source.
 * \brief Minimum allowed pitch.
 */
#define NLSOUND_MIN_PITCH 0.0f
/**
 * The maximum allowed pitch is specified as 8.0f.
 * Tests indicate that with OpenAL the pitch can be set to 8.0f, 
 * and with FMod to somewhere around 9.0f. The XAudio2 implementation
 * uses this value directly to configure the maximum pitch, which can
 * technically be as high as XAUDIO2_MAX_FREQ_RATIO which is 1024.0f.
 * If you pass a value outside the minimum and maximum bounds,
 * it will automatically be clamped between them.
 * \brief Maximum allowed pitch.
 */
#define NLSOUND_MAX_PITCH 8.0f

/**
 * The default gain is specified as 1.0f, which is the full volume.
 * Using this value, the output volume is the same as the original
 * input volume of the sample buffer.
 * \brief Default gain (volume).
 */
#define NLSOUND_DEFAULT_GAIN 1.0f
/**
 * The default pitch is specified as 1.0.
 * Using this value, the sample will be played at it's original rate.
 * \brief Default pitch.
 */
#define NLSOUND_DEFAULT_PITCH 1.0f

/**
 * The default direct gain is specified as 1.0f.
 * This is the default volume for the direct dry output channel.
 * \brief Default direct gain.
 */
#define NLSOUND_DEFAULT_DIRECT_GAIN 1.0f
/**
 * The default effect gain is specified as 1.0f.
 * This is the default volume for the effect output channel.
 * \brief Default effect gain.
 */
#define NLSOUND_DEFAULT_EFFECT_GAIN 1.0f

/**
 * The default filter cutoff high frequency is specified as 5000.0f.
 * Under OpenAL EFX this is always the reference high frequency.
 * \brief Default filter cutoff high frequency.
 */
#define NLSOUND_DEFAULT_FILTER_PASS_HF 5000.0f
/**
 * The default filter cutoff low frequency is specified as 250.0f.
 * Under OpenAL EFX this is always the reference low frequency.
 * \brief Default filter cutoff low frequency.
 */
#define NLSOUND_DEFAULT_FILTER_PASS_LF 250.0f
/**
 * The default filter pass gain is specified as 1.0f.
 * Using this value, the sound that is outside the cutoff frequencies
 * passes without attenuation, so that essentially the resulting
 * output is not filtered at all.
 * \brief Default filter pass gain.
 */
#define NLSOUND_DEFAULT_FILTER_PASS_GAIN 1.0f

namespace NLSOUND {
	class IBuffer;
	class ILoader;
	class IEffect;

/**
 * Sound source interface (implemented in sound driver dynamic library)
 * 
 * - If the buffer is mono, the source is played in 3D mode. For 
 * arguments as 3D vectors, use the NeL vector coordinate system:
\verbatim
   (top)
    z
    |  y (front)
    | /
    -----x (right)
\endverbatim
 *
 * - If the buffer is multi-channel, only distance rolloff is applied.
 * - All streaming related functionalities are thread-safe.
 * 
 * \author Olivier Cado
 * \author Nevrax France
 * \author Jan Boon (Kaetemi)
 * \date 2001-2009
 * \brief Interface for physical sound sources.
 */
class ISource
{
public:
	enum TFilter
	{
		FilterLowPass, 
		FilterBandPass, 
		FilterHighPass, 
	};
	
	/// Constructor
	ISource() { }
	/// Destructor
	virtual ~ISource() { }
	
	/// \name Initialization
	//@{
	/**
	 * This function is used to switch between streaming and static 
	 * buffer modes. By default, streaming mode is off.
	 * Streaming mode must be enabled before calling functions 
	 * related to buffer streaming, such as  'submitStreamingBuffer' 
	 * and 'countStreamingBuffers'.
	 * In the default static buffer mode, only one buffer is set 
	 * active on this source, and can be replayed or looped as many 
	 * times as needed.
	 * When in streaming mode, multiple buffers are chained after each
	 * other, and you must make sure that the source does not run out 
	 * of data, to avoid sound stopping glitches.
	 * The source must be stopped before trying to change the mode, 
	 * if a static buffer was set it must be detached as well.
	 * \brief Switch between streaming and static buffer modes.
	 * \param streaming Set to true to enable streaming mode, or 
	 * false to use static buffers. If you omit this parameter, it
	 * will be set as true, and streaming will be enabled.
	 */
	virtual void setStreaming(bool streaming = true) = 0;
	/**
	 * Sets the buffer that is used for playback by this source.
	 * The static buffer will remain active on the source until 
	 * this function is called with a NULL parameter, or until the 
	 * mode is changed to streaming. It is possible to play or loop 
	 * this buffer as often as needed,  as long as it is attached 
	 * to this source.
	 * If you attach a multichannel (stereo) buffer, the source will 
	 * automatically be set to relative mode. Setting a mono channel 
	 * buffer will automatically set the relative mode back to it's
	 * original setting. It is not possible to enable relative mode 
	 * with a multichannel buffer, instead you must manually split 
	 * the sound data into multiple channels and use multiple sources.
	 * You cannot use this function in streaming mode, where you must 
	 * use submitStreamingBuffer() instead.
	 * A buffer can be attached to more than once source.
	 * Before destroying a buffer, it must be detached from all 
	 * sources it's attached to, which can be done by calling this 
	 * function with a NULL parameter as buffer on all relevant sources.
	 * If you set the buffer while the source is already playing or in
	 * paused status, this function does nothing. You must stop() the 
	 * source, or wait until it isStopped() before changing the buffer.
	 * \brief Attach a buffer to this source.
	 * \param buffer The buffer to be attached on this source. It must 
	 * be created on the same sound driver instance as this source. Set 
	 * this parameter to NULL to detach a previously set buffer.
	 */
	virtual void setStaticBuffer(IBuffer *buffer) = 0;
	/**
	 * Returns the buffer that was attached to this source. A buffer 
	 * can by attached to this source by calling setStaticBuffer().
	 * If no static buffer is set, or if the source is in streaming 
	 * mode, this function will return NULL.
	 * \brief Get the buffer that is attached to this source.
	 * \return A pointer to the attached IBuffer, or NULL if none.
	 */
	virtual IBuffer *getStaticBuffer() = 0;
	///// Add a buffer to the streaming queue.  A buffer of 100ms length is optimal for streaming. Buffers between 50ms and 100ms are optimal for streaming.
	///// Should be called by a thread which checks countStreamingBuffers every 50ms.Queue cleared on stop.
	virtual void submitStreamingBuffer(IBuffer *buffer) = 0;
	/**
	 * Returns the amount of buffers that are currently in the queue.
	 * This includes buffers that are waiting to be played, as well as 
	 * the buffer that is currently playing. Buffers that have finished 
	 * playing are removed from the queue, and do not count towards the 
	 * result of this function.
	 * It is recommended to have at least 2 or 3 buffers queued up when 
	 * streaming, to avoid sound stopping glitches.
	 * If you need to know the total amount of buffers that were added 
	 * into the queue, you should count them manually by increasing a 
	 * value each time 'submitStreamingBuffer' is called.
	 * This function always returns 0 when the source is in static 
	 * mode, when the source is stopped, or when no buffers have been
	 * added yet into the queue.
	 * \brief Get the number of buffers in the streaming queue.
	 * \return The number of buffers in the queue.
	 */
	virtual uint countStreamingBuffers() const = 0;
	//@}
	
	/// \name Playback control
	//@{
	/**
	 * This function is used to enable or disable looping this source.
	 * By default, looping is off. The current looping status can be 
	 * obtained by calling getLooping().
	 * It is possible to call this function either before the source 
	 * has started playing, or while it is already playing.
	 * To cleanly exit a loop, you can call setLooping(false) instead 
	 * of using stop(). Calling stop() on a source that is looping will 
	 * immediately stop the source, which may suffer sound clicks.
	 * The looping state is useful on static buffers only. Setting this 
	 * while in streaming mode will have no effect, however, the loop
	 * setting will be remembered when switching back to static mode.
	 * \brief Enable or disable looping this source.
	 * \param l Set to true to enable, false to disable looping or to 
	 * exit the current loop.
	 */
	virtual void setLooping(bool l = true) = 0;
	/**
	 * Returns the looping status that was last set by setLooping().
	 * This function will always return the last set looping option.
	 * If you call this while in streaming mode, the result will be the 
	 * setting that will be used when you go back to static buffer mode.
	 * Looping has no effect on streaming.
	 * \brief Get the currently set looping status.
	 * \return Returns true if looping is enabled, false if disabled.
	 */
	virtual bool getLooping() const = 0;
	
	/**
	 * This function is used to enter playing mode. It will return 
	 * true in case of success. The playing status can also be verified 
	 * by calling isPlaying() on this source.
	 * If the source is stopped, or has not started playing yet, the 
	 * source will start playing from the beginning from the currently 
	 * attached static buffer when in static mode. If no static buffer 
	 * was set, the source will remain in stopped mode, and this 
	 * will return false. A buffer can be attached to this source by 
	 * calling the setStaticBuffer() method.
	 * When the source is stopped and in streaming mode, this function 
	 * will always enter playing state. If no buffers were queued yet, 
	 * it will effectively start playing this source once the first 
	 * buffer has been submitted. It is recommended to queue up the 
	 * first few buffers before calling play() to avoid running out of 
	 * buffers early. In streaming mode, buffers can be submitted to 
 	 * the queue by calling the submitStreamingBuffer() method.
	 * If the source is in paused mode, it will simply resume. 
	 * In case it was already in playing mode, the source will be 
	 * automatically stopped before re-entering playing status. In 
	 * static buffer mode this is essentially the same as restarting 
	 * the source from the beginning, while doing this in streaming 
	 * mode will result in the streaming queue being cleared.
	 * \brief Start or resume playback.
	 * \return Returns true in case of success.
	 */
	virtual bool play() = 0;
	/**
	 * This function stops playing the source immediately. If the 
	 * source is already stopped, it does nothing. If the source was 
	 * paused, this will do the same as if it was playing.
	 * When the source is in static mode, the attached buffer will 
	 * remain attached for later usage. To exit a loop, it is 
	 * recommended to call setLooping(false) instead of stop(), 
	 * to avoid sound clicks. The source will enter stopped state 
	 * automatically when it has finished playing.
	 * If this source uses streaming buffers, calling this function 
	 * will immediately stop output, and clear the buffer queue, which 
	 * means that any queued buffers that have not played yet are lost.
	 * \brief Stop playback.
	 */
	virtual void stop() = 0;
	/**
	 * This function pauses playback of this source at it's current 
	 * position. If the source is stopped or already paused, this 
	 * function does nothing. You can verify if the source is paused 
	 * by calling isPaused(). To resume playback, you must call play().
	 * \brief Pause playback.
	 */
	virtual void pause() = 0;
	/**
	 * Returns if the source is currently playing.
	 * In streaming mode this will return true, even if no buffers are 
	 * available for playback when in playing status.
	 * This will also return true if the source is paused. To know if 
	 * the source has been paused, use isPaused().
	 * \brief Get the playing state.
	 * \return Returns the playing state.
	 */
	virtual bool isPlaying() const = 0;
	/**
	 * Returns true if the source has not started playing yet, when it 
	 * has been stopped using the stop() method, or when the static 
	 * buffer has finished playback.
	 * If the source is playing or paused, this returns false.
	 * \brief Get the stopped state.
	 * \return Returns the stopped state.
	 */
	virtual bool isStopped() const = 0;
	/**
	 * Returns true if the source has been paused by calling pause().
	 * If the source is playing, stopped, or has finished playback 
	 * this returns false.
	 * \brief Get the paused state.
	 * \return Returns the paused state.
	 */
	virtual bool isPaused() const = 0;
	/**
	 * Returns the number of milliseconds the current static buffer or 
	 * streaming queue has effectively been playing.
	 * In streaming mode, the time spent during buffer outruns is not 
	 * counted towards the playback time, and the playback time is 
	 * be the current time position in the entire submitted queue.
	 * When using static buffers, the result is the total time that the 
	 * attached buffer has been playing. If the source is looping, the 
	 * time will be the total of all playbacks of the buffer.
	 * When the source is stopped, this will return the time where the 
	 * source was stopped. The value is reset to 0 when re-entering 
	 * playing status.
	 * A buffer that is played at a higher pitch will result in a lower 
	 * playback time. The result is not the buffer's playback position, 
	 * but the actual time that has passed playing.
	 * It is not guaranteed that this function returns an accurate 
	 * value, or that it even works. If it is not implemented, the result 
	 * will always be 0.
	 * \brief Get the current playback time in milliseconds.
	 * \return Returns the current playback time in milliseconds.
	 */
	virtual uint32 getTime() = 0;
	//@}
	
	/// \name Source properties
	//@{
	/// Set the position vector (default: (0,0,0)).
	virtual void setPos(const NLMISC::CVector& pos, bool deffered = true) = 0;
	/// Get the position vector.
	virtual const NLMISC::CVector &getPos() const = 0;
	/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
	virtual void setVelocity(const NLMISC::CVector& vel, bool deferred = true) = 0;
	/// Get the velocity vector
	virtual void getVelocity(NLMISC::CVector& vel) const = 0;
	/// Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
	virtual void setDirection(const NLMISC::CVector& dir) = 0;
	/// Get the direction vector
	virtual void getDirection(NLMISC::CVector& dir) const = 0;
	/// Set the gain (volume value inside [0 , 1]). (default: 1)
	virtual void setGain(float gain = NLSOUND_DEFAULT_GAIN) = 0;
	/// Get the gain
	virtual float getGain() const = 0;
	/// Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift of one octave.
	virtual void setPitch(float pitch = NLSOUND_DEFAULT_PITCH) = 0;
	/// Get the pitch
	virtual float getPitch() const = 0;
	/// Set the source relative mode. If true, positions are interpreted relative to the listener position
	virtual void setSourceRelativeMode(bool mode = true) = 0;
	/// Get the source relative mode
	virtual bool getSourceRelativeMode() const = 0;
	/// Set the min and max distances (default: 1, sqrt(MAX_FLOAT)) (3D mode only)
	virtual void setMinMaxDistances(float mindist, float maxdist, bool deferred = true) = 0;
	/// Get the min and max distances
	virtual void getMinMaxDistances(float& mindist, float& maxdist) const = 0;
	/// Set the cone angles (in radian) and gain (in [0 , 1]) (default: 2PI, 2PI, 0)
	virtual void setCone(float innerAngle, float outerAngle, float outerGain) = 0;
	/// Get the cone angles (in radian)
	virtual void getCone(float& innerAngle, float& outerAngle, float& outerGain) const = 0;
	/** 
	 * Set the alpha value for the volume-distance curve.
	 * 
	 * Used only with OptionManualRolloff.
	 * Value ranges from -1.0 to 1.0, the default is 0.0.
	 * 
	 * - alpha = 0.0: The volume will decrease linearly between 0dB 
	 *   and -100 dB.
	 * - alpha = 1.0: The volume will decrease linearly between 1.0f 
	 *   and 0.0f (linear scale).
	 * - alpha = -1.0: The volume will decrease inversely with the 
	 *   distance (1/dist). This is the default distance attenuation 
	 *   used without OptionManualRolloff.
	 * 
	 * For any other value of alpha, an interpolation is be done 
	 * between the two adjacent curves. For example, if alpha equals 
	 * 0.5, the volume will be halfway between the linear dB curve and 
	 * the linear amplitude curve.
	 * \brief Set the alpha value for the volume-distance curve
	 */
	virtual void setAlpha(double a) = 0;
	//@}
	
	/// \name Direct output
	//@{
	/// Enable or disable direct output [true/false], default: true
	virtual void setDirect(bool enable = true) = 0;
	/// Return if the direct output is enabled
	virtual bool getDirect() const = 0;
	/// Set the gain for the direct path
	virtual void setDirectGain(float gain = NLSOUND_DEFAULT_DIRECT_GAIN) = 0;
	/// Get the gain for the direct path
	virtual float getDirectGain() const = 0;
	
	/// Enable or disable the filter for the direct channel
	virtual void enableDirectFilter(bool enable = true) = 0;
	/// Check if the filter on the direct channel is enabled
	virtual bool isDirectFilterEnabled() const = 0;
	/// Set the filter parameters for the direct channel
	virtual void setDirectFilter(TFilter filter, float lowFrequency = NLSOUND_DEFAULT_FILTER_PASS_LF, float highFrequency = NLSOUND_DEFAULT_FILTER_PASS_HF, float passGain = NLSOUND_DEFAULT_FILTER_PASS_GAIN) = 0;
	/// Get the filter parameters for the direct channel
	virtual void getDirectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const = 0;
	/// Set the direct filter gain
	virtual void setDirectFilterPassGain(float passGain = NLSOUND_DEFAULT_FILTER_PASS_GAIN) = 0;
	/// Get the direct filter gain
	virtual float getDirectFilterPassGain() const = 0;
	//@}
	
	/// \name Effect output
	//@{
	/// Set the effect send for this source, NULL to disable. [IEffect], default: NULL
	virtual void setEffect(IReverbEffect *reverbEffect) = 0;
	/// Get the effect send for this source
	virtual IEffect *getEffect() const = 0;
	/// Set the gain for the effect path
	virtual void setEffectGain(float gain = NLSOUND_DEFAULT_EFFECT_GAIN) = 0;
	/// Get the gain for the effect path
	virtual float getEffectGain() const = 0;
	
	/// Enable or disable the filter for the effect channel
	virtual void enableEffectFilter(bool enable = true) = 0;
	/// Check if the filter on the effect channel is enabled
	virtual bool isEffectFilterEnabled() const = 0;
	/// Set the filter parameters for the effect channel
	virtual void setEffectFilter(TFilter filter, float lowFrequency = NLSOUND_DEFAULT_FILTER_PASS_LF, float highFrequency = NLSOUND_DEFAULT_FILTER_PASS_HF, float passGain = NLSOUND_DEFAULT_FILTER_PASS_GAIN) = 0;
	/// Get the filter parameters for the effect channel
	virtual void getEffectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const = 0;
	/// Set the effect filter gain
	virtual void setEffectFilterPassGain(float passGain = NLSOUND_DEFAULT_FILTER_PASS_GAIN) = 0;
	/// Get the effect filter gain
	virtual float getEffectFilterPassGain() const = 0;
	//@}
	
protected:
	
	// common method used only with OptionManualRolloff. return the volume in 1/100th DB  ( = mB)modified
	static sint32 computeManualRollOff(sint32 volumeMB, sint32 mbMin, sint32 mbMax, double alpha, float sqrdist, float distMin, float distMax);
	// common method used only with OptionManualRolloff. return the rolloff in amplitude ratio (gain)
	static float computeManualRolloff(double alpha, float sqrdist, float distMin, float distMax);
};


} // NLSOUND


#endif // NL_SOURCE_H

/* End of source.h */
