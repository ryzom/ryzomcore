// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"

#include "emoji_manager.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/gui/view_text.h"

#include <algorithm>

using namespace std;
using namespace NLMISC;

CEmojiManager *CEmojiManager::_Instance = NULL;

// The generated table, plus an optional hand-maintained file loaded after it so
// local corrections win without anyone editing the generated one.
static const char *EmojiTableFile     = "emoji.txt";
static const char *EmojiOverrideFile  = "emoji_overrides.txt";

//=================================================================================
CEmojiManager::CEmojiManager() : _Loaded(false)
{
	for (uint i = 0; i < 256; ++i)
		_CanStartUtf8[i] = false;
}

//=================================================================================
CEmojiManager::~CEmojiManager()
{
}

//=================================================================================
CEmojiManager &CEmojiManager::getInstance()
{
	if (!_Instance)
		_Instance = new CEmojiManager();
	return *_Instance;
}

//=================================================================================
void CEmojiManager::releaseInstance()
{
	if (_Instance)
	{
		delete _Instance;
		_Instance = NULL;
	}
}

//=================================================================================
// Turn "1f468 200d 1f469" into the UTF-8 bytes for that codepoint sequence.
static bool codepointsToUtf8(const string &field, string &out)
{
	out.clear();
	string::size_type i = 0;
	while (i < field.size())
	{
		while (i < field.size() && field[i] == ' ')
			++i;
		string::size_type start = i;
		while (i < field.size() && field[i] != ' ')
			++i;
		if (start == i)
			break;

		uint32 cp = 0;
		if (sscanf(field.c_str() + start, "%x", &cp) != 1)
			return false;

		// UTF-8 encode. Done by hand rather than via CUtfStringView so a single
		// malformed line cannot take the whole table down with it.
		if (cp < 0x80)
		{
			out += (char)cp;
		}
		else if (cp < 0x800)
		{
			out += (char)(0xC0 | (cp >> 6));
			out += (char)(0x80 | (cp & 0x3F));
		}
		else if (cp < 0x10000)
		{
			out += (char)(0xE0 | (cp >> 12));
			out += (char)(0x80 | ((cp >> 6) & 0x3F));
			out += (char)(0x80 | (cp & 0x3F));
		}
		else if (cp <= 0x10FFFF)
		{
			out += (char)(0xF0 | (cp >> 18));
			out += (char)(0x80 | ((cp >> 12) & 0x3F));
			out += (char)(0x80 | ((cp >> 6) & 0x3F));
			out += (char)(0x80 | (cp & 0x3F));
		}
		else
		{
			return false;
		}
	}
	return !out.empty();
}

//=================================================================================
void CEmojiManager::addEntry(const string &name, const string &utf8, const string &texture)
{
	CEntry &e = _ByName[name];
	e.Utf8 = utf8;
	e.Texture = texture;
}

//=================================================================================
bool CEmojiManager::loadTable(const string &filename, bool required)
{
	string path = CPath::lookup(filename, false, false, false);
	if (path.empty())
	{
		if (required)
			nlwarning("Emoji: '%s' not found, chat emoji will stay as plain text", filename.c_str());
		return false;
	}

	// CIFile, not ifstream: the table ships inside gamedev.bnp, and only CPath
	// and CIFile can see inside a bnp.
	CIFile f;
	if (!f.open(path))
	{
		nlwarning("Emoji: cannot open '%s'", path.c_str());
		return false;
	}

	string buffer;
	try
	{
		uint32 size = f.getFileSize();
		buffer.resize(size);
		if (size)
			f.serialBuffer((uint8 *)&buffer[0], size);
	}
	catch (const Exception &e)
	{
		nlwarning("Emoji: error reading '%s': %s", path.c_str(), e.what());
		return false;
	}
	f.close();

	uint added = 0, bad = 0;
	string::size_type pos = 0;
	while (pos < buffer.size())
	{
		string::size_type eol = buffer.find('\n', pos);
		if (eol == string::npos)
			eol = buffer.size();
		string line = buffer.substr(pos, eol - pos);
		pos = eol + 1;

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty() || line[0] == '#')
			continue;

		// name \t codepoints \t image-stem
		string::size_type t1 = line.find('\t');
		if (t1 == string::npos) { ++bad; continue; }
		string::size_type t2 = line.find('\t', t1 + 1);
		if (t2 == string::npos) { ++bad; continue; }

		string name  = line.substr(0, t1);
		string codes = line.substr(t1 + 1, t2 - t1 - 1);
		string stem  = line.substr(t2 + 1);

		string utf8;
		if (name.empty() || name.size() > MaxNameBytes || !codepointsToUtf8(codes, utf8))
		{
			++bad;
			continue;
		}

		// The table stores the image stem without an extension on purpose. The
		// build tools deal in .png, but loadTextures rewrites .png to .tga while
		// reading an atlas UV list, so what the renderer knows the tile by is
		// .tga. Whichever extension the file carried, one side would be wrong.
		addEntry(name, utf8, stem.empty() ? string() : stem + ".tga");
		++added;
	}

	if (bad)
		nlwarning("Emoji: %u malformed line(s) in '%s'", bad, path.c_str());
	nlinfo("Emoji: loaded %u entries from '%s'", added, path.c_str());
	return added > 0;
}

//=================================================================================
void CEmojiManager::init()
{
	if (_Loaded)
		return;
	_Loaded = true;

	loadTable(EmojiTableFile, true);
	// Optional, and loaded second so it overrides the generated table.
	loadTable(EmojiOverrideFile, false);

	// Build the reverse index used to spot emoji that were typed or pasted
	// directly rather than written as ":name:".
	std::vector<std::string::size_type> lengths;
	for (std::map<string, CEntry>::const_iterator it = _ByName.begin(); it != _ByName.end(); ++it)
	{
		const CEntry &e = it->second;
		if (e.Utf8.empty())
			continue;
		// Several names share one emoji; first one wins, which is fine because
		// only the entry's data is needed here, not which name it came from.
		if (_ByUtf8.find(e.Utf8) == _ByUtf8.end())
		{
			_ByUtf8[e.Utf8] = &e;
			_CanStartUtf8[(unsigned char)e.Utf8[0]] = true;
			if (std::find(lengths.begin(), lengths.end(), e.Utf8.size()) == lengths.end())
				lengths.push_back(e.Utf8.size());
		}
	}
	// Longest first: a family emoji has to win over the single person its
	// sequence starts with, or the rest of the sequence is left behind.
	std::sort(lengths.begin(), lengths.end());
	std::reverse(lengths.begin(), lengths.end());
	_Utf8Lengths = lengths;

	nlinfo("Emoji: %u name(s), %u distinct emoji, %u sequence length(s)",
		(uint)_ByName.size(), (uint)_ByUtf8.size(), (uint)_Utf8Lengths.size());
}

//=================================================================================
bool CEmojiManager::mayContainEmoji(const string &text) const
{
	if (_ByName.empty())
		return false;
	for (string::size_type i = 0; i < text.size(); ++i)
	{
		if (text[i] == ':')
			return true;
		if (_CanStartUtf8[(unsigned char)text[i]])
			return true;
	}
	return false;
}

//=================================================================================
// Is there a ":name:" at index? Returns its total length including both colons.
// Names are matched as raw bytes: no character class, see the header.
static inline std::string::size_type shortcodeAt(const string &text,
	std::string::size_type index, std::string::size_type to, string &name)
{
	if (index >= to || text[index] != ':')
		return 0;

	std::string::size_type end = index + 1;
	std::string::size_type limit = std::min(to, index + 1 + CEmojiManager::MaxNameBytes + 1);
	while (end < limit)
	{
		unsigned char c = (unsigned char)text[end];
		if (c == ':')
		{
			if (end == index + 1)
				return 0;			// "::" is not a name
			name.assign(text, index + 1, end - index - 1);
			return end - index + 1;
		}
		// A name never contains whitespace or control bytes. Stopping on them
		// keeps a stray colon in ordinary prose from starting a long scan.
		if (c <= 0x20 || c == 0x7F)
			return 0;
		++end;
	}
	return 0;
}

//=================================================================================
bool CEmojiManager::matchAt(const string &text, std::string::size_type index,
	std::string::size_type to, std::string::size_type &len, const CEntry *&entry) const
{
	if (index >= to)
		return false;

	// ":name:"
	if (text[index] == ':')
	{
		string name;
		std::string::size_type l = shortcodeAt(text, index, to, name);
		if (l)
		{
			std::map<string, CEntry>::const_iterator it = _ByName.find(name);
			if (it != _ByName.end())
			{
				len = l;
				entry = &it->second;
				return true;
			}
		}
		return false;
	}

	// a literal emoji, longest sequence first
	if (!_CanStartUtf8[(unsigned char)text[index]])
		return false;
	for (uint i = 0; i < _Utf8Lengths.size(); ++i)
	{
		std::string::size_type l = _Utf8Lengths[i];
		if (index + l > to)
			continue;
		std::map<string, const CEntry *>::const_iterator it =
			_ByUtf8.find(text.substr(index, l));
		if (it != _ByUtf8.end())
		{
			len = l;
			entry = it->second;
			return true;
		}
	}
	return false;
}

//=================================================================================
std::string CEmojiManager::substituteShortcodes(const string &text) const
{
	if (_ByName.empty())
		return text;

	string out;
	out.reserve(text.size());

	std::string::size_type i = 0;
	while (i < text.size())
	{
		// Step over format tags rather than scanning inside them. A tooltip tag
		// carries arbitrary text, so "@{Hsee :smile: here}" holds something that
		// looks exactly like a shortcode; substituting it would cut the tag in
		// half and the rest of the line would render as the tag's leftovers.
		uint tagLen = NLGUI::CViewText::getFormatTagLength(text, (uint)i);
		if (tagLen)
		{
			out.append(text, i, tagLen);
			i += tagLen;
			continue;
		}

		if (text[i] == ':')
		{
			string name;
			std::string::size_type l = shortcodeAt(text, i, text.size(), name);
			if (l)
			{
				std::map<string, CEntry>::const_iterator it = _ByName.find(name);
				if (it != _ByName.end())
				{
					out += it->second.Utf8;
					i += l;
					continue;
				}
			}
		}

		out += text[i];
		++i;
	}
	return out;
}
