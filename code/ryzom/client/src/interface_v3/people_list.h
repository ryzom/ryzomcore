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



#ifndef CL_PEOPLE_LIST_H
#define CL_PEOPLE_LIST_H


// client
#include "nel/gui/group_container.h"
#include "chat_window.h"
#include "interface_pointer.h"
// NeL
#include "nel/misc/ucstring.h"
#include "nel/misc/rgba.h"




/** Describe a list of people and its properties.
  * It is used to create the list
  */
struct CPeopleListDesc
{
	enum TContactType { Team, Contact, Ignore, Unknown };
	ucstring     PeopleListTitle;    // title of the people list
	TContactType ContactType;
	std::string  FatherContainer;           // name of the father container
	std::string  BaseContainerTemplateName; // name of the template for the base container
	sint		 InsertPosition;            // -1 if the people list should be inserted at the end of the container list, or the index otherwise
	std::string  Id;                        // Id of this container
	bool         Savable;                   // Save the infos for this people list ? The default is false
	bool         Localize;                  // Localize with ci18n ?
	std::string	 AHOnActive;
	std::string	 AHOnActiveParams;
	std::string	 AHOnDeactive;
	std::string	 AHOnDeactiveParams;
	std::string	 HeaderColor;
	// ctor
	CPeopleListDesc() : ContactType(Unknown),
						InsertPosition(-1),
						Localize(false)
	{}
};


/** class to manage a list of people or a contact list
  */
class CPeopleList : public CGroupContainer::IChildrenObs
{
public:
	// default ctor
	CPeopleList();
	/** create a list of people from the given description
	  * \param desc description of the people list
	  * \param chat optional chat box in the list
	  * \return true if the list could be
	  */
	bool create(const CPeopleListDesc &desc, const CChatWindowDesc *chat = NULL);
	// Get index from the name of a people, or -1 if not found
	sint getIndexFromName(const ucstring &name) const;
	// Get index from the id of the container that represent the people
	sint getIndexFromContainerID(const std::string &id) const;
	// Get the number of people in this list
	uint getNumPeople() const { return (uint)_Peoples.size(); }
	// Get name of a people
	ucstring getName(uint index) const;
	// Sort people alphabetically
	void sort();

	enum TSortOrder
	{
		sort_index = 0,
		START_SORT_ORDER = sort_index,
		sort_name,
		sort_online,
		END_SORT_ORDER
	};

	void sortEx(TSortOrder order);

	/** Add a people to the list, and returns its index or -1 if the creation failed
	  * If this is a team mate, tells its index so that ic can be bound to the database in the right location
	  */
	sint addPeople(const ucstring &name, uint teamMateIndex = 0);
	// swap people position between the 2 given indexs
	void swapPeople(uint index1, uint index2);
	// Remove the people at the given index
	void removePeople(uint index);

	// For client/server comms
	uint32	getContactId(uint index);
	void	setContactId(uint index, uint32 contactId);
	sint	getIndexFromContactId(uint32 contactId);

	/** Display a message for the given people
	  * If the window is closed, it causes it to blink (and also the parent window)
	  */
	void displayMessage(uint index, const ucstring &msg, NLMISC::CRGBA col, uint numBlinks = 0);
	void displayLocalPlayerTell(const ucstring &receiver, uint index, const ucstring &msg, uint numBlinks = 0);
	// Is the given people window visible ?
	bool isPeopleChatVisible(uint index) const;
	// reset remove everything from the interface
	void reset();
	// remove all people from the list
	void removeAllPeoples();
	// set the menu for the title bar of the people
	void setPeopleMenu(const std::string &menuName);
	// Extended : set menus for each state of a people (blocked and / or  online)
	void setPeopleMenuEx(const std::string &offlineUnblockedMenuName,
						 const std::string &onlineUnblockedMenuName,
					     const std::string &onlineAbroadUnblockedMenuName,
					     const std::string &offlineBockedMenuName,
					     const std::string &onlineBlockedMenuName,
					     const std::string &onlineAbroadBlockedMenuName
		                );
	// Hide / show the symbol to say that a people is online (if the symbol is available)
	void setOnline(uint index, TCharConnectionState online);
	// Hide / show the symbol to say that a people is blocked (don't receive messages from him)
	void setBlocked(uint index, bool blocked);
	TCharConnectionState getOnline(uint index) const;
	bool getBlocked(uint index) const;
	// Set the menu for the whole container
	void setMenu(const std::string &menuName);
	// Get id of the main container. This is the value that was passed during the creation ('Id' field of CPeopleListDesc)
	const std::string &getContainerID() const { return _ContainerID; }
	// Get the global chat window if there is one
	CChatWindow *getChat() const { return _ChatWindow; }

	// Open or close if opened, the chat corresponding to the entry index in the friend list
	void openCloseChat(sint index, bool bOpen);

/////////////////////////////////////////////////////////////////////////////////////////////
private:
	struct CPeople
	{
		CPeople() : Container(NULL), Chat(NULL), Online(ccs_offline), Blocked(false), ContactId(0) {}
		NLMISC::CRefPtr<CGroupContainer> Container; // todo : replace this with a CChatWindow one day, for consistency
		NLMISC::CRefPtr<CGroupContainer> Chat;
		uint							GlobalID;
		TCharConnectionState			Online;
		bool							Blocked;
		uint32							ContactId;
		bool operator < (const CPeople &other) const { return getName() < other.getName(); }
		ucstring		getName() const { return Container->getUCTitle(); }
	};
	typedef std::vector<CPeople> TPeopleVect;
private:
	CGroupContainerPtr				_BaseContainer;
	NLMISC::CRefPtr<CChatWindow>	_ChatWindow;
	TPeopleVect						_Peoples;
	CPeopleListDesc::TContactType   _ContactType;
	std::string						_ContainerID;
	uint							_CurrPeopleID; // an increasing index to create id for people
	std::string						_PeopleMenuOfflineUnblocked;
	std::string						_PeopleMenuOnlineUnblocked;
	std::string						_PeopleMenuOnlineAbroadUnblocked;
	std::string						_PeopleMenuOfflineBlocked;
	std::string						_PeopleMenuOnlineBlocked;
	std::string						_PeopleMenuOnlineAbroadBlocked;
	bool                            _Savable;
private:
	void			  updatePeopleMenu(uint index);
	// from CGroupContainer::IChildrenObs
	virtual void childrenMoved(uint srcIndex, uint destIndex, CGroupContainer *children);

	static bool sortExByContactId(const CPeople& a, const CPeople& b);
	static bool sortExByName(const CPeople& a, const CPeople& b);
	static bool sortExByOnline(const CPeople& a, const CPeople& b);
};

#endif
