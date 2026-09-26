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

#ifndef CL_EMOJI_MANAGER_H
#define CL_EMOJI_MANAGER_H

#include "nel/misc/types_nl.h"

#include <map>
#include <string>
#include <vector>

/** Emoji lookup for the chat.
  *
  * Holds the generated name table (see ryzom/tools/emoji) and answers two
  * questions for the chat renderer: what does ":smile:" mean, and does an emoji
  * start at this byte.
  *
  * Deliberately knows nothing about the interface database or about views. The
  * display settings live on CChatTextManager next to the other chat settings,
  * and building views is the chat renderer's job.
  */
class CEmojiManager
{
public:

	/// One emoji, reachable by name or by its literal unicode form.
	struct CEntry
	{
		/** The name this entry is filed under, with no colons, e.g. "fire".
		  *
		  * Several names share one emoji, and each gets its own entry, so this
		  * is the name that was matched -- ":flame:" keeps "flame". A literal
		  * emoji has no name of its own and resolves through the reverse index,
		  * which keeps the first name in alphabetical order; for the emoji that
		  * have several that is Zulip's canonical one often enough to be worth
		  * showing ("fire", not "lit"). Used for the chat's hover tooltip.
		  */
		std::string		Name;
		/// The emoji as UTF-8, e.g. "\xf0\x9f\x98\x84". Never empty.
		std::string		Utf8;
		/// Texture name for image mode, e.g. "emoji_u1f604.tga". Empty if none.
		std::string		Texture;
		/** Unicode's short name for it, e.g. "face with tears of joy". Only the
		  * emoji the picker lists have one; it is what the picker shows under
		  * the cursor. From emoji_picker.txt, not from the name table.
		  */
		std::string		Desc;
	};

	/** One tab of the emoji picker: a Unicode group, with the emoji it holds
	  * in Unicode's own order. Empty until emoji_picker.txt is loaded.
	  */
	struct CGroup
	{
		/// Translated if the interface has a string for it, English otherwise.
		std::string						Label;
		/// Never empty: a group with nothing we can show is not kept.
		std::vector<const CEntry *>		Emoji;
	};

	static CEmojiManager &getInstance();
	static void releaseInstance();

	/** Load the table. Safe to call more than once; later calls do nothing.
	  * Missing data files are not fatal: the chat simply shows names as text.
	  */
	void init();

	/// True if the table loaded and holds at least one emoji.
	bool isReady() const { return !_ByName.empty(); }

	/** Cheap "is it even worth scanning this" test. A chat line containing
	  * neither a colon nor a byte that can begin an emoji cannot contain one.
	  */
	bool mayContainEmoji(const std::string &text) const;

	/** Replace every known ":name:" with its literal unicode form.
	  *
	  * Used by the unicode display mode, where the whole message stays a single
	  * CViewText. That is the reason this mode cannot break text colour: nothing
	  * is split, so no format tag state has to be carried anywhere.
	  *
	  * Format tags are stepped over, never looked inside.
	  */
	std::string substituteShortcodes(const std::string &text) const;

	/** Is there an emoji at \p index, as either ":name:" or a literal unicode
	  * sequence? On a hit, \p len gets its length in bytes and \p entry the
	  * emoji. \p to bounds the search.
	  *
	  * Used by image mode, which has to know where each emoji begins and ends so
	  * it can cut the line around it.
	  */
	bool matchAt(const std::string &text, std::string::size_type index,
				 std::string::size_type to,
				 std::string::size_type &len, const CEntry *&entry) const;

	/** The picker's tabs, in Unicode's order. Empty if emoji_picker.txt is
	  * missing, which costs the picker and nothing else.
	  */
	const std::vector<CGroup> &getGroups() const { return _Groups; }

	/// The emoji of that name, or NULL. The name carries no colons.
	const CEntry *find(const std::string &name) const;

	/// Longest name we will consider between two colons, in bytes.
	static const uint MaxNameBytes = 64;

private:

	CEmojiManager();
	~CEmojiManager();

	static CEmojiManager *_Instance;

	bool loadTable(const std::string &filename, bool required);
	/** Load the picker layout. Runs after the reverse index is built, because
	  * it also decides which of an emoji's names that index hands out: the
	  * canonical one the picker inserts, rather than the first alphabetically.
	  */
	void loadPicker(const std::string &filename);
	void addEntry(const std::string &name, const std::string &utf8,
				  const std::string &texture);

	/** ":name:" -> entry.
	  *
	  * Keyed by the raw bytes between the colons, with no character class
	  * applied. Zulip ships names whose canonical spelling is not ASCII --
	  * ":pinata:" is only an alias, ":piñata:" is what its picker inserts --
	  * so an [a-z0-9_+-] rule would silently drop them. Comparing bytes is both
	  * simpler and correct, and needs no unicode tables.
	  */
	std::map<std::string, CEntry>			_ByName;

	/// literal UTF-8 emoji -> entry, for emoji typed or pasted directly
	std::map<std::string, const CEntry *>	_ByUtf8;

	/** Byte values that can begin a literal emoji. Lets the scanner reject
	  * ordinary text with one array lookup instead of a hash probe per
	  * position, which matters because this runs on every chat line.
	  */
	bool									_CanStartUtf8[256];

	/** Distinct byte lengths of literal emoji, longest first. Sequences have to
	  * be matched longest-first, or a family emoji would match as its first
	  * person and the rest of the sequence would be left behind as loose glyphs.
	  */
	std::vector<std::string::size_type>		_Utf8Lengths;

	/// The picker's tabs. Holds pointers into _ByName, whose nodes never move.
	std::vector<CGroup>						_Groups;

	bool									_Loaded;
};

#endif // CL_EMOJI_MANAGER_H
