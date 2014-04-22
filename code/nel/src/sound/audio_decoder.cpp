/**
 * \file audio_decoder.cpp
 * \brief IAudioDecoder
 * \date 2012-04-11 09:34GMT
 * \author Jan Boon (Kaetemi)
 * IAudioDecoder
 */

/* 
 * Copyright (C) 2008-2012  by authors
 * 
 * This file is part of RYZOM CORE.
 * RYZOM CORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * RYZOM CORE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "stdsound.h"
#include <nel/sound/audio_decoder.h>

// STL includes

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/path.h>

// Project includes
#include <nel/sound/audio_decoder_vorbis.h>

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
	std::string lookup = CPath::lookup(filepath, false);
	if (lookup.empty())
	{ 
		nlwarning("Music file %s does not exist!", filepath.c_str());
		return NULL; 
	}
	std::string type = CFile::getExtension(filepath);

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
	std::string type_lower = toLower(type);
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
	std::string lookup = CPath::lookup(filepath, false);
	if (lookup.empty())
	{ 
		nlwarning("Music file %s does not exist!", filepath.c_str());
		return false; 
	}
	std::string type = CFile::getExtension(filepath);
	std::string type_lower = NLMISC::toLower(type);

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

} /* namespace NLSOUND */

/* end of file */
