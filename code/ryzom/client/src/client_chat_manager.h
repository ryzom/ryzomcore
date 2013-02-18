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



#ifndef CLIENT_CHAT_MANAGER_H
#define CLIENT_CHAT_MANAGER_H

// misc
#include "nel/misc/types_nl.h"

// game share
//#include "game_share/chat_static_database.h"
#include "game_share/chat_group.h"
#include "game_share/dyn_chat.h"


// std
#include <map>
#include <string>

namespace NLMISC{
class CCDBNodeLeaf;
}


//#define OLD_STRING_SYSTEM

/**
 * CDynamicStringInfos
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */

#ifdef OLD_STRING_SYSTEM

struct CDynamicStringInfos
{
	/// string
	ucstring Str;

	/// index in the infos buffer, same as the index in the client dynamic string known buffer
	uint32 Index;

	/// true if the string has a Huffman code
	bool IsHuffman;

	/// true if we received the association
	bool Associated;

	/// Constructor
	CDynamicStringInfos() : Associated(false), Index(0xffffffff), IsHuffman(true), Str("???") { }

};

#endif

/**
 * CChatDynamicDatabase
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
#ifdef OLD_STRING_SYSTEM

class CChatDynamicDatabase
{
public :

	/// Add a string to dynamic base( used by client, add the string with its code )
	/// \param index the index of the string
	/// \param str the string
	/// \param huffCode the Huffman code(may be empty)
	/// \return the index of the string
	uint32 add( uint32 index, ucstring& str, std::vector<bool>& huffCode );

	/// Get the string from its Huffman code
	/// \param str will be filled with the string
	/// \param bms contains the Huffman code
	void decodeString( ucstring& str, NLMISC::CBitMemStream& bms );

	/// Get infos on the dynamic string
	CDynamicStringInfos * getDynamicStringInfos( uint32 index );

	/// Destructor
	~CChatDynamicDatabase();

private :

	/// Provides a Huffman tree to get huffman code of a string
/	CHuffman _Huffman;

	/// Dynamic base
	std::map< uint32, CDynamicStringInfos *> _Data;

	/// Map to find index from the string (only for uncoded strings)
	std::map< ucstring, uint32> _StringToIndex;

};
#endif





// ***************************************************************************
/**
 * CClientChatManager
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CClientChatManager
{
public:
	class IChatDisplayer
	{
	public:
		/**
		 *	\param mode in which channel should this message goes
		 *	\param dynChatId is valid only if mode==dyn_chat. This the Id of channel (not the index in DB!)
		 */
		virtual void	displayChat(TDataSetIndex compressedSenderIndex, const ucstring &ucstr, const ucstring &rawMessage, CChatGroup::TGroupType mode, NLMISC::CEntityId dynChatId, ucstring &senderName, uint bubbleTimer=0) =0;
		/**
		 *	display a player tell message
		 */
		virtual void	displayTell(/*TDataSetIndex senderIndex, */const ucstring &ucstr, const ucstring &senderName) =0;
		/**
		 *	Clear a channel.
		 *	\param dynChatDbIndex is valid only if mode==dyn_chat. Contrary to displayChat, this is the Db Index (0..MaxDynChanPerPlayer)
		 */
		virtual void	clearChannel(CChatGroup::TGroupType mode, uint32 dynChatDbIndex) =0;
	};

public :
	CClientChatManager();
	/**
	 * Init the manager. Init the static database
	 */
	void init( const std::string& staticDBFileName );

	// InGame init/release. call init after init of database
	void	initInGame();
	void	releaseInGame();

	/**
	 * Return a reference on the static database
	 */
//	CChatStaticDatabase& getStaticDB() { return _StaticDB; }

	/**
	 * Return a reference on the dynamic database
	 */
//	#ifdef OLD_STRING_SYSTEM
//		CChatDynamicDatabase& getDynamicDB() { return _DynamicDB; }
//	#endif

	/**
	 * Transmit a chat message to the last target group (see setChatMode)
	 * \param str is the chat content (truncated to 255 char max)
	 * \param isChatTeam special case for Chat TEAM
	 */
	void chat( const ucstring& str, bool isChatTeam = false );

	/**
	 * Transmit a chat message to the receiver
	 * \param receiver is the name of the listening char (truncated to 255 char max)
	 * \param str is the chat content (truncated to 255 char max)
	 */
	void tell( const std::string& receiver, const ucstring& str );

	/** Get the last name of the people with which a 'tell' has been done, then move that name at the start of the list
	  */
	const ucstring *cycleLastTell();

	/** Set the max number of name in the tell list
	  */
	void  setTellListSize(uint numPeople);


	/**
	 * Add or remove a character from the ignore list
	 * \param filter is the filter id
	 */
	void filter( uint8 filter );

	/**
	 * Change the chat mode
	 * \param mode is the chat mode( say/shout/group/clade )
	 * \param dynChannelId the dynamic channel id (if group==dyn_chat).
	 *	Not the db index! Use getDynamicChannelIdFromDbIndex() if you got a dbIndex
	 */
	void setChatMode(CChatGroup::TGroupType group, TChanID dynamicChannelId=NLMISC::CEntityId::Unknown);

	/// Reset the chat mode to force the client to resend it. Used during far TP.
	void resetChatMode();

	/**
	 * update chat mode button
     */
	void updateChatModeAndButton(uint mode, uint32 dynamicChannelDbIndex = 0);

	/**
	 * Get the string for a tell. display now if ready or delay in flushBuffer()
	 */
	void processTellString(NLMISC::CBitMemStream& bms, IChatDisplayer &chatDisplayer);

	/**
	 * Get the string for a far tell. display now if ready or delay in flushBuffer()
	 */
	void processFarTellString(NLMISC::CBitMemStream& bms, IChatDisplayer &chatDisplayer);

	/**
	 * Extract and decode the chat string from the stream. display now if ready or delay in flushBuffer()
	 */
	void processChatString( NLMISC::CBitMemStream& bms, IChatDisplayer &chatDisplayer);

	/**
	 * Extract and decode the chat string from the stream. display now if ready or delay in flushBuffer()
	 *	Difference with processTellString() is that processTellString2() receive a DynamicString for the message
	 */
	void processTellString2(NLMISC::CBitMemStream& bms, IChatDisplayer &chatDisplayer);

	/**
	 * Extract and decode the chat string from the stream. display now if ready or delay in flushBuffer()
	 *	Difference with processChatString() is that processChatString2() receive a DynamicString for the message
	 */
	void processChatString2(NLMISC::CBitMemStream& bms, IChatDisplayer &chatDisplayer);

	/**
	 * Extract and decode the chat string from the stream.
	 *	the stream here is only a iunt32 for the id of the dynamic string
	 *	\param type where do you want this string to go (dyn_chat is not allowed)
	 */
	void processChatStringWithNoSender( NLMISC::CBitMemStream& bms, CChatGroup::TGroupType type, IChatDisplayer &chatDisplayer);

	/**
	 * Process any waiting string
	 */
	void flushBuffer(IChatDisplayer &chatDisplayer);

	/**
	 * Extract and decode the chat string from a vector a uint64
	 * \param args vector of argument
	 * \param str string with parameter values at end
	 * \param result decoded string
	 * \return true if the string is finalize, false if some param are missing from network
	 */
	bool getString( ucstring &result, std::vector<uint64>& args, const ucstring& strbase );

	// build a sentence to be displayed in the chat (e.g add "you say", "you shout", "[user name] says" or "[user name] shout")
	static void	 buildChatSentence(TDataSetIndex compressedSenderIndex, const ucstring &sender, const ucstring &msg, CChatGroup::TGroupType type, ucstring &result);
	// build a sentence to be displayed in the tell
	static void	 buildTellSentence(const ucstring &sender, const ucstring &msg, ucstring &result);


	/// \name Dynamic Chat channel mgt
	// @{
	// Use info from DB SERVER:DYN_CHAT. return 0 if fails
	TChanID	getDynamicChannelIdFromDbIndex(uint32 dbIndex);
	// Use info from DB SERVER:DYN_CHAT. return -1 if fails
	sint32	getDynamicChannelDbIndexFromId(TChanID channelId);
	// return true if channel exist (getDynamicChannelDbIndexFromId>=0 && <MaxDynChan)
	bool	isDynamicChannelExist(TChanID channelId);
	// get the name id from the db index
	uint32	getDynamicChannelNameFromDbIndex(uint32 dbIndex);
	// @}


private :

	uint8				_ChatMode;

	/// static database
//	CChatStaticDatabase _StaticDB;

	/// dynamic database
//	#ifdef OLD_STRING_SYSTEM
//		CChatDynamicDatabase _DynamicDB;
//	#endif

	// List of messages that wait for String to complete.
	struct CChatMsgNode
	{
		TDataSetIndex	CompressedIndex;
		uint32			SenderNameId;
		uint8			ChatMode;
		NLMISC::CEntityId	DynChatChanID;
		// For Chat and Tell messages
		ucstring		Content;
		// For Chat2 and Tell2 messages
		uint32			PhraseId;
		// Use PhraseId or Content?
		bool			UsePhraseId;
		// displayTell() or displayChat()
		bool			DisplayAsTell;

		CChatMsgNode(const CChatMsg &chatMsg, bool displayAsTell)
		{
			CompressedIndex= chatMsg.CompressedIndex;
			SenderNameId= chatMsg.SenderNameId;
			ChatMode= chatMsg.ChatMode;
			DynChatChanID= chatMsg.DynChatChanID;
			Content= chatMsg.Content;
			PhraseId= 0;
			UsePhraseId= false;
			DisplayAsTell= displayAsTell;
		}

		CChatMsgNode(const CChatMsg2 &chatMsg, bool displayAsTell)
		{
			CompressedIndex= chatMsg.CompressedIndex;
			SenderNameId= chatMsg.SenderNameId;
			ChatMode= chatMsg.ChatMode;
			DynChatChanID= NLMISC::CEntityId::Unknown;
			PhraseId= chatMsg.PhraseId;
			UsePhraseId= true;
			DisplayAsTell= displayAsTell;
		}
	};
	std::list<CChatMsgNode>		_ChatBuffer;

	// peoples
	std::list<ucstring> _TellPeople; // the last people on which tells ha been done
	uint				_NumTellPeople;
	uint				_MaxNumTellPeople;

	/// \name Dynamic Chat channel mgt
	// @{
	TChanID				_ChatDynamicChannelId;
	NLMISC::CCDBNodeLeaf	*_DynamicChannelNameLeaf[CChatGroup::MaxDynChanPerPlayer];
	NLMISC::CCDBNodeLeaf	*_DynamicChannelIdLeaf[CChatGroup::MaxDynChanPerPlayer];
	// Id cached. If different from precedent, then the channel must be flushed
	enum	{DynamicChannelEmptyId=-1};
	uint32				_DynamicChannelIdCache[CChatGroup::MaxDynChanPerPlayer];
	void				updateDynamicChatChannels(IChatDisplayer &chatDisplayer);
	// @}

private :

	/**
	 * Extract and decode the chat string from the stream
	 * \param bms the bit mem stream
	 * \param str string with parameter values at end (str will change after)
	 * \return decoded string (str)
	 */
	ucstring getString( NLMISC::CBitMemStream& bms, ucstring& str );

};

/** This class enable you to store a static string + arg and evaluate it every frame.
    All args could perhaps not be translated because all dynamic string are not received right now.
	So you can call getString() every time to
 */
#ifdef OLD_STRING_SYSTEM
class CNetworkString
{
	ucstring StaticString;
public:
	std::vector<uint64> Args;

	bool getString (ucstring &result, CClientChatManager *mng);

	void setString (const ucstring &staticStringId, CClientChatManager *mng);
};
#endif



#endif // CLIENT_CHAT_MANAGER_H

/* End of client_chat_manager.h */
