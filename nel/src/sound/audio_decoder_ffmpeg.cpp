// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2018-2019  Winch Gate Property Limited
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
#include <libavutil/version.h>
}

// av_register_all() is deprecated since FFmpeg 4.0 (avformat 58.9.100) and removed in 5.0
#define NL_FFMPEG_HAS_REGISTER_ALL (LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100))
// AVChannelLayout and swr_alloc_set_opts2() exist since FFmpeg 5.1 (avutil 57.24.100);
// AVCodecContext::channels and swr_alloc_set_opts() are removed in FFmpeg 7.0
#define NL_FFMPEG_HAS_CHANNEL_LAYOUT (LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 24, 100))

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

#if NL_FFMPEG_HAS_REGISTER_ALL
		av_register_all();
#endif

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
	if (available == 0) return AVERROR_EOF;

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
#define FFMPEG_BITS_PER_SAMPLE 16
#define FFMPEG_SAMPLE_FORMAT  AV_SAMPLE_FMT_S16

CAudioDecoderFfmpeg::CAudioDecoderFfmpeg(NLMISC::IStream *stream, bool loop)
: IAudioDecoder(),
	_Stream(stream), _Loop(loop), _IsMusicEnded(false), _StreamSize(0), _IsSupported(false),
	_AvioContext(NULL), _FormatContext(NULL),
	_AudioContext(NULL), _AudioStreamIndex(0),
	_SwrContext(NULL), _Frame(NULL), _Packet(NULL)
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
		{
			av_free(avio_ctx_buffer);
			throw Exception("Can't allocate avio context");
		}

		_FormatContext->pb = _AvioContext;
		sint ret = avformat_open_input(&_FormatContext, NULL, NULL, NULL);
		if (ret < 0)
			throw Exception("avformat_open_input: %d", ret);

		_Frame = av_frame_alloc();
		_Packet = av_packet_alloc();
		if (!_Frame || !_Packet)
			throw Exception("Can't allocate frame and packet");

		// find stream and then audio codec to see if ffmpeg supports this
		_IsSupported = false;
		if (avformat_find_stream_info(_FormatContext, NULL) >= 0)
		{
			_AudioStreamIndex = av_find_best_stream(_FormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
			if (_AudioStreamIndex >= 0)
			{
				// the codec context is owned by the decoder since the stream's own context was removed in FFmpeg 5
				AVCodecParameters *codecpar = _FormatContext->streams[_AudioStreamIndex]->codecpar;
				const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
				if (codec != NULL)
				{
					_AudioContext = avcodec_alloc_context3(codec);
					if (_AudioContext != NULL
						&& avcodec_parameters_to_context(_AudioContext, codecpar) >= 0
						&& avcodec_open2(_AudioContext, codec, NULL) >= 0)
					{
						_IsSupported = true;
					}
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
		avcodec_free_context(&_AudioContext);

	if (_Frame)
		av_frame_free(&_Frame);

	if (_Packet)
		av_packet_free(&_Packet);

	if (_FormatContext)
		avformat_close_input(&_FormatContext);

	if (_AvioContext && _AvioContext->buffer)
		av_freep(&_AvioContext->buffer);

	if (_AvioContext)
		avio_context_free(&_AvioContext);
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

bool CAudioDecoderFfmpeg::createResampler()
{
#if NL_FFMPEG_HAS_CHANNEL_LAYOUT
	// AV_CHANNEL_LAYOUT_STEREO is a C designated initializer, not usable from C++17
	AVChannelLayout out_layout;
	av_channel_layout_default(&out_layout, FFMPEG_CHANNELS);
	AVChannelLayout in_layout;
	if (_AudioContext->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC)
		av_channel_layout_default(&in_layout, _AudioContext->ch_layout.nb_channels);
	else
		av_channel_layout_copy(&in_layout, &_AudioContext->ch_layout);
	sint ret = swr_alloc_set_opts2(&_SwrContext,
			// output
			&out_layout, FFMPEG_SAMPLE_FORMAT, FFMPEG_SAMPLE_RATE,
			// input
			&in_layout, _AudioContext->sample_fmt, _AudioContext->sample_rate,
			0, NULL);
	av_channel_layout_uninit(&in_layout);
	av_channel_layout_uninit(&out_layout);
	if (ret < 0)
	{
		nlwarning("FFMPEG: error creating resampler: %s", av_err2string(ret).c_str());
		return false;
	}
#else
	sint64 in_channel_layout = _AudioContext->channel_layout
		? _AudioContext->channel_layout
		: av_get_default_channel_layout(_AudioContext->channels);
	_SwrContext = swr_alloc_set_opts(NULL,
			// output
			AV_CH_LAYOUT_STEREO, FFMPEG_SAMPLE_FORMAT, FFMPEG_SAMPLE_RATE,
			// input
			in_channel_layout, _AudioContext->sample_fmt, _AudioContext->sample_rate,
			0, NULL);
	if (!_SwrContext)
	{
		nlwarning("FFMPEG: error creating resampler");
		return false;
	}
#endif
	sint ret2 = swr_init(_SwrContext);
	if (ret2 < 0)
	{
		nlwarning("FFMPEG: error initializing resampler: %s", av_err2string(ret2).c_str());
		swr_free(&_SwrContext);
		return false;
	}
	return true;
}

uint32 CAudioDecoderFfmpeg::convertFrame(uint8 *buffer, uint32 maximum)
{
	uint32 out_bps = av_get_bytes_per_sample(FFMPEG_SAMPLE_FORMAT) * FFMPEG_CHANNELS;
	uint32 max_samples = maximum / out_bps;

	uint32 out_samples = av_rescale_rnd(swr_get_delay(_SwrContext, _AudioContext->sample_rate) + _Frame->nb_samples,
			FFMPEG_SAMPLE_RATE, _AudioContext->sample_rate, AV_ROUND_UP);

	if (max_samples > out_samples)
		max_samples = out_samples;

	// samples that do not fit stay buffered in the resampler and come out with the next frame
	sint converted = swr_convert(_SwrContext, &buffer, max_samples, (const uint8 **)_Frame->extended_data, _Frame->nb_samples);
	if (converted < 0)
	{
		nlwarning("FFMPEG: error converting audio frame: %s", av_err2string(converted).c_str());
		return 0;
	}
	return out_bps * (uint32)converted;
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

	if (!_SwrContext && !createResampler())
	{
		_IsSupported = false;
		_IsMusicEnded = true;
		return 1;
	}

	uint32 bytes_read = 0;

	sint ret;
	while(bytes_read < minimum)
	{
		// read packet from stream; at the end of the stream the decoder is drained with an empty packet
		bool draining = false;
		if ((ret = av_read_frame(_FormatContext, _Packet)) < 0)
		{
			if (ret != AVERROR_EOF)
				nlwarning("FFMPEG: error reading packet: %s", av_err2string(ret).c_str());
			draining = true;
		}

		if (draining || _Packet->stream_index == _AudioStreamIndex)
		{
			ret = avcodec_send_packet(_AudioContext, draining ? NULL : _Packet);
			if (ret < 0 && ret != AVERROR_EOF)
			{
				nlwarning("FFMPEG: error sending packet to decoder: %s", av_err2string(ret).c_str());
			}
			else
			{
				// a packet can contain multiple frames
				while ((ret = avcodec_receive_frame(_AudioContext, _Frame)) >= 0)
				{
					uint32 size = convertFrame(buffer, maximum - bytes_read);
					bytes_read += size;
					buffer += size;

					av_frame_unref(_Frame);
				}
				if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
				{
					nlwarning("FFMPEG: error decoding audio frame: %s", av_err2string(ret).c_str());
				}
			}
		}

		if (draining)
		{
			_IsMusicEnded = true;
			// TODO: looping
			break;
		}

		av_packet_unref(_Packet);
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
