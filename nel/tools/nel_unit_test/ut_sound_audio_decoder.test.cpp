// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

#include <nel/misc/debug.h>
#include <nel/misc/mem_stream.h>

#include <nel/sound/audio_decoder.h>
#include <nel/sound/audio_decoder_vorbis.h>
#include <nel/sound/driver/buffer.h>
#include <nel/sound/driver/sound_driver.h>

#ifdef NL_UT_HAVE_VORBISENC
#include <vorbis/vorbisenc.h>
#endif

using NLMISC::CMemStream;
using NLSOUND::CAudioDecoderVorbis;
using NLSOUND::IAudioDecoder;
using NLSOUND::IBuffer;

// The audio decoders read from a stream and need no audio device. The streams are built in memory: bytes that are no
// Ogg Vorbis stream by hand, Ogg Vorbis streams with the encoder library where it is installed.

namespace {

typedef std::vector<uint8> TBytes;

void fillStream(CMemStream &stream, const TBytes &bytes)
{
	stream.fill(&bytes[0], (uint32)bytes.size());
}

// bytes that are no Ogg stream
TBytes makeNoise()
{
	TBytes bytes(4096);
	for (size_t i = 0; i < bytes.size(); ++i)
		bytes[i] = (uint8)(i * 31 + 7);
	return bytes;
}

// the checksum of an Ogg page, computed with its checksum field at 0
uint32 oggChecksum(const TBytes &page)
{
	uint32 crc = 0;
	for (size_t i = 0; i < page.size(); ++i)
	{
		crc ^= (uint32)page[i] << 24;
		for (uint bit = 0; bit < 8; ++bit)
			crc = (crc & 0x80000000) ? (crc << 1) ^ 0x04c11db7 : crc << 1;
	}
	return crc;
}

// an Ogg stream of one page that starts something else than Vorbis, as an Opus file named .ogg does
TBytes makeOggOfOtherData()
{
	const char packet[] = "OpusHead\x01\x02\0\0\x80\xbb\0\0\0\0\0";
	TBytes page(27, 0);
	memcpy(&page[0], "OggS", 4);
	page[5] = 0x02 | 0x04; // the first and the last page of the stream
	page[14] = 1; // the serial number of the stream
	page[26] = 1; // one segment, of the length that follows
	page.push_back((uint8)(sizeof(packet) - 1));
	page.insert(page.end(), packet, packet + sizeof(packet) - 1);
	const uint32 crc = oggChecksum(page);
	for (uint i = 0; i < 4; ++i)
		page[22 + i] = (uint8)(crc >> (8 * i));
	return page;
}

std::vector<TBytes> makeOtherData()
{
	std::vector<TBytes> files;
	files.push_back(makeNoise());
	files.push_back(makeOggOfOtherData());
	return files;
}

// where the pages of an Ogg stream start
std::vector<size_t> findOggPages(const TBytes &ogg)
{
	std::vector<size_t> pages;
	for (size_t i = 0; i + 27 <= ogg.size(); ++i)
	{
		if (memcmp(&ogg[i], "OggS", 4) == 0)
			pages.push_back(i);
	}
	return pages;
}

// An Ogg page of the stream of the serial number given that brings no packet: it holds the start of one, which the next
// page does not continue. Its page number says that the page before it is missing, which the decoder reports as an
// interruption (OV_HOLE).
TBytes makeOggPageOfNoPacket(const uint8 *serial, uint32 pageNumber)
{
	TBytes page(27, 0);
	memcpy(&page[0], "OggS", 4);
	memset(&page[6], 0xff, 8); // no packet ends on this page
	memcpy(&page[14], serial, 4);
	for (uint i = 0; i < 4; ++i)
		page[18 + i] = (uint8)(pageNumber >> (8 * i));
	page[26] = 1; // one segment, of the 255 bytes of a packet that goes on
	page.push_back(255);
	page.resize(page.size() + 255, 0);
	const uint32 crc = oggChecksum(page);
	for (uint i = 0; i < 4; ++i)
		page[22 + i] = (uint8)(crc >> (8 * i));
	return page;
}

#ifdef NL_UT_HAVE_VORBISENC

const uint SampleRate = 44100;

void appendPage(TBytes &out, const ogg_page &page)
{
	out.insert(out.end(), page.header, page.header + page.header_len);
	out.insert(out.end(), page.body, page.body + page.body_len);
}

// An Ogg Vorbis stream of frames samples per channel, where sample(channel, frame) gives each of them
template <class TSample>
TBytes encodeVorbis(uint channels, uint frames, TSample sample)
{
	TBytes out;
	vorbis_info info;
	vorbis_info_init(&info);
	if (vorbis_encode_init_vbr(&info, (long)channels, (long)SampleRate, 1.0f) != 0)
	{
		ADD_FAILURE() << "the encoder refuses " << channels << " channels";
		vorbis_info_clear(&info);
		return out;
	}
	vorbis_comment comment;
	vorbis_comment_init(&comment);
	vorbis_comment_add_tag(&comment, "ARTIST", "the unit tests");
	vorbis_comment_add_tag(&comment, "TITLE", "a test tone");
	vorbis_dsp_state dsp;
	vorbis_analysis_init(&dsp, &info);
	vorbis_block block;
	vorbis_block_init(&dsp, &block);
	ogg_stream_state stream;
	ogg_stream_init(&stream, 0x4e654c);

	ogg_packet headers[3];
	vorbis_analysis_headerout(&dsp, &comment, &headers[0], &headers[1], &headers[2]);
	for (uint i = 0; i < 3; ++i)
		ogg_stream_packetin(&stream, &headers[i]);
	ogg_page page;
	while (ogg_stream_flush(&stream, &page))
		appendPage(out, page);

	uint done = 0;
	bool ended = false;
	while (!ended)
	{
		// blocks of 1024 samples, then none, which tells the encoder that the stream ends
		const uint count = std::min(frames - done, 1024u);
		if (count)
		{
			float **buffer = vorbis_analysis_buffer(&dsp, (int)count);
			for (uint channel = 0; channel < channels; ++channel)
				for (uint i = 0; i < count; ++i)
					buffer[channel][i] = sample(channel, done + i);
		}
		vorbis_analysis_wrote(&dsp, (int)count);
		done += count;
		while (vorbis_analysis_blockout(&dsp, &block) == 1)
		{
			vorbis_analysis(&block, nullptr);
			vorbis_bitrate_addblock(&block);
			ogg_packet packet;
			while (vorbis_bitrate_flushpacket(&dsp, &packet))
			{
				ogg_stream_packetin(&stream, &packet);
				while (!ended && ogg_stream_pageout(&stream, &page))
				{
					appendPage(out, page);
					ended = ogg_page_eos(&page) != 0;
				}
			}
		}
		if (!count && !ended)
		{
			// nothing was left to end the stream with
			while (ogg_stream_flush(&stream, &page))
				appendPage(out, page);
			ended = true;
		}
	}

	ogg_stream_clear(&stream);
	vorbis_block_clear(&block);
	vorbis_dsp_clear(&dsp);
	vorbis_comment_clear(&comment);
	vorbis_info_clear(&info);
	return out;
}

// a tone of its own for each channel: a whole number of periods in 4096 samples, so that the tones of the channels
// can be told apart in a window of that length
const uint ToneWindow = 4096;

uint toneBin(uint channel)
{
	return 100 + 40 * channel;
}

float tone(uint channel, uint frame)
{
	const double pi = 3.14159265358979323846;
	return 0.5f * (float)sin(2.0 * pi * (double)toneBin(channel) * (double)frame / (double)ToneWindow);
}

// noise, which the encoder cannot make small: a stream of many pages
float noise(uint channel, uint frame)
{
	uint32 value = (frame * 2 + channel) * 2654435761u;
	value ^= value >> 15;
	value *= 2246822519u;
	value ^= value >> 13;
	return (float)(value & 0xffff) / 65536.f - 0.5f;
}

// reads the stream to its end, in calls of the sizes the stream sources use; the calls are counted, so that a stream
// that does not end fails the test
std::vector<sint16> decodeAll(IAudioDecoder &decoder)
{
	std::vector<sint16> samples;
	std::vector<uint8> buffer(16384);
	for (uint calls = 0; calls < 10000 && !decoder.isMusicEnded(); ++calls)
	{
		const uint32 bytes = decoder.getNextBytes(&buffer[0], (uint32)buffer.size() / 2, (uint32)buffer.size());
		EXPECT_LE(bytes, buffer.size());
		const sint16 *read = (const sint16 *)&buffer[0];
		samples.insert(samples.end(), read, read + bytes / 2);
	}
	EXPECT_TRUE(decoder.isMusicEnded());
	return samples;
}

#endif // NL_UT_HAVE_VORBISENC

} // namespace

class CUTSoundAudioDecoder : public testing::Test
{
protected:
	void SetUp() override
	{
		NLMISC::createDebug();
	}
};

// With ffmpeg, IAudioDecoder creates the ffmpeg decoder for every file and never this one. The other tests create the
// Vorbis decoder themselves and are the same with and without ffmpeg.
#ifndef NL_UT_SOUND_FFMPEG

TEST_F(CUTSoundAudioDecoder, vorbisDecoderOfOtherDataIsNotCreated)
{
	// the decoder was created whatever the stream holds, and every call on it failed
	const std::vector<TBytes> files = makeOtherData();
	for (size_t i = 0; i < files.size(); ++i)
	{
		SCOPED_TRACE(i);
		CMemStream stream(true);
		fillStream(stream, files[i]);
		IAudioDecoder *decoder = IAudioDecoder::createAudioDecoder("ogg", &stream, false);
		EXPECT_EQ(nullptr, decoder);
		delete decoder;
	}
}

#endif // NL_UT_SOUND_FFMPEG

TEST_F(CUTSoundAudioDecoder, vorbisReadOfOtherDataEnds)
{
	// the read never returned: it asked the closed file for more until it had the minimum
	const std::vector<TBytes> files = makeOtherData();
	for (size_t i = 0; i < files.size(); ++i)
	{
		SCOPED_TRACE(i);
		CMemStream stream(true);
		fillStream(stream, files[i]);
		CAudioDecoderVorbis decoder(&stream, false);
		EXPECT_FALSE(decoder.isFormatSupported());
		std::vector<uint8> buffer(4096);
		EXPECT_EQ(0u, decoder.getNextBytes(&buffer[0], 1024, 4096));
		EXPECT_TRUE(decoder.isMusicEnded());
		EXPECT_EQ(0u, decoder.getNextBytes(&buffer[0], 1024, 4096));
	}
}

TEST_F(CUTSoundAudioDecoder, vorbisInfoOfOtherDataFails)
{
	// the comments of the closed file were read
	const std::vector<TBytes> files = makeOtherData();
	for (size_t i = 0; i < files.size(); ++i)
	{
		SCOPED_TRACE(i);
		CMemStream stream(true);
		fillStream(stream, files[i]);
		std::string artist = "artist", title = "title";
		float length = 1.f;
		EXPECT_FALSE(CAudioDecoderVorbis::getInfo(&stream, artist, title, length));
		EXPECT_EQ("", artist);
		EXPECT_EQ("", title);
		EXPECT_EQ(0.f, length);
	}
}

#ifdef NL_UT_HAVE_VORBISENC

TEST_F(CUTSoundAudioDecoder, vorbisLoopOfAnEmptyStreamEnds)
{
	// a stream without samples was rewound forever when it loops
	const TBytes ogg = encodeVorbis(2, 0, tone);
	ASSERT_FALSE(ogg.empty());
	CMemStream stream(true);
	fillStream(stream, ogg);
	CAudioDecoderVorbis decoder(&stream, true);
	EXPECT_TRUE(decoder.isFormatSupported());
	EXPECT_EQ(2, decoder.getChannels());
	std::vector<uint8> buffer(4096);
	EXPECT_EQ(0u, decoder.getNextBytes(&buffer[0], 1024, 4096));
	EXPECT_TRUE(decoder.isMusicEnded());
}

TEST_F(CUTSoundAudioDecoder, vorbisReadErrorAfterSamplesEnds)
{
	// A stream that opens, read with room for a sample and a half: the first read of the loop gives the sample, the
	// next one fails, as no sample fits what is left (OV_EINVAL). The stream ends with less than the minimum, where the
	// loop asked again forever. With a stream that can seek, the library reports damage as an interruption and a cut
	// file as its end: this is the read error there is after the stream has opened.
	const TBytes ogg = encodeVorbis(2, SampleRate / 10, tone);
	ASSERT_FALSE(ogg.empty());
	CMemStream stream(true);
	fillStream(stream, ogg);
	CAudioDecoderVorbis decoder(&stream, false);
	ASSERT_TRUE(decoder.isFormatSupported());
	std::vector<uint8> buffer(16);
	EXPECT_EQ(4u, decoder.getNextBytes(&buffer[0], 6, 6));
	EXPECT_TRUE(decoder.isMusicEnded());
	EXPECT_EQ(0u, decoder.getNextBytes(&buffer[0], 6, 6));
}

TEST_F(CUTSoundAudioDecoder, vorbisCutStreamEnds)
{
	// a file cut in the middle of a page ends there
	const uint frames = SampleRate;
	TBytes ogg = encodeVorbis(2, frames, noise);
	ASSERT_FALSE(ogg.empty());
	ogg.resize(ogg.size() * 2 / 3);
	CMemStream stream(true);
	fillStream(stream, ogg);
	CAudioDecoderVorbis decoder(&stream, false);
	ASSERT_TRUE(decoder.isFormatSupported());
	const size_t samples = decodeAll(decoder).size();
	EXPECT_GT(samples, (size_t)0);
	EXPECT_LT(samples, (size_t)frames * 2);
}

TEST_F(CUTSoundAudioDecoder, vorbisManyInterruptionsInARowEndTheStream)
{
	// Pages that bring no packet and each follow a missing page, put between the headers and the samples of a stream:
	// the decoder reports an interruption for each. 20 of them are passed over, and the samples after them are read.
	// With 40 of them the stream ends, at the 32nd interruption in a row.
	const TBytes ogg = encodeVorbis(2, SampleRate / 2, noise);
	ASSERT_FALSE(ogg.empty());
	const std::vector<size_t> pages = findOggPages(ogg);
	// the pages of the headers have a sample position of 0
	size_t firstSamples = 0;
	for (size_t i = 0; i < pages.size() && !firstSamples; ++i)
	{
		const uint8 zero[8] = { 0 };
		if (memcmp(&ogg[pages[i] + 6], zero, 8) != 0)
			firstSamples = pages[i];
	}
	ASSERT_NE((size_t)0, firstSamples);

	const uint counts[] = { 20, 40 };
	for (uint i = 0; i < 2; ++i)
	{
		SCOPED_TRACE(counts[i]);
		TBytes file(ogg.begin(), ogg.begin() + firstSamples);
		for (uint page = 0; page < counts[i]; ++page)
		{
			const TBytes noPacket = makeOggPageOfNoPacket(&ogg[14], 1000 + page * 2);
			file.insert(file.end(), noPacket.begin(), noPacket.end());
		}
		file.insert(file.end(), ogg.begin() + firstSamples, ogg.end());
		CMemStream stream(true);
		fillStream(stream, file);
		CAudioDecoderVorbis decoder(&stream, false);
		ASSERT_TRUE(decoder.isFormatSupported());
		std::vector<uint8> buffer(4096);
		const uint32 bytes = decoder.getNextBytes(&buffer[0], 1024, 4096);
		if (counts[i] < 32)
		{
			EXPECT_GE(bytes, 1024u);
			EXPECT_FALSE(decoder.isMusicEnded());
		}
		else
		{
			EXPECT_EQ(0u, bytes);
			EXPECT_TRUE(decoder.isMusicEnded());
		}
	}
}

TEST_F(CUTSoundAudioDecoder, vorbisStreamDecodesPastADamagedPage)
{
	// the whole stream, then the same with a page that fails its checksum: the decoder reports a hole and goes on
	const uint frames = SampleRate;
	const TBytes ogg = encodeVorbis(2, frames, noise);
	ASSERT_FALSE(ogg.empty());
	{
		CMemStream stream(true);
		fillStream(stream, ogg);
		CAudioDecoderVorbis decoder(&stream, false);
		EXPECT_EQ(2, decoder.getChannels());
		EXPECT_EQ(SampleRate, decoder.getSamplesPerSec());
		EXPECT_EQ(frames * 2, decodeAll(decoder).size());

		std::string artist, title;
		float length = 0.f;
		stream.seek(0, NLMISC::IStream::begin);
		EXPECT_TRUE(CAudioDecoderVorbis::getInfo(&stream, artist, title, length));
		EXPECT_EQ("the unit tests", artist);
		EXPECT_EQ("a test tone", title);
		EXPECT_FLOAT_EQ(1.f, length);
	}
	{
		// the pages start with OggS: damage the one in the middle of the file
		std::vector<size_t> pages;
		for (size_t i = 0; i + 4 <= ogg.size(); ++i)
		{
			if (memcmp(&ogg[i], "OggS", 4) == 0)
				pages.push_back(i);
		}
		ASSERT_GE(pages.size(), 8u);
		TBytes damaged = ogg;
		for (size_t i = pages[pages.size() / 2] + 100; i < pages[pages.size() / 2] + 200; ++i)
			damaged[i] ^= 0x55;
		CMemStream stream(true);
		fillStream(stream, damaged);
		CAudioDecoderVorbis decoder(&stream, false);
		const size_t samples = decodeAll(decoder).size();
		EXPECT_GT(samples, (size_t)frames);
		EXPECT_LT(samples, (size_t)frames * 2);
	}
}

TEST_F(CUTSoundAudioDecoder, vorbisOfMoreThanTwoChannelsIsMixedDownToStereo)
{
	// Each channel carries a tone of its own, and the share of each tone in the left and in the right channel of the
	// output is measured. The decoder gave all the channels as they are, which the sound drivers played as stereo.
	// The channels of each count, in the order of the Vorbis specification: L and R are front left and right, C the
	// front center, l and r a side or rear left and right, c the rear center, - the low frequency channel.
	const char *layouts[] = { "LCR", "LRlr", "LCRlr", "LCRlr-", "LCRlrc-", "LCRlrlr-" };
	const double pi = 3.14159265358979323846;
	const double reduced = sqrt(0.5);
	const uint frames = ToneWindow * 4;
	for (uint channels = 3; channels <= 8; ++channels)
	{
		SCOPED_TRACE(channels);
		const char *layout = layouts[channels - 3];
		ASSERT_EQ((size_t)channels, strlen(layout));
		const TBytes ogg = encodeVorbis(channels, frames, tone);
		ASSERT_FALSE(ogg.empty());
		CMemStream stream(true);
		fillStream(stream, ogg);
		CAudioDecoderVorbis decoder(&stream, false);
		ASSERT_TRUE(decoder.isFormatSupported());
		EXPECT_EQ(2, decoder.getChannels());
		EXPECT_EQ(16, decoder.getBitsPerSample());
		EXPECT_EQ(SampleRate, decoder.getSamplesPerSec());
		const std::vector<sint16> samples = decodeAll(decoder);
		ASSERT_EQ((size_t)frames * 2, samples.size());

		// The shares: a front channel in full on its side, the front center and a side or rear channel at -3 dB, the
		// rear center at -6 dB on both sides. They are scaled to add up to 1 in each output channel, so that the mix
		// cannot clip.
		double gains[8][2];
		double sum = 0.0;
		for (uint channel = 0; channel < channels; ++channel)
		{
			const char role = layout[channel];
			const double both = role == 'C' ? reduced : (role == 'c' ? 0.5 : 0.0);
			gains[channel][0] = role == 'L' ? 1.0 : (role == 'l' ? reduced : both);
			gains[channel][1] = role == 'R' ? 1.0 : (role == 'r' ? reduced : both);
			sum += gains[channel][0];
		}

		for (uint channel = 0; channel < channels; ++channel)
		{
			// the amplitude of the tone of this channel in the second window of the output, in phase with the input
			double measured[2] = { 0.0, 0.0 };
			for (uint i = 0; i < ToneWindow; ++i)
			{
				const double reference = sin(2.0 * pi * (double)toneBin(channel) * (double)i / (double)ToneWindow);
				for (uint side = 0; side < 2; ++side)
					measured[side] += reference * (double)samples[(ToneWindow + i) * 2 + side] / 32768.0;
			}
			for (uint side = 0; side < 2; ++side)
			{
				EXPECT_NEAR(0.5 * gains[channel][side] / sum, measured[side] * 2.0 / (double)ToneWindow, 0.01)
				    << "channel " << channel << " in output channel " << side;
			}
		}
	}
}

TEST_F(CUTSoundAudioDecoder, vorbisOfMoreThanEightChannelsIsRefused)
{
	// the Vorbis specification gives no meaning to the channels of such a stream
	const TBytes ogg = encodeVorbis(9, ToneWindow, tone);
	ASSERT_FALSE(ogg.empty());
	CMemStream stream(true);
	fillStream(stream, ogg);
	CAudioDecoderVorbis decoder(&stream, false);
	EXPECT_FALSE(decoder.isFormatSupported());
	EXPECT_TRUE(decoder.isMusicEnded());
	std::vector<uint8> buffer(4096);
	EXPECT_EQ(0u, decoder.getNextBytes(&buffer[0], 1024, 4096));
}

TEST_F(CUTSoundAudioDecoder, vorbisMixDownReadsTheMinimumOrEnds)
{
	// A read gives the minimum at least, or the stream has ended: with a stream that is mixed down it gave less
	// without an end when the room was no multiple of a sample of the output. The stereo stream is read by the
	// library, and the stream that is mixed down must do the same.
	const uint frames = SampleRate / 10;
	for (uint channels = 2; channels <= 3; ++channels)
	{
		SCOPED_TRACE(channels);
		const TBytes ogg = encodeVorbis(channels, frames, tone);
		ASSERT_FALSE(ogg.empty());
		std::vector<uint8> buffer(4096);
		// room for a sample and a half, for less than a sample, and for nothing: what fits, then the stream ends,
		// as no sample fits what is left
		const uint32 requests[][3] = { { 6, 6, 4 }, { 2, 3, 0 }, { 0, 0, 0 } };
		for (uint i = 0; i < 3; ++i)
		{
			SCOPED_TRACE(i);
			CMemStream stream(true);
			fillStream(stream, ogg);
			CAudioDecoderVorbis decoder(&stream, false);
			ASSERT_TRUE(decoder.isFormatSupported());
			EXPECT_EQ(requests[i][2], decoder.getNextBytes(&buffer[0], requests[i][0], requests[i][1]));
			EXPECT_TRUE(decoder.isMusicEnded());
		}
		// with room for whole samples, every read gives the minimum until the stream ends
		CMemStream stream(true);
		fillStream(stream, ogg);
		CAudioDecoderVorbis decoder(&stream, false);
		ASSERT_TRUE(decoder.isFormatSupported());
		uint32 total = 0;
		for (uint calls = 0; calls < 10000 && !decoder.isMusicEnded(); ++calls)
		{
			const uint32 bytes = decoder.getNextBytes(&buffer[0], 1000, 4096);
			EXPECT_TRUE(bytes >= 1000 || decoder.isMusicEnded()) << bytes << " bytes in call " << calls;
			total += bytes;
		}
		EXPECT_TRUE(decoder.isMusicEnded());
		EXPECT_EQ(frames * 4, total);
	}
}

#endif // NL_UT_HAVE_VORBISENC

TEST_F(CUTSoundAudioDecoder, bufferFormatOfMoreThanTwoChannelsIsUnknown)
{
	// the sample format of the drivers that play mono and stereo only: any other channel count was stereo
	NLSOUND::TSampleFormat format = NLSOUND::Mono8;
	IBuffer::bufferFormatToSampleFormat(IBuffer::FormatPcm, 6, 16, format);
	EXPECT_EQ(NLSOUND::SampleFormatUnknown, format);
	IBuffer::bufferFormatToSampleFormat(IBuffer::FormatPcm, 3, 8, format);
	EXPECT_EQ(NLSOUND::SampleFormatUnknown, format);
	IBuffer::bufferFormatToSampleFormat(IBuffer::FormatPcm, 0, 16, format);
	EXPECT_EQ(NLSOUND::SampleFormatUnknown, format);

	IBuffer::bufferFormatToSampleFormat(IBuffer::FormatPcm, 2, 16, format);
	EXPECT_EQ(NLSOUND::Stereo16, format);
	IBuffer::bufferFormatToSampleFormat(IBuffer::FormatPcm, 2, 8, format);
	EXPECT_EQ(NLSOUND::Stereo8, format);
	IBuffer::bufferFormatToSampleFormat(IBuffer::FormatPcm, 1, 16, format);
	EXPECT_EQ(NLSOUND::Mono16, format);
	IBuffer::bufferFormatToSampleFormat(IBuffer::FormatPcm, 1, 8, format);
	EXPECT_EQ(NLSOUND::Mono8, format);
}
