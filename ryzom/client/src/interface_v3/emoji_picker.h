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

#ifndef CL_EMOJI_PICKER_H
#define CL_EMOJI_PICKER_H

#include "nel/misc/types_nl.h"

#include <string>

namespace NLGUI
{
	class CCtrlBase;
	class CGroupEditBox;
}

/** Emoji picker window shared by all chat inputs; its frame is in interaction.xml.
  */
class CEmojiPicker
{
public:

	static CEmojiPicker &getInstance();
	static void releaseInstance();

	// Show the picker for the chat input of the caller's window, or hide it.
	void toggle(NLGUI::CCtrlBase *caller);
	// Show the picker to react to a chat message instead of writing.
	void openForReaction();
	void hide();
	void opened();
	// Called by the grid every frame with its current width.
	void updateGrid(sint32 gridWidth);
	void setGroup(uint group);
	void setFilter(const std::string &filter);
	// Insert ":name:" into the chat input, or send it as reaction.
	void pick(const std::string &name);

private:

	CEmojiPicker();

	static CEmojiPicker *_Instance;

	bool show();
	void buildTabs();
	void fillGrid(sint32 gridWidth);
	void updateTabs();

	NLGUI::CGroupEditBox *getTargetEb() const;

	// Kept as id because chat windows come and go.
	std::string		_TargetEb;
	uint			_Group;
	std::string		_Filter;
	bool			_TabsBuilt;
	bool			_NeedFill;
	sint32			_BuiltForW;
	bool			_OpenedByPlayer;
	bool			_Reacting;
};

#endif // CL_EMOJI_PICKER_H
