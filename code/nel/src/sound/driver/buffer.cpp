// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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

#include "stdsound_lowlevel.h"

#include "nel/sound/driver/buffer.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/stream.h"

namespace NLSOUND {
	
// for compatibility
void IBuffer::setFormat(TSampleFormat format, uint freq)
{
	TBufferFormat bufferFormat;
	uint8 channels;
	uint8 bitsPerSample;
	sampleFormatToBufferFormat(format, bufferFormat, channels, bitsPerSample);
	setFormat(bufferFormat, channels, bitsPerSample, (uint32)freq);
}

// for compatibility, very lazy checks (assume it's set by old setFormat)
void IBuffer::getFormat(TSampleFormat& format, uint& freq) const
{
	TBufferFormat bufferFormat;
	uint8 channels;
	uint8 bitsPerSample;
	uint32 frequency;
	getFormat(bufferFormat, channels, bitsPerSample, frequency);
	freq = (uint)frequency;
	bufferFormatToSampleFormat(bufferFormat, channels, bitsPerSample, format);
}

/// Convert old sample format to new buffer format
void IBuffer::sampleFormatToBufferFormat(TSampleFormat sampleFormat, TBufferFormat &bufferFormat, uint8 &channels, uint8 &bitsPerSample)
{
	switch (sampleFormat)
	{
	case Mono8:
		bufferFormat = FormatPcm;
		channels = 1;
		bitsPerSample = 8;
		break;
	case Mono16ADPCM:
		bufferFormat = FormatDviAdpcm;
		channels = 1;
		bitsPerSample = 16;
		break;
	case Mono16:
		bufferFormat = FormatPcm;
		channels = 1;
		bitsPerSample = 16;
		break;
	case Stereo8:
		bufferFormat = FormatPcm;
		channels = 2;
		bitsPerSample = 8;
		break;
	case Stereo16:
		bufferFormat = FormatPcm;
		channels = 2;
		bitsPerSample = 16;
		break;
	default:		
		bufferFormat = FormatUnknown;
		channels = 0;
		bitsPerSample = 0;
		break;
	}
}

/// Convert new buffer format to old sample format
void IBuffer::bufferFormatToSampleFormat(TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample, TSampleFormat &sampleFormat)
{
	switch (bufferFormat)
	{
	case FormatPcm:
		switch (channels)
		{
		case 1:
			switch (bitsPerSample)
			{
			case 8:
				sampleFormat = Mono8;
				break;
			default:
				sampleFormat = Mono16;
				break;
			}
			break;
		default:
			switch (bitsPerSample)
			{
			case 8:
				sampleFormat = Stereo8;
				break;
			default:
				sampleFormat = Stereo16;
				break;
			}
			break;
		}
		break;
	case FormatDviAdpcm:
		sampleFormat = Mono16ADPCM;
		break;
	case FormatUnknown:
	default:
		sampleFormat = SampleFormatUnknown;
		break;
	}
}

uint IBuffer::getPCMSizeFromDuration(float duration, uint8 channels, uint8 bitsPerSample, uint32 frequency)
{
	return (uint)(duration
		* ((float)frequency)
		* (((float)bitsPerSample) / 8.0f)
		* ((float)channels));
}

float IBuffer::getDurationFromPCMSize(uint size, uint8 channels, uint8 bitsPerSample, uint32 frequency)
{
	return ((float)size) 
		/ ((float)channels)
		/ (((float)bitsPerSample) / 8.0f)
		/ ((float)frequency);
}

const sint IBuffer::_IndexTable[16] =
{
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

const uint IBuffer::_StepsizeTable[89] =
{
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};


void IBuffer::encodeADPCM(const sint16 *indata, uint8 *outdata, uint nbSample, TADPCMState &state)
{
	const sint16 *inp = indata;				/* Input buffer pointer */
	uint8 *outp = outdata;				/* output buffer pointer */
	int val;							/* Current input sample value */
	int sign;							/* Current adpcm sign bit */
	int delta;							/* Current adpcm output value */
	int diff;							/* Difference between val and valprev */
	int valpred = state.PreviousSample;	/* Predicted output value */
	int vpdiff;							/* Current change to valpred */
	int index = state.StepIndex;		/* Current step change index */
	int step = _StepsizeTable[index];	/* Stepsize */
	uint8 outputbuffer = 0;				/* place to keep previous 4-bit value */
	int bufferstep = 1;					/* toggle between outputbuffer/output */

	for ( ; nbSample > 0 ; nbSample-- )
	{
		val = *inp++;

		/* Step 1 - compute difference with previous value */
		diff = val - valpred;
		sign = (diff < 0) ? 8 : 0;
		if ( sign ) diff = (-diff);

		/* Step 2 - Divide and clamp */
		/* Note:
		** This code *approximately* computes:
		**    delta = diff*4/step;
		**    vpdiff = (delta+0.5)*step/4;
		** but in shift step bits are dropped. The net result of this is
		** that even if you have fast mul/div hardware you cannot put it to
		** good use since the fixup would be too expensive.
		*/
		delta = 0;
		vpdiff = (step >> 3);

		if ( diff >= step )
		{
			delta = 4;
			diff -= step;
			vpdiff += step;
		}
		step >>= 1;
		if ( diff >= step  )
		{
			delta |= 2;
			diff -= step;
			vpdiff += step;
		}
		step >>= 1;
		if ( diff >= step )
		{
			delta |= 1;
			vpdiff += step;
		}

		/* Step 3 - Update previous value */
		if ( sign )
			valpred -= vpdiff;
		else
			valpred += vpdiff;

		/* Step 4 - Clamp previous value to 16 bits */
		if ( valpred > 32767 )
		{
			nlwarning("over+ %d",valpred);
			valpred = 32767;
		}
		else if ( valpred < -32768 )
		{
			nlwarning("over- %d",valpred);
			valpred = -32768;
		}

		/* Step 5 - Assemble value, update index and step values */
		delta |= sign;

		index += _IndexTable[delta];
		if ( index < 0 )
			index = 0;
		if ( index > 88 )
			index = 88;
		step = _StepsizeTable[index];

		/* Step 6 - Output value */
		if ( bufferstep )
		{
			outputbuffer = (delta << 4) & 0xf0;
		}
		else
		{
			*outp++ = (delta & 0x0f) | outputbuffer;
		}
		bufferstep = !bufferstep;
	}

	/* Output last step, if needed */
	if ( !bufferstep )
		*outp++ = outputbuffer;

	state.PreviousSample = sint16(valpred);
	state.StepIndex = uint8(index);
}

void IBuffer::decodeADPCM(const uint8 *indata, sint16 *outdata, uint nbSample, TADPCMState &state)
{
    const uint8 *inp = indata;				/* Input buffer pointer */
    sint16 *outp = outdata;				/* output buffer pointer */
    int sign;							/* Current adpcm sign bit */
    int delta;							/* Current adpcm output value */
    int valpred = state.PreviousSample;	/* Predicted value */
    int vpdiff;							/* Current change to valpred */
    int index = state.StepIndex;		/* Current step change index */
    int step = _StepsizeTable[index];	/* Stepsize */
    uint8 inputbuffer = 0;				/* place to keep next 4-bit value */
    int bufferstep = 0;					/* toggle between inputbuffer/input */

    for ( ; nbSample > 0 ; nbSample-- )
	{

		/* Step 1 - get the delta value */
		if ( bufferstep )
		{
			delta = inputbuffer & 0xf;
		}
		else
		{
			inputbuffer = *inp++;
			delta = (inputbuffer >> 4) & 0xf;
		}
		bufferstep = !bufferstep;

		/* Step 2 - Find new index value (for later) */
		index += _IndexTable[delta];
		if ( index < 0 )
			index = 0;
		if ( index > 88 )
			index = 88;

		/* Step 3 - Separate sign and magnitude */
		sign = delta & 8;
		delta = delta & 7;

		/* Step 4 - Compute difference and new predicted value */
		/*
		** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
		** in adpcm_coder.
		*/
		vpdiff = step >> 3;
		if ( delta & 4 )
			vpdiff += step;
		if ( delta & 2 )
			vpdiff += step>>1;
		if ( delta & 1 )
			vpdiff += step>>2;

		if ( sign )
			valpred -= vpdiff;
		else
			valpred += vpdiff;

		/* Step 5 - clamp output value */
		if ( valpred > 32767 )
			valpred = 32767;
		else if ( valpred < -32768 )
			valpred = -32768;

		/* Step 6 - Update step value */
		step = _StepsizeTable[index];

		/* Step 7 - Output value */
		*outp++ = sint16(valpred);
    }

    state.PreviousSample = sint16(valpred);
    state.StepIndex = uint8(index);
}

static bool checkFourCC(const uint8 *left, const char *right)
{
	return (left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3]);
}

static bool readHeader(const uint8 *header, const char *fourcc, uint32 &size, const uint8 *&data)
{
	memcpy(&size, header + 4, sizeof(uint32));
	data = header + 8;
	return (header[0] == fourcc[0] && header[1] == fourcc[1] && header[2] == fourcc[2] && header[3] == fourcc[3]);
}

static bool findChunk(const uint8 *src, uint32 srcSize, const char *fourcc, uint32 &size, const uint8 *&data)
{
	uint32 offset = 0;
	while (offset + 8 < srcSize)
	{
		bool found = readHeader(src + offset, fourcc, size, data);
		if (found) return true;
		offset += 8 + size;
	}
	return false;
}

/// Read a wav file. Data type uint8 is used as unspecified buffer format.
bool IBuffer::readWav(const uint8 *wav, uint size, std::vector<uint8> &result, TBufferFormat &bufferFormat, uint8 &channels, uint8 &bitsPerSample, uint32 &frequency)
{
#if 0
	// Create mmio stuff
	MMIOINFO mmioinfo;
	memset(&mmioinfo, 0, sizeof(MMIOINFO));
	mmioinfo.fccIOProc = FOURCC_MEM;
	mmioinfo.pchBuffer = (HPSTR)wav;
	mmioinfo.cchBuffer = size;
	HMMIO hmmio = mmioOpen(NULL, &mmioinfo, MMIO_READ | MMIO_DENYWRITE);
	if (!hmmio) { throw ESoundDriver("Failed to open the file"); }
	
	// Find wave
	MMCKINFO mmckinforiff;
	memset(&mmckinforiff, 0, sizeof(MMCKINFO));
	mmckinforiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if (mmioDescend(hmmio, &mmckinforiff, NULL, MMIO_FINDRIFF) != MMSYSERR_NOERROR) { mmioClose(hmmio, 0); throw ESoundDriver("mmioDescend WAVE failed"); }
	
	// Find fmt
	MMCKINFO mmckinfofmt;
	memset(&mmckinfofmt, 0, sizeof(MMCKINFO));
	mmckinfofmt.ckid = mmioFOURCC('f', 'm', 't', ' '); 
	if (mmioDescend(hmmio, &mmckinfofmt, &mmckinforiff, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) { mmioClose(hmmio, 0); throw ESoundDriver("mmioDescend fmt failed"); }
	WAVEFORMATEX *wavefmt = (WAVEFORMATEX *)(&wav[mmckinfofmt.dwDataOffset]);
	if (mmioAscend(hmmio, &mmckinfofmt, 0) != MMSYSERR_NOERROR) { mmioClose(hmmio, 0); throw ESoundDriver("mmioAscend fmt failed"); }
	
	// Find data
	MMCKINFO mmckinfodata;
	memset(&mmckinfodata, 0, sizeof(MMCKINFO));
	mmckinfodata.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hmmio, &mmckinfodata, &mmckinforiff, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) { mmioClose(hmmio, 0); throw ESoundDriver("mmioDescend data failed"); }
	BYTE *wavedata = (BYTE *)(&wav[mmckinfodata.dwDataOffset]);
	if (mmioAscend(hmmio, &mmckinfodata, 0) != MMSYSERR_NOERROR) { mmioClose(hmmio, 0); throw ESoundDriver("mmioAscend data failed"); }
	
	// Close mmio
	mmioClose(hmmio, 0);
	
	// Copy stuff
	bufferFormat = (TBufferFormat)wavefmt->wFormatTag;
	channels = (uint8)wavefmt->nChannels;
	bitsPerSample = (uint8)wavefmt->wBitsPerSample;
	frequency = wavefmt->nSamplesPerSec;
	result.resize(mmckinfodata.cksize);
	NLMISC::CFastMem::memcpy(&result[0], wavedata, mmckinfodata.cksize);
	return true;

#else
	// read the RIFF header and check if it contains WAVE data
	const uint8 *riffHeader = wav;
	uint32 riffSize;
	const uint8 *riffData;
	if (!readHeader(riffHeader, "RIFF", riffSize, riffData))
	{
		nlwarning("WAV: Cannot find RIFF identifier");
		return false;
	}
	if (riffSize <= 4)
	{
		nlwarning("WAV: Empty RIFF file");
		return false;
	}
	if (!checkFourCC(riffData, "WAVE"))
	{
		nlwarning("WAV: RIFF file does not contain WAVE data");
		return false;
	}
	uint32 waveSize = riffSize - 4;
	const uint8 *waveData = riffData + 4;
	
	// find the 'fmt ' chunk
	uint32 fmtSize;
	const uint8 *fmtData;
	if (!findChunk(waveData, waveSize, "fmt ", fmtSize, fmtData))
	{
		nlwarning("WAV: Cannot find 'fmt ' chunk");
		return false;
	}
	if (fmtSize < 16)
	{
		nlwarning("WAV: The 'fmt ' chunk is incomplete");
		return false;
	}
	
	// find the 'data' chunk
	uint32 dataSize;
	const uint8 *dataData;
	if (!findChunk(waveData, waveSize, "data", dataSize, dataData))
	{
		nlwarning("WAV: Cannot find 'data' chunk");
		return false;
	}
	if (dataData + dataSize > wav + size)
	{
		uint32 cut = (uint32)((dataData + dataSize) - (wav + size));
		nlwarning("WAV: Oversize 'data' chunk with dataSize %u and wav size %u, cutting %u bytes", (uint32)dataSize, (uint32)size, (uint32)cut);
		dataSize -= cut;
	}
	
	// read the 'fmt ' chunk
	uint16 fmtFormatTag; // 0-1
	memcpy(&fmtFormatTag, fmtData + 0, sizeof(uint16));
	uint16 fmtChannels; // 2-3
	memcpy(&fmtChannels, fmtData + 2, sizeof(uint16));
	uint32 fmtSamplesPerSec; // 4-7
	memcpy(&fmtSamplesPerSec, fmtData + 4, sizeof(uint32));
	//uint32 fmtAvgBytesPerSec; // 8-11
	//uint16 fmtBlockAlign; // 12-13
	uint16 fmtBitsPerSample; // 14-15
	memcpy(&fmtBitsPerSample, fmtData + 14, sizeof(uint16));
	//uint16 fmtExSize; // 16-17 // only if fmtSize > 16
	
	bufferFormat = (TBufferFormat)fmtFormatTag;
	channels = (uint8)fmtChannels;
	bitsPerSample = (uint8)fmtBitsPerSample;
	frequency = fmtSamplesPerSec;
	result.resize(dataSize);
	NLMISC::CFastMem::memcpy(&result[0], dataData, dataSize);
	
	return true;
	
#endif
}

/// Write a wav file. Data type uint8 does not imply a buffer of any format.
bool IBuffer::writeWav(const uint8 *buffer, uint size, TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample, uint32 frequency, NLMISC::IStream &out)
{
	nlassert(!out.isReading());
	
	const uint32 headerSize = 8; // 32 TAG + 32 SIZE
	uint32 fmtSize = 16;
	uint32 dataSize = (uint32)size;
	
	// create riff header
	const char *riffFourCC = "RIFF"; // Chunk FourCC
	uint32 riffSize = 4 // Type FourCC
		+ headerSize + fmtSize // fmt Chunk
		+ headerSize + dataSize; // data Chunk
	// create riff data
	const char *waveFourCC = "WAVE"; // Type FourCC
	
	// write riff chunk header
	out.serialBuffer(const_cast<uint8 *>(static_cast<const uint8 *>(static_cast<const void *>(riffFourCC))), 4);
	out.serial(riffSize);
	// write riff chunk data
	out.serialBuffer(const_cast<uint8 *>(static_cast<const uint8 *>(static_cast<const void *>(waveFourCC))), 4);
	
	// riff subchunks
	// create format header
	const char *fmtFourCC = "fmt ";
	// create format data
	uint16 fmtFormatTag = (uint16)bufferFormat; // 0-1
	uint16 fmtChannels = (uint16)channels; // 2-3
	uint32 fmtSamplesPerSec = (uint32)frequency; // 4-7
	uint16 fmtBitsPerSample = (uint16)bitsPerSample; // 14-15
	uint16 fmtBlockAlign = fmtChannels * fmtBitsPerSample / 8; // 12-13
	uint32 fmtAvgBytesPerSec = fmtSamplesPerSec * fmtBlockAlign; // 8-11
	// uint16 fmtExSize; // 16-17 // only if fmtSize > 16

	// write format chunk header
	out.serialBuffer(const_cast<uint8 *>(static_cast<const uint8 *>(static_cast<const void *>(fmtFourCC))), 4);
	out.serial(fmtSize);
	// write format chunk data
	out.serial(fmtFormatTag);
	out.serial(fmtChannels);
	out.serial(fmtSamplesPerSec);
	out.serial(fmtAvgBytesPerSec);
	out.serial(fmtBlockAlign);
	out.serial(fmtBitsPerSample);
	
	// create data header
	const char *dataFourCC = "data";
	
	// write data chunk header
	out.serialBuffer(const_cast<uint8 *>(static_cast<const uint8 *>(static_cast<const void *>(dataFourCC))), 4);
	out.serial(dataSize);
	// write data chunk data
	out.serialBuffer(const_cast<uint8 *>(buffer), size);

	return true;
}

/// Convert buffer data to 16bit Mono PCM.
bool IBuffer::convertToMono16PCM(const uint8 *buffer, uint size, std::vector<sint16> &result, TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample)
{
	if (size == 0 || channels == 0 || bitsPerSample == 0) return false;
	
	switch (bufferFormat)
	{
	case FormatPcm:
		{
			result.resize((std::vector<sint16>::size_type)(((uint64)size / (uint64)channels) * 8UL / (uint64)bitsPerSample));
			uint samples = (uint)result.size();
			switch (bitsPerSample)
			{
			case 8:
				{
					const sint8 *src8 = (const sint8 *)(const void *)buffer;
					uint j = 0;
					for (uint i = 0; i < samples; ++i)
					{
						sint32 sample = 0;
						for (uint k = 0; k < channels; ++k)
							sample += (sint32)src8[j + k];
						j += channels;
						sample *= 256;
						sample /= channels;
						result[i] = (sint16)sample;
					}
				}
				return true;
			case 16:
				{
					const sint16 *src16 = (const sint16 *)(const void *)buffer;
					uint j = 0;
					for (uint i = 0; i < samples; ++i)
					{
						sint32 sample = 0;
						for (uint k = 0; k < channels; ++k)
							sample += (sint32)src16[j + k];
						j += channels;
						sample /= channels;
						result[i] = (sint16)sample;
					}
				}
				return true;
			case 32:
				{
					const sint32 *src32 = (const sint32 *)(const void *)buffer;
					uint j = 0;
					for (uint i = 0; i < samples; ++i)
					{
						sint64 sample = 0;
						for (uint k = 0; k < channels; ++k)
							sample += (sint64)src32[j + k];
						j += channels;
						sample /= 65536;
						sample /= channels;
						result[i] = (sint16)sample;
					}
				}
				return true;
			default:
				return false;
			}
		}
		return true;
	case FormatDviAdpcm:
		{
			uint samples = size * 2;
			result.resize(samples);
			TADPCMState	state;
			state.PreviousSample = 0;
			state.StepIndex = 0;
			decodeADPCM(buffer, &result[0], samples, state);
		}
		return true;
	case FormatUnknown:
	default:
		return false;
	}
}

/// Convert 16bit Mono PCM buffer data to ADPCM.
bool IBuffer::convertMono16PCMToMonoADPCM(const sint16 *buffer, uint samples, std::vector<uint8> &result)
{
	if (samples == 0) return false;

	// Allocate ADPCM dest
	samples &= 0xfffffffe;
	result.resize(samples / 2);

	// Encode
	TADPCMState	state;
	state.PreviousSample = 0;
	state.StepIndex = 0;
	encodeADPCM(buffer, &result[0], samples, state);

	return true;
}

} // NLSOUND
