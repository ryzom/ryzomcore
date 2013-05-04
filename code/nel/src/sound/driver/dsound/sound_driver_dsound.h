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

#ifndef NL_SOUND_DRIVER_DSOUND_H
#define NL_SOUND_DRIVER_DSOUND_H

#include "nel/sound/driver/sound_driver.h"

#include "source_dsound.h"
#include "buffer_dsound.h"

namespace NLSOUND {

class IListener;
class ISource;
class IBuffer;
class CListenerDSound;
class CSourceDSound;
class CBufferDSound;

class CSoundDriverDSound : public ISoundDriver
{
public:
    /// Constructor
    CSoundDriverDSound(ISoundDriver::IStringMapperProvider *stringMapper);

    virtual ~CSoundDriverDSound();

	/// Return the instance of the singleton
	static CSoundDriverDSound *instance() { return _Instance; }
	
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

	/// Count the number of available hardware streaming 3D buffers
    uint countHw3DBuffers();

	/// Count the number of available hardware streaming 2D buffers
    uint countHw2DBuffers();

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

#if EAX_AVAILABLE == 1
	LPKSPROPERTYSET		createPropertySet(CSourceDSound *source);
#endif

	/// Create a music channel, destroy with destroyMusicChannel
	virtual IMusicChannel *createMusicChannel() { return NULL; }

	/// Destroy a music channel
	virtual void destroyMusicChannel(IMusicChannel * /* musicChannel */) { nlassert(false); }
	
	/** Get music info. Returns false if the song is not found or the function is not implemented. 
	 *  If the song has no name, result is filled with the filename.
	 *  \param filepath path to file, CPath::lookup done by driver
	 *  \param artist returns the song artist (empty if not available)
	 *  \param title returns the title (empty if not available)
	 */
	virtual bool getMusicInfo(const std::string & /* filepath */, std::string &artist, std::string &title) { artist.clear(); title.clear(); return false; }

private:

	// The callback for the multimedia timer
    static void CALLBACK TimerCallback(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

	// The refence to the singleton.
    static CSoundDriverDSound* _Instance;

	// The period of the timer.
    static uint32 _TimerPeriod;

	friend CBufferDSound::~CBufferDSound();
	friend CSourceDSound::~CSourceDSound();

 	/// Remove a buffer (should be called by the friend destructor of the buffer class)
	virtual void removeBuffer(IBuffer *buffer);

	/// Remove a source (should be called by the friend destructor of the source class)
	virtual void removeSource(ISource *source);


	virtual void	startBench();
	virtual void	endBench();
	virtual void	displayBench(NLMISC::CLog *log);
	
	
	/// Get audio/container extensions that are supported natively by the driver implementation.
	virtual void getMusicExtensions(std::vector<std::string> & /* extensions */) const { }
	/// Return if a music extension is supported by the driver's music channel.
	virtual bool isMusicExtensionSupported(const std::string & /* extension */) const { return false; }
	
	
	// The DirectSound object
    LPDIRECTSOUND			_DirectSound;

    // The application-wide primary buffer
    LPDIRECTSOUNDBUFFER	_PrimaryBuffer;

    // The capabilities of the driver
    DSCAPS _Caps;

    // Array with the allocated sources
    //CSourceDSound** _Sources;
	std::set<CSourceDSound*> _Sources;

	// The number of allocated sources
    uint _SourceCount;

	// The Windows ID of the multimedia timer used in the update.
    UINT _TimerID;

	// The timer resolution.
    uint32 _TimerResolution;

	/// The EAX support is requested and accepted (ie, there is enougth hardware 3D buffer)
	bool	_UseEAX;
	/// The string mapper provided by client code
	IStringMapperProvider	*_StringMapper;
	/// Driver options
	TSoundOptions _Options;

#if NLSOUND_PROFILE
protected:


    uint _TimerInterval[1024];
    uint _TimerIntervalCount;
    NLMISC::TTicks _TimerDate;
    double _TotalTime;
    double _TotalUpdateTime;
	uint32 _UpdateCount;
	uint32 _UpdateSources;
	uint32 _UpdateExec;


public:

    void printDriverInfo(FILE* fp);
    uint countTimerIntervals();
    uint getTimerIntervals(uint index);
    void addTimerInterval(uint32 dt);
    double getCPULoad();
	double getTotalTime()            { return _TotalTime; };
	double getAverageUpdateTime()    { return (_UpdateCount) ?  _TotalUpdateTime / _UpdateCount : 0.0; }
	uint32 getAverageUpdateSources() { return (_UpdateExec) ?  _UpdateSources / _UpdateExec : 0; }
	double getUpdatePercentage()     { return (_UpdateCount) ? (double) _UpdateExec / (double) _UpdateCount: 0; }
#endif



};

} // NLSOUND

#endif // NL_SOUND_DRIVER_DSOUND_H
