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

/** The emoji picker window.
  *
  * The window's frame -- filter box, tab strip, scrolling area -- is in
  * interaction.xml next to the chat it serves. What it holds is built here,
  * because it is one tile per emoji and there are 1883 of them: no XML worth
  * reading could say that, and only the client knows which emoji it can name.
  *
  * One window serves every chat. It remembers which edit box asked for it, so
  * a pick lands in the input the player was typing in, whether that is the
  * main chat, a tell or a dynamic channel.
  */
class CEmojiPicker
{
public:

	static CEmojiPicker &getInstance();
	static void releaseInstance();

	/** Show the picker for the input next to \p caller, or hide it if it is
	  * already showing for that same input. \p caller is the chat window's
	  * emoji button; the edit box is found from it.
	  */
	void toggle(NLGUI::CCtrlBase *caller);

	void hide();

	/// The window became active: build its contents. Also covers the picker
	/// coming back open from the saved interface at login.
	void opened();

	/** Called by the grid once a frame with its current width: fills it when
	  * something asked for new contents, or when the window was resized and
	  * the rows no longer match the width they were built for.
	  */
	void updateGrid(sint32 gridWidth);

	/// Switch to a tab. Clears the filter, since the two fight over the grid.
	void setGroup(uint group);

	/// Called when the filter box changes. An empty filter shows the tab again.
	void setFilter(const std::string &filter);

	/// Insert ":name:" where the player was typing, and give the input focus back.
	void pick(const std::string &name);

private:

	CEmojiPicker();

	static CEmojiPicker *_Instance;

	/// Build the tab strip, once. Does nothing if the emoji table is empty.
	void buildTabs();
	/// Fill the grid from the active tab, or from the filter if there is one.
	void fillGrid(sint32 gridWidth);
	void updateTabs();

	NLGUI::CGroupEditBox *getTargetEb() const;

	/// Id of the edit box a pick goes to. Kept as an id, not a pointer: chat
	/// windows come and go.
	std::string		_TargetEb;
	uint			_Group;
	std::string		_Filter;
	bool			_TabsBuilt;
	/// Keeps a re-layout from starting another one.
	bool			_Refilling;
	/// Something changed what the grid should hold; the grid fills next frame.
	bool			_NeedFill;
	/// Width the current contents were laid out for, -1 when never filled.
	sint32			_BuiltForW;
	/// True only between a click on a chat's emoji button and the next close.
	bool			_OpenedByPlayer;
};

#endif // CL_EMOJI_PICKER_H
