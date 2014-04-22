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

#ifndef NL_SOURCE_COMMON_H
#define NL_SOURCE_COMMON_H

#include "nel/misc/types_nl.h"
#include "nel/sound/u_source.h"
#include "nel/sound/u_stream_source.h"
#include "nel/3d/cluster.h"
#include "nel/sound/sound.h"
#include "nel/sound/group_controller.h"
#include "nel/misc/sheet_id.h"

namespace NLSOUND {

class CSourceCommon : public UStreamSource
{
	friend class CAudioMixerUser;
public:

	enum TSOURCE_TYPE
	{
		SOURCE_SIMPLE,
		SOURCE_COMPLEX,
		SOURCE_BACKGROUND,
		SOURCE_MUSIC, // DEPRECATED
		SOURCE_STREAM,
		SOURCE_STREAM_FILE
	};

	/// When groupController is NULL it will use the groupcontroller specified in the TSoundId. You should manually specify the groupController if this source is a child of another source, so that the parent source controller of the user-specified .sound file is the one that will be used.
	CSourceCommon(TSoundId id, bool spawn, TSpawnEndCallback cb, void *cbUserParam, NL3D::CCluster *cluster, CGroupController *groupController);

	~CSourceCommon();


	/// Get the type of the source.
	virtual TSOURCE_TYPE	getType() const = 0;

	void					setPriority( TSoundPriority pr);
	/// Return the priority
	TSoundPriority			getPriority() const								{ return _Priority; }
	void					setLooping( bool loop );
	bool					getLooping() const;
	void					play();
	void					stop();
	bool					isPlaying()									{ return _Playing; }
	void					setPos( const NLMISC::CVector& pos ) ;
	const NLMISC::CVector	&getPos() const;
	void					setVelocity( const NLMISC::CVector& vel );
	void					setDirection( const NLMISC::CVector& dir );
	void					setGain( float gain );
	void					setRelativeGain( float gain );
	float					getRelativeGain() const;
	/// Called whenever the gain is changed trough setGain, setRelativeGain or the group controller's gain settings change.
	virtual void					updateFinalGain()							{ }
	void					setSourceRelativeMode( bool mode );
	/// return the user param for the user callback
	void							*getCallbackUserParam(void) const			{ return _CbUserParam; }
	/// Tells this source not to call its callbacks when it ends. This is valid for spawned sources only.
	virtual	void					unregisterSpawnCallBack()					{ _SpawnEndCb = NULL; }
	/// Get the velocity vector
	virtual void					getVelocity( NLMISC::CVector& vel ) const	{ vel = _Velocity; }
	/// Get the direction vector
	virtual void					getDirection( NLMISC::CVector& dir ) const	{ dir = _Direction; }
	/// Get the gain
	virtual float					getGain() const								{ return _Gain; }
	/// Get the final gain, including group controller changes. Use this when setting the physical source output gain.
	inline float					getFinalGain() const						{ return _Gain * _GroupController->getFinalGain(); }
	/// Get the pitch
	virtual float					getPitch() const							{ return _Pitch; }
	/// Get the source relative mode
	virtual bool					getSourceRelativeMode() const				{ return _RelativeMode; }
	/// Set the position vector to return for a stereo source (default: NULL)
	// void							set3DPositionVector( const NLMISC::CVector *pos )	{ _3DPosition = pos; }
	/// Return the spawn state
	bool							isSpawn() const								{ return _Spawn; }
	/** Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
	 * of one octave. 0 is not a legal value.
	 */
	virtual void					setPitch( float pitch );

	virtual uint32					getTime();

	NL3D::CCluster					*getCluster() const { return _Cluster; }

	/** This method is called from backgroup sound to check if
	 *	subsound need to be restarted (ie subsound in loop mode).
	 */
	virtual void					checkup() {}
	
	/// \name Streaming source controls
	//@{
	/// Set the sample format. (channels = 1, 2, ...; bitsPerSample = 8, 16; frequency = samples per second, 44100, ...)
	virtual void					setFormat(uint8 /* channels */, uint8 /* bitsPerSample */, uint32 /* frequency */) { nlassert(false); }
	/// Return the sample format information.
	virtual void					getFormat(uint8 &/* channels */, uint8 &/* bitsPerSample */, uint32 &/* frequency */) const { nlassert(false); }
	/// Get a writable pointer to the buffer of specified size. Use capacity to specify the required bytes. Returns NULL when all the buffer space is already filled. Call setFormat() first.
	virtual uint8					*lock(uint /* capacity */) { nlassert(false); return NULL; }
	/// Notify that you are done writing to the locked buffer, so it can be copied over to hardware if needed. Set size to the number of bytes actually written to the buffer. Returns true if ok.
	virtual bool					unlock(uint /* size */) { nlassert(false); return false; }
	/// Get the recommended buffer size to use with lock()/unlock()
	virtual void					getRecommendedBufferSize(uint &/* samples */, uint &/* bytes */) const { nlassert(false); }
	/// Get the recommended sleep time based on the size of the last submitted buffer and the available buffer space
	virtual uint32					getRecommendedSleepTime() const { nlassert(false); return 0; }
	/// Return if there are still buffers available for playback.
	virtual bool					hasFilledBuffersAvailable() const { nlassert(false); return false; }
	//@}
	
protected:
	// Dynamic properties
	TSoundPriority					_Priority;
	bool							_Playing;
	bool							_Looping;

	NLMISC::CVector					_Position;
	NLMISC::CVector					_Velocity;
	NLMISC::CVector					_Direction;
	float							_Gain;
	float							_Pitch;
	bool							_RelativeMode;

	/// Gain not affected by setRelativeGain and used to compute _Gain.
	float							_InitialGain;

	// Playing start time
	NLMISC::TTime					_PlayStart;

	// Position to return, for a stereo source
	// const NLMISC::CVector			*_3DPosition;

	// Spawn state
	const bool						_Spawn;
	TSpawnEndCallback				_SpawnEndCb;
	void							*_CbUserParam;
	NL3D::CCluster					*_Cluster;

	/// An optional user var controler.
	NLMISC::TStringId				_UserVarControler;

	/// Group controller for gain
	CGroupController				*_GroupController;

};

} // NLSOUND


#endif //NL_SOURCE_COMMON_H
