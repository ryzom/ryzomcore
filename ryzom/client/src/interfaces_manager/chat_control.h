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



#ifndef NL_CHAT_CONTROL_H
#define NL_CHAT_CONTROL_H

// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/ucstring.h"
// Client
#include "multi_list.h"
// Game Share
#include "game_share/ryzom_entity_id.h"
// Std
#include <map>


///////////
// Using //
///////////
using NLMISC::CRGBA;
using std::map;


/**
 * class for the basic chat control interface (double list, mute/unmute of players, different colors can be assigned for each player)
 * text can be saved in .txt format. The list is sensible to move-over, left click and right click
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CChatControl : public CMultiList
{
public:
	/// Default Constructor
	CChatControl(uint id);

	/// Constructor
	CChatControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel,  uint leftFunc, uint rightFunc, const CPen &pen);
	CChatControl(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel,  uint leftFunc, uint rightFunc, uint32 fontSize, CRGBA color, bool shadow);



private:
	/**
	* add a line of text (private because this function, herited from CMultiList should be avoided for chat box,
	* use add( const NLMISC::CEntityId sid, const ucstring &name, const ucstring &text) or add(const ucstring &text) for player msg and system msg
	*/
	virtual void add(const std::list<ucstring> &str);

public:

	/**
	* add text from a player message to the chat box
	* \param NLMISC::CEntityId& sid of the player who sent the msg
	* \param ucstring& name of the player who sent the msg
	* \param ucstring& the message to display
	*/
	void add( const NLMISC::CEntityId &sid, const ucstring &name, const ucstring &text);

	/**
	* display text from a system message (not a player message, for player use the other public add method)
	* \param ucstring& the message to display
	*/
	void add( const ucstring &text);

	/// Display the Bitmap.
	virtual void display();

	/**
	* clear the control, erasing all stored sentences
	*/
	void clear();


	/// Manage the left click of the mouse for the list
	virtual void click(float x, float y, bool &taken);

	/// Manage the right click of the mouse for the list
	virtual void clickRight(float x, float y, bool &taken);

	/// called when the mouse move, indicate new mouse coordinates
	virtual void mouseMove( float x, float y);

	/**
	* unmute all the players
	*/
	inline void unmuteAll() { _MutedPlayers.clear(); }

	/**
	* set the color for system messages
	* \param the new color
	*/
	void setSysColor(CRGBA &color)
	{
		_SysTextColor = color; // map the command dummy NLMISC::CEntityId with the display color
		_PlayersColor.insert( std::make_pair( _CommandSid, &_SysTextColor ) );
	}

	/**
	* set the muted color
	* \param the new color
	*/
	void setMutedColor(CRGBA &color) { _MutedColor = color; }

	/**
	* set the color for highlighted messages
	* \param the new color
	*/
	void setHighlightColor(CRGBA &color) { _HighlightColor = color; }

	/**
	* set the color for highlighted messages from muted players
	* \param the new color
	*/
	void setMutedHighlightColor(CRGBA &color) { _MutedHighlightColor = color; }


	/**
	* get a pointer to the name of the currently selected player, or NULL
	* \return ucstring* pointer to the player name (or NULL if there is no player selected)
	*/
	ucstring *CChatControl::getSelectedPlayerName() const { return _SelectedPlayer; }

	/**
	* get adress of  the NLMISC::CEntityId of the currently selected player, or NULL if no player selected
	* \return NLMISC::CEntityId* pointer to the player NLMISC::CEntityId (or NULL)
	*/
	NLMISC::CEntityId *CChatControl::getSelectedPlayerSid() const { return _SelectedPlayerSid; }

	/**
	* mute the specified player
	* \param ucstring& name of the player to mute
	*/
	void mutePlayer(const NLMISC::CEntityId &sid) { _MutedPlayers.insert(sid); }

	/**
	* unmute the specified player
	* \param ucstring& name of the player to unmute
	*/
	void unmutePlayer(const NLMISC::CEntityId &sid) { _MutedPlayers.erase(sid); }

	/**
	* return true if the specified player is muted
	* \param ucstring& name of the player
	* \return bool true if the player is muted
	*/
	bool isMuted(const NLMISC::CEntityId &sid) const { return ( _MutedPlayers.find(sid) != _MutedPlayers.end() ); }

	/**
	* get a color not allready used for display
	* \return CRBGA a color different from the ones allready used
	*/
	CRGBA getNewColor() const;

	/// scroll verticaly by 'scroll' units in either direction
	void scrollV(sint32 scroll);

private:
	/// Initialize the control (1 function called for all constructors -> easier).
	inline void init(uint leftFunc, uint rightFunc);

	/**
	* search into _NamesYPos list for the specified y coordinate
	* \param the searched y coordinate
	* \param NLMISC::CEntityId*& the variable that will receive the adress of the NLMISC::CEntityId of the player (or NULL if not found)
	* \param ucstring*& the variable that will receive the pointer to the name of the player (or NULL if not found)
	* \return bool true if a player was found, false otherwise
	*/
	bool searchPlayerAtPos(float y, NLMISC::CEntityId *&sid, ucstring *&name) const;

// attributes
private:
	/// the set of muted player names
	typedef std::set<NLMISC::CEntityId>	TSidSet;
	TSidSet		_MutedPlayers;

	/// the set of NLMISC::CEntityId (match the list of ucstring associated to player names and messages)
	typedef std::list<NLMISC::CEntityId>	TSidList;
	TSidList  	_PlayersSid;

	/// the set of colors already used for display
	typedef std::set<CRGBA>	TSetColors;
	TSetColors	_UsedColors;

	/// map NLMISC::CEntityId with the associated display color for that player
	typedef map<const NLMISC::CEntityId, const CRGBA *>	TMapSidToColor;
	TMapSidToColor	_PlayersColor;

	/**
	* list Y coordinates of displayed player name, used when a user left click on the control to determine if he clicked on a player name,
	* and which one(to mute that player)
	* NB : the second type (ucstring*) is a pointer to the player name in _ItemsList
	*/
	typedef std::list< std::pair< std::pair<float,float>, std::pair<const ucstring*, const NLMISC::CEntityId*> > >  TPairPFloatPStrSid;
	TPairPFloatPStrSid _NamesYPos;

	/// mouse current position
	mutable float	_MouseX;
	mutable float	_MouseY;

	/// heigth of the last computed string, used to determine if the mouse if over a line or not
	float			_LastStringHeight;

	/// the text displayed as 'player name' for system messages
	ucstring		_SystemDisplayText;

	/// text color for system messages
	CRGBA			_SysTextColor;
	/// text color for muted players messages (the messages received before the player was muted)
	CRGBA			_MutedColor;
	/// text highlight color
	CRGBA			_HighlightColor;
	/// text highlight color for muted players
	CRGBA			_MutedHighlightColor;

	/// the currently selected character name (either by left click or right click) (pointer to the player name in _ItemsList)
	ucstring*		_SelectedPlayer;

	/// the currently selected character NLMISC::CEntityId (either by left click or right click)
	NLMISC::CEntityId*			_SelectedPlayerSid;

	/// number of the function to run when the user left click on a character name
	uint			_LeftClickFunction;

	/// number of the function to run when user right click on a player name
	uint			_RightClickFunction;

	/**
	* the ending position in the list of NLMISC::CEntityId for display, by default it's the last item in the list (the newest one)
	* if auto scroll mode is 'on' (_AutoScroll == true), this iterator is allways equal to the last item in the list
	* this iterator is used to keep the NLMISC::CEntityId List and the ucstring list coordinated
	*/
	TSidList::reverse_iterator	_EndingSidIterator;


	/// Dummy NLMISC::CEntityId used for system messages
	NLMISC::CEntityId _CommandSid;
};


#endif // NL_CHAT_CONTROL_H

/* End of chat_control.h */
