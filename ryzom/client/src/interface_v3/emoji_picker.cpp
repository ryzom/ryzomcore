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

#include "emoji_picker.h"
#include "emoji_manager.h"

#include "nel/gui/action_handler.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/ctrl_scroll.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_list.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/i18n.h"

using namespace std;
using namespace NLMISC;
using namespace NLGUI;

// The window lives in interaction.xml; these are the parts this file fills.
#define EMOJI_PICKER_WIN	"ui:interface:emoji_picker"
#define EMOJI_PICKER_TABS	EMOJI_PICKER_WIN ":content:tabs"
// "text_list" and "scroll_bar" are the names scroll_text binds together.
#define EMOJI_PICKER_GRID	EMOJI_PICKER_WIN ":content:grid_area:text_list"
#define EMOJI_PICKER_SCROLL	EMOJI_PICKER_WIN ":content:grid_area:scroll_bar"
#define EMOJI_PICKER_FILTER	EMOJI_PICKER_WIN ":content:filter:eb"

namespace
{
	// The atlas tiles are 32px. How many fit on a row is measured from the
	// grid rather than assumed: the window's borders, its inner margins and the
	// scrollbar all eat into the width, and guessing that arithmetic is what
	// clipped the last column. The button is a little smaller than its cell so
	// the grid has gaps rather than a solid sheet of emoji.
	const sint32 CellSize = 32;
	const sint32 FallbackColumns = 8;
	const sint32 TabSize  = 28;

	// A filter that matches half the table would build thousands of controls
	// for a list nobody is going to read to the end.
	const uint MaxFilterHits = 400;

	// Dimmed until the pointer is on it, which is the only hover feedback a
	// button whose face is a picture can give.
	const CRGBA EmojiIdle(255, 255, 255, 190);
	const CRGBA EmojiLit(255, 255, 255, 255);

	CInterfaceGroup *getGroupFromId(const char *id)
	{
		return dynamic_cast<CInterfaceGroup *>(
			CWidgetManager::getInstance()->getElementFromId(id));
	}

	/** The chat input the picker should type into, found from the button that
	  * opened it: every chat window holds its edit box as "ebw:eb" somewhere
	  * above the button, whether it is the main chat, a tell or a channel.
	  */
	CGroupEditBox *findChatEditBox(CCtrlBase *caller)
	{
		CInterfaceGroup *g = caller ? caller->getParent() : NULL;
		while (g)
		{
			CInterfaceGroup *ebw = g->getGroup("ebw");
			if (ebw)
			{
				CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(ebw->getGroup("eb"));
				if (eb)
					return eb;
			}
			g = g->getParent();
		}
		return NULL;
	}

	/** Is this edit box a chat input?
	  *
	  * Every chat window, from the main one to a tell, holds its input as "ebw"
	  * beside the chat box itself, "cb". Asking for that shape is what keeps a
	  * pick out of the friend list's search box or the picker's own filter.
	  */
	bool isChatEditBox(CGroupEditBox *eb)
	{
		if (!eb)
			return false;
		CInterfaceGroup *ebw = eb->getParent();
		if (!ebw)
			return false;
		CInterfaceGroup *content = ebw->getParent();
		return content != NULL && content->getGroup("cb") != NULL;
	}

	/// ":fire: fire" -- what the name is to type, then what the emoji is.
	string tooltipFor(const CEmojiManager::CEntry *e)
	{
		string help = ":" + e->Name + ":";
		if (!e->Desc.empty())
			help += "  " + e->Desc;
		return help;
	}

	CCtrlButton *createEmojiButton(CInterfaceGroup *parent, const string &id,
		const string &texture, sint32 size)
	{
		CCtrlButton *b = new CCtrlButton(CViewBase::TCtorParam());
		b->setId(id);
		b->setParent(parent);
		b->setParentPos(NULL);
		b->setPosRef(Hotspot_TL);
		b->setParentPosRef(Hotspot_TL);
		b->setTexture(texture);
		b->setTexturePushed(texture);
		b->setTextureOver(texture);
		b->setScale(true);
		b->setW(size);
		b->setH(size);
		// The tiles carry their own colours and must not be tinted by the
		// interface colour, the same as emoji in a chat line.
		b->setModulateGlobalColorAll(false);
		b->setColor(EmojiIdle);
		b->setColorOver(EmojiLit);
		b->setColorPushed(EmojiLit);
		return b;
	}
}

//=================================================================================
/** The grid, as a group of its own so that it can notice being resized.
  *
  * How many emoji fit on a row is a property of the window's current width, and
  * the window can be dragged wider at any time. Laying the grid out once and
  * hoping is what left a dead strip down one side; this asks each frame whether
  * the width it was built for is still the width it has.
  */
class CGroupEmojiGrid : public CGroupList
{
public:
	DECLARE_UI_CLASS(CGroupEmojiGrid)

	CGroupEmojiGrid(const TCtorParam &param) : CGroupList(param) {}

	virtual void checkCoords()
	{
		CGroupList::checkCoords();
		// Once a frame, with the window laid out and nothing walking this
		// group's children any more, so they can be thrown away and rebuilt.
		if (getActive())
			CEmojiPicker::getInstance().updateGrid(getWReal());
	}
};

NLMISC_REGISTER_OBJECT(CViewBase, CGroupEmojiGrid, std::string, "emoji_grid");
REGISTER_UI_CLASS(CGroupEmojiGrid)

CEmojiPicker *CEmojiPicker::_Instance = NULL;

//=================================================================================
CEmojiPicker::CEmojiPicker() : _Group(0), _TabsBuilt(false), _Refilling(false),
	_NeedFill(false), _BuiltForW(-1), _OpenedByPlayer(false)
{
}

//=================================================================================
CEmojiPicker &CEmojiPicker::getInstance()
{
	if (!_Instance)
		_Instance = new CEmojiPicker;
	return *_Instance;
}

//=================================================================================
void CEmojiPicker::releaseInstance()
{
	delete _Instance;
	_Instance = NULL;
}

//=================================================================================
CGroupEditBox *CEmojiPicker::getTargetEb() const
{
	// Follow the keyboard, not the button. One window serves every chat, so the
	// player can open it from a tell and then click into the main chat; what
	// they are typing in is where the emoji belongs. Clicking a button does not
	// move the keyboard capture, so it still points at the input they left.
	CGroupEditBox *focused = dynamic_cast<CGroupEditBox *>(
		CWidgetManager::getInstance()->getCaptureKeyboard());
	if (isChatEditBox(focused))
		return focused;

	// Nothing has the keyboard, or something that is not a chat: fall back to
	// the input whose button opened the picker.
	if (_TargetEb.empty())
		return NULL;
	return dynamic_cast<CGroupEditBox *>(
		CWidgetManager::getInstance()->getElementFromId(_TargetEb));
}

//=================================================================================
void CEmojiPicker::toggle(CCtrlBase *caller)
{
	CInterfaceGroup *win = getGroupFromId(EMOJI_PICKER_WIN);
	if (!win)
	{
		nlwarning("Emoji: the picker window is missing from the interface");
		return;
	}

	CGroupEditBox *eb = findChatEditBox(caller);
	if (!eb)
	{
		nlwarning("Emoji: no chat input found next to the picker button");
		return;
	}

	// The same button again closes it; another chat's button aims it there
	// instead of closing, which is what makes one window enough for them all.
	if (win->getActive() && _TargetEb == eb->getId())
	{
		hide();
		return;
	}
	_TargetEb = eb->getId();

	_OpenedByPlayer = true;
	win->setActive(true);
	CWidgetManager::getInstance()->setTopWindow(win);

	// Fill here, with setActive finished and the window laid out. The grid
	// group asks for itself too (and that is what covers a picker the saved
	// interface brings back at login), but this path does not depend on it.
	win->updateCoords();
	buildTabs();
	_NeedFill = true;
	CGroupEmojiGrid *grid = dynamic_cast<CGroupEmojiGrid *>(getGroupFromId(EMOJI_PICKER_GRID));
	updateGrid(grid ? grid->getWReal() : 0);
	// Keyboard focus stays in the chat input on purpose: the picker is a second
	// way to type, not a place to type.
	CWidgetManager::getInstance()->setCaptureKeyboard(eb);
}

//=================================================================================
void CEmojiPicker::hide()
{
	_OpenedByPlayer = false;
	CInterfaceGroup *win = getGroupFromId(EMOJI_PICKER_WIN);
	if (win)
		win->setActive(false);
}

//=================================================================================
void CEmojiPicker::buildTabs()
{
	if (_TabsBuilt)
		return;

	CInterfaceGroup *tabs = getGroupFromId(EMOJI_PICKER_TABS);
	if (!tabs)
		return;

	const std::vector<CEmojiManager::CGroup> &groups = CEmojiManager::getInstance().getGroups();
	for (uint i = 0; i < groups.size(); ++i)
	{
		// The group's first emoji is its icon, which is how it works out that
		// Smileys is a smiley and Flags is a flag: Unicode's order puts the
		// obvious one first.
		const CEmojiManager::CEntry *icon = groups[i].Emoji.front();
		CCtrlButton *b = createEmojiButton(tabs,
			tabs->getId() + ":tab" + toString(i), icon->Texture, TabSize - 6);
		b->setType(CCtrlBaseButton::ToggleButton);
		b->setX((sint32)i * TabSize + 2);
		b->setY(-2);
		b->setDefaultContextHelp(groups[i].Label);
		b->setActionOnLeftClick("emoji_picker_tab");
		b->setParamsOnLeftClick("group=" + toString(i));
		tabs->addCtrl(b);
	}

	_TabsBuilt = true;
	updateTabs();
}

//=================================================================================
void CEmojiPicker::updateTabs()
{
	CInterfaceGroup *tabs = getGroupFromId(EMOJI_PICKER_TABS);
	if (!tabs)
		return;

	const std::vector<CCtrlBase *> &ctrls = tabs->getControls();
	for (uint i = 0; i < ctrls.size(); ++i)
	{
		CCtrlBaseButton *b = dynamic_cast<CCtrlBaseButton *>(ctrls[i]);
		if (b)
			b->setPushed(_Filter.empty() && i == _Group);
	}
}

//=================================================================================
void CEmojiPicker::setGroup(uint group)
{
	const std::vector<CEmojiManager::CGroup> &groups = CEmojiManager::getInstance().getGroups();
	if (group >= groups.size())
		return;
	_Group = group;

	// A tab and a filter cannot both decide what the grid shows, and the tab
	// was just clicked.
	if (!_Filter.empty())
	{
		_Filter.clear();
		CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(
			CWidgetManager::getInstance()->getElementFromId(EMOJI_PICKER_FILTER));
		if (eb)
			eb->setInputString(string());
	}

	_NeedFill = true;
}

//=================================================================================
void CEmojiPicker::setFilter(const string &filter)
{
	string f = toLowerAscii(filter);
	if (f == _Filter)
		return;
	_Filter = f;
	_NeedFill = true;
}

//=================================================================================
void CEmojiPicker::fillGrid(sint32 gridWidth)
{
	CGroupEmojiGrid *grid = dynamic_cast<CGroupEmojiGrid *>(getGroupFromId(EMOJI_PICKER_GRID));
	if (!grid)
	{
		// An unknown group type is silently replaced by a plain group by the
		// parser (interface_parser.cpp:1166), so this is the only place the
		// mistake can be caught.
		CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId(EMOJI_PICKER_GRID);
		nlwarning("Emoji picker: the grid is missing or is a %s, not an emoji_grid",
			e ? e->getClassName().c_str() : "(nothing)");
		return;
	}

	grid->deleteAllChildren();
	// deleteAllChildren does not give the height back: CGroupList adds each
	// child's height to its own and delChild never subtracts it (the code that
	// did is commented out in removeHead). Left alone, the list claims to be as
	// tall as every tab ever shown put together, and the scrollbar lets you
	// scroll that far into nothing.
	grid->setH(0);
	updateTabs();

	// What to show: one tab, or everything the filter matches.
	const std::vector<CEmojiManager::CGroup> &groups = CEmojiManager::getInstance().getGroups();
	std::vector<const CEmojiManager::CEntry *> shown;
	if (_Filter.empty())
	{
		if (_Group < groups.size())
			shown = groups[_Group].Emoji;
	}
	else
	{
		for (uint g = 0; g < groups.size() && shown.size() < MaxFilterHits; ++g)
		{
			const std::vector<const CEmojiManager::CEntry *> &in = groups[g].Emoji;
			for (uint i = 0; i < in.size() && shown.size() < MaxFilterHits; ++i)
			{
				// Both halves of what the tooltip shows are searchable: a
				// player either knows the name or knows the words for it.
				if (toLowerAscii(in[i]->Name).find(_Filter) != string::npos ||
					toLowerAscii(in[i]->Desc).find(_Filter) != string::npos)
					shown.push_back(in[i]);
			}
		}
	}

	// Measured by the caller, not calculated here: how much of the grid is
	// actually visible is the container's borders, the inner margins and the
	// scrollbar together, and adding those up by hand got it wrong twice.
	sint32 columns = gridWidth / CellSize;
	if (columns < 1)
		columns = FallbackColumns;

	nldebug("Emoji picker: grid %d wide, %d columns of %d, %u emoji",
		gridWidth, columns, CellSize, (uint)shown.size());

	CInterfaceGroup *row = NULL;
	for (uint i = 0; i < shown.size(); ++i)
	{
		const sint32 col = (sint32)(i % columns);
		if (col == 0)
		{
			row = new CInterfaceGroup(CViewBase::TCtorParam());
			row->setId(grid->getId() + ":row" + toString(i / columns));
			row->setResizeFromChildH(false);
			row->setW(columns * CellSize);
			row->setH(CellSize);
			grid->addChild(row);
		}

		CCtrlButton *b = createEmojiButton(row,
			row->getId() + ":e" + toString(col), shown[i]->Texture, CellSize - 4);
		b->setX(col * CellSize + 2);
		b->setY(-2);
		b->setDefaultContextHelp(tooltipFor(shown[i]));
		b->setActionOnLeftClick("emoji_pick");
		b->setParamsOnLeftClick("name=" + shown[i]->Name);
		row->addCtrl(b);
	}

	grid->invalidateCoords();

	// Back to the top, whatever the last list was scrolled to.
	CCtrlScroll *scroll = dynamic_cast<CCtrlScroll *>(
		CWidgetManager::getInstance()->getElementFromId(EMOJI_PICKER_SCROLL));
	if (scroll)
		scroll->setTrackPos(0);

	// The one number worth keeping: a grid whose holder has collapsed clips
	// everything away, and the rows are built either way, so this is what
	// tells the two apart.
	nldebug("Emoji picker: grid %dx%d, clipped to %d, %u rows",
		grid->getWReal(), grid->getHReal(), grid->getMaxHReal(),
		(uint)grid->getNumChildren());
}

//=================================================================================
void CEmojiPicker::updateGrid(sint32 gridWidth)
{
	if (_Refilling)
		return;
	// Nothing asked for a new grid and the old one still fits its width.
	if (!_NeedFill && gridWidth == _BuiltForW)
		return;

	_NeedFill = false;
	_BuiltForW = gridWidth;
	_Refilling = true;
	fillGrid(gridWidth);
	_Refilling = false;
}

//=================================================================================
void CEmojiPicker::opened()
{
	// Only a click on a chat's emoji button opens this. The saved interface
	// can also bring a window back at login, and a record written while the
	// window still saved its open state stays in that file until it is next
	// written -- so refuse to come back by ourselves rather than trust it.
	if (!_OpenedByPlayer)
	{
		hide();
		return;
	}

	if (CEmojiManager::getInstance().getGroups().empty())
	{
		nlwarning("Emoji: no picker data loaded, see emoji_picker.txt");
		return;
	}
	buildTabs();
	// Not filled here: this runs inside setActive, with the window still
	// settling into shape, and what it built there did not survive. The grid
	// asks for its own contents on the next frame instead.
	_NeedFill = true;
}

//=================================================================================
void CEmojiPicker::pick(const string &name)
{
	CGroupEditBox *eb = getTargetEb();
	if (!eb)
	{
		// The chat window was closed while the picker stayed open.
		hide();
		return;
	}

	eb->writeString(":" + name + ":");
	// Back to typing: the player picked an emoji in the middle of a sentence.
	CWidgetManager::getInstance()->setCaptureKeyboard(eb);
}

// ***************************************************************************
// Action handlers
// ***************************************************************************

class CHandlerEmojiPickerOpened : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CEmojiPicker::getInstance().opened();
	}
};
REGISTER_ACTION_HANDLER(CHandlerEmojiPickerOpened, "emoji_picker_opened");

class CHandlerEmojiPickerOpen : public IActionHandler
{
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CEmojiPicker::getInstance().toggle(pCaller);
	}
};
REGISTER_ACTION_HANDLER(CHandlerEmojiPickerOpen, "emoji_picker_open");

class CHandlerEmojiPickerTab : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		uint group = 0;
		if (fromString(getParam(Params, "group"), group))
			CEmojiPicker::getInstance().setGroup(group);
	}
};
REGISTER_ACTION_HANDLER(CHandlerEmojiPickerTab, "emoji_picker_tab");

class CHandlerEmojiPickerFilter : public IActionHandler
{
	virtual void execute(CCtrlBase *pCaller, const string &/* Params */)
	{
		CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(pCaller);
		if (eb)
			CEmojiPicker::getInstance().setFilter(eb->getInputString());
	}
};
REGISTER_ACTION_HANDLER(CHandlerEmojiPickerFilter, "emoji_picker_filter");

class CHandlerEmojiPick : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const string &Params)
	{
		const string name = getParam(Params, "name");
		if (!name.empty())
			CEmojiPicker::getInstance().pick(name);
	}
};
REGISTER_ACTION_HANDLER(CHandlerEmojiPick, "emoji_pick");
