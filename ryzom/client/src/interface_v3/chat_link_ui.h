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

#ifndef CL_CHAT_LINK_UI_H
#define CL_CHAT_LINK_UI_H

#include "game_share/chat_message.h"
#include "nel/misc/smart_ptr.h"

namespace NLGUI
{
	class CCtrlBase;
	class CGroupEditBox;
	class CViewLink;
}

namespace CHAT_SHARE
{
	class CRequestScope
	{
	public:
		CRequestScope(const CChatMessageRequest *request, NLGUI::CGroupEditBox *editBox = NULL);
		~CRequestScope();
		bool wasSent() const;
	private:
		CRequestScope(const CRequestScope &);
		CRequestScope &operator=(const CRequestScope &);
		const CChatMessageRequest *_Previous;
		NLMISC::CRefPtr<NLGUI::CGroupEditBox> _PreviousEditBox;
		bool _PreviousSent;
	};

	void share(const std::string &name, const CChatMessageReference &reference, const std::string &destination);
	bool canShareItem(const NLMISC::CSheetId &sheetId);
	const CChatMessageRequest *getCurrentRequest();
	void setCurrentRequestSent(bool sent);
	bool isChatCommand(const CChatMessageRequest &request, std::string &name);
	bool executeCommand(const CChatMessageRequest &request, NLGUI::CGroupEditBox *editBox);
	bool sendRequest(CChatGroup::TGroupType group, NLMISC::CEntityId dynamicChannelId,
		std::string receiver, bool command);
	bool buildRequest(const NLGUI::CGroupEditBox *editBox, CChatMessageRequest &request);
	// True when the input holds links or group mentions and must be sent as a request.
	bool hasReferences(const NLGUI::CGroupEditBox *editBox);
	void reportInvalidLink();
	void setMapPosition(const CChatMessagePosition *position);
	std::string getPartName(const CChatMessagePart &part);
	NLGUI::CViewLink *createAttachmentView(const CChatMessagePart &part, bool justified);
	bool getAttachmentSheetId(NLGUI::CCtrlBase *caller, NLMISC::CSheetId &sheetId);
	void releasePreviewSheets();
}

#endif
