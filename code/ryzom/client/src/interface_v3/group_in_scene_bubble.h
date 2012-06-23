// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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




#ifndef CL_GROUP_IN_SCENE_BUBBLE_HELP_H
#define CL_GROUP_IN_SCENE_BUBBLE_HELP_H

#include "nel/misc/types_nl.h"
#include "../time_client.h"
#include "group_in_scene.h"


// The manager
class CGroupInSceneBubbleManager
{
public:
	CGroupInSceneBubbleManager();

	void init ();
	void release ();
	void update ();

	// Get a CGroupBubble
	class CGroupInSceneBubble *newBubble (const ucstring &text);

	// Add a message popup. if 0, get the OptionTimeoutMessages
	void addMessagePopup (const ucstring &message, NLMISC::CRGBA col = CRGBA::White, uint time = 0);

	// The same as previous but centered in the screen. if 0, get the OptionTimeoutMessages
	void addMessagePopupCenter (const ucstring &message, NLMISC::CRGBA col = CRGBA::White, uint time = 0);

	// Add a skill popup
	void addSkillPopup (uint skillId, sint delta, uint time);

	// Add a context help with a string
	void addContextHelp (const ucstring &message, const std::string &target, uint time);

	// Add a context help
	void addContextHelpHTML (const std::string &filename, const std::string &target, uint time);

	// Ignore a context help
	void ignoreContextHelp (CInterfaceGroup *groupToRemove);

	// Open a bubble chat (with next and skip button)
	void chatOpen (uint32 nUID, const ucstring &ucsText, uint bubbleTimer = 0);

	// Dynamic Chat

	// Open a Dynamic Chat
	void dynChatOpen (uint32 nBotUID, uint32 nBotName, const std::vector<uint32> &DynStrs);

	// Open a Dynamic Chat from webig
	void webIgChatOpen (uint32 nBotUID, std::string sBotName, const std::vector<std::string> &strs, const std::vector<std::string> &links);

	// Close a Dynamic Chat
	void dynChatClose (uint32 nBotUID);

	//
	void dynChatNext (uint32 nBubbleNb);

	//
	void dynChatSkip (uint32 nBubbleNb);

	// Get the Bot UID from the Bubble Nb
	uint32 dynChatGetBotUID (uint32 nBubbleNb);

	// Get string id of an option in a dyn chat window
	uint32 dynChatGetOptionStringId(uint32 nBubbleNb, uint option);

	static void serialInSceneBubbleInfo(NLMISC::IStream &f);

private:

	// Current bubble
	uint _CurrentBubble;

	// Vector of bubbles
	std::vector<CGroupInSceneBubble *>	_Bubbles;

	// A window popup
	class CPopup
	{
	public:

		// The interface group for the popup
		CInterfaceGroup	*Group;

		// The timeout
		double			TimeOut;

		// Timeout ?
		bool timeOut () const
		{
			return TimeInSec>=TimeOut;
		}
	};

	// Contextual help popup
	class CPopupContext : public CPopup
	{
	public:
		std::string		Url;
		std::string		Target;
		sint32			TargetX;
		sint32			TargetY;
		sint32			TargetW;
		sint32			TargetH;
	};

	// Build a context window template
	CPopupContext *buildContextHelp (const std::string &templateName, const std::string &targetName, CInterfaceElement *&target,
		uint time);

	// The vector of skill popup
	uint						_PopupCount;
	std::vector<CPopup>			_MessagePopup;
	std::vector<CPopup>			_MessagePopupCentered;
	std::vector<CPopupContext>	_BubblePopup;

	// Ignore the context help
	static std::set<std::string>	_IgnoreContextHelp;

	// Group to delete
	std::vector<CInterfaceGroup*>	_GroupToDelete;

	class CDynBubble
	{
	public:
		uint32 BotUID;
		uint32 BotName;
		uint32 DescWaiting;
		CGroupInSceneBubble	*Bubble;
	public:
		CDynBubble()
		{
			DescWaiting = 0;
			Bubble = NULL;
		}
		void displayOptions(bool bShow);
		// get string id for an option in the dynchat
		uint32 getOptionStringId(uint option);
		void next();
		void skip();
	};
	std::vector<CDynBubble>	_DynBubbles;
	CDynBubble *getDynBubble(uint32 nDynBubbleNb);

private:

	void fadeWindow (CInterfaceGroup *group, double timeout);
	bool checkTimeOut (std::vector<CPopup> &rList);;
	void alignMessagePopup (std::vector<CPopup> &rList, bool bCenter);
};

extern CGroupInSceneBubbleManager InSceneBubbleManager;


/**
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2003
 */
class CGroupInSceneBubble : public CGroupInScene
{
public:

	// Constructor
	CGroupInSceneBubble(const TCtorParam &param);
	~CGroupInSceneBubble();

	// Timeout ?
	bool timeOut () const;

	// Get the timeout ?
	double getTimeout () const;

	// Link to a character
	void link (class CCharacterCL	*entity, uint duration);

	// Unlink a bubble
	void unlink ();

	// Set text
	void setText (const ucstring &text);

	// Called from action handler
	void next();

	// Called from action handler
	void skip();

	bool isOnLastPage() const { return _TextParts.empty() || _CurrentPart == (_TextParts.size()-1); }

	// Tell the bubble's user if the bubble can be activated.
	bool canBeShown() const
	{
		return _CanBeShown;
	}

private:

	void setRawText (const ucstring &text);
	void displayNextAndSkip (bool show);

private:

	// Timeout
	double			_TimeOut;
	double			_Duration;

	// Does the bubble can be shown
	bool			_CanBeShown:1;

	// The current character
	CCharacterCL	*_Character;

	// Multi part bubble
	std::vector<ucstring>	_TextParts;
	uint32					_CurrentPart;
};

#endif // CL_GROUP_IN_SCENE_BUBBLE_HELP_H
