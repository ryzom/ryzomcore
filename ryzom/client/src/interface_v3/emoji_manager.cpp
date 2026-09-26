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
#include "nel/misc/i18n.h"
#include "nel/gui/view_text.h"

#include <algorithm>

using namespace std;
using namespace NLMISC;

CEmojiManager *CEmojiManager::_Instance = NULL;

// The generated table, plus an optional hand-maintained file loaded after it so
// local corrections win without anyone editing the generated one.
static const char *EmojiTableFile     = "emoji.txt";
static const char *EmojiOverrideFile  = "emoji_overrides.txt";
// The picker's layout: which tabs it has and what sits in them. Generated from
// Unicode's emoji-test.txt, see tools/emoji/gen_emoji_picker.py.
static const char *EmojiPickerFile    = "emoji_picker.txt";

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
	e.Name = name;
	e.Utf8 = utf8;
	e.Texture = texture;
}

//=================================================================================
// Read one of the emoji data files whole. Returns false, quietly unless the
// file is required, when there is nothing to read.
//
// CIFile, not ifstream: these ship inside gamedev.bnp, and only CPath and
// CIFile can see inside a bnp.
static bool readEmojiFile(const string &filename, bool required, string &buffer, string &path)
{
	path = CPath::lookup(filename, false, false, false);
	if (path.empty())
	{
		if (required)
			nlwarning("Emoji: '%s' not found, chat emoji will stay as plain text", filename.c_str());
		return false;
	}

	CIFile f;
	if (!f.open(path))
	{
		nlwarning("Emoji: cannot open '%s'", path.c_str());
		return false;
	}

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
	return true;
}

//=================================================================================
// Hand back the next line of \p buffer, without its newline, and advance \p pos.
static string nextLine(const string &buffer, string::size_type &pos)
{
	string::size_type eol = buffer.find('\n', pos);
	if (eol == string::npos)
		eol = buffer.size();
	string line = buffer.substr(pos, eol - pos);
	pos = eol + 1;
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	return line;
}

//=================================================================================
bool CEmojiManager::loadTable(const string &filename, bool required)
{
	string path, buffer;
	if (!readEmojiFile(filename, required, buffer, path))
		return false;

	uint added = 0, bad = 0;
	string::size_type pos = 0;
	while (pos < buffer.size())
	{
		string line = nextLine(buffer, pos);
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

	// Last: it wants the reverse index to exist, and it corrects it.
	loadPicker(EmojiPickerFile);

	nlinfo("Emoji: %u name(s), %u distinct emoji, %u sequence length(s), %u picker group(s)",
		(uint)_ByName.size(), (uint)_ByUtf8.size(), (uint)_Utf8Lengths.size(),
		(uint)_Groups.size());
}

//=================================================================================
void CEmojiManager::loadPicker(const string &filename)
{
	string path, buffer;
	// Not required: without it the chat is untouched and only the picker is
	// empty, which the picker itself reports.
	if (!readEmojiFile(filename, false, buffer, path))
		return;

	uint kept = 0, unknown = 0, bad = 0;
	string::size_type pos = 0;
	while (pos < buffer.size())
	{
		string line = nextLine(buffer, pos);
		if (line.empty() || line[0] == '#')
			continue;

		// "g \t i18n-key \t English label"  or  "e \t name \t description"
		string::size_type t1 = line.find('\t');
		if (t1 == string::npos) { ++bad; continue; }
		string::size_type t2 = line.find('\t', t1 + 1);
		if (t2 == string::npos) { ++bad; continue; }

		const string kind  = line.substr(0, t1);
		const string field = line.substr(t1 + 1, t2 - t1 - 1);
		const string rest  = line.substr(t2 + 1);

		if (kind == "g")
		{
			CGroup g;
			// The English label from the file is the fallback, so a group is
			// still named when the translation has not landed yet.
			g.Label = (!field.empty() && CI18N::hasTranslation(field)) ? CI18N::get(field) : rest;
			_Groups.push_back(g);
			continue;
		}
		if (kind != "e")
		{
			++bad;
			continue;
		}
		if (_Groups.empty())
		{
			// An emoji before any group line: the file is not what we think.
			++bad;
			continue;
		}

		std::map<string, CEntry>::iterator it = _ByName.find(field);
		if (it == _ByName.end())
		{
			// The table and the picker were generated from different tables.
			++unknown;
			continue;
		}
		CEntry &e = it->second;
		if (e.Texture.empty())
			continue;

		e.Desc = rest;
		_Groups.back().Emoji.push_back(&e);
		// The picker names an emoji the way Zulip does, so let the whole client
		// do the same: a pasted emoji now reads ":upside_down:" on hover rather
		// than the first alias alphabetically, ":oops:".
		if (!e.Utf8.empty())
			_ByUtf8[e.Utf8] = &e;
		++kept;
	}

	// Drop groups nothing survived in, so the picker has no empty tabs.
	for (std::vector<CGroup>::iterator it = _Groups.begin(); it != _Groups.end();)
		it = it->Emoji.empty() ? _Groups.erase(it) : it + 1;

	if (bad)
		nlwarning("Emoji: %u malformed line(s) in '%s'", bad, path.c_str());
	if (unknown)
		nlwarning("Emoji: %u emoji in '%s' are not in the name table; regenerate it",
			unknown, path.c_str());
	nlinfo("Emoji: picker has %u emoji in %u group(s)", kept, (uint)_Groups.size());
}

//=================================================================================
const CEmojiManager::CEntry *CEmojiManager::find(const string &name) const
{
	std::map<string, CEntry>::const_iterator it = _ByName.find(name);
	return it == _ByName.end() ? NULL : &it->second;
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
