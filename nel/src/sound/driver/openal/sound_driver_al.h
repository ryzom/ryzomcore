// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_SOUND_DRIVER_AL_H
#define NL_SOUND_DRIVER_AL_H

#include "nel/sound/driver/sound_driver.h"

namespace NLSOUND {
	class CBufferAL;
	class CListenerAL;
	class CSourceAL;
	class CEffectAL;

// alGenBuffers, alGenSources
//typedef ALAPI ALvoid ALAPIENTRY (*TGenFunctionAL) ( ALsizei, ALuint* );
typedef void (*TGenFunctionAL) ( ALsizei, ALuint* );

// alDeleteBuffers
typedef void (*TDeleteFunctionAL) ( ALsizei, const ALuint* );

// alIsBuffer, alIsSource
//typedef ALAPI ALboolean ALAPIENTRY (*TTestFunctionAL) ( ALuint );
typedef ALboolean (*TTestFunctionAL) ( ALuint );

#if !FINAL_VERSION
void alTestWarning(const char *src);
#else
#define alTestWarning(__src)
#endif

#ifdef NL_DEBUG
void alTestError();
#else
#define alTestError() alTestWarning("alTestError")
#endif

/**
 * OpenAL sound driver
 *
 * The caller of the create methods is responsible for the deletion of the created objects
 * These objects must be deleted before deleting the ISoundDriver instance.
 *
 *
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CSoundDriverAL : public ISoundDriver, public NLMISC::CManualSingleton<CSoundDriverAL>
{
private:
	// outside pointers
	/// The string mapper provided by client code.
	IStringMapperProvider *_StringMapper;

	// openal pointers
	// OpenAL device
	ALCdevice *_AlDevice;
	// OpenAL context
	ALCcontext *_AlContext;

	// system vars
	// Allocated buffers
	std::vector<ALuint> _Buffers;
	// Allocated sources
	std::set<CSourceAL *> _Sources;
	// Allocated effects
	std::set<CEffectAL *> _Effects;
	// Number of exported buffers (including any deleted buffers)
	uint _NbExpBuffers;
	// Number of exported sources (including any deleted sources)
	uint _NbExpSources;

	// user vars
	// Rolloff factor (not in the listener in OpenAL, but relative to the sources)
	float _RolloffFactor; // ***todo*** move
	/// Driver options
	TSoundOptions _Options;
public:

	/// Constructor
	CSoundDriverAL(ISoundDriver::IStringMapperProvider *stringMapper);
	/// Destructor
	virtual ~CSoundDriverAL();

	inline ALCdevice *getAlDevice() { return _AlDevice; }
	inline ALCcontext *getAlContext() { return _AlContext; }
	inline float getRolloffFactor() { return _RolloffFactor; }

	/// Return a list of available devices for the user. The value at index 0 is empty, and is used for automatic device selection.
	virtual void getDevices(std::vector<std::string> &devices);
	/// Initialize the driver with a user selected device. If device.empty(), the default or most appropriate device is used.
	virtual void initDevice(const std::string &device, TSoundOptions options);

	/// Return options that are enabled (including those that cannot be disabled on this driver).
	virtual TSoundOptions getOptions();
	/// Return if an option is enabled (including those that cannot be disabled on this driver).
	virtual bool getOption(TSoundOptions option);

	/// Create a sound buffer
	virtual	IBuffer *createBuffer();
	/// Create the listener instance
	virtual	IListener *createListener();
	/// Create a source
	virtual	ISource *createSource();
	/// Create a reverb effect
	virtual IReverbEffect *createReverbEffect();
	/// Return the maximum number of sources that can created
	virtual uint countMaxSources();
	/// Return the maximum number of effects that can be created
	virtual uint countMaxEffects();
	
	virtual void startBench();
	virtual void endBench();
	virtual void displayBench(NLMISC::CLog *log);


	/// Change the rolloff factor and apply to all sources
	void applyRolloffFactor(float f);

	/// Commit all the changes made to 3D settings of listener and sources
	virtual void commit3DChanges();

	/// Write information about the driver to the output stream.
	virtual void writeProfile(std::string& out);

	/// Remove a buffer
	void removeBuffer(CBufferAL *buffer);
	/// Remove a source
	void removeSource(CSourceAL *source);
	/// Remove an effect
	void removeEffect(CEffectAL *effect);

	/// Get audio/container extensions that are supported natively by the driver implementation.
	virtual void getMusicExtensions(std::vector<std::string> & /* extensions */) const { }
	/// Return if a music extension is supported by the driver's music channel.
	virtual bool isMusicExtensionSupported(const std::string & /* extension */) const { return false; }

protected:

	/// Allocate nb new buffers or sources
	void					allocateNewItems( TGenFunctionAL algenfunc, TTestFunctionAL altestfunc,
											  std::vector<ALuint>& names, uint index, uint nb );
	
	/// Generate nb buffers
	void					generateItems( TGenFunctionAL algenfunc, TTestFunctionAL altestfunc, uint nb, ALuint *array );

	/// Remove names of deleted items and return the number of valid items
	uint					compactAliveNames( std::vector<ALuint>& names, TTestFunctionAL testfunc );

	/// Create a sound buffer or a sound source
	ALuint					createItem( TGenFunctionAL algenfunc, TTestFunctionAL altestfunc,
										std::vector<ALuint>& names, uint& index, uint allocrate );

	/// Delete a buffer or a source
	bool					deleteItem( ALuint name, TDeleteFunctionAL aldeletefunc, std::vector<ALuint>& names );
};


} // NLSOUND


#endif // NL_SOUND_DRIVER_AL_H

/* End of sound_driver_al.h */
