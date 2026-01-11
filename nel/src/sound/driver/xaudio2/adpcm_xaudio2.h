// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2008-2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NLSOUND_ADPCM_XAUDIO2_H
#define NLSOUND_ADPCM_XAUDIO2_H

#include "nel/sound/driver/buffer.h"

namespace NLSOUND {
	class CBufferXAudio2;

/**
 * \brief CAdpcmXAudio2
 * \date 2008-09-07 03:53GMT
 * \author Jan Boon (Kaetemi)
 * CAdpcmXAudio2 is a utility class for realtime streaming and decoding of Intel ADPCM data to an XAudio2 source voice.
 */
class CAdpcmXAudio2 : public IXAudio2VoiceCallback
{
protected:
	// stuff
	/// Source voice to which the data is sent.
	IXAudio2SourceVoice *_SourceVoice;
	/// ADPCM data.
	const uint8 *_SourceData;
	/// ADPCM data size.
	uint32 _SourceSize;
	/// Sample data rate.
	uint _SampleRate;
	/// Size in uint8 50ms ADPCM. (50ms 4bit ADPCM in uint8 = _SampleRate / 40)
	uint _AdpcmSize;
	/// Samples for 50ms. Also size in uint16 for 100ms PCM. (50ms 16bit PCM in uint16 = _SampleRate / 20)
	uint _SampleNb;
	/// Current position in ADPCM buffer in uint8.
	uint _AdpcmIndex;
	
	// other stuff
	IBuffer::TADPCMState _State;
	/// Looping or not.
	bool _Loop;
	/// Number of buffers.
	static const uint _BufferNb = 3;
	/// Data in buffer (in uint16).
	static const uint _BufferMax = 2400;
	/// Three buffers of up to 50ms of 48kHz 16bit PCM data each.
	sint16 _Buffer[_BufferNb][_BufferMax];
	/// Buffer that will be written to next.
	uint _BufferNext;
	/// Mutex for cross-thread access from XAudio2 callbacks.
	NLMISC::CMutex _Mutex;
	/// Unique id for buffer.
	uintptr_t _LastBufferContext;
	/// Current buffer.
	void *_ValidBufferContext[_BufferNb];
public:
	CAdpcmXAudio2(bool loop);
	virtual ~CAdpcmXAudio2();

	/// Create the source voice, it must have this class as callback stuff.
	inline void setSourceVoice(IXAudio2SourceVoice *sourceVoice) { _SourceVoice = sourceVoice; }
	
	/// Set looping state.
	inline void setLooping(bool l) { _Loop = l; }
	/// Submit the next ADPCM buffer, only 1 buffer can be submitted at a time! Plays as soon as submitted (if sourcevoice is playing).
	void submitSourceBuffer(CBufferXAudio2 *buffer);
	/// Reset the decoder, clear the queued buffer
	void flushSourceBuffers();
	/// Returns NULL if the buffer has ended playing, never NULL for loops!
	inline const uint8 *getSourceData() const { return _SourceData; }
	
private:
	/// (Internal) Process and submit the ADPCM data.
	void processBuffers();
	
	// XAudio2 Callbacks
	// Called just before this voice's processing pass begins.
	STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired);
	// Called just after this voice's processing pass ends.
	STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS);
	// Called when this voice has just finished playing a buffer stream
	// (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
	STDMETHOD_(void, OnStreamEnd) (THIS);
	// Called when this voice is about to start processing a new buffer.
	STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext);
	// Called when this voice has just finished processing a buffer.
	// The buffer can now be reused or destroyed.
	STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext);
	// Called when this voice has just reached the end position of a loop.
	STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext);
	// Called in the event of a critical error during voice processing,
	// such as a failing XAPO or an error from the hardware XMA decoder.
	// The voice may have to be destroyed and re-created to recover from
	// the error.  The callback arguments report which buffer was being
	// processed when the error occurred, and its HRESULT code.
	STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error);
}; /* class CAdpcmXAudio2 */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_ADPCM_XAUDIO2_H */

/* end of file */
