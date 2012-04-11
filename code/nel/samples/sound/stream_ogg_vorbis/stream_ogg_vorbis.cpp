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

#include <nel/misc/types_nl.h>

// STL includes
#include <stdio.h>
#include <conio.h>

// 3rd Party includes
#ifdef NL_OS_WINDOWS
#	pragma warning( push )
#	pragma warning( disable : 4244 )
#endif
#include <vorbis/vorbisfile.h>
#ifdef NL_OS_WINDOWS
#	pragma warning( pop )
#endif

// NeL includes
#include <nel/misc/app_context.h>
#include <nel/misc/debug.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/path.h>
#include <nel/misc/vector.h>
#include <nel/misc/i18n.h>
#include <nel/sound/u_audio_mixer.h>
#include <nel/sound/u_listener.h>
#include <nel/sound/u_stream_source.h>
#include <nel/sound/u_group_controller.h>
#include <nel/misc/hierarchical_timer.h>

// Project includes

#ifndef NL_SOUND_DATA
#define NL_SOUND_DATA "."
#endif // NL_SOUND_DATA

#define SAMPLE_OGG "D:/source/kaetemi/toverhex/src/samples/music_stream/data/aeon_1_10_mystic_river.ogg"

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

namespace NLSAMPLE {

/**
 * \brief IAudioDecoder
 * \date 2008-08-30 11:38GMT
 * \author Jan Boon (Kaetemi)
 * IAudioDecoder is only used by the driver implementation to stream 
 * music files into a readable format (it's a simple decoder interface).
 * You should not call these functions (getSongTitle) on nlsound or user level, 
 * as a driver might have additional music types implemented.
 * TODO: Split IAudioDecoder into IAudioDecoder (actual decoding) and IMediaDemuxer (stream splitter), and change the interface to make more sense.
 * TODO: Allow user application to register more decoders.
 * TODO: Look into libavcodec for decoding audio?
 */
class IAudioDecoder
{
private:
	// pointers
	/// Stream from file created by IAudioDecoder
	NLMISC::IStream *_InternalStream;

public:
	IAudioDecoder();
	virtual ~IAudioDecoder();

	/// Create a new music buffer, may return NULL if unknown type, destroy with delete. Filepath lookup done here. If async is true, it will stream from hd, else it will load in memory first.
	static IAudioDecoder *createAudioDecoder(const std::string &filepath, bool async, bool loop);

	/// Create a new music buffer from a stream, type is file extension like "ogg" etc.
	static IAudioDecoder *createAudioDecoder(const std::string &type, NLMISC::IStream *stream, bool loop);

	/// Get information on a music file (only artist and title at the moment).
	static bool getInfo(const std::string &filepath, std::string &artist, std::string &title);
	
	/// Get audio/container extensions that are currently supported by the nel sound library.
	static void getMusicExtensions(std::vector<std::string> &extensions);

	/// Return if a music extension is supported by the nel sound library.
	static bool isMusicExtensionSupported(const std::string &extension);

	/// Get how many bytes the music buffer requires for output minimum.
	virtual uint32 getRequiredBytes() = 0;

	/// Get an amount of bytes between minimum and maximum (can be lower than minimum if at end).
	virtual uint32 getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum) = 0;

	/// Get the amount of channels (2 is stereo) in output.
	virtual uint8 getChannels() = 0;

	/// Get the samples per second (often 44100) in output.
	virtual uint getSamplesPerSec() = 0;

	/// Get the bits per sample (often 16) in output.
	virtual uint8 getBitsPerSample() = 0;

	/// Get if the music has ended playing (never true if loop).
	virtual bool isMusicEnded() = 0;

	/// Get the total time in seconds.
	virtual float getLength() = 0;
}; /* class IAudioDecoder */

/**
 * \brief CAudioDecoderVorbis
 * \date 2008-08-30 11:38GMT
 * \author Jan Boon (Kaetemi)
 * CAudioDecoderVorbis
 * Create trough IAudioDecoder, type "ogg"
 */
class CAudioDecoderVorbis : public IAudioDecoder
{
protected:
	// outside pointers
	NLMISC::IStream *_Stream;

	// pointers
	
	// instances
	OggVorbis_File _OggVorbisFile;
	bool _Loop;
	bool _IsMusicEnded;
	sint32 _StreamOffset;
	sint32 _StreamSize;
public:
	CAudioDecoderVorbis(NLMISC::IStream *stream, bool loop);
	virtual ~CAudioDecoderVorbis();
	inline NLMISC::IStream *getStream() { return _Stream; }
	inline sint32 getStreamSize() { return _StreamSize; }
	inline sint32 getStreamOffset() { return _StreamOffset; }

	/// Get information on a music file (only artist and title at the moment).
	static bool getInfo(NLMISC::IStream *stream, std::string &artist, std::string &title);

	/// Get how many bytes the music buffer requires for output minimum.
	virtual uint32 getRequiredBytes();

	/// Get an amount of bytes between minimum and maximum (can be lower than minimum if at end).
	virtual uint32 getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum);

	/// Get the amount of channels (2 is stereo) in output.
	virtual uint8 getChannels();

	/// Get the samples per second (often 44100) in output.
	virtual uint getSamplesPerSec();

	/// Get the bits per sample (often 16) in output.
	virtual uint8 getBitsPerSample();

	/// Get if the music has ended playing (never true if loop).
	virtual bool isMusicEnded();

	/// Get the total time in seconds.
	virtual float getLength();
}; /* class CAudioDecoderVorbis */

IAudioDecoder::IAudioDecoder() : _InternalStream(NULL)
{
	
}

IAudioDecoder::~IAudioDecoder()
{
	if (_InternalStream) { delete _InternalStream; _InternalStream = NULL; }
}

IAudioDecoder *IAudioDecoder::createAudioDecoder(const std::string &filepath, bool async, bool loop)
{
	string lookup = CPath::lookup(filepath, false);
	if (lookup.empty())
	{ 
		nlwarning("Music file %s does not exist!", filepath.c_str());
		return NULL; 
	}
	string type = CFile::getExtension(filepath);

	CIFile *ifile = new CIFile();
	ifile->setCacheFileOnOpen(!async);
	ifile->allowBNPCacheFileOnOpen(!async);
	ifile->open(lookup);

	IAudioDecoder *mb = createAudioDecoder(type, ifile, loop);

	if (mb) mb->_InternalStream = ifile;
	else delete ifile;

	return mb;
}

IAudioDecoder *IAudioDecoder::createAudioDecoder(const std::string &type, NLMISC::IStream *stream, bool loop)
{
	if (!stream)
	{
		nlwarning("Stream is NULL");
		return NULL;
	}
	string type_lower = toLower(type);
	if (type_lower == "ogg")
	{
		return new CAudioDecoderVorbis(stream, loop);
	}
	else
	{
		nlwarning("Music file type unknown: '%s'", type_lower.c_str());
		return NULL;
	}
}

bool IAudioDecoder::getInfo(const std::string &filepath, std::string &artist, std::string &title)
{
	string lookup = CPath::lookup(filepath, false);
	if (lookup.empty())
	{ 
		nlwarning("Music file %s does not exist!", filepath.c_str());
		return false; 
	}
	string type = CFile::getExtension(filepath);
	string type_lower = toLower(type);

	if (type_lower == "ogg")
	{
		CIFile ifile; 
		ifile.setCacheFileOnOpen(false); 
		ifile.allowBNPCacheFileOnOpen(false);
		ifile.open(lookup);
		return CAudioDecoderVorbis::getInfo(&ifile, artist, title);
	}
	else
	{
		nlwarning("Music file type unknown: '%s'", type_lower.c_str());
		artist.clear(); title.clear();
		return false;
	}
}

/// Get audio/container extensions that are currently supported by the nel sound library.
void IAudioDecoder::getMusicExtensions(std::vector<std::string> &extensions)
{
	extensions.push_back("ogg");
	// extensions.push_back("wav"); // TODO: Easy.
}

/// Return if a music extension is supported by the nel sound library.
bool IAudioDecoder::isMusicExtensionSupported(const std::string &extension)
{
	return (extension == "ogg");
}

size_t vorbisReadFunc(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	CAudioDecoderVorbis *audio_decoder_vorbis = (CAudioDecoderVorbis *)datasource;
	NLMISC::IStream *stream = audio_decoder_vorbis->getStream();
	nlassert(stream->isReading());
	sint32 length = (sint32)(size * nmemb);
	if (length > audio_decoder_vorbis->getStreamSize() - stream->getPos())
		length = audio_decoder_vorbis->getStreamSize() - stream->getPos();
	stream->serialBuffer((uint8 *)ptr, length);
	return length;
}

int vorbisSeekFunc(void *datasource, ogg_int64_t offset, int whence)
{
	if (whence == SEEK_CUR && offset == 0)
	{
		// nlwarning(NLSOUND_XAUDIO2_PREFIX "This seek call doesn't do a damn thing, wtf.");
		return 0; // ooookkaaaaaayyy
	}

	CAudioDecoderVorbis *audio_decoder_vorbis = (CAudioDecoderVorbis *)datasource;

	NLMISC::IStream::TSeekOrigin origin;
	switch (whence)
	{
	case SEEK_SET:
		origin = NLMISC::IStream::begin;
		break;
	case SEEK_CUR:
		origin = NLMISC::IStream::current;
		break;
	case SEEK_END:
		origin = NLMISC::IStream::end;
		break;
	default:
		// nlwarning(NLSOUND_XAUDIO2_PREFIX "Seeking to fake origin.");
		return -1;
	}

	if (audio_decoder_vorbis->getStream()->seek(SEEK_SET ? audio_decoder_vorbis->getStreamOffset() + (sint32)offset : (sint32)offset, origin)) return 0;
	else return -1;
}

//int vorbisCloseFunc(void *datasource)
//{
//	//CAudioDecoderVorbis *audio_decoder_vorbis = (CAudioDecoderVorbis *)datasource;
//}

long vorbisTellFunc(void *datasource)
{
	CAudioDecoderVorbis *audio_decoder_vorbis = (CAudioDecoderVorbis *)datasource;
	return (long)(audio_decoder_vorbis->getStream()->getPos() - audio_decoder_vorbis->getStreamOffset());
}

static ov_callbacks OV_CALLBACKS_NLMISC_STREAM = {
  (size_t (*)(void *, size_t, size_t, void *))  vorbisReadFunc,
  (int (*)(void *, ogg_int64_t, int))		  vorbisSeekFunc,
  (int (*)(void *))							 NULL, //vorbisCloseFunc,
  (long (*)(void *))							vorbisTellFunc
};

static UAudioMixer *s_AudioMixer = NULL;
static UStreamSource *s_StreamSource = NULL;
static IAudioDecoder *s_AudioDecoder = NULL;
static UGroupController *s_GroupController = NULL;

static void initSample()
{
	if (!INelContext::isContextInitialised())
		new CApplicationContext();
	CPath::addSearchPath(NL_SOUND_DATA"/database/build/", true, false);
	
	printf("Sample demonstrating OGG playback using UStreamSource.");
	printf("\n\n");
	
	s_AudioMixer = UAudioMixer::createAudioMixer();
	
	printf("Select NLSOUND Driver:\n");
	printf(" [1] FMod\n");
	printf(" [2] OpenAl\n");
	printf(" [3] DSound\n");
	printf(" [4] XAudio2\n");
	printf("> ");
	int selection = getchar(); getchar();
	printf("\n");
	
	// init with 128 tracks, EAX enabled, no ADPCM, and automatic sample bank loading
	s_AudioMixer->init(8, true, false, NULL, true, (UAudioMixer::TDriver)(selection - '0'));
	s_AudioMixer->setLowWaterMark(1);

	CVector initpos(0.0f, 0.0f, 0.0f);
	CVector frontvec(0.0f, 1.0f, 0.0f);
	CVector upvec(0.0f, 0.0f, 1.0f);
	s_AudioMixer->getListener()->setPos(initpos);
	s_AudioMixer->getListener()->setOrientation(frontvec, upvec);
	
	//NLMISC::CHTimer::startBench();

	USource *source = s_AudioMixer->createSource(CStringMapper::map("default_stream"));
	nlassert(source);
	s_StreamSource = dynamic_cast<UStreamSource *>(source);
	nlassert(s_StreamSource);
	source->setSourceRelativeMode(true);

	string sample = SAMPLE_OGG;
	s_AudioDecoder = IAudioDecoder::createAudioDecoder(sample, false, false);
	s_StreamSource->setFormat(s_AudioDecoder->getChannels(), s_AudioDecoder->getBitsPerSample(), (uint32)s_AudioDecoder->getSamplesPerSec());
	s_StreamSource->setPitch(2.0f);

	s_GroupController = s_AudioMixer->getGroupController("dialog");
}

//CMutex *s_Mutex = NULL;
//
//class CTestThread : public IRunnable
//{
//	virtual void run()
//	{
//		for (int i = 0; i < 100000000; ++i)
//		{
//			s_Mutex->enter();
//			s_Mutex->leave();
//		}
//	}
//};

static void bufferMore(uint bytes) // buffer from bytes to bytes * 2
{
	uint8 *buffer = s_StreamSource->lock(bytes * 2);
	if (buffer)
	{
		uint32 result = s_AudioDecoder->getNextBytes(buffer, bytes, bytes * 2);
		s_StreamSource->unlock(result);
		// printf("|");
	}
}

static void runSample()
{
	uint samples, bytes;
	s_StreamSource->getRecommendedBufferSize(samples, bytes);

	bufferMore(bytes);
	//bufferMore(bytes);

	s_StreamSource->play();
	
	printf("Change volume with - and +\n");
	printf("Press ANY other key to exit\n");
	while (!s_AudioDecoder->isMusicEnded())
	{
		if (_kbhit())
		{
			switch (_getch())
			{
			case '+':
				s_GroupController->setUserGain(s_GroupController->getUserGain() + 0.1f);
				break;
			case '-':
				s_GroupController->setUserGain(s_GroupController->getUserGain() - 0.1f);
				break;
			default:
				return;
			}
		}
		bufferMore(bytes);
		s_AudioMixer->update();
		//if (!s_StreamSource->asUSource()->isst)
		//{
		//	printf("*!playing!*");
		//	s_StreamSource->asUSource()->play();
		//}
		nlSleep(s_StreamSource->getRecommendedSleepTime());
	}
	printf("Last buffer streamed\n");
	while (s_StreamSource->hasFilledBuffersAvailable())
	{
		nlSleep(40);
		s_AudioMixer->update();
	}
	
	s_StreamSource->stop();
	
	printf("End of song\n");
	printf("Press ANY key to exit\n");
	while (!_kbhit()) { s_AudioMixer->update(); nlSleep(10); } _getch();
	return;
}

static void releaseSample()
{
	//NLMISC::CHTimer::clear();
	s_GroupController = NULL;
	delete s_AudioDecoder; s_AudioDecoder = NULL;
	delete s_StreamSource; s_StreamSource = NULL;
	delete s_AudioMixer; s_AudioMixer = NULL;
}

CAudioDecoderVorbis::CAudioDecoderVorbis(NLMISC::IStream *stream, bool loop) 
: _Stream(stream), _Loop(loop), _StreamSize(0), _IsMusicEnded(false)
{
	_StreamOffset = stream->getPos();
	stream->seek(0, NLMISC::IStream::end);
	_StreamSize = stream->getPos();
	stream->seek(_StreamOffset, NLMISC::IStream::begin);
	ov_open_callbacks(this, &_OggVorbisFile, NULL, 0, OV_CALLBACKS_NLMISC_STREAM);
}

CAudioDecoderVorbis::~CAudioDecoderVorbis()
{
	ov_clear(&_OggVorbisFile);
}

/// Get information on a music file (only artist and title at the moment).
bool CAudioDecoderVorbis::getInfo(NLMISC::IStream *stream, std::string &artist, std::string &title)
{
	CAudioDecoderVorbis mbv(stream, false); // just opens and closes the oggvorbisfile thing :)
	vorbis_comment *vc = ov_comment(&mbv._OggVorbisFile, -1);
	char *title_c = vorbis_comment_query(vc, "title", 0);
	if (title_c) title = title_c; else title.clear();
	char *artist_c = vorbis_comment_query(vc, "artist", 0);
	if (artist_c) artist = artist_c; else artist.clear();
	return true;
}

uint32 CAudioDecoderVorbis::getRequiredBytes()
{
	return 0; // no minimum requirement of bytes to buffer out
}

uint32 CAudioDecoderVorbis::getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum)
{
	sint current_section = 0; // ???
	if (_IsMusicEnded) return 0;
	nlassert(minimum <= maximum); // can't have this..
	uint32 bytes_read = 0;
#ifdef NL_BIG_ENDIAN
	sint endianness = 1;
#else
	sint endianness = 0;
#endif
	do
	{
		// signed 16-bit or unsigned 8-bit little-endian samples
		sint br = ov_read(&_OggVorbisFile, (char *)&buffer[bytes_read], maximum - bytes_read, 
			endianness, // Specifies big or little endian byte packing. 0 for little endian, 1 for b ig endian. Typical value is 0.
			getBitsPerSample() == 8 ? 1 : 2, 
			getBitsPerSample() == 8 ? 0 : 1, // Signed or unsigned data. 0 for unsigned, 1 for signed. Typically 1.
			&current_section);
		// nlinfo(NLSOUND_XAUDIO2_PREFIX "current_section: %i", current_section);
		if (br > 0)
		{
			bytes_read += (uint32)br;
		}
		else if (br == 0) // EOF
		{
			if (_Loop)
			{
				ov_pcm_seek(&_OggVorbisFile, 0);
				//_Stream->seek(0, NLMISC::IStream::begin);
			}
			else 
			{
				_IsMusicEnded = true;
				break; 
			}
		}
		else
		{ 
			// error
			switch(br)
			{
			case OV_HOLE:
				nlwarning("ov_read returned OV_HOLE");
				break;
			case OV_EINVAL:
				nlwarning("ov_read returned OV_EINVAL");
				break;
			case OV_EBADLINK:
				nlwarning("ov_read returned OV_EBADLINK");
				break;
			default:
				nlwarning("ov_read returned %d", br);
			}
		}
	} while (bytes_read < minimum);
	return bytes_read;
}

uint8 CAudioDecoderVorbis::getChannels()
{
	vorbis_info *vi = ov_info(&_OggVorbisFile, -1);
	return (uint8)vi->channels;
}

uint CAudioDecoderVorbis::getSamplesPerSec()
{
	vorbis_info *vi = ov_info(&_OggVorbisFile, -1);
	return (uint)vi->rate;
}

uint8 CAudioDecoderVorbis::getBitsPerSample()
{
	return 16;
}

bool CAudioDecoderVorbis::isMusicEnded()
{
	return _IsMusicEnded;
}

float CAudioDecoderVorbis::getLength()
{
	return (float)ov_time_total(&_OggVorbisFile, -1);
}


} /* namespace NLSAMPLE */

int main()
{
	NLSAMPLE::initSample();
	NLSAMPLE::runSample();
	NLSAMPLE::releaseSample();
	return 0;
}

/* end of file */
