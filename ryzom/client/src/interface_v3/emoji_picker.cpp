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
#include "chat_text_manager.h"

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

#define EMOJI_PICKER_WIN	"ui:interface:emoji_picker"
#define EMOJI_PICKER_TABS	EMOJI_PICKER_WIN ":content:tabs"
#define EMOJI_PICKER_GRID	EMOJI_PICKER_WIN ":content:grid_area:text_list"
#define EMOJI_PICKER_SCROLL	EMOJI_PICKER_WIN ":content:grid_area:scroll_bar"
#define EMOJI_PICKER_FILTER	EMOJI_PICKER_WIN ":content:filter:eb"

namespace
{
	// Atlas tiles are 32px.
	const sint32 CellSize = 32;
	const sint32 FallbackColumns = 8;
	const sint32 TabSize  = 28;
	// Bounds the number of buttons built for a broad filter.
	const uint MaxFilterHits = 400;

	const CRGBA EmojiIdle(255, 255, 255, 190);
	const CRGBA EmojiLit(255, 255, 255, 255);

	CInterfaceGroup *getGroupFromId(const char *id)
	{
		return dynamic_cast<CInterfaceGroup *>(
			CWidgetManager::getInstance()->getElementFromId(id));
	}

	// Every chat window, including tells and channels, holds its input as "ebw:eb".
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
		b->setModulateGlobalColorAll(false);
		b->setColor(EmojiIdle);
		b->setColorOver(EmojiLit);
		b->setColorPushed(EmojiLit);
		return b;
	}
}

//=================================================================================
// Rebuilds its rows when the picker window is resized.
class CGroupEmojiGrid : public CGroupList
{
public:
	DECLARE_UI_CLASS(CGroupEmojiGrid)

	CGroupEmojiGrid(const TCtorParam &param) : CGroupList(param) {}

	virtual void checkCoords()
	{
		CGroupList::checkCoords();
		// Children may only be replaced here, once the layout pass is over.
		if (getActive())
			CEmojiPicker::getInstance().updateGrid(getWReal());
	}
};

NLMISC_REGISTER_OBJECT(CViewBase, CGroupEmojiGrid, std::string, "emoji_grid");
REGISTER_UI_CLASS(CGroupEmojiGrid)

CEmojiPicker *CEmojiPicker::_Instance = NULL;

//=================================================================================
CEmojiPicker::CEmojiPicker() : _Group(0), _TabsBuilt(false),
	_NeedFill(false), _BuiltForW(-1), _OpenedByPlayer(false), _Reacting(false)
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
	// The chat input that has the keyboard wins over the one whose button
	// opened the picker: one window serves every chat.
	CGroupEditBox *focused = dynamic_cast<CGroupEditBox *>(
		CWidgetManager::getInstance()->getCaptureKeyboard());
	if (getChatTextMngr().isChatInput(focused))
		return focused;

	if (_TargetEb.empty())
		return NULL;
	return dynamic_cast<CGroupEditBox *>(
		CWidgetManager::getInstance()->getElementFromId(_TargetEb));
}

//=================================================================================
void CEmojiPicker::toggle(CCtrlBase *caller)
{
	CGroupEditBox *eb = findChatEditBox(caller);
	if (!eb)
	{
		nlwarning("Emoji: no chat input found next to the picker button");
		return;
	}

	CInterfaceGroup *win = getGroupFromId(EMOJI_PICKER_WIN);
	if (win && win->getActive() && !_Reacting && _TargetEb == eb->getId())
	{
		hide();
		return;
	}
	if (!show())
		return;
	_TargetEb = eb->getId();
	_Reacting = false;
	CWidgetManager::getInstance()->setCaptureKeyboard(eb);
}

//=================================================================================
void CEmojiPicker::openForReaction()
{
	_Reacting = true;
	show();
}

//=================================================================================
bool CEmojiPicker::show()
{
	CInterfaceGroup *win = getGroupFromId(EMOJI_PICKER_WIN);
	if (!win)
	{
		nlwarning("Emoji: the picker window is missing from the interface");
		return false;
	}
	if (CEmojiManager::getInstance().getGroups().empty())
		nlwarning("Emoji: no picker data loaded, see emoji_picker.txt");

	_OpenedByPlayer = true;
	win->setActive(true);
	CWidgetManager::getInstance()->setTopWindow(win);

	win->updateCoords();
	buildTabs();
	_NeedFill = true;
	CGroupEmojiGrid *grid = dynamic_cast<CGroupEmojiGrid *>(getGroupFromId(EMOJI_PICKER_GRID));
	updateGrid(grid ? grid->getWReal() : 0);
	return true;
}

//=================================================================================
void CEmojiPicker::hide()
{
	_OpenedByPlayer = false;
	_Reacting = false;
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
		// Unicode lists the most typical emoji of a group first; it is the tab icon.
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
		// The parser replaces an unknown group type by a plain group.
		CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId(EMOJI_PICKER_GRID);
		nlwarning("Emoji picker: the grid is missing or is a %s, not an emoji_grid",
			e ? e->getClassName().c_str() : "(nothing)");
		return;
	}

	grid->deleteAllChildren();
	// CGroupList does not shrink when its children are deleted.
	grid->setH(0);
	updateTabs();

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
				if (toLowerAscii(in[i]->Name).find(_Filter) != string::npos ||
					toLowerAscii(in[i]->Desc).find(_Filter) != string::npos)
					shown.push_back(in[i]);
			}
		}
	}

	sint32 columns = gridWidth / CellSize;
	if (columns < 1)
		columns = FallbackColumns;

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

	CCtrlScroll *scroll = dynamic_cast<CCtrlScroll *>(
		CWidgetManager::getInstance()->getElementFromId(EMOJI_PICKER_SCROLL));
	if (scroll)
		scroll->setTrackPos(0);
}

//=================================================================================
void CEmojiPicker::updateGrid(sint32 gridWidth)
{
	if (!_NeedFill && gridWidth == _BuiltForW)
		return;

	_NeedFill = false;
	_BuiltForW = gridWidth;
	fillGrid(gridWidth);
}

//=================================================================================
void CEmojiPicker::opened()
{
	// The saved interface may reopen the window at login; only the chat button does.
	if (!_OpenedByPlayer)
		hide();
}

//=================================================================================
void CEmojiPicker::pick(const string &name)
{
	if (_Reacting)
	{
		hide();
		getChatTextMngr().react(name);
		return;
	}
	CGroupEditBox *eb = getTargetEb();
	if (!eb)
	{
		hide();
		return;
	}

	eb->writeString(":" + name + ":");
	CWidgetManager::getInstance()->setCaptureKeyboard(eb);
}

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
