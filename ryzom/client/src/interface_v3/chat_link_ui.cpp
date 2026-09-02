// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
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

#include "chat_link_ui.h"
#include "nel/gui/ctrl_base_button.h"
#include "nel/gui/group_paragraph.h"
#include "action_handler_help.h"
#include "chat_window.h"
#include "dbctrl_sheet.h"
#include "interface_manager.h"
#include "inventory_manager.h"
#include "people_interraction.h"
#include "sphrase_manager.h"

#include "nel/gui/group_editbox.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/algo.h"
#include "nel/misc/i18n.h"
#include "nel/misc/utf_string_view.h"

using namespace NLGUI;
using namespace NLMISC;

namespace CHAT_LINK
{
	namespace
	{
		const char *PhraseLinkDb = "UI:PHRASE_LINK:0";
		std::map<uint, CDBCtrlSheet*> LinkedItemSheets;
		CDBCtrlSheet *LinkedPhraseSheet = NULL;
		CSmartPtr<CSPhraseComAdpater> LinkedPhraseTooltipAdapter;

		void splitActionParams(const std::string &params, std::string &url, std::string &title)
		{
			std::string::size_type separator = params.find('\n');
			url = params.substr(0, separator);
			title = separator == std::string::npos ? std::string() : params.substr(separator + 1);
		}

		bool fitsMessage(const std::string &utf8Text)
		{
			return CUtfStringView(utf8Text).toUtf32().size() <= MaxMessageCharacters;
		}

		CGroupEditBox *chatInputFromControl(CCtrlBase *control)
		{
			CGroupEditBox *editBox = dynamic_cast<CGroupEditBox*>(control);
			if (!editBox)
				return NULL;
			const std::string &handler = editBox->getAHOnEnter();
			if (handler == "contact_entry")
				return editBox;
			CChatWindow *chatWindow = handler == "chat_box_entry" ?
				getChatWndMgr().getChatWindowFromCaller(editBox) : NULL;
			return chatWindow && chatWindow->getEditBox() == editBox ? editBox : NULL;
		}

		CGroupEditBox *targetChatInput()
		{
			CWidgetManager *widgets = CWidgetManager::getInstance();
			CGroupEditBox *editBox = chatInputFromControl(widgets->getCaptureKeyboard());
			if (!editBox)
				editBox = chatInputFromControl(widgets->getOldCaptureKeyboard());
			if (!editBox && PeopleInterraction.ChatGroup.Window)
				editBox = PeopleInterraction.ChatGroup.Window->getEditBox();
			return editBox;
		}

		void updateLinkTooltip(CCtrlBase *caller, const std::string &windowName, const std::string &text)
		{
			CCtrlLink *link = dynamic_cast<CCtrlLink*>(caller);
			if (link)
				link->setContextHelpWindowName(windowName);
			CWidgetManager::getInstance()->getContextHelpText() = text;
		}

		void setItemLinkDbValue(const std::string &dbPath, const std::string &leaf, sint64 value)
		{
			CDBManager::getInstance()->getDbProp(dbPath + ":" + leaf)->setValue64(value);
		}

		CDBCtrlSheet *prepareLinkedItemSheet(CItemSnapshot &snapshot)
		{
			const uint32 slotId = getInventory().createItemLinkInfo(snapshot.Info);
			if (slotId == 0)
				return NULL;
			const uint previewSlot = slotId & CItemInfos::SlotIdIndexBitMask;
			const std::string dbPath = NLMISC::toString("UI:ITEM_LINK:%u", previewSlot);

			setItemLinkDbValue(dbPath, "SHEET", snapshot.SheetId);
			setItemLinkDbValue(dbPath, "QUALITY", snapshot.Quality);
			setItemLinkDbValue(dbPath, "QUANTITY", snapshot.Quantity);
			setItemLinkDbValue(dbPath, "WEIGHT", snapshot.Weight);
			setItemLinkDbValue(dbPath, "USER_COLOR", snapshot.UserColor);
			setItemLinkDbValue(dbPath, "NAMEID", 0);
			setItemLinkDbValue(dbPath, "INFO_VERSION", snapshot.Info.versionInfo);
			setItemLinkDbValue(dbPath, "ENCHANT", snapshot.Enchant);
			setItemLinkDbValue(dbPath, "RM_CLASS_TYPE", snapshot.RMClassType);
			setItemLinkDbValue(dbPath, "RM_FABER_STAT_TYPE", snapshot.RMFaberStatType);
			setItemLinkDbValue(dbPath, "PREREQUISIT_VALID", 1);
			setItemLinkDbValue(dbPath, "CREATE_TIME", 0);
			setItemLinkDbValue(dbPath, "SERIAL", 0);
			setItemLinkDbValue(dbPath, "WORNED", 0);

			CDBCtrlSheet *&linkedItemSheet = LinkedItemSheets[previewSlot];
			if (!linkedItemSheet)
			{
				linkedItemSheet = new CDBCtrlSheet(CViewBase::TCtorParam());
				linkedItemSheet->setId(NLMISC::toString("item_chat_link_preview_%u", previewSlot));
				linkedItemSheet->setType(CCtrlSheetInfo::SheetType_Item);
				linkedItemSheet->setSheet(dbPath);
			}

			linkedItemSheet->setItemActualNameOverride(snapshot.Name);
			return linkedItemSheet;
		}

		CDBCtrlSheet *prepareLinkedPhraseSheet(const CSPhraseCom &phrase)
		{
			if (phrase.Bricks.empty())
				return NULL;

			CDBManager::getInstance()->getDbProp(std::string(PhraseLinkDb) + ":SHEET")
				->setValue32(phrase.Bricks[0].asInt());

			if (!LinkedPhraseSheet)
			{
				LinkedPhraseSheet = new CDBCtrlSheet(CViewBase::TCtorParam());
				LinkedPhraseSheet->setId("phrase_chat_link_preview");
				LinkedPhraseSheet->setType(CCtrlSheetInfo::SheetType_SBrick);
				LinkedPhraseSheet->setSheet(PhraseLinkDb);
			}
			return LinkedPhraseSheet;
		}

		class CHandlerOpenItemChatLink : public IActionHandler
		{
		public:
			virtual void execute(CCtrlBase * /* caller */, const std::string &params)
			{
				std::string url;
				std::string title;
				splitActionParams(params, url, title);
				CItemSnapshot snapshot;
				if (!decodeItemSnapshot(url, snapshot))
				{
					CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiChatLinkInvalid"));
					return;
				}
				snapshot.Name = title.empty() ? CI18N::get("uiChatLinkDefaultItemName") : title;

				CDBCtrlSheet *linkedItemSheet = prepareLinkedItemSheet(snapshot);
				if (!linkedItemSheet || !linkedItemSheet->asItemSheet())
				{
					if (linkedItemSheet)
						getInventory().removeItemLinkInfo(getInventory().getItemSlotId(linkedItemSheet));
					CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiChatLinkInvalid"));
					return;
				}

				CAHManager::getInstance()->runActionHandler("open_item_help", linkedItemSheet,
					"force_keep=0|reuse_same_aspect=0|prefer_new=1");
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerOpenItemChatLink, "open_item_chat_link");

		class CHandlerOpenPhraseChatLink : public IActionHandler
		{
		public:
			virtual void execute(CCtrlBase * /* caller */, const std::string &params)
			{
				std::string url;
				std::string title;
				splitActionParams(params, url, title);
				CSPhraseCom phrase;
				if (!decodePhrase(url, phrase))
				{
					CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiChatLinkInvalid"));
					return;
				}
				phrase.Name.fromUtf8(title.empty() ? CI18N::get("uiAction") : title);

				CDBCtrlSheet *linkedPhraseSheet = prepareLinkedPhraseSheet(phrase);
				if (!linkedPhraseSheet || !linkedPhraseSheet->asSBrickSheet())
				{
					CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiChatLinkInvalid"));
					return;
				}

				openSabrinaPhraseHelp(linkedPhraseSheet, phrase);
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerOpenPhraseChatLink, "open_phrase_chat_link");

		class CHandlerItemChatLinkTooltip : public IActionHandler
		{
		public:
			virtual void execute(CCtrlBase *caller, const std::string & /* params */)
			{
				CCtrlBaseButton *button = dynamic_cast<CCtrlBaseButton*>(caller);
				if (!button)
					return;
				std::string url;
				std::string title;
				splitActionParams(button->getParamsOnLeftClick(), url, title);
				CItemSnapshot snapshot;
				if (!decodeItemSnapshot(url, snapshot))
				{
					updateLinkTooltip(caller, "context_help", CI18N::get("uiChatLinkInvalid"));
					return;
				}
				snapshot.Name = title.empty() ? CI18N::get("uiChatLinkDefaultItemName") : title;
				CDBCtrlSheet *sheet = prepareLinkedItemSheet(snapshot);
				if (!sheet || !sheet->asItemSheet())
				{
					if (sheet)
						getInventory().removeItemLinkInfo(getInventory().getItemSlotId(sheet));
					updateLinkTooltip(caller, "context_help", CI18N::get("uiChatLinkInvalid"));
					return;
				}

				std::string windowName = sheet->getContextHelpWindowName();
				std::string tooltip = snapshot.Name;
				if (windowName == "buff_item_context_help" || windowName == "crystallized_spell_context_help")
				{
					CControlSheetInfoWaiter waiter;
					waiter.CtrlSheet = sheet;
					waiter.LuaMethodName = windowName == "buff_item_context_help" ?
						"updateBuffItemTooltip" : "updateCrystallizedSpellTooltip";
					tooltip = waiter.infoValidated();
					if (tooltip.empty())
						tooltip = snapshot.Name;
				}
				updateLinkTooltip(caller, windowName, tooltip);
				getInventory().removeItemLinkInfo(getInventory().getItemSlotId(sheet));
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerItemChatLinkTooltip, "item_chat_link_tooltip");

		class CHandlerPhraseChatLinkTooltip : public IActionHandler
		{
		public:
			virtual void execute(CCtrlBase *caller, const std::string & /* params */)
			{
				CCtrlBaseButton *button = dynamic_cast<CCtrlBaseButton*>(caller);
				if (!button)
					return;
				std::string url;
				std::string title;
				splitActionParams(button->getParamsOnLeftClick(), url, title);
				CSPhraseCom phrase;
				if (!decodePhrase(url, phrase))
				{
					updateLinkTooltip(caller, "context_help", CI18N::get("uiChatLinkInvalid"));
					return;
				}
				phrase.Name.fromUtf8(title.empty() ? CI18N::get("uiAction") : title);
				LinkedPhraseTooltipAdapter = new CSPhraseComAdpater;
				LinkedPhraseTooltipAdapter->Phrase = phrase;
				std::string tooltip = LinkedPhraseTooltipAdapter->updateTooltip();
				if (tooltip.empty())
					tooltip = phrase.Name.toUtf8();
				updateLinkTooltip(caller, "action_context_help", tooltip);
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerPhraseChatLinkTooltip, "phrase_chat_link_tooltip");
	}

	TInsertResult insertIntoChat(const std::string &marker)
	{
		CGroupEditBox *editBox = targetChatInput();
		if (!editBox)
			return InsertNoChat;

		::u32string input = editBox->getInputStringRef();
		::u32string insertion = CUtfStringView(marker).toUtf32();
		sint32 cursor = editBox->getCursorPos();
		NLMISC::clamp(cursor, sint32(0), sint32(input.size()));
		if (cursor > 0 && input[cursor - 1] != (u32char)' ')
			insertion.insert(insertion.begin(), (u32char)' ');
		if (cursor < (sint32)input.size() && input[cursor] != (u32char)' ')
			insertion.push_back((u32char)' ');
		if (input.size() + insertion.size() > MaxMessageCharacters)
			return InsertInputFull;

		input.insert(cursor, insertion);
		editBox->setInputStringRef(input);
		editBox->setCursorPos(cursor + (sint32)insertion.size());
		editBox->stopParentBlink();
		CWidgetManager::getInstance()->setCaptureKeyboard(editBox);
		editBox->bypassNextKey();
		return InsertOk;
	}

	bool captureItemSnapshot(CDBCtrlSheet *item, uint32 slotId, CItemSnapshot &snapshot)
	{
		if (!item || item->getType() != CCtrlSheetInfo::SheetType_Item || item->getSheetId() == 0)
			return false;
		snapshot.SheetId = item->getSheetId();
		snapshot.Quality = item->getQuality();
		snapshot.Quantity = item->getQuantity();
		snapshot.Weight = item->getItemWeight();
		snapshot.UserColor = item->getItemColor();
		snapshot.Enchant = item->getEnchant();
		snapshot.RMClassType = item->getItemRMClassType();
		snapshot.RMFaberStatType = item->getItemRMFaberStatType();
		snapshot.Name = item->getItemActualName();
		snapshot.Info = getInventory().getItemInfo(slotId);
		if (snapshot.Name.empty())
			snapshot.Name = CI18N::get("uiChatLinkDefaultItemName");
		return true;
	}

	std::string createItemMarker(CItemSnapshot snapshot, bool *customTextOmitted)
	{
		if (customTextOmitted)
			*customTextOmitted = false;
		std::string url;
		if (encodeItemSnapshot(snapshot, url))
		{
			std::string link = makeVisibleLink(snapshot.Name, url);
			if (fitsMessage(link))
				return link;
		}

		if (!snapshot.Info.CustomText.empty())
		{
			snapshot.Info.CustomText.clear();
			url.clear();
			if (encodeItemSnapshot(snapshot, url))
			{
				std::string link = makeVisibleLink(snapshot.Name, url);
				if (fitsMessage(link))
				{
					if (customTextOmitted)
						*customTextOmitted = true;
					return link;
				}
			}
		}
		return std::string();
	}

	std::string createPhraseMarker(const CSPhraseCom &phrase)
	{
		if (phrase.empty())
			return std::string();

		CSPhraseCom linkPhrase = phrase;
		if (linkPhrase.Name.empty())
			linkPhrase.Name.fromUtf8(CI18N::get("uiAction"));

		std::string url;
		if (!encodePhrase(linkPhrase, url))
			return std::string();
		std::string link = makeVisibleLink(linkPhrase.Name.toUtf8(), url);
		return fitsMessage(link) ? link : std::string();
	}

	void releasePreviewSheets()
	{
		for (std::map<uint, CDBCtrlSheet*>::iterator it = LinkedItemSheets.begin(); it != LinkedItemSheets.end(); ++it)
		{
			getInventory().removeItemLinkInfo(getInventory().getItemSlotId(it->second));
			delete it->second;
		}
		LinkedItemSheets.clear();
		delete LinkedPhraseSheet;
		LinkedPhraseSheet = NULL;
		LinkedPhraseTooltipAdapter = NULL;
	}
}
