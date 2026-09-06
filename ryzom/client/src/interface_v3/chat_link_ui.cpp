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
#include "action_handler_help.h"
#include "chat_text_manager.h"
#include "chat_window.h"
#include "dbctrl_sheet.h"
#include "interface_manager.h"
#include "inventory_manager.h"
#include "people_interraction.h"
#include "sphrase_manager.h"
#include "../string_manager_client.h"

#include "nel/gui/db_manager.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_paragraph.h"
#include "nel/gui/view_link.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/algo.h"
#include "nel/misc/i18n.h"
#include "nel/misc/utf_string_view.h"

using namespace NLGUI;
using namespace NLMISC;

namespace CHAT_SHARE
{
	namespace
	{
		class CAttachmentView : public CViewLink
		{
		public:
			CAttachmentView(const TCtorParam &param) : CViewLink(param) {}
			CChatMessagePart Part;
		};

		const char *PhraseLinkDb = "UI:PHRASE_LINK:0";
		// Help windows store raw sheet pointers, keep previews alive until interface release.
		std::map<uint, CDBCtrlSheet*> LinkedItemSheets;
		CDBCtrlSheet *LinkedPhraseSheet = NULL;
		const CChatMessageRequest *CurrentRequest = NULL;

		enum TParseResult
		{
			ParseOk,
			ParseInvalidToken,
			ParseTooLong
		};

		void displayRequestError()
		{
			CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiChatLinkDoesNotFit"));
		}

		CGroupEditBox *chatInputFromControl(CCtrlBase *control)
		{
			CGroupEditBox *editBox = dynamic_cast<CGroupEditBox*>(control);
			return isChatInput(editBox) ? editBox : NULL;
		}

		CGroupEditBox *targetChatInput(const std::string &destination)
		{
			if (destination == "main")
				return PeopleInterraction.ChatGroup.Window ?
					PeopleInterraction.ChatGroup.Window->getEditBox() : NULL;
			if (destination != "last")
				return NULL;

			CWidgetManager *widgets = CWidgetManager::getInstance();
			CGroupEditBox *editBox = chatInputFromControl(widgets->getCaptureKeyboard());
			if (!editBox)
				editBox = chatInputFromControl(widgets->getOldCaptureKeyboard());
			if (!editBox && PeopleInterraction.ChatGroup.Window)
				editBox = PeopleInterraction.ChatGroup.Window->getEditBox();
			return editBox;
		}

		bool buildRawRequest(const CGroupEditBox *editBox, CChatMessageRequest &request)
		{
			request.Text.clear();
			request.References.clear();
			if (!editBox || editBox->getTextTags().empty())
				return false;

			const ::u32string &input = editBox->getInputStringRef();
			request.Text = CUtfStringView(input).toUtf16();
			if (request.Text.size() > CHAT_MESSAGE::MaxTextLength ||
				editBox->getTextTags().size() > CHAT_MESSAGE::MaxReferences)
				return false;

			const std::vector<CGroupEditBox::CTextTag> &tags = editBox->getTextTags();
			for (std::vector<CGroupEditBox::CTextTag>::const_iterator it = tags.begin(); it != tags.end(); ++it)
			{
				if (it->Start > input.size() || it->Length > input.size() - it->Start)
					return false;
				const ucstring prefix = CUtfStringView(input.substr(0, it->Start)).toUtf16();
				const ucstring title = CUtfStringView(input.substr(it->Start, it->Length)).toUtf16();
				if (title.empty())
					return false;
				CChatMessageReference reference;
				reference.Start = (uint16)prefix.size();
				reference.Length = (uint16)title.size();
				reference.Type = (CChatMessageReference::TType)it->Type;
				reference.Value = it->Value;
				request.References.push_back(reference);
			}
			return request.isValid();
		}

		TParseResult appendParsedText(const ucstring &text, ucstring &result)
		{
			std::string parsed = text.toUtf8();
			if (!CInterfaceManager::parseTokens(parsed))
				return ParseInvalidToken;
			const ucstring parsedText = CUtfStringView(parsed).toUtf16();
			if (parsedText.size() > CHAT_MESSAGE::MaxTextLength - result.size())
				return ParseTooLong;
			result += parsedText;
			return ParseOk;
		}

		TParseResult parseRequestTokens(CChatMessageRequest &request)
		{
			if (!request.isValid())
				return ParseTooLong;

			const ucstring source = request.Text;
			ucstring parsed;
			uint32 sourcePosition = 0;
			for (std::vector<CChatMessageReference>::iterator it = request.References.begin();
				it != request.References.end(); ++it)
			{
				const uint32 referenceStart = it->Start;
				const TParseResult result = appendParsedText(
					source.substr(sourcePosition, referenceStart - sourcePosition), parsed);
				if (result != ParseOk)
					return result;
				it->Start = (uint16)parsed.size();
				if (it->Length > CHAT_MESSAGE::MaxTextLength - parsed.size())
					return ParseTooLong;
				parsed += source.substr(referenceStart, it->Length);
				sourcePosition = referenceStart + it->Length;
			}
			const TParseResult result = appendParsedText(source.substr(sourcePosition), parsed);
			if (result != ParseOk)
				return result;
			request.Text.swap(parsed);
			return request.isValid() ? ParseOk : ParseTooLong;
		}

		bool isCommandSpace(ucchar character)
		{
			return character == (ucchar)' ' || character == (ucchar)'\t' ||
				character == (ucchar)'\n' || character == (ucchar)'\r';
		}

		void normalizeCommandText(const CChatMessageRequest &source, ucstring::size_type start,
			ucstring &text, std::vector<uint16> &positions)
		{
			const uint16 invalidPosition = CHAT_MESSAGE::MaxTextLength + 1;
			positions.assign(source.Text.size() + 1, invalidPosition);
			text.clear();
			positions[start] = 0;
			std::vector<CChatMessageReference>::const_iterator reference = source.References.begin();
			ucstring::size_type position = start;
			while (position < source.Text.size())
			{
				if (reference != source.References.end() && position == reference->Start)
				{
					const ucstring::size_type referenceEnd = reference->Start + reference->Length;
					while (position < referenceEnd)
					{
						text += source.Text[position++];
						positions[position] = (uint16)text.size();
					}
					++reference;
					continue;
				}
				if (!isCommandSpace(source.Text[position]))
				{
					text += source.Text[position++];
					positions[position] = (uint16)text.size();
					continue;
				}

				const ucstring::size_type spaceStart = position;
				const ucstring::size_type nextReference = reference == source.References.end() ?
					source.Text.size() : reference->Start;
				while (position < nextReference && isCommandSpace(source.Text[position]))
					++position;
				if (!text.empty() && position < source.Text.size())
					text += (ucchar)' ';
				for (ucstring::size_type i = spaceStart + 1; i <= position; ++i)
					positions[i] = (uint16)text.size();
			}
		}

		bool selectCommandText(const CChatMessageRequest &source, uint32 argumentsBeforeText,
			CChatMessageRequest &request)
		{
			if (!source.isValid() || source.Text.empty() || source.Text[0] != (ucchar)'/')
				return false;

			ucstring::size_type position = 1;
			for (uint32 i = 0; i < argumentsBeforeText; ++i)
			{
				while (position < source.Text.size() && isCommandSpace(source.Text[position]))
					++position;
				if (position == source.Text.size())
					return false;
				while (position < source.Text.size() && !isCommandSpace(source.Text[position]))
					++position;
			}

			const ucstring::size_type separatorStart = position;
			while (position < source.Text.size() && isCommandSpace(source.Text[position]))
				++position;
			ucstring::size_type textStart = position;
			for (std::vector<CChatMessageReference>::const_iterator it = source.References.begin();
				it != source.References.end(); ++it)
			{
				const uint32 referenceEnd = it->Start + it->Length;
				if (it->Start >= separatorStart && it->Start < textStart && referenceEnd > textStart)
				{
					textStart = it->Start;
					break;
				}
			}
			for (std::vector<CChatMessageReference>::const_iterator it = source.References.begin();
				it != source.References.end(); ++it)
			{
				if (it->Start < textStart)
					return false;
			}

			ucstring normalized;
			std::vector<uint16> positions;
			normalizeCommandText(source, textStart, normalized, positions);
			request.Text.swap(normalized);
			request.References.clear();
			for (std::vector<CChatMessageReference>::const_iterator it = source.References.begin();
				it != source.References.end(); ++it)
			{
				const uint32 referenceEnd = it->Start + it->Length;
				if (it->Start < textStart || referenceEnd >= positions.size() ||
					positions[it->Start] > CHAT_MESSAGE::MaxTextLength ||
					positions[referenceEnd] <= positions[it->Start])
					return false;
				CChatMessageReference reference = *it;
				reference.Start = positions[it->Start];
				reference.Length = positions[referenceEnd] - reference.Start;
				request.References.push_back(reference);
			}
			return request.isValid();
		}

		void setItemLinkDbValue(const std::string &dbPath, const std::string &leaf, sint64 value)
		{
			CDBManager::getInstance()->getDbProp(dbPath + ":" + leaf)->setValue64(value);
		}

		CDBCtrlSheet *prepareItem(const CChatMessageItem &item)
		{
			if (item.SheetId == CSheetId::Unknown)
				return NULL;
			const uint32 slotId = getInventory().createItemLinkInfo(item.Info);
			if (slotId == 0)
				return NULL;
			const uint previewSlot = slotId & CItemInfos::SlotIdIndexBitMask;
			const std::string dbPath = NLMISC::toString("UI:ITEM_LINK:%u", previewSlot);

			setItemLinkDbValue(dbPath, "SHEET", item.SheetId.asInt());
			setItemLinkDbValue(dbPath, "QUALITY", item.Quality);
			setItemLinkDbValue(dbPath, "QUANTITY", item.Quantity);
			setItemLinkDbValue(dbPath, "WEIGHT", item.Weight);
			setItemLinkDbValue(dbPath, "USER_COLOR", item.UserColor);
			setItemLinkDbValue(dbPath, "NAMEID", item.NameId);
			setItemLinkDbValue(dbPath, "INFO_VERSION", item.Info.versionInfo);
			setItemLinkDbValue(dbPath, "ENCHANT", item.Enchant);
			setItemLinkDbValue(dbPath, "RM_CLASS_TYPE", item.RMClassType);
			setItemLinkDbValue(dbPath, "RM_FABER_STAT_TYPE", item.RMFaberStatType);
			setItemLinkDbValue(dbPath, "PREREQUISIT_VALID", 1);
			setItemLinkDbValue(dbPath, "CREATE_TIME", 0);
			setItemLinkDbValue(dbPath, "SERIAL", 0);
			setItemLinkDbValue(dbPath, "WORNED", 0);

			CDBCtrlSheet *&sheet = LinkedItemSheets[previewSlot];
			if (!sheet)
			{
				sheet = new CDBCtrlSheet(CViewBase::TCtorParam());
				sheet->setId(NLMISC::toString("item_chat_link_preview_%u", previewSlot));
				sheet->setType(CCtrlSheetInfo::SheetType_Item);
				sheet->setSheet(dbPath);
			}
			return sheet;
		}

		CSPhraseCom localizedPhrase(const CChatMessagePhrase &messagePhrase)
		{
			CSPhraseCom phrase = messagePhrase.Phrase;
			if (phrase.Bricks.empty() && messagePhrase.SheetId != CSheetId::Unknown)
				CSPhraseManager::getInstance()->buildPhraseFromSheet(phrase, messagePhrase.SheetId.asInt());
			if (phrase.Name.empty() && messagePhrase.SheetId != CSheetId::Unknown)
				phrase.Name.fromUtf8(STRING_MANAGER::CStringManagerClient::getSPhraseLocalizedName(messagePhrase.SheetId));
			return phrase;
		}

		CDBCtrlSheet *preparePhrase(const CSPhraseCom &phrase)
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

		CAttachmentView *attachmentFromCaller(CCtrlBase *caller)
		{
			CCtrlLink *link = dynamic_cast<CCtrlLink*>(caller);
			return link ? dynamic_cast<CAttachmentView*>(link->getLinkView()) : NULL;
		}

		void updateLinkTooltip(CCtrlBase *caller, const std::string &windowName, const std::string &text)
		{
			CCtrlLink *link = dynamic_cast<CCtrlLink*>(caller);
			if (link)
				link->setContextHelpWindowName(windowName);
			CWidgetManager::getInstance()->getContextHelpText() = text;
		}

		class CHandlerOpenChatAttachment : public IActionHandler
		{
		public:
			virtual void execute(CCtrlBase *caller, const std::string & /* params */)
			{
				CAttachmentView *view = attachmentFromCaller(caller);
				if (!view)
					return;
				if (view->Part.Type == CChatMessagePart::Item)
				{
					CDBCtrlSheet *sheet = prepareItem(view->Part.ItemValue);
					if (sheet && sheet->asItemSheet())
						CAHManager::getInstance()->runActionHandler("open_item_help", sheet,
							"force_keep=0|reuse_same_aspect=0|prefer_new=1");
					else if (sheet)
						getInventory().removeItemLinkInfo(getInventory().getItemSlotId(sheet));
				}
				else if (view->Part.Type == CChatMessagePart::Phrase)
				{
					CSPhraseCom phrase = localizedPhrase(view->Part.PhraseValue);
					CDBCtrlSheet *sheet = preparePhrase(phrase);
					if (sheet && sheet->asSBrickSheet())
						openSabrinaPhraseHelp(sheet, phrase);
				}
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerOpenChatAttachment, "open_chat_attachment");

		class CHandlerChatAttachmentTooltip : public IActionHandler
		{
		public:
			virtual void execute(CCtrlBase *caller, const std::string & /* params */)
			{
				CAttachmentView *view = attachmentFromCaller(caller);
				if (!view)
					return;
				if (view->Part.Type == CChatMessagePart::Item)
				{
					CDBCtrlSheet *sheet = prepareItem(view->Part.ItemValue);
					if (!sheet || !sheet->asItemSheet())
					{
						if (sheet)
							getInventory().removeItemLinkInfo(getInventory().getItemSlotId(sheet));
						return;
					}
					const uint32 slotId = getInventory().getItemSlotId(sheet);
					std::string tooltip;
					sheet->getContextHelpToolTip(tooltip);
					if (tooltip.empty())
						tooltip = sheet->getItemActualName();
					updateLinkTooltip(caller, sheet->getContextHelpWindowName(), tooltip);
					getInventory().removeItemLinkInfo(slotId);
				}
				else if (view->Part.Type == CChatMessagePart::Phrase)
				{
					CSmartPtr<CSPhraseComAdpater> adapter = new CSPhraseComAdpater;
					adapter->Phrase = localizedPhrase(view->Part.PhraseValue);
					updateLinkTooltip(caller, "action_context_help", adapter->updateTooltip());
				}
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerChatAttachmentTooltip, "chat_attachment_tooltip");
	}

	CRequestScope::CRequestScope(const CChatMessageRequest *request)
	: _Previous(CurrentRequest)
	{
		CurrentRequest = request;
	}

	CRequestScope::~CRequestScope()
	{
		CurrentRequest = _Previous;
	}

	NLMISC::CRGBA itemColor()
	{
		return CRGBA(255, 205, 80, 255);
	}

	NLMISC::CRGBA phraseColor()
	{
		return CRGBA(110, 205, 255, 255);
	}

	bool isChatInput(CGroupEditBox *editBox)
	{
		if (!editBox)
			return false;
		const std::string &handler = editBox->getAHOnEnter();
		if (handler == "contact_entry")
			return true;
		CChatWindow *chatWindow = handler == "chat_box_entry" ?
			getChatWndMgr().getChatWindowFromCaller(editBox) : NULL;
		return chatWindow && chatWindow->getEditBox() == editBox;
	}

	bool hasCurrentRequest()
	{
		return CurrentRequest != NULL;
	}

	TShareResult share(const std::string &name, CChatMessageReference::TType type,
		uint32 value, NLMISC::CRGBA color, const std::string &destination)
	{
		if (name.empty())
			return ShareUnavailable;

		const ::u32string title = CUtfStringView(name).toUtf32();
		if (destination == "clipboard")
		{
			CGroupEditBox::CTextTag tag;
			tag.Start = 0;
			tag.Length = (uint32)title.size();
			tag.Type = (uint32)type;
			tag.Value = value;
			tag.Color = color;
			std::vector<CGroupEditBox::CTextTag> textTags;
			textTags.push_back(tag);
			return CGroupEditBox::copyToClipboard(title, textTags) ? ShareOk : ShareUnavailable;
		}

		CGroupEditBox *editBox = targetChatInput(destination);
		if (!editBox)
			return ShareUnavailable;
		if (editBox->getTextTags().size() >= CHAT_MESSAGE::MaxReferences)
			return ShareInputFull;

		const ::u32string &input = editBox->getInputStringRef();
		::u32string insertion = title;
		sint32 cursor = editBox->getCursorPos();
		NLMISC::clamp(cursor, sint32(0), sint32(input.size()));
		uint32 titleStart = (uint32)cursor;
		if (cursor > 0 && input[cursor - 1] != (u32char)' ')
		{
			insertion.insert(insertion.begin(), (u32char)' ');
			++titleStart;
		}
		if (cursor < (sint32)input.size() && input[cursor] != (u32char)' ')
			insertion.push_back((u32char)' ');
		if (CUtfStringView(input).toUtf16().size() + CUtfStringView(insertion).toUtf16().size() >
			CHAT_MESSAGE::MaxTextLength)
			return ShareInputFull;

		editBox->stopParentBlink();
		editBox->setFocusOnText();
		editBox->setCursorPos(cursor);
		CGroupEditBox::setSelectCursorPos(cursor);
		editBox->writeString(CUtfStringView(insertion).toUtf8(), true, false);
		editBox->addTextTag(titleStart, (uint32)title.size(), (uint32)type, value, color);
		const sint32 newCursor = cursor + (sint32)insertion.size();
		editBox->setCursorPos(newCursor);
		CGroupEditBox::setSelectCursorPos(newCursor);
		editBox->bypassNextKey();
		return ShareOk;
	}

	bool buildRequest(const CGroupEditBox *editBox, CChatMessageRequest &request)
	{
		if (CurrentRequest)
		{
			request = *CurrentRequest;
			return true;
		}
		if (!buildRawRequest(editBox, request))
		{
			displayRequestError();
			return false;
		}
		const TParseResult result = parseRequestTokens(request);
		if (result == ParseTooLong)
			displayRequestError();
		return result == ParseOk;
	}

	bool buildCommandRequest(const CGroupEditBox *editBox, uint32 argumentsBeforeText,
		CChatMessageRequest &request)
	{
		CChatMessageRequest fullRequest;
		if (!buildRequest(editBox, fullRequest))
			return false;
		if (fullRequest.Text.empty() || fullRequest.Text[0] != (ucchar)'/')
		{
			request = fullRequest;
			return true;
		}
		if (selectCommandText(fullRequest, argumentsBeforeText, request))
			return true;
		displayRequestError();
		return false;
	}

	std::string getPartName(const CChatMessagePart &part)
	{
		if (part.Type == CChatMessagePart::Item)
		{
			CDBCtrlSheet *sheet = prepareItem(part.ItemValue);
			const std::string name = sheet ? sheet->getItemActualName() : std::string();
			if (sheet)
				getInventory().removeItemLinkInfo(getInventory().getItemSlotId(sheet));
			return name;
		}
		if (part.Type == CChatMessagePart::Phrase)
			return localizedPhrase(part.PhraseValue).Name.toUtf8();
		return part.TextValue.toUtf8();
	}

	CViewLink *createAttachmentView(const CChatMessagePart &part, bool justified)
	{
		CAttachmentView *view = new CAttachmentView(CViewBase::TCtorParam());
		view->Part = part;
		view->setId("attachment");
		view->setUnderlined(true);
		view->setShadow(getChatTextMngr().isTextShadowed());
		view->setShadowOutline(false);
		view->setFontSize(getChatTextMngr().getTextFontSize());
		view->setMultiLine(true);
		view->setTextMode(justified ? CViewText::Justified : CViewText::DontClipWord);
		view->setMultiLineSpace(getChatTextMngr().getTextMultiLineSpace());
		view->setModulateGlobalColor(false);
		view->setColor(part.Type == CChatMessagePart::Item ? itemColor() : phraseColor());
		view->LinkTitle = getPartName(part);
		view->setText(view->LinkTitle);
		view->setActionOnLeftClick("open_chat_attachment");
		view->setActionOnContextHelp("chat_attachment_tooltip");
		return view;
	}

	bool getAttachmentSheetId(CCtrlBase *caller, CSheetId &sheetId)
	{
		CAttachmentView *view = attachmentFromCaller(caller);
		if (!view)
			return false;

		if (view->Part.Type == CChatMessagePart::Item)
			sheetId = view->Part.ItemValue.SheetId;
		else if (view->Part.Type == CChatMessagePart::Phrase)
			sheetId = view->Part.PhraseValue.SheetId;
		else
			return false;
		return sheetId != CSheetId::Unknown;
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
	}
}
