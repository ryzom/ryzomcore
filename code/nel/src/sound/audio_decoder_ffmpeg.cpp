// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2018  Winch Gate Property Limited
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


#include "stdsound.h"

#include <nel/sound/audio_decoder_ffmpeg.h>

#define __STDC_CONSTANT_MACROS
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

// Visual Studio does not support AV_TIME_BASE_Q macro in C++
#undef AV_TIME_BASE_Q
static const AVRational AV_TIME_BASE_Q = {1, AV_TIME_BASE};

namespace {

const std::string av_err2string(sint err)
{
	char buf[AV_ERROR_MAX_STRING_SIZE];
	av_strerror(err, buf, AV_ERROR_MAX_STRING_SIZE);
	return (std::string)buf;
}

void nel_logger(void *ptr, int level, const char *fmt, va_list vargs)
{
	static char msg[1024];

	const char *module = NULL;

	// AV_LOG_DEBUG, AV_LOG_TRACE
	if (level >= AV_LOG_DEBUG) return;

	if (ptr)
	{
		AVClass *avc = *(AVClass**) ptr;
		module = avc->item_name(ptr);
	}

	vsnprintf(msg, sizeof(msg), fmt, vargs);
	msg[sizeof(msg)-1] = '\0';

	switch(level)
	{
	case AV_LOG_PANIC:
		// ffmpeg is about to crash so lets throw
		nlerror("FFMPEG(P): (%s) %s", module, msg);
		break;
	case AV_LOG_FATAL:
		// ffmpeg had unrecoverable error, corrupted stream or such
		nlerrornoex("FFMPEG(F): (%s) %s", module, msg);
		break;
	case AV_LOG_ERROR:
		nlwarning("FFMPEG(E): (%s) %s", module, msg);
		break;
	case AV_LOG_WARNING:
		nlwarning("FFMPEG(W): (%s) %s", module, msg);
		break;
	case AV_LOG_INFO:
		nlinfo("FFMPEG(I): (%s) %s", module, msg);
		break;
	case AV_LOG_VERBOSE:
		nldebug("FFMPEG(V): (%s) %s", module, msg);
		break;
	case AV_LOG_DEBUG:
		nldebug("FFMPEG(D): (%s) %s", module, msg);
		break;
	default:
		nlinfo("FFMPEG: invalid log level:%d (%s) %s", level, module, msg);
		break;
	}
}

class CFfmpegInstance
{
public:
	CFfmpegInstance()
	{
		av_log_set_level(AV_LOG_DEBUG);
		av_log_set_callback(nel_logger);

		av_register_all();

		//avformat_network_init();
	}

	virtual ~CFfmpegInstance()
	{
		//avformat_network_deinit();
	}
};

CFfmpegInstance ffmpeg;

// Send bytes to ffmpeg
int avio_read_packet(void *opaque, uint8 *buf, int buf_size)
{
	NLSOUND::CAudioDecoderFfmpeg *decoder = static_cast<NLSOUND::CAudioDecoderFfmpeg *>(opaque);
	NLMISC::IStream *stream = decoder->getStream();
	nlassert(stream->isReading());

	uint32 available = decoder->getStreamSize() - stream->getPos();
	if (available == 0) return 0;

	buf_size = FFMIN(buf_size, available);
	stream->serialBuffer((uint8 *)buf, buf_size);
	return buf_size;
}

sint64 avio_seek(void *opaque, sint64 offset, int whence)
{
	NLSOUND::CAudioDecoderFfmpeg *decoder = static_cast<NLSOUND::CAudioDecoderFfmpeg *>(opaque);
	NLMISC::IStream *stream = decoder->getStream();
	nlassert(stream->isReading());

	NLMISC::IStream::TSeekOrigin origin;
	switch(whence)
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
		case AVSEEK_SIZE:
			return decoder->getStreamSize();
		default:
			return -1;
	}

	stream->seek((sint32) offset, origin);
	return stream->getPos();
}

}//ns

namespace NLSOUND {

// swresample will convert audio to this format
#define FFMPEG_SAMPLE_RATE 44100
#define FFMPEG_CHANNELS 2
#define FFMPEG_CHANNEL_LAYOUT AV_CH_LAYOUT_STEREO
#define FFMPEG_BITS_PER_SAMPLE 16
#define FFMPEG_SAMPLE_FORMAT  AV_SAMPLE_FMT_S16

CAudioDecoderFfmpeg::CAudioDecoderFfmpeg(NLMISC::IStream *stream, bool loop)
: IAudioDecoder(),
	_Stream(stream), _Loop(loop), _IsMusicEnded(false), _StreamSize(0), _IsSupported(false),
	_AvioContext(NULL), _FormatContext(NULL),
	_AudioContext(NULL), _AudioStreamIndex(0),
	_SwrContext(NULL)
{
	_StreamOffset = stream->getPos();
	stream->seek(0, NLMISC::IStream::end);
	_StreamSize = stream->getPos();
	stream->seek(_StreamOffset, NLMISC::IStream::begin);

	try {
		_FormatContext = avformat_alloc_context();
		if (!_FormatContext)
			throw Exception("Can't create AVFormatContext");

		// avio_ctx_buffer can be reallocated by ffmpeg and assigned to avio_ctx->buffer
		uint8 *avio_ctx_buffer = NULL;
		size_t avio_ctx_buffer_size = 4096;
		avio_ctx_buffer = static_cast<uint8 *>(av_malloc(avio_ctx_buffer_size));
		if (!avio_ctx_buffer)
			throw Exception("Can't allocate avio context buffer");

		_AvioContext = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, this, &avio_read_packet, NULL, &avio_seek);
		if (!_AvioContext)
			throw Exception("Can't allocate avio context");

		_FormatContext->pb = _AvioContext;
		sint ret = avformat_open_input(&_FormatContext, NULL, NULL, NULL);
		if (ret < 0)
			throw Exception("avformat_open_input: %d", ret);

		// find stream and then audio codec to see if ffmpeg supports this
		_IsSupported = false;
		if (avformat_find_stream_info(_FormatContext, NULL) >= 0)
		{
			_AudioStreamIndex = av_find_best_stream(_FormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
			if (_AudioStreamIndex >= 0)
			{
				_AudioContext = _FormatContext->streams[_AudioStreamIndex]->codec;
				av_opt_set_int(_AudioContext, "refcounted_frames", 1, 0);

				AVCodec *codec = avcodec_find_decoder(_AudioContext->codec_id);
				if (codec != NULL && avcodec_open2(_AudioContext, codec, NULL) >= 0)
				{
					_IsSupported = true;
				}
			}
		}
	}
	catch(...)
	{
		release();

		throw;
	}

	if (!_IsSupported)
	{
		nlwarning("FFMPEG: Decoder created, unknown stream format / codec");
	}
}

CAudioDecoderFfmpeg::~CAudioDecoderFfmpeg()
{
	release();
}

void CAudioDecoderFfmpeg::release()
{
	if (_SwrContext)
		swr_free(&_SwrContext);

	if (_AudioContext)
		avcodec_close(_AudioContext);

	if (_FormatContext)
		avformat_close_input(&_FormatContext);

	if (_AvioContext && _AvioContext->buffer)
		av_freep(&_AvioContext->buffer);

	if (_AvioContext)
		av_freep(&_AvioContext);
}

bool CAudioDecoderFfmpeg::isFormatSupported() const
{
	return _IsSupported;
}

/// Get information on a music file.
bool CAudioDecoderFfmpeg::getInfo(NLMISC::IStream *stream, std::string &artist, std::string &title, float &length)
{
	CAudioDecoderFfmpeg ffmpeg(stream, false);
	if (!ffmpeg.isFormatSupported())
	{
		title.clear();
		artist.clear();
		length = 0.f;

		return false;
	}

	AVDictionaryEntry *tag = NULL;
	while((tag = av_dict_get(ffmpeg._FormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
	{
		if (!strcmp(tag->key, "artist"))
		{
			artist = tag->value;
		}
		else if (!strcmp(tag->key, "title"))
		{
			title = tag->value;
		}
	}

	length = ffmpeg.getLength();

	return true;
}

uint32 CAudioDecoderFfmpeg::getRequiredBytes()
{
	return 0; // no minimum requirement of bytes to buffer out
}

uint32 CAudioDecoderFfmpeg::getNextBytes(uint8 *buffer, uint32 minimum, uint32 maximum)
{
	if (_IsMusicEnded) return 0;
	nlassert(minimum <= maximum); // can't have this..

	// TODO: CStreamFileSource::play() will stall when there is no frames on warmup
	// supported can be set false if there is an issue creating converter
	if (!_IsSupported)
	{
		_IsMusicEnded = true;
		return 1;
	}

	uint32 bytes_read = 0;

	AVFrame frame = {0};
	AVPacket packet = {0};

	if (!_SwrContext)
	{
		sint64 in_channel_layout = av_get_default_channel_layout(_AudioContext->channels);
		_SwrContext = swr_alloc_set_opts(NULL,
				// output
				FFMPEG_CHANNEL_LAYOUT, FFMPEG_SAMPLE_FORMAT, FFMPEG_SAMPLE_RATE,
				// input
				in_channel_layout, _AudioContext->sample_fmt, _AudioContext->sample_rate,
				0, NULL);
		swr_init(_SwrContext);
	}

	sint ret;
	while(bytes_read < minimum)
	{
		// read packet from stream
		if ((ret = av_read_frame(_FormatContext, &packet)) < 0)
		{
			_IsMusicEnded = true;
			// TODO: looping
			break;
		}

		if (packet.stream_index == _AudioStreamIndex)
		{
			// packet can contain multiple frames
			AVPacket first = packet;
			int got_frame = 0;
			do {
				got_frame = 0;
				ret = avcodec_decode_audio4(_AudioContext, &frame, &got_frame, &packet);
				if (ret < 0)
				{
					nlwarning("FFMPEG: error decoding audio frame: %s", av_err2string(ret).c_str());
					break;
				}
				packet.size -= ret;
				packet.data += ret;

				if (got_frame)
				{
					uint32 out_bps = av_get_bytes_per_sample(FFMPEG_SAMPLE_FORMAT) * FFMPEG_CHANNELS;
					uint32 max_samples = (maximum - bytes_read) / out_bps;

					uint32 out_samples = av_rescale_rnd(swr_get_delay(_SwrContext, _AudioContext->sample_rate) + frame.nb_samples,
							FFMPEG_SAMPLE_RATE, _AudioContext->sample_rate, AV_ROUND_UP);

					if (max_samples > out_samples)
						max_samples = out_samples;

					uint32 converted = swr_convert(_SwrContext, &buffer, max_samples, (const uint8 **)frame.extended_data, frame.nb_samples);
					uint32 size = out_bps * converted;

					bytes_read += size;
					buffer += size;

					av_frame_unref(&frame);
				}
			} while (got_frame && packet.size > 0);

			av_packet_unref(&first);
		}
		else
		{
			ret = 0;
			av_packet_unref(&packet);
		}
	}

	return bytes_read;
}

uint8 CAudioDecoderFfmpeg::getChannels()
{
	return FFMPEG_CHANNELS;
}

uint CAudioDecoderFfmpeg::getSamplesPerSec()
{
	return FFMPEG_SAMPLE_RATE;
}

uint8 CAudioDecoderFfmpeg::getBitsPerSample()
{
	return FFMPEG_BITS_PER_SAMPLE;
}

bool CAudioDecoderFfmpeg::isMusicEnded()
{
	return _IsMusicEnded;
}

float CAudioDecoderFfmpeg::getLength()
{
	float length = 0.f;
	if (_FormatContext->duration != AV_NOPTS_VALUE)
	{
		length = _FormatContext->duration * av_q2d(AV_TIME_BASE_Q);
	}
	else if (_FormatContext->streams[_AudioStreamIndex]->duration != AV_NOPTS_VALUE)
	{
		length = _FormatContext->streams[_AudioStreamIndex]->duration * av_q2d(_FormatContext->streams[_AudioStreamIndex]->time_base);
	}
	return length;
}

void CAudioDecoderFfmpeg::setLooping(bool loop)
{
	 _Loop = loop;
}

} /* namespace NLSOUND */

/* end of file */
