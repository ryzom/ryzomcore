/**
 * \file audio_decoder.cpp
 * \brief IAudioDecoder
 * \date 2012-04-11 09:34GMT
 * \author Jan Boon (Kaetemi)
 * IAudioDecoder
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2008-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include <nel/sound/audio_decoder.h>

// STL includes

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/path.h>

// Project includes
#include <nel/sound/audio_decoder_vorbis.h>
#include <nel/sound/audio_decoder_mp3.h>

#ifdef FFMPEG_ENABLED
#include <nel/sound/audio_decoder_ffmpeg.h>
#endif

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

IAudioDecoder::IAudioDecoder() : _InternalStream(NULL)
{
	
}

IAudioDecoder::~IAudioDecoder()
{
	if (_InternalStream) { delete _InternalStream; _InternalStream = NULL; }
}

IAudioDecoder *IAudioDecoder::createAudioDecoder(const std::string &filepath, bool async, bool loop)
{
	std::string type = CFile::getExtension(filepath);

	CIFile *ifile = new CIFile();
	ifile->setCacheFileOnOpen(!async);
	ifile->allowBNPCacheFileOnOpen(!async);
	ifile->open(filepath);

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
#ifdef FFMPEG_ENABLED
	try {
		CAudioDecoderFfmpeg *decoder = new CAudioDecoderFfmpeg(stream, loop);
		return static_cast<IAudioDecoder *>(decoder);
	}
	catch(const Exception &e)
	{
		nlwarning("Exception %s during ffmpeg setup", e.what());
		return NULL;
	}
#else
	std::string type_lower = toLowerAscii(type);
	if (type_lower == "ogg")
	{
		return new CAudioDecoderVorbis(stream, loop);
	}
#if !defined(NL_OS_WINDOWS) || (NL_COMP_VC_VERSION > 90) /* VS2008 does not have stdint.h */
	else if (type_lower == "mp3")
	{
		return new CAudioDecoderMP3(stream, loop);
	}
#endif
	else
	{
		nlwarning("Music file type unknown: '%s'", type_lower.c_str());
		return NULL;
	}
#endif
}

bool IAudioDecoder::getInfo(const std::string &filepath, std::string &artist, std::string &title, float &length)
{
	if (filepath.empty() || !CFile::fileExists(filepath))
	{
		nlwarning("Music file '%s' does not exist!", filepath.c_str());
		return false;
	}

#ifdef FFMPEG_ENABLED
	CIFile ifile;
	ifile.setCacheFileOnOpen(false);
	ifile.allowBNPCacheFileOnOpen(false);
	if (ifile.open(filepath))
		return CAudioDecoderFfmpeg::getInfo(&ifile, artist, title, length);
#else
	std::string type = CFile::getExtension(filepath);
	std::string type_lower = NLMISC::toLowerAscii(type);

	if (type_lower == "ogg")
	{
		CIFile ifile;
		ifile.setCacheFileOnOpen(false);
		ifile.allowBNPCacheFileOnOpen(false);
		if (ifile.open(filepath))
			return CAudioDecoderVorbis::getInfo(&ifile, artist, title, length);

		nlwarning("Unable to open: '%s'", filepath.c_str());
	}
#if !defined(NL_OS_WINDOWS) || (NL_COMP_VC_VERSION > 90) /* VS2008 does not have stdint.h */
	else if (type_lower == "mp3")
	{
		CIFile ifile;
		ifile.setCacheFileOnOpen(false);
		ifile.allowBNPCacheFileOnOpen(false);
		if (ifile.open(filepath))
			return CAudioDecoderMP3::getInfo(&ifile, artist, title, length);

		nlwarning("Unable to open: '%s'", filepath.c_str());
	}
#endif
	else
	{
		nlwarning("Music file type unknown: '%s'", type_lower.c_str());
	}
#endif

	artist.clear(); title.clear();
	return false;
}

/// Get audio/container extensions that are currently supported by the nel sound library.
void IAudioDecoder::getMusicExtensions(std::vector<std::string> &extensions)
{
	// only add ogg format if not already in extensions list
	if (std::find(extensions.begin(), extensions.end(), "ogg") == extensions.end())
	{
		extensions.push_back("ogg");
	}
	if (std::find(extensions.begin(), extensions.end(), "mp3") == extensions.end())
	{
		extensions.push_back("mp3");
	}
#ifdef FFMPEG_ENABLED
	extensions.push_back("mp3");
	extensions.push_back("flac");
	extensions.push_back("aac");
#endif

	// extensions.push_back("wav"); // TODO: Easy.
}

/// Return if a music extension is supported by the nel sound library.
bool IAudioDecoder::isMusicExtensionSupported(const std::string &extension)
{
#ifdef FFMPEG_ENABLED
	return (extension == "ogg" || extension == "mp3" || extension == "flac" || extension == "aac");
#else
	return (extension == "ogg");
#endif
}

} /* namespace NLSOUND */

/* end of file */
