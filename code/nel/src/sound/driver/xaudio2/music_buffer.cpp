/**
 * \file music_buffer.cpp
 * \brief IMusicBuffer
 * \date 2008-08-30 11:38GMT
 * \author Jan Boon (Kaetemi)
 * IMusicBuffer
 */

/* 
 * Copyright (C) 2008  Jan Boon (Kaetemi)
 * 
 * This file is part of NLSOUND Music Library.
 * NLSOUND Music Library is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * NLSOUND Music Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NLSOUND Music Library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 */

#include "stdxaudio2.h"
#include "music_buffer.h"

// STL includes

// NeL includes
#include <nel/misc/stream.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

// Project includes
#include "music_buffer_vorbis.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

IMusicBuffer::IMusicBuffer() : _InternalStream(NULL)
{
	
}

IMusicBuffer::~IMusicBuffer()
{
	if (_InternalStream) { delete _InternalStream; _InternalStream = NULL; }
}

IMusicBuffer *IMusicBuffer::createMusicBuffer(const std::string &filepath, bool async, bool loop)
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

	IMusicBuffer *mb = createMusicBuffer(type, ifile, loop);

	if (mb) mb->_InternalStream = ifile;
	else delete ifile;

	return mb;
}

IMusicBuffer *IMusicBuffer::createMusicBuffer(const std::string &type, NLMISC::IStream *stream, bool loop)
{
	if (!stream)
	{
		nlwarning("Stream is NULL");
		return NULL;
	}
	string type_lower = toLower(type);
	if (type_lower == "ogg")
	{
		return new CMusicBufferVorbis(stream, loop);
	}
	else
	{
		nlwarning("Music file type unknown: '%s'", type_lower.c_str());
		return NULL;
	}
}

bool IMusicBuffer::getInfo(const std::string &filepath, std::string &artist, std::string &title)
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
		return CMusicBufferVorbis::getInfo(&ifile, artist, title);
	}
	else
	{
		nlwarning("Music file type unknown: '%s'", type_lower.c_str());
		artist.clear(); title.clear();
		return false;
	}
}

} /* namespace NLSOUND */

/* end of file */
