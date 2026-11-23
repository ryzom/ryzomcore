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
#include "group_map.h"
#include "interface_manager.h"
#include "inventory_manager.h"
#include "macrocmd_manager.h"
#include "people_interraction.h"
#include "sphrase_manager.h"
#include "../client_chat_manager.h"
#include "../commands.h"
#include "../connection.h"
#include "../continent.h"
#include "../continent_manager.h"
#include "../entity_cl.h"
#include "../sheet_manager.h"
#include "../string_manager_client.h"
#include "../user_entity.h"
#include "game_share/shard_names.h"

#include "nel/gui/db_manager.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_html.h"
#include "nel/gui/group_paragraph.h"
#include "nel/gui/lua_manager.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/view_link.h"
#include "nel/gui/view_text.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/i18n.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/sstring.h"
#include "nel/misc/utf_string_view.h"

#include <ctime>

using namespace NLGUI;
using namespace NLMISC;

extern CContinentManager ContinentMngr;
extern CClientChatManager ChatMngr;
extern CLog g_log;

namespace CHAT_SHARE
{
	namespace
	{
		uint64 NextAttachmentId = 0;

		class CAttachmentView : public CViewLink
		{
		public:
			CAttachmentView(const TCtorParam &param) : CViewLink(param), AttachmentId(++NextAttachmentId) {}
			const uint64 AttachmentId;
			CChatMessagePart Part;
		};

		enum TShareResult
		{
			ShareOk,
			ShareUnavailable,
			ShareInputFull
		};

		const CRGBA ItemLinkColor(255, 205, 80, 255);
		const CRGBA LinkColor(110, 205, 255, 255);

		const char *PhraseLinkDb = "UI:PHRASE_LINK:0";
		// Help windows store raw sheet pointers, keep previews alive until interface release.
		std::map<uint, CDBCtrlSheet*> LinkedItemSheets;
		CDBCtrlSheet *LinkedPhraseSheet = NULL;
		CSmartPtr<CSPhraseComAdpater> LinkedPhraseTooltip;
		CChatMessagePosition SelectedMapPosition;
		bool HasSelectedMapPosition = false;
		// Position of the open chat_position_map window.
		CChatMessagePosition ShownPosition;
		const CChatMessageRequest *CurrentRequest = NULL;
		CRefPtr<CGroupEditBox> CurrentEditBox;
		bool CurrentRequestSent = false;
		CChatMessageRequest PendingTell;

		// @team and @guild follow the client language; @all is a command word like the chat commands.
		CChatMessageMention::TScope getMentionScope(const ucstring &name)
		{
			const std::string lower = toLower(name.toUtf8());
			if (lower == toLower(CI18N::get("uiTeam")))
				return CChatMessageMention::Team;
			if (lower == toLower(CI18N::get("uiGuild")))
				return CChatMessageMention::Guild;
			if (lower == "all")
				return CChatMessageMention::All;
			return CChatMessageMention::Player;
		}

		// Append input text with its tokens parsed, and a reference for each group mention in it.
		bool appendRequestText(CChatMessageRequest &request, const ::u32string &input)
		{
			std::string parsed = CUtfStringView(input).toUtf8();
			if (!CInterfaceManager::parseTokens(parsed))
				return false;
			const ucstring segment = CUtfStringView(parsed).toUtf16();
			if (segment.size() > CHAT_MESSAGE::MaxTextLength - request.Text.size())
				return false;
			const uint32 start = (uint32)request.Text.size();
			request.Text += segment;
			for (uint32 i = start; i < request.Text.size(); ++i)
			{
				const uint32 end = CHAT_MESSAGE::getMentionEnd(request.Text, i);
				if (end == i)
					continue;
				const CChatMessageMention::TScope scope = getMentionScope(request.Text.substr(i + 1, end - i - 1));
				if (scope != CChatMessageMention::Player)
				{
					CChatMessageReference reference;
					reference.Start = (uint16)i;
					reference.Length = (uint16)(end - i);
					reference.Type = CChatMessageReference::Mention;
					reference.Value = scope;
					request.References.push_back(reference);
				}
				i = end - 1;
			}
			return true;
		}

		CAttachmentView *attachmentFromCaller(CCtrlBase *caller)
		{
			CCtrlLink *link = dynamic_cast<CCtrlLink*>(caller);
			return link ? dynamic_cast<CAttachmentView*>(link->getLinkView()) : NULL;
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

		void updateLinkTooltip(CCtrlBase *caller, const std::string &windowName, const std::string &text)
		{
			CCtrlLink *link = dynamic_cast<CCtrlLink*>(caller);
			if (link)
				link->setContextHelpWindowName(windowName);
			CWidgetManager::getInstance()->getContextHelpText() = text;
		}

		std::string getPositionName(const CChatMessagePosition &position)
		{
			return position.Kind == CChatMessagePosition::UserLandMark ? position.FlagName.toUtf8() :
				STRING_MANAGER::CStringManagerClient::getPlaceLocalizedName(position.Place);
		}

		std::string getPositionContext(const CChatMessagePosition &position)
		{
			std::string context = CI18N::get("uiChatLocation") + ": ";
			context += STRING_MANAGER::CStringManagerClient::getPlaceLocalizedName(position.Place);
			context += "\n" + CI18N::get("uiREGION") + ": ";
			context += STRING_MANAGER::CStringManagerClient::getPlaceLocalizedName(position.Region);
			context += "\n" + CI18N::get("uiSearchCommandContinentName") + ": ";
			context += STRING_MANAGER::CStringManagerClient::getPlaceLocalizedName(position.Continent);
			return context;
		}

		// Client continent containing the point; shared positions only carry its place name.
		CContinent *getContinentAt(const CVector &point, std::string &name)
		{
			CWorldSheet *world = dynamic_cast<CWorldSheet*>(SheetMngr.get(CSheetId("ryzom.world")));
			if (!world)
				return NULL;
			for (uint i = 0; i < world->ContLocs.size(); ++i)
			{
				CContinent *continent = ContinentMngr.get(world->ContLocs[i].SelectionName);
				if (continent && continent->Zone.contains(point))
				{
					name = world->ContLocs[i].SelectionName;
					return continent;
				}
			}
			return NULL;
		}

		std::string getLocalPlaceName(const CChatMessagePosition &position, bool ownPosition)
		{
			CVector point;
			CContinent *continent = NULL;
			std::string continentName;
			if (ownPosition)
			{
				if (!UserEntity)
					return std::string();
				point = UserEntity->pos();
				continent = ContinentMngr.cur();
				continentName = ContinentMngr.getCurrentContinentSelectName();
			}
			else
				point = CVector(position.X * 0.001f, position.Y * 0.001f, 0.0f);
			if (!continent || !continent->Zone.contains(point))
				continent = getContinentAt(point, continentName);
			if (!continent)
				return std::string();

			const CContLandMark *place = NULL, *region = NULL;
			double selectedArea = 0;
			for (uint i = 0; i < continent->ContLandMarks.size(); ++i)
			{
				const CContLandMark &landmark = continent->ContLandMarks[i];
				if (landmark.TitleTextID.empty() || !landmark.Zone.contains(point))
					continue;
				if (landmark.Type == CContLandMark::Region)
				{
					if (!region)
						region = &landmark;
					continue;
				}
				if (landmark.Type != CContLandMark::Capital && landmark.Type != CContLandMark::Village &&
					landmark.Type != CContLandMark::Outpost && landmark.Type != CContLandMark::Place &&
					landmark.Type != CContLandMark::Street)
					continue;
				double area = 0;
				for (uint j = 0; j < landmark.Zone.VPoints.size(); ++j)
				{
					const CVector &a = landmark.Zone.VPoints[j];
					const CVector &b = landmark.Zone.VPoints[(j + 1) % landmark.Zone.VPoints.size()];
					area += double(a.x) * b.y - double(b.x) * a.y;
				}
				area = fabs(area);
				if (!place || area < selectedArea ||
					(area == selectedArea && landmark.TitleTextID < place->TitleTextID))
				{
					place = &landmark;
					selectedArea = area;
				}
			}
			if (place)
				return STRING_MANAGER::CStringManagerClient::getPlaceLocalizedName(place->TitleTextID);
			if (region)
				return STRING_MANAGER::CStringManagerClient::getPlaceLocalizedName(region->TitleTextID);
			CWorldSheet *world = dynamic_cast<CWorldSheet*>(SheetMngr.get(CSheetId("ryzom.world")));
			if (world)
				for (uint i = 0; i < world->Maps.size(); ++i)
					if (world->Maps[i].ContinentName == continentName &&
						world->Maps[i].Name == "continent_" + continentName)
						return STRING_MANAGER::CStringManagerClient::getPlaceLocalizedName(world->Maps[i].Name);
			return std::string();
		}

		void showPositionMap(const CChatMessagePosition &position)
		{
			CWidgetManager *widgets = CWidgetManager::getInstance();
			CGroupContainer *dialog = dynamic_cast<CGroupContainer*>(widgets->getElementFromId("ui:interface:chat_position_map"));
			CGroupHTML *html = dialog ? dynamic_cast<CGroupHTML*>(dialog->getGroup("html")) : NULL;
			CViewText *title = dialog ? dynamic_cast<CViewText*>(dialog->getView("title")) : NULL;
			CViewText *footer = dialog ? dynamic_cast<CViewText*>(dialog->getView("footer")) : NULL;
			CViewText *context = dialog ? dynamic_cast<CViewText*>(dialog->getView("context")) : NULL;
			CViewBitmap *marker = dialog ? dynamic_cast<CViewBitmap*>(dialog->getView("marker")) : NULL;
			if (!html || !title || !footer || !context || !marker || position.Place.empty() ||
				position.Continent.empty())
				return;

			marker->setActive(false);
			marker->setParentPos(NULL);
			CLuaState *lua = CLuaManager::getInstance().getLuaState();
			CLuaStackRestorer restorer(lua, lua->getTop());
			lua->pushGlobalTable();
			CLuaObject game(*lua);
			game = game["game"];
			lua->push(position.X * 0.001);
			lua->push(position.Y * 0.001);
			lua->push(8);
			lua->push("");
			lua->push("");
			lua->push("chat_position_map_image");
			if (!game.callMethodByNameNoThrow("staticMapImage", 6, 1) || !lua->isString(-1))
				return;
			html->setHTML(lua->toString(-1));

			CViewBitmap *image = dynamic_cast<CViewBitmap*>(html->getView("chat_position_map_image"));
			if (image)
			{
				const bool flag = position.Kind == CChatMessagePosition::UserLandMark;
				marker->setScale(false);
				if (flag)
					marker->setTexture("lm_user.tga");
				else if (position.Kind == CChatMessagePosition::PlayerPosition)
					marker->setTexture("teammate_map.tga");
				else
				{
					marker->setTexture("w_radar_point.tga");
					marker->setScale(true);
					marker->setW(14);
					marker->setH(14);
				}
				marker->setColor(flag ? position.FlagColor : CRGBA::White);
				marker->setParentPos(image);
				marker->setRenderLayer(image->getRenderLayer() + 1);
				marker->setActive(true);
			}

			ShownPosition = position;
			title->setText(getPositionName(position));
			const std::string sender = CEntityCL::removeTitleAndShardFromName(position.SenderName.toUtf8());
			std::string caption = CI18N::get("uiMFAuthor") + ": " + sender;
			const time_t timestamp = position.Timestamp;
			const tm *posted = localtime(&timestamp);
			char time[32] = "";
			if (posted)
				strftime(time, sizeof(time), "%H:%M", posted);
			caption += " \xC2\xB7 " + CI18N::get("uiOutpostTitleTime") + ": " + time;
			footer->setText(caption);
			context->setText(getPositionContext(position));

			dialog->setActive(true);
			dialog->updateCoords();
			CCtrlMover *mover = dialog->getCtrlMover();
			if (mover)
			{
				mover->setParentPos(dialog);
				mover->setParentPosRef(Hotspot_TL);
				mover->setPosRef(Hotspot_TL);
				mover->setX(0);
				mover->setY(0);
				mover->setH(dialog->getYReal() + dialog->getHReal() - html->getYReal() - html->getHReal());
				mover->updateCoords();
			}
			widgets->setTopWindow(dialog);
		}

		// Open the macro editor with the commands this client knows, the player saves it from there.
		void editSharedMacro(const CChatMessageMacro &shared)
		{
			CMacroCmdManager *manager = CMacroCmdManager::getInstance();
			CMacroCmd macro;
			macro.Name = shared.Name;
			macro.DispText = shared.DispText;
			macro.BitmapBack = shared.BitmapBack;
			macro.BitmapIcon = shared.BitmapIcon;
			macro.BitmapOver = shared.BitmapOver;
			for (uint i = 0; i < shared.Commands.size(); ++i)
			{
				const CAction::CName name(shared.Commands[i].Name.c_str(), shared.Commands[i].Params.c_str());
				uint j = 0;
				while (j < manager->ActionManagers.size() && !manager->ActionManagers[j]->getBaseAction(name))
					++j;
				if (j < manager->ActionManagers.size())
					macro.addCommand(shared.Commands[i].Name, shared.Commands[i].Params);
			}
			if (macro.Commands.empty())
				CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiBCNotAvailable"));
			else
				manager->editNewMacro(macro);
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
					if (!canShareItem(view->Part.ItemValue.SheetId))
						return;
					if (CInterfaceHelp::activateChatItemWindow(view->AttachmentId))
						return;
					CDBCtrlSheet *sheet = prepareItem(view->Part.ItemValue);
					if (sheet && sheet->asItemSheet())
						CAHManager::getInstance()->runActionHandler("open_item_help", sheet,
							"force_keep=0|prefer_new=1|chat_link_id=" + toString(view->AttachmentId));
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
				else if (view->Part.Type == CChatMessagePart::Position)
					showPositionMap(view->Part.PositionValue);
				else if (view->Part.Type == CChatMessagePart::Macro)
					editSharedMacro(view->Part.MacroValue);
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
					const std::string windowName = sheet->getContextHelpWindowName();
					std::string tooltip = sheet->getItemActualName();
					if (windowName == "buff_item_context_help" || windowName == "crystallized_spell_context_help")
					{
						// Linked items already carry their info and have no inventory cache identity.
						CControlSheetInfoWaiter waiter;
						waiter.CtrlSheet = sheet;
						waiter.LuaMethodName = windowName == "buff_item_context_help" ?
							"updateBuffItemTooltip" : "updateCrystallizedSpellTooltip";
						tooltip = waiter.infoValidated();
					}
					updateLinkTooltip(caller, windowName, tooltip);
					getInventory().removeItemLinkInfo(slotId);
				}
				else if (view->Part.Type == CChatMessagePart::Phrase)
				{
					// Lua retains LastTooltipPhrase for the per-frame cooldown update.
					if (!LinkedPhraseTooltip)
						LinkedPhraseTooltip = new CSPhraseComAdpater;
					LinkedPhraseTooltip->Phrase = localizedPhrase(view->Part.PhraseValue);
					updateLinkTooltip(caller, "action_context_help", LinkedPhraseTooltip->updateTooltip());
				}
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerChatAttachmentTooltip, "chat_attachment_tooltip");

		class CHandlerChatPositionOnMap : public IActionHandler
		{
		public:
			virtual void execute(CCtrlBase * /* caller */, const std::string &params)
			{
				CWidgetManager *widgets = CWidgetManager::getInstance();
				CInterfaceGroup *window = dynamic_cast<CInterfaceGroup*>(widgets->getElementFromId("ui:interface:map"));
				CGroupMap *map = dynamic_cast<CGroupMap*>(widgets->getElementFromId("ui:interface:map:content:map_content:actual_map"));
				if (!window || !map)
					return;
				window->setActive(true);
				widgets->setTopWindow(window);
				window->updateCoords();

				const bool flag = ShownPosition.Kind == CChatMessagePosition::UserLandMark;
				const ucstring title = flag ? ShownPosition.FlagName : ucstring::makeFromUtf8(getPositionName(ShownPosition));
				const CVector2f position(ShownPosition.X * 0.001f, ShownPosition.Y * 0.001f);
				std::string continent;
				if (!getContinentAt(CVector(position.x, position.y, 0.f), continent) ||
					!map->showChatPosition(position, continent, title, flag ? ShownPosition.FlagColor : CRGBA::White))
				{
					CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiBCNotAvailable"));
					return;
				}
				if (params == "landmark")
					map->createUserLandMarkAt(position, title);
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerChatPositionOnMap, "chat_position_on_map");

		class CHandlerCloseChatPosition : public IActionHandler
		{
		public:
			virtual void execute(CCtrlBase * /* caller */, const std::string &params)
			{
				CWidgetManager *widgets = CWidgetManager::getInstance();
				CGroupContainer *dialog = dynamic_cast<CGroupContainer*>(widgets->getElementFromId("ui:interface:chat_position_map"));
				CViewBitmap *marker = dialog ? dynamic_cast<CViewBitmap*>(dialog->getView("marker")) : NULL;
				if (params != "deactive")
				{
					if (dialog)
						dialog->setActive(false);
					return;
				}
				if (marker)
				{
					marker->setActive(false);
					marker->setParentPos(NULL);
				}
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerCloseChatPosition, "close_chat_position");

		class CHandlerChatLinkTell : public IActionHandler
		{
		public:
			virtual void execute(CCtrlBase *caller, const std::string &params)
			{
				CWidgetManager *widgets = CWidgetManager::getInstance();
				CInterfaceGroup *dialog = dynamic_cast<CInterfaceGroup*>(
					widgets->getElementFromId("ui:interface:chat_link_tell"));
				CGroupEditBox *receiver = dialog ? dynamic_cast<CGroupEditBox*>(dialog->getGroup("receiver:eb")) : NULL;
				if (params == "cancel")
				{
					getChatTextMngr().clearQuote(receiver);
					PendingTell = CChatMessageRequest();
					return;
				}
				if (!dialog || widgets->getModalWindow() != dialog)
					return;
				if (params == "accepted")
				{
					if (caller == receiver && PendingTell.ClientRequestId != 0)
						widgets->disableModalWindow();
					return;
				}
				if (!PendingTell.isValid())
				{
					reportInvalidLink();
					return;
				}
				if (!receiver || receiver->getInputString().empty())
					return;
				const uint32 requestId = getChatTextMngr().beginQuoteSend(receiver);
				if (requestId == 0)
					return;
				PendingTell.ClientRequestId = requestId;
				if (!ChatMngr.tell(receiver->getInputString(), PendingTell))
					getChatTextMngr().finishQuoteSend(requestId, false);
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerChatLinkTell, "chat_link_tell");

		TShareResult shareTo(const std::string &name, const CChatMessageReference &reference,
			const std::string &destination)
		{
			const ::u32string title = CUtfStringView(name).toUtf32();
			if (title.empty())
				return ShareUnavailable;
			if (CUtfStringView(title).toUtf16().size() > CHAT_MESSAGE::MaxTextLength)
				return ShareInputFull;
			// Input tags keep the reference until the message is sent; the editbox
			// only knows the text range and color.
			CMemStream stream;
			CChatMessageReference tagReference = reference;
			stream.serial(tagReference);
			const std::string tagData((const char*)stream.buffer(), stream.length());
			const CRGBA color = reference.Type == CChatMessageReference::Item ? ItemLinkColor : LinkColor;
			if (destination == "clipboard")
			{
				CGroupEditBox::CTextTag tag;
				tag.Start = 0;
				tag.Length = (uint32)title.size();
				tag.Reference = tagData;
				tag.Color = color;
				std::vector<CGroupEditBox::CTextTag> tags(1, tag);
				return CGroupEditBox::copyToClipboard(title, tags) ? ShareOk : ShareUnavailable;
			}
			if (destination == "tell")
			{
				CChatMessageRequest request;
				request.Text = CUtfStringView(title).toUtf16();
				request.References.push_back(reference);
				request.References[0].Start = 0;
				request.References[0].Length = (uint16)request.Text.size();
				CWidgetManager *widgets = CWidgetManager::getInstance();
				CInterfaceGroup *dialog = dynamic_cast<CInterfaceGroup*>(
					widgets->getElementFromId("ui:interface:chat_link_tell"));
				CGroupEditBox *receiver = dialog ? dynamic_cast<CGroupEditBox*>(dialog->getGroup("receiver:eb")) : NULL;
				if (!receiver || !request.isValid())
					return ShareUnavailable;
				getChatTextMngr().clearQuote(receiver);
				receiver->setInputString(std::string());
				PendingTell = request;
				widgets->enableModalWindow(NULL, dialog);
				return ShareOk;
			}
			if (destination != "main")
				return ShareUnavailable;
			CGroupEditBox *editBox = PeopleInterraction.ChatGroup.Window ?
				PeopleInterraction.ChatGroup.Window->getEditBox() : NULL;
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
			if (!editBox->writeString(CUtfStringView(insertion).toUtf8(), true, false, false))
				return ShareInputFull;
			editBox->addTextTag(titleStart, (uint32)title.size(), color, tagData);
			const sint32 newCursor = cursor + (sint32)insertion.size();
			editBox->setCursorPos(newCursor);
			CGroupEditBox::setSelectCursorPos(newCursor);
			editBox->bypassNextKey();
			return ShareOk;
		}

		class CHandlerShareMapPosition : public IActionHandler
		{
		public:
			virtual void execute(CCtrlBase * /* caller */, const std::string &params)
			{
				const std::string source = getParam(params, "source");
				const bool ownPosition = source == "player";
				if ((!ownPosition && source != "map" && source != "landmark") ||
					(!ownPosition && (!HasSelectedMapPosition ||
					 (source == "landmark") != (SelectedMapPosition.Kind == CChatMessagePosition::UserLandMark))))
					return;
				const CChatMessagePosition selected = SelectedMapPosition;
				CWidgetManager::getInstance()->disableModalWindow();
				CChatMessageReference reference;
				reference.Type = ownPosition ? CChatMessageReference::Position : CChatMessageReference::MapPosition;
				if (!ownPosition)
					reference.PositionValue = selected;
				const std::string name = !ownPosition && selected.Kind == CChatMessagePosition::UserLandMark ?
					selected.FlagName.toUtf8() : getLocalPlaceName(selected, ownPosition);
				share(name, reference, getParam(params, "destination"));
			}
		};
		REGISTER_ACTION_HANDLER(CHandlerShareMapPosition, "share_map_position");

		uint32 getTextBeforeFirstLink(const CChatMessageRequest &request, CSString &prefix, bool &endsWithSeparator)
		{
			const uint32 prefixLength = request.References.empty() ?
				(uint32)request.Text.size() : request.References[0].Start;
			prefix = CUtfStringView(request.Text.substr(0, prefixLength)).toUtf8();
			endsWithSeparator = (!prefix.empty() && CSString::isWhiteSpace(prefix[prefix.size() - 1])) ||
				(prefixLength < request.Text.size() && request.Text[prefixLength] <= 127 &&
					CSString::isWhiteSpace((char)request.Text[prefixLength]));
			return prefixLength;
		}
	}

	CRequestScope::CRequestScope(const CChatMessageRequest *request, CGroupEditBox *editBox) :
		_Previous(CurrentRequest), _PreviousEditBox(CurrentEditBox), _PreviousSent(CurrentRequestSent)
	{
		CurrentRequest = request;
		CurrentEditBox = editBox;
		CurrentRequestSent = false;
	}

	CRequestScope::~CRequestScope()
	{
		CurrentRequest = _Previous;
		CurrentEditBox = _PreviousEditBox;
		CurrentRequestSent = _PreviousSent;
	}

	bool CRequestScope::wasSent() const
	{
		return CurrentRequestSent;
	}

	void share(const std::string &name, const CChatMessageReference &reference, const std::string &destination)
	{
		const TShareResult result = shareTo(name, reference, destination);
		if (result == ShareInputFull)
			reportInvalidLink();
		else if (result == ShareUnavailable)
			CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiBCNotAvailable"));
	}

	bool canShareItem(const CSheetId &sheetId)
	{
		const CItemSheet *item = dynamic_cast<const CItemSheet*>(SheetMngr.get(sheetId));
		return item != NULL;
	}

	const CChatMessageRequest *getCurrentRequest()
	{
		return CurrentRequest;
	}

	void setCurrentRequestSent(bool sent)
	{
		CurrentRequestSent = sent;
	}

	bool isChatCommand(const CChatMessageRequest &request, std::string &name)
	{
		CSString prefix;
		bool prefixEndsWithSeparator;
		const uint32 prefixLength = getTextBeforeFirstLink(request, prefix, prefixEndsWithSeparator);
		prefix = prefix.leftCrop(1).leftStrip();
		CSString commandName = prefix.strtok(" \t\r\n", !prefix.empty() && prefix[0] == '"', false, true, false);
		if (!commandName.empty() && commandName[0] == '"')
			commandName = commandName.unquote(true, false);
		name = commandName;
		if (!request.References.empty() && prefix.empty() && !prefixEndsWithSeparator)
			return false;
		CUserCommand *userCommand = dynamic_cast<CUserCommand*>(ICommand::getCommand(name));
		CUserCommand::CMode *mode = NULL;
		if (userCommand)
		{
			const uint32 offset = prefixLength - (uint32)CUtfStringView(prefix).toUtf16().size();
			CSString arguments = CUtfStringView(request.Text.substr(offset)).toUtf8();
			uint numArgs = 0;
			while (!arguments.strtok(" \t\r\n").empty())
				++numArgs;
			std::map<uint, CUserCommand::CMode>::iterator fixedMode = userCommand->FixedArgModes.find(numArgs);
			if (fixedMode != userCommand->FixedArgModes.end())
				mode = &fixedMode->second;
			else if (!userCommand->InfiniteMode.Keywords.empty() && numArgs >= userCommand->InfiniteMode.KeywordsCount)
				mode = &userCommand->InfiniteMode;
		}
		return mode && (mode->Action == "talk" || mode->Action == "tell");
	}

	bool executeCommand(const CChatMessageRequest &request, CGroupEditBox *editBox)
	{
		if (!request.References.empty())
		{
			std::string name;
			if (!isChatCommand(request, name))
			{
				if (!ICommand::exists(name))
					CInterfaceManager::getInstance()->displaySystemInfo(name + ": " + CI18N::get("uiCommandNotExists"));
				else
					reportInvalidLink();
				return false;
			}
		}
		CRequestScope scope(&request, editBox);
		ICommand::execute(request.Text.toUtf8().substr(1), g_log);
		return scope.wasSent();
	}

	bool sendRequest(CChatGroup::TGroupType group, CEntityId dynamicChannelId,
		std::string receiver, bool command)
	{
		if (!CurrentRequest)
			return false;
		if (CurrentEditBox == NULL)
			return true;
		CChatMessageRequest request = *CurrentRequest;
		request.QuoteMessageId = getChatTextMngr().getQuoteMessageId(CurrentEditBox);
		if (!request.QuoteMessageId.empty())
		{
			CChatGroup::TGroupType sourceGroup;
			CEntityId sourceDynamicChannelId;
			std::string sourceReceiver;
			if (!getChatTextMngr().getQuoteTarget(CurrentEditBox, sourceGroup, sourceDynamicChannelId, sourceReceiver))
				return true;
			if (command &&
				((group == CChatGroup::arround ? CChatGroup::say : group) !=
				 (sourceGroup == CChatGroup::arround ? CChatGroup::say : sourceGroup) ||
				 (group == CChatGroup::dyn_chat && dynamicChannelId != sourceDynamicChannelId) ||
				 (group == CChatGroup::tell && compareCaseInsensitive(
					 CShardNames::getInstance().makeFullNameFromRelative(PlayerSelectedMainland, receiver),
					 CShardNames::getInstance().makeFullNameFromRelative(PlayerSelectedMainland, sourceReceiver)) != 0)))
			{
				CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiChatQuoteSourceChannelOnly"));
				return true;
			}
			group = sourceGroup;
			dynamicChannelId = sourceDynamicChannelId;
			receiver = sourceReceiver;
		}
		if (command)
		{
			CSString text;
			bool prefixEndsWithSeparator;
			const uint32 prefixLength = getTextBeforeFirstLink(request, text, prefixEndsWithSeparator);
			if (text.empty() || text[0] != '/')
				return true;
			text = text.substr(1);
			text.strtok(" \t\r\n", true, false, true, false);
			if (group == CChatGroup::tell)
			{
				if (text.strtok(" \t\r\n").empty() ||
					(!request.References.empty() && text.empty() && !prefixEndsWithSeparator))
				{
					reportInvalidLink();
					return true;
				}
			}
			text = text.leftStrip();
			const uint32 offset = prefixLength - (uint32)CUtfStringView(text).toUtf16().size();
			const ucstring body = request.Text.substr(offset);
			for (std::vector<CChatMessageReference>::iterator it = request.References.begin();
				it != request.References.end(); ++it)
			{
				it->Start -= offset;
			}
			request.Text = body;
			if (!body.empty() && body[0] == '/' &&
				(request.References.empty() || request.References[0].Start != 0))
			{
				setCurrentRequestSent(executeCommand(request, CurrentEditBox));
				return true;
			}
		}
		request.ClientRequestId = getChatTextMngr().beginQuoteSend(CurrentEditBox);
		if (request.ClientRequestId == 0)
			return true;
		const bool queued = request.isValid() && (group == CChatGroup::tell ?
			ChatMngr.tell(receiver, request) : ChatMngr.chat(request, group, dynamicChannelId));
		setCurrentRequestSent(queued);
		if (!queued)
			getChatTextMngr().finishQuoteSend(request.ClientRequestId, false);
		return true;
	}

	void reportInvalidLink()
	{
		CInterfaceManager::getInstance()->displaySystemInfo(CI18N::get("uiChatLinkDoesNotFit"));
	}

	bool buildRequest(const CGroupEditBox *editBox, CChatMessageRequest &request)
	{
		request = CChatMessageRequest();
		if (!editBox)
			return false;
		const ::u32string &input = editBox->getInputStringRef();
		const std::vector<CGroupEditBox::CTextTag> &tags = editBox->getTextTags();
		if (tags.size() > CHAT_MESSAGE::MaxReferences ||
			CUtfStringView(input).toUtf16().size() > CHAT_MESSAGE::MaxTextLength)
			return false;

		uint32 inputPosition = 0;
		for (std::vector<CGroupEditBox::CTextTag>::const_iterator it = tags.begin();
			it != tags.end(); ++it)
		{
			if (!it->Length || it->Start < inputPosition || it->Start > input.size() ||
				it->Length > input.size() - it->Start || it->Reference.empty() ||
				it->Reference.size() > CHAT_MESSAGE::MaxSerializedSize ||
				!appendRequestText(request, input.substr(inputPosition, it->Start - inputPosition)))
				return false;

			CChatMessageReference reference;
			try
			{
				CMemStream stream(true);
				stream.fill((const uint8*)it->Reference.data(), (uint32)it->Reference.size());
				stream.serial(reference);
				if ((uint32)stream.getPos() != stream.length())
					return false;
			}
			catch (const Exception &)
			{
				return false;
			}
			const ucstring title = CUtfStringView(input.substr(it->Start, it->Length)).toUtf16();
			if (title.empty() || title.size() > CHAT_MESSAGE::MaxTextLength - request.Text.size())
				return false;
			reference.Start = (uint16)request.Text.size();
			reference.Length = (uint16)title.size();
			request.Text += title;
			request.References.push_back(reference);
			inputPosition = it->Start + it->Length;
		}
		return appendRequestText(request, input.substr(inputPosition)) &&
			!request.References.empty() && request.isValid();
	}

	bool hasReferences(const CGroupEditBox *editBox)
	{
		if (!editBox)
			return false;
		if (!editBox->getTextTags().empty())
			return true;
		const ucstring text = CUtfStringView(editBox->getInputStringRef()).toUtf16();
		for (uint32 i = 0; i < text.size(); ++i)
		{
			const uint32 end = CHAT_MESSAGE::getMentionEnd(text, i);
			if (end != i && getMentionScope(text.substr(i + 1, end - i - 1)) != CChatMessageMention::Player)
				return true;
		}
		return false;
	}

	void setMapPosition(const CChatMessagePosition *position)
	{
		HasSelectedMapPosition = position && position->isValid();
		SelectedMapPosition = HasSelectedMapPosition ? *position : CChatMessagePosition();
	}

	std::string getPartName(const CChatMessagePart &part)
	{
		switch (part.Type)
		{
		case CChatMessagePart::Item:
			{
				CDBCtrlSheet *sheet = prepareItem(part.ItemValue);
				const std::string name = sheet ? sheet->getItemActualName() : std::string();
				if (sheet)
					getInventory().removeItemLinkInfo(getInventory().getItemSlotId(sheet));
				return name;
			}
		case CChatMessagePart::Phrase:
			return localizedPhrase(part.PhraseValue).Name.toUtf8();
		case CChatMessagePart::Position:
			return getPositionName(part.PositionValue);
		case CChatMessagePart::Macro:
			return part.MacroValue.Name;
		default:
			return part.TextValue.toUtf8();
		}
	}

	CViewLink *createAttachmentView(const CChatMessagePart &part, bool justified)
	{
		CAttachmentView *view = new CAttachmentView(CViewBase::TCtorParam());
		view->Part = part;
		view->setId("attachment");
		bool canOpen = true;
		if (part.Type == CChatMessagePart::Item)
			canOpen = canShareItem(part.ItemValue.SheetId);
		else if (part.Type == CChatMessagePart::Position)
			canOpen = !part.PositionValue.Place.empty() && !part.PositionValue.Continent.empty();
		view->setUnderlined(canOpen);
		view->setShadow(getChatTextMngr().isTextShadowed());
		view->setShadowOutline(false);
		view->setFontSize(getChatTextMngr().getTextFontSize());
		view->setMultiLine(true);
		view->setTextMode(justified ? CViewText::Justified : CViewText::DontClipWord);
		view->setMultiLineSpace(getChatTextMngr().getTextMultiLineSpace());
		view->setModulateGlobalColor(false);
		view->setColor(part.Type == CChatMessagePart::Item ? ItemLinkColor : LinkColor);
		view->LinkTitle = getPartName(part);
		view->setText(view->LinkTitle);
		if (canOpen)
			view->setActionOnLeftClick("open_chat_attachment");
		if (part.Type == CChatMessagePart::Item || part.Type == CChatMessagePart::Phrase)
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
		PendingTell = CChatMessageRequest();
		for (std::map<uint, CDBCtrlSheet*>::iterator it = LinkedItemSheets.begin(); it != LinkedItemSheets.end(); ++it)
		{
			getInventory().removeItemLinkInfo(getInventory().getItemSlotId(it->second));
			delete it->second;
		}
		LinkedItemSheets.clear();
		delete LinkedPhraseSheet;
		LinkedPhraseSheet = NULL;
		LinkedPhraseTooltip = NULL;
	}
}
