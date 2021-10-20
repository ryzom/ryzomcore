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

#ifndef NL_SOUND_DRIVER_H
#define NL_SOUND_DRIVER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/file.h"
#include "nel/misc/string_mapper.h"

#include "effect.h"

/// This namespace contains the sound classes
namespace NLSOUND
{
	class IBuffer;
	class IListener;
	class ISource;
	class IMusicChannel;
	class ISubmix;

// don't use eax.h anymore
#if !defined( EAX_AVAILABLE )
#	define EAX_AVAILABLE 0
#endif

/*
 * Deprecated sound sample format.
 * For compatibility with old code.
 * Do not modify.
 */
enum TSampleFormat { Mono8, Mono16ADPCM, Mono16, Stereo8, Stereo16, SampleFormatUnknown = (~0) };

/**
 * Abstract sound driver (implemented in sound driver dynamic library)
 *
 * The caller of the create methods is responsible for the deletion of the created objects.
 * These objects must be deleted before deleting the ISoundDriver instance.
 *
 * The driver is a singleton. To access, only the pointer returned by createDriver()
 * is provided.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class ISoundDriver
{
public:
	/// Driver Creation Choice
	enum TDriver
	{
		/// DriverAuto automatically picks the most awesome driver (picks first available in this order: FMod, OpenAl, XAudio2, DSound).
		DriverAuto = 0,
		/// DriverFMod is a driver that runs on FMod, nice quality, but not fully implemented.
		DriverFMod,
		/// DriverOpenAl is the recommended driver, especially if you have hardware sound acceleration.
		DriverOpenAl,
		/// DriverDSound is deprecated.
		DriverDSound,
		/// DriverXAudio2 runs on a fully software-based audio processing API without artificial limits.
		DriverXAudio2,
		NumDrivers
	};

	/// Driver creation flags, to configure behaviour, or check feature availability.
	enum TSoundOptions
	{
		/// Enable EAX/I3DL2 environment effects. (not implemented on FMod driver).
		OptionEnvironmentEffects = 0x01, 
		/// Allow the user to use the ADPCM encoding. (verify availability with getOption)
		OptionAllowADPCM = 0x02, 
		/// Force software buffering (always true for XAudio2).
		OptionSoftwareBuffer = 0x04, 
		/**
		 *	Configuration to use manual or API (directx or open AL) rolloff factor.
		 *	0 => API (directx, open AL, etc) rollOff control.
		 *		ISource::setAlpha() will fail.
		 *		IListener::setRollOffFactor() works
		 *	1 => Manual rollOff control
		 *		ISource::setAlpha() change the shape of attenuation (change the curve)
		 *		IListener::setRollOffFactor() will fail
		 */
		OptionManualRolloff = 0x08, 
		/// Enable local copy of buffer (used by OpenAL driver, required to build sample bank).
		OptionLocalBufferCopy = 0x10, 
		/// Use to check availability of buffer streaming. (verify with getOption)
		OptionHasBufferStreaming = 0x20, 
	};

	/** The interface must be implemented and provided to the driver
	 *	in order to have a coherent string mapping.
	 *	The driver must not call directly CStringMapper method because
	 *	the static map container are located in a lib, so the main
	 *	code and the driver have theire own version of the static
	 *	container !
	 */
	class IStringMapperProvider
	{
	public:
		virtual ~IStringMapperProvider() {}
		/// map a string
		virtual NLMISC::TStringId map(const std::string &str) = 0;
		/// unmap a string
		virtual const std::string &unmap(const NLMISC::TStringId &stringId) = 0;
	};

	/// Version of the driver interface. To increment when the interface change.
	static const uint32 InterfaceVersion;

	/// Return driver name from type.
	static const char *getDriverName(TDriver driverType);
	/** The static method which builds the sound driver instance
	 * In case of failure, can throw one of these ESoundDriver exception objects:
	 * ESoundDriverNotFound, ESoundDriverCorrupted, ESoundDriverOldVersion, ESoundDriverUnknownVersion
	*/
	static ISoundDriver *createDriver(IStringMapperProvider *stringMapper, TDriver driverType = DriverAuto);

	/// Constructor
	ISoundDriver() { }
	/// Destructor
	virtual	~ISoundDriver() {}

	/// Return a list of available devices for the user. The value at index 0 is empty, and is used for automatic device selection.
	virtual void getDevices(std::vector<std::string> &devices) = 0;
	/** Initialize the driver with a user selected device. 
	 * In case of failure, can throw one of these ESoundDriver exception objects:
	 * ESoundDriverNotFound, ESoundDriverCorrupted, ESoundDriverOldVersion, ESoundDriverUnknownVersion
	 * The driver instance should be deleted by the user after init failure.
	 *
	 * You can request support for EAX. If EAX is requested and if there is enougth hardware
	 * buffer replay, then only hardware buffer are created when calling createBuffer.
	 * If the number of available hardware buffer is less than 10, then EAX is ignored.
	 *
	 *	\param device If device.empty(), the default or most appropriate device is used.
	 *	\param options The features you want enabled. Verify after init() with getOption() or getOptions().
	*/
	virtual void initDevice(const std::string &device, TSoundOptions options) = 0;

	/// Return options that are enabled (including those that cannot be disabled on this driver).
	virtual TSoundOptions getOptions() = 0;
	/// Return if an option is enabled (including those that cannot be disabled on this driver).
	virtual bool getOption(TSoundOptions option) = 0;

	/// Commit all the changes made to 3D settings of listener and sources
	virtual void commit3DChanges() = 0;

	/// Create the listener instance
	virtual	IListener *createListener() = 0;
	/// Create a source, destroy with delete
	virtual	ISource *createSource() = 0;
	/// Create a sound buffer, destroy with delete
	virtual	IBuffer *createBuffer() = 0;
	/// Create a reverb effect
	virtual IReverbEffect *createReverbEffect() { return NULL; }
	/// Return the maximum number of sources that can created
	virtual uint countMaxSources() = 0;
	/// Return the maximum number of effects that can be created
	virtual uint countMaxEffects() { return 0; }
	
	/// Write information about the driver to the output stream.
	virtual void writeProfile(std::string& out) = 0;
	
	/// Stuff
	virtual void startBench() = 0;
	virtual void endBench() = 0;
	virtual void displayBench(NLMISC::CLog *log) = 0;

	/// Filled at createDriver()
	const std::string &getDllName() const { return _DllName; }
	
	/// \name Stuff for drivers that have native music support
	//@{
	/// Create a native music channel, only supported by the FMod driver.
	virtual IMusicChannel *createMusicChannel() { return NULL; }
	/** Get music info. Returns false if the song is not found or the function is not implemented.
	 *  \param filepath full path to file
	 *  \param artist returns the song artist (empty if not available)
	 *  \param title returns the title (empty if not available)
	 */
	virtual bool getMusicInfo(const std::string &/* filepath */, std::string &artist, std::string &title, float &length) { artist.clear(); title.clear(); length = 0.f; return false; }
	/// Get audio/container extensions that are supported natively by the driver implementation.
	virtual void getMusicExtensions(std::vector<std::string> &extensions) const = 0;
	/// Return if a music extension is supported by the driver's music channel.
	virtual bool isMusicExtensionSupported(const std::string &extension) const = 0;
	//@}
	
private:
	std::string _DllName;
	
};


/**
 * Sound driver exceptions
 */
class ESoundDriver : public NLMISC::Exception
{
public:
	ESoundDriver() : NLMISC::Exception( "Sound driver error" ) {}
	ESoundDriver( const char *reason ) : NLMISC::Exception( reason ) {}
	ESoundDriver( const std::string &reason ) : NLMISC::Exception( reason.c_str() ) {}
};


/**
 * ESoundDriverNotFound
 */
class ESoundDriverNotFound : public ESoundDriver
{
public:
	ESoundDriverNotFound(const std::string &dllName) : ESoundDriver( dllName + " or third-party library not found" ) {}
};


/**
 * ESoundDriverCorrupted
 */
class ESoundDriverCorrupted : public ESoundDriver
{
public:
	ESoundDriverCorrupted(const std::string &dllName) : ESoundDriver( std::string("Can't get NLSOUND_createISoundDriverInstance from ") + dllName + " (Bad dll?)" ) {}
};


/**
 * ESoundDriverOldVersion
 */
class ESoundDriverOldVersion : public ESoundDriver
{
public:
	ESoundDriverOldVersion(const std::string &dllName) : ESoundDriver( dllName + " is a too old version. Ask for a more recent file" ) {}
};


/**
 * ESoundDriverUnknownVersion
 */
class ESoundDriverUnknownVersion : public ESoundDriver
{
public:
	ESoundDriverUnknownVersion(const std::string &dllName) : ESoundDriver( dllName + " is more recent than the application" ) {}
};


/**
 * ESoundDriverCantCreateDriver
 */
class ESoundDriverCantCreateDriver : public ESoundDriver
{
public:
	ESoundDriverCantCreateDriver(const std::string &dllName) : ESoundDriver( dllName + " can't create driver" ) {}
};


/**
 * ESoundDriverGenBuf
 */
class ESoundDriverGenBuf : public ESoundDriver
{
public:
	ESoundDriverGenBuf() : ESoundDriver( "Unable to generate sound buffers" ) {}
};


/**
 * ESoundDriverGenBuf
 */
class ESoundDriverGenSrc : public ESoundDriver
{
public:
	ESoundDriverGenSrc() : ESoundDriver( "Unable to generate sound sources" ) {}
};


/**
 * ESoundDriverNotSupp
 */
class ESoundDriverNotSupp : public ESoundDriver
{
public:
	ESoundDriverNotSupp() : ESoundDriver("Operation is not supported by the current sound driver") { }
	ESoundDriverNotSupp(const char *reason) : ESoundDriver(reason) { }
	ESoundDriverNotSupp(const std::string &reason) : ESoundDriver(reason.c_str()) { }
};


/**
 * ESoundDriverNoEnvironmentEffects : ESoundDriverNotSupp : ESoundDriver : NLMISC::Exception
 */
class ESoundDriverNoEnvironmentEffects : public ESoundDriverNotSupp
{
public:
	ESoundDriverNoEnvironmentEffects() : ESoundDriverNotSupp("Environment effects are not supported by the current sound driver") { }
};


/**
 * ESoundDriverNoADPCM : ESoundDriverNotSupp : ESoundDriver : NLMISC::Exception
 */
class ESoundDriverNoADPCM : public ESoundDriverNotSupp
{
public:
	ESoundDriverNoADPCM() : ESoundDriverNotSupp("ADPCM is not supported by the current sound driver") { }
};


/**
 * ESoundDriverNoBufferStreaming : ESoundDriverNotSupp : ESoundDriver : NLMISC::Exception
 */
class ESoundDriverNoBufferStreaming : public ESoundDriverNotSupp
{
public:
	ESoundDriverNoBufferStreaming() : ESoundDriverNotSupp("Buffer streaming is not supported by the current sound driver") { }
};


/**
 * ESoundDriverNoManualRolloff : ESoundDriverNotSupp : ESoundDriver : NLMISC::Exception
 */
class ESoundDriverNoManualRolloff : public ESoundDriverNotSupp
{
public:
	ESoundDriverNoManualRolloff() : ESoundDriverNotSupp("Manual rolloff alpha is not supported by the current sound driver") { }
};


} // NLSOUND


#endif // NL_SOUND_DRIVER_H

/* End of sound_driver.h */
