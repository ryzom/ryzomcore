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

#ifndef NL_SOUND_DRIVER_FMOD_H
#define NL_SOUND_DRIVER_FMOD_H

#include "nel/sound/driver/sound_driver.h"

namespace NLSOUND {
	class IListener;
	class ISource;
	class IBuffer;
	class CListenerFMod;
	class CSourceFMod;
	class CBufferFMod;
	class CMusicChannelFMod;

// ***************************************************************************
/*
	DRIVER FMOD LIMITATIONS:

	- EAX is not supported (setEnvironnement() / setEAXProperty() no-op)
	- ADPCM is not supported (decompressed and the format change internaly )
	- deffered param on ISource::setPos() etc... does not work. Always deffered.
	- No cooperative level change in FMod as in DSOUND (default???)

*/


// ***************************************************************************
class CSoundDriverFMod : public ISoundDriver, public NLMISC::CManualSingleton<CSoundDriverFMod>
{
public:

    /// Constructor
    CSoundDriverFMod(ISoundDriver::IStringMapperProvider *stringMapper);

    virtual ~CSoundDriverFMod();
	
	/// Return a list of available devices for the user. The value at index 0 is empty, and is used for automatic device selection.
	virtual void getDevices(std::vector<std::string> &devices);
	/// Initialize the driver with a user selected device. If device.empty(), the default or most appropriate device is used.
	virtual void initDevice(const std::string &device, TSoundOptions options);

	/// Return options that are enabled (including those that cannot be disabled on this driver).
	virtual TSoundOptions getOptions();
	/// Return if an option is enabled (including those that cannot be disabled on this driver).
	virtual bool getOption(TSoundOptions option);

	/// Create the listener instance
	virtual	IListener *createListener();

	/// Create a sound buffer
	virtual	IBuffer *createBuffer();

    // Source management

	/// Create a source
	virtual	ISource *createSource();

	/// Commit all the changes made to 3D settings of listener and sources
	virtual void commit3DChanges();

	/// Return the maximum number of sources that can created
	virtual uint countMaxSources();

	/// Count the number of sources that are actually playing.
    uint countPlayingSources();

	/// Update all the driver and its sources. To be called only by the timer callback.
	void update();

	/// Write information about the driver to the output stream.
	void writeProfile(std::string& out);


    /** Set the gain (volume value inside [0 , 1]). (default: 1)
	 * 0.0 -> silence
	 * 0.5 -> -6dB
	 * 1.0 -> no attenuation
	 * values > 1 (amplification) not supported by most drivers
	 */
    void setGain( float gain );

    /// Get the gain
	float getGain();

	/// Return the string mapper
	IStringMapperProvider	*getStringMapper()	{return _StringMapper;}

	// Tool method to transform from Nel coords to FMod coords
	static void	toFModCoord(const NLMISC::CVector &in, float out[3]);

	bool	fmodOk() const {return _FModOk;}

	bool	forceSofwareBuffer() const {return _ForceSoftwareBuffer;}

	/// Create a music channel, destroy with destroyMusicChannel
	virtual IMusicChannel *createMusicChannel();
	
	/** Get music info. Returns false if the song is not found or the function is not implemented.
	 *  \param filepath full path to file
	 *  \param artist returns the song artist (empty if not available)
	 *  \param title returns the title (empty if not available)
	 */
	virtual bool getMusicInfo(const std::string &filepath, std::string &artist, std::string &title, float &length);

	// also check that the channel still exist (avoid any free problem)
	void markMusicChannelEnded(void *stream, CMusicChannelFMod *musicChannel);

	/// (Internal) Remove a buffer (should be called by the destructor of the buffer class)
	void removeBuffer(CBufferFMod *buffer);
	/// (Internal) Remove a source (should be called by the destructor of the source class)
	void removeSource(CSourceFMod *source);
	/// (Internal) Remove a music channel (should be called by the destructor of the music channel class)
	void removeMusicChannel(CMusicChannelFMod *musicChannel);

	/// Get audio/container extensions that are supported natively by the driver implementation.
	virtual void getMusicExtensions(std::vector<std::string> &extensions) const;
	/// Return if a music extension is supported by the driver's music channel.
	virtual bool isMusicExtensionSupported(const std::string &extension) const;

private:	
	virtual void startBench();
	virtual void endBench();
	virtual void displayBench(NLMISC::CLog *log);

	void updateMusic();

	/// The string mapper provided by client code
	IStringMapperProvider *_StringMapper;

    // Array with the allocated sources
	std::set<CSourceFMod*> _Sources;
	/// Array with the allocated music channels
	std::set<CMusicChannelFMod *> _MusicChannels;

	/// if correctly created
	bool _FModOk;
	/// If want to create buffer in software (no hardware)
	bool _ForceSoftwareBuffer;

	/// Master Volume [0,1]
	float _MasterGain;

	/// Driver options
	TSoundOptions _Options;

};


} // NLSOUND

#endif // NL_SOUND_DRIVER_FMOD_H
