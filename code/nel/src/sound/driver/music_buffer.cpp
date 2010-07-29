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

#include "stdsound_lowlevel.h"

#include "nel/sound/driver/music_buffer_vorbis.h"
#include "nel/sound/driver/music_buffer.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND
{

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
	ifile->setAsyncLoading(async);
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
