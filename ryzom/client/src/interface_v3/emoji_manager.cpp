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
#include "nel/misc/common.h"
#include "nel/misc/i18n.h"
#include "nel/misc/utf_string_view.h"
#include "nel/gui/view_text.h"

#include <algorithm>

using namespace std;
using namespace NLMISC;

CEmojiManager *CEmojiManager::_Instance = NULL;

//=================================================================================
CEmojiManager::CEmojiManager() : _Loaded(false)
{
	for (uint i = 0; i < 256; ++i)
		_CanStartUtf8[i] = false;
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
	delete _Instance;
	_Instance = NULL;
}

//=================================================================================
// Read the tab separated lines of an emoji data file, skipping comments.
static bool readEmojiFile(const string &filename, bool required, vector<vector<string> > &lines)
{
	const string path = CPath::lookup(filename, false, false, false);
	CIFile f;
	string buffer;
	if (path.empty() || !f.open(path) || !f.readAll(buffer))
	{
		if (required)
			nlwarning("Emoji: cannot read '%s'", filename.c_str());
		return false;
	}
	vector<string> rows;
	explode(buffer, string("\n"), rows, true);
	for (uint i = 0; i < rows.size(); ++i)
	{
		string row = rows[i];
		if (row[row.size() - 1] == '\r')
			row.resize(row.size() - 1);
		if (row.empty() || row[0] == '#')
			continue;
		lines.push_back(vector<string>());
		explode(row, string("\t"), lines.back());
	}
	return true;
}

//=================================================================================
void CEmojiManager::loadTable(const string &filename, bool required)
{
	// name, hex codepoints, image stem
	vector<vector<string> > lines;
	if (!readEmojiFile(filename, required, lines))
		return;

	uint bad = 0;
	for (uint i = 0; i < lines.size(); ++i)
	{
		const vector<string> &fields = lines[i];
		string utf8;
		if (fields.size() == 3 && !fields[0].empty() && fields[0].size() <= MaxNameBytes)
		{
			vector<string> codepoints;
			explode(fields[1], string(" "), codepoints, true);
			for (uint j = 0; j < codepoints.size(); ++j)
			{
				uint32 codepoint = 0;
				if (sscanf(codepoints[j].c_str(), "%x", &codepoint) != 1 || codepoint > 0x10FFFF)
				{
					utf8.clear();
					break;
				}
				CUtfStringView::append(utf8, codepoint);
			}
		}
		if (utf8.empty())
		{
			++bad;
			continue;
		}

		CEntry &entry = _ByName[fields[0]];
		entry.Name = fields[0];
		entry.Utf8 = utf8;
		// Atlas UV lists name their tiles .tga, whatever the source images were.
		entry.Texture = fields[2].empty() ? string() : fields[2] + ".tga";
	}

	if (bad)
		nlwarning("Emoji: %u malformed line(s) in '%s'", bad, filename.c_str());
	nlinfo("Emoji: loaded %u entries from '%s'", (uint)_ByName.size(), filename.c_str());
}

//=================================================================================
void CEmojiManager::init()
{
	if (_Loaded)
		return;
	_Loaded = true;

	loadTable("emoji.txt", true);
	// Optional local corrections, loaded second so they win.
	loadTable("emoji_overrides.txt", false);

	// Reverse index for emoji typed or pasted as unicode.
	for (std::map<string, CEntry>::const_iterator it = _ByName.begin(); it != _ByName.end(); ++it)
	{
		const CEntry &entry = it->second;
		if (_ByUtf8.find(entry.Utf8) != _ByUtf8.end())
			continue;
		_ByUtf8[entry.Utf8] = &entry;
		_CanStartUtf8[(unsigned char)entry.Utf8[0]] = true;
		if (std::find(_Utf8Lengths.begin(), _Utf8Lengths.end(), entry.Utf8.size()) == _Utf8Lengths.end())
			_Utf8Lengths.push_back(entry.Utf8.size());
	}
	// Longest first, or a family emoji would match as its first person.
	std::sort(_Utf8Lengths.begin(), _Utf8Lengths.end());
	std::reverse(_Utf8Lengths.begin(), _Utf8Lengths.end());

	loadPicker("emoji_picker.txt");
}

//=================================================================================
void CEmojiManager::loadPicker(const string &filename)
{
	// "g, i18n key, English label" starts a group, "e, name, description" adds an emoji.
	vector<vector<string> > lines;
	if (!readEmojiFile(filename, true, lines))
		return;

	uint bad = 0;
	for (uint i = 0; i < lines.size(); ++i)
	{
		const vector<string> &fields = lines[i];
		if (fields.size() != 3 || (fields[0] != "g" && (fields[0] != "e" || _Groups.empty())))
		{
			++bad;
			continue;
		}
		if (fields[0] == "g")
		{
			CGroup group;
			group.Label = CI18N::hasTranslation(fields[1]) ? CI18N::get(fields[1]) : fields[2];
			_Groups.push_back(group);
			continue;
		}

		std::map<string, CEntry>::iterator it = _ByName.find(fields[1]);
		if (it == _ByName.end())
		{
			++bad;
			continue;
		}
		CEntry &entry = it->second;
		if (entry.Texture.empty())
			continue;
		entry.Desc = fields[2];
		_Groups.back().Emoji.push_back(&entry);
		// Pasted emoji show the name the picker inserts, like Zulip does.
		_ByUtf8[entry.Utf8] = &entry;
	}

	for (std::vector<CGroup>::iterator it = _Groups.begin(); it != _Groups.end();)
		it = it->Emoji.empty() ? _Groups.erase(it) : it + 1;

	if (bad)
		nlwarning("Emoji: %u unusable line(s) in '%s'", bad, filename.c_str());
}

//=================================================================================
bool CEmojiManager::mayContainEmoji(const string &text) const
{
	if (_ByName.empty())
		return false;
	for (string::size_type i = 0; i < text.size(); ++i)
	{
		if (text[i] == ':' || _CanStartUtf8[(unsigned char)text[i]])
			return true;
	}
	return false;
}

//=================================================================================
// Length of the ":name:" at index including both colons, or 0.
static std::string::size_type shortcodeAt(const string &text,
	std::string::size_type index, std::string::size_type to, string &name)
{
	if (index >= to || text[index] != ':')
		return 0;

	const std::string::size_type limit = std::min(to, index + CEmojiManager::MaxNameBytes + 2);
	for (std::string::size_type end = index + 1; end < limit; ++end)
	{
		const unsigned char c = (unsigned char)text[end];
		if (c == ':')
		{
			if (end == index + 1)
				return 0;
			name.assign(text, index + 1, end - index - 1);
			return end - index + 1;
		}
		// Names never hold whitespace, so a colon in prose stops at the next word.
		if (c <= 0x20 || c == 0x7F)
			return 0;
	}
	return 0;
}

//=================================================================================
bool CEmojiManager::matchAt(const string &text, std::string::size_type index,
	std::string::size_type to, std::string::size_type &len, const CEntry *&entry) const
{
	if (index >= to)
		return false;

	if (text[index] == ':')
	{
		string name;
		const std::string::size_type length = shortcodeAt(text, index, to, name);
		std::map<string, CEntry>::const_iterator it = length ? _ByName.find(name) : _ByName.end();
		if (it == _ByName.end())
			return false;
		len = length;
		entry = &it->second;
		return true;
	}

	if (!_CanStartUtf8[(unsigned char)text[index]])
		return false;
	for (uint i = 0; i < _Utf8Lengths.size(); ++i)
	{
		const std::string::size_type length = _Utf8Lengths[i];
		if (index + length > to)
			continue;
		std::map<string, const CEntry *>::const_iterator it = _ByUtf8.find(text.substr(index, length));
		if (it != _ByUtf8.end())
		{
			len = length;
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
		// A tooltip tag holds free text, which must not be cut by a substitution.
		const uint tagLength = NLGUI::CViewText::getFormatTagLength(text, (uint)i);
		if (tagLength)
		{
			out.append(text, i, tagLength);
			i += tagLength;
			continue;
		}

		string name;
		const std::string::size_type length = shortcodeAt(text, i, text.size(), name);
		std::map<string, CEntry>::const_iterator it = length ? _ByName.find(name) : _ByName.end();
		if (it != _ByName.end())
		{
			out += it->second.Utf8;
			i += length;
			continue;
		}
		out += text[i];
		++i;
	}
	return out;
}
