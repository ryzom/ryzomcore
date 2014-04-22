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



#ifndef STRING_MANAGER_H
#define STRING_MANAGER_H

#include "nel/misc/time_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/file.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/sheet_id.h"
#include "nel/net/service.h"
#include "nel/net/message.h"
#include "nel/georges/u_form.h"
#include "game_share/ryzom_entity_id.h"
//#include "game_share/chat_dynamic_database.h"
#include "game_share/string_manager_sender.h"
#include "game_share/gender.h"
#include "game_share/r2_share_itf.h"

#include <map>
#include <string>
#include <list>


namespace NLMISC
{
	class CBitMemStream;
}

class CStringManager;
class CCharacterInfos;


/** Manage string organized as conditional clause grouped into phrase.
 *	This class can choose at runtime one of the clause depending
 *	on passed parameters.
 */
class CStringManager
{
public:
	/// This is a list of available language.
	enum TLanguages
	{
		work,
		english,
		german,
		french,
		russian,
		spanish,
		NB_LANGUAGES
	};

	// entry used to store associations between user id , front end id and language codes
	struct SUserLanguageEntry
	{
		inline SUserLanguageEntry( NLNET::TServiceId frontEndId, TLanguages language )
			:	FrontEndId(frontEndId),
				Language(language)
		{}
		NLNET::TServiceId	FrontEndId;
		TLanguages			Language;
	};
	//type of the container storing associations between user id , front end id and language codes
	typedef std::map< uint32, SUserLanguageEntry > TUserLanguagesContainer;

	/// Types of operator used in clauses.
	enum TOperator
	{
		equal,
		notEqual,
		greater,
		greaterEqual,
		less,
		lessEqual,
		nop
	};

	/** Types of property specification.
	 *	This is use int the conditions string.
	 *	Only genre is supported for now.
	 *	Property spec allow the writing of
	 *	condition that depend on a specific property
	 *	of an entity.
	 *	With no property spec, the default meaning
	 *	of the entity if used (if available !).
	 */
	enum TProperty
	{
		genre,
		none
	};

	/** Describe a parameter
	 *	Each parameter have a name and a type (as in C).
	 *	The name can be spelled like the type ;)
	 */
	struct TParamId
	{
		/// Parameter index.
		uint			Index;
		/// Type of the parameter
		STRING_MANAGER::TParamType		Type;
		/// Name of the parameter
		std::string		Name;
	};

	/** Describe a replacement point into the text string.
	 *	Replacement are described in the text string by a
	 *	replacement tag between too '$' char.
	 *	Replacement tag must contain the name of the parameter
	 *	to fill in and optionally a format specifier.
	 */
	struct TReplacement
	{
		/// Index of the parameter
		uint					ParamIndex;
		/// Format of the replacement (mainly refer to entities excel files columns names).
		std::string				Format;
		/// The first place for insertion of the replacement (opening $)
		ucstring::size_type		InsertPlace;
		/// The first continuation char after the replacement tag (after the closing $)
		ucstring::size_type		ContinuePlace;
	};

	/** Store a parameter during message decoding.
	*/
	struct TStringParam
	{
		TParamId			ParamId;

		/// All this properties should have been in an unnamed union
		/// but CEntityId has a copy constructor, so it can't be
		/// in an union.
		/// So, I completely removed the union. :(
		NLMISC::CEntityId	EId;
	protected:
		TAIAlias	_AIAlias;
	public:
		sint32				Int;
		uint32				Time;
		uint64				Money;
		uint32				Enum;
		uint32				StringId;
		NLMISC::CSheetId	SheetId;
		std::string			Identifier;
		ucstring			Literal;

	};

	/** This describe a condition.
	 *	Condition are use to select the good clause in a phrase
	 *	depending on parameter value.
	 *	Condition can also be combined in the condition string
	 *	with a '&' char.
	 *	Combination of condition is always AND (as '&' suggest).
	 */
	struct TCondition
	{
		/// This is the parameter index. If 0, then it is the special 'self' var
		uint		ParamIndex;
		/// This is the parameter name (or operand name)
//		std::string	Operand;
		/// This is the specific optional property of the parameter to test.
		std::string Property;
		/// This is the test operator.
		TOperator	Operator;
		/// This is the reference value as string
		std::string	ReferenceStr;
		/// This is the reference value as integer.
		sint32		ReferenceInt;
	};

	struct CPhrase;
	class CParameterTraits;

	typedef std::vector<std::pair<STRING_MANAGER::TParamType, std::string> >	TParameterTraitList;

	/** This describe a clause.
	 *	Clause are a combination of a list of zero or more conditions and an associated string.
	 *	There is also the prebuild client string (with % style and type insertion points)
	 */
	struct CClause
	{
		/** The list of condition to validate this clause
		 *	This is a double vector.
		 *	First level is for ORed condition, second level is
		 *	for the ANDed one.
		 */
		std::vector<std::vector<TCondition> >	Conditions;
		/// The text string as read in the string file.
		ucstring					String;
		/// The text string build for the client.
		ucstring					ClientString;
		/// The client string id.
		uint32						ClientStringId;
		/// The parameters and format in order of apparency in the client string
		std::vector<TReplacement>	Replacements;

		/// Evaluate this clause.
		bool eval(TLanguages lang, const CCharacterInfos *charInfo, const CPhrase *phrase);
	};

	/** Base class for parameter 'traits'.
	 *	When adding new type of param, you must provide a new implementation
	 *	of this class and then, update the createParameterTraits() static method
	 *	to create
	*/
	class CParameterTraits : public TStringParam
	{
		/** Static storage for parameter traits model*/
//		static CParameterTraits	*_Models[];
		static std::vector<CParameterTraits	*>	_Models;

		/// Create a copy of the class, used by factory method createParameterTraits()
		virtual CParameterTraits	*clone() =0;

	public:

		static void init();

		/// The name of the type, used to retrieve words translation file.
		const std::string	TypeName;

		/** Factory like method.
		 */
		static CParameterTraits *createParameterTraits(STRING_MANAGER::TParamType type);

		/** Return the list of supported param type name.*/
		static std::vector<std::pair<STRING_MANAGER::TParamType, std::string> >	getParameterTraitsNames();

		CParameterTraits(STRING_MANAGER::TParamType type, const std::string &typeName)
			: TypeName(typeName)
		{
			ParamId.Type = type;
		}
		virtual ~CParameterTraits()
		{
		};

		/// Return parameter id, ie, the name that is use to identity the data row in the csv file.
		virtual const std::string &getParameterId() const =0;
		/// Extract the parameter value from a message.
		virtual bool extractFromMessage(NLNET::CMessage &message, bool debug) =0;
		/// Fill a bitmemstrean with the parameter value.
		virtual void fillBitMemStream( const CCharacterInfos *charInfo,TLanguages language, const TReplacement &rep, NLMISC::CBitMemStream &bms);
		/// Eval a condition with this parameter.
		virtual bool eval(TLanguages lang, const CCharacterInfos *charInfo, const TCondition &cond) const;
		/// set a default value
		virtual void setDefaultValue() =0;
	};

	/** This describe a phrase.
	 *	Phrase are what we want to say to the client.
	 *	Phrase are uniquely named and specific a list
	 *	of parameters and a list of clauses.
	 *	The clauses are choose depending on the parameters value.
	 */
	struct CPhrase
	{
		std::string						Name;
		std::vector<CParameterTraits*>	Params;
		std::vector<CClause>			Clauses;

		const CClause &eval(const std::vector<NLMISC::CEntityId> &entities);

		// This is a quite dirty hack to avoid temporary phrases to delete their params after copied to static phrases
		CPhrase&	operator = (const CPhrase& p)
		{
			CPhrase&	pp = const_cast<CPhrase&>(p);
			Name = pp.Name;
			Params = pp.Params;
			pp.Params.clear();
			Clauses = pp.Clauses;
			return *this;
		}

		~CPhrase()
		{
			while (!Params.empty())
			{
				delete Params.back();
				Params.pop_back();
			}
		}
	};

	/** Container for entity words file.
	 *	Entity words file are excell sheet that contains the different
	 *	words associated to a type of entity. Columns are named, and
	 *	there must be a columns called 'name'.
	 *	Each row contain the words for a specific entity of a given type.
	 *	The, you can retrieve word by asking this class with the entity
	 *	name and the column name.
	 */
	class CEntityWords
	{
		friend class CStringManager;
		/// number of column for this entity words
		uint32							_NbColums;
		/// Row name/index.
		std::map<std::string, uint32>	_RowInfo;
		/// column names/index
		std::map<std::string, uint32>	_ColumnInfo;
		/** Raw data storage. Must be (NbColums * ColumnInfo.size()) * sizeof(uint32)
		 *	Each entry store the id of the mapped string.
		 *	NB : the external memory pointed by Data is not managed. CEntityWord can be copied, stored
		 *	in stl container, but the external allocation remain the same.
		 *	Only CStringManager is aware for when to alloc or release the Data memory.
		 *	This give a little more reponsability to CStringManager but greatly improve
		 *	copy operation.
		 *	We just need to take care about Data memory when we destroy the last
		 *	of a CEntityWords copies. This should append rarely (perhaps when reloading
		 *	entity translation files...)
		 */
		uint32	*_Data;

	public:

		// default constructor
		CEntityWords() : _NbColums(0), _Data(NULL)	{}

		/// Retrieve a cell string id
		uint32	getStringId(const std::string &rowName, const std::string columnName) const;

		/** Retrieve the index for a given row name.
		 *	Return 0xffffffff if the specified row cannot be found.
		 */
		uint32 getRowId(const std::string &rowName) const
		{
			std::map<std::string, uint32>::const_iterator it(_RowInfo.find(rowName));
			if (it != _RowInfo.end())
				return it->second;

			return 0xffffffff;
		}
		/** Retrieve the index for a given column name.
		 *	Return 0xffffffff if the specified column cannot be found.
		 */
		uint32 getColumnId(const std::string &columnName) const
		{
			std::map<std::string, uint32>::const_iterator it(_ColumnInfo.find(columnName));
			if (it != _ColumnInfo.end())
				return it->second;

			return 0xffffffff;
		}
		/// Retrieve a cell string id. High speed version.
		uint32	getStringId(uint32 rowIndex, uint32 columnIndex) const
		{
			if (rowIndex >= _RowInfo.size())
				return 0;
			if (columnIndex >= _NbColums)
				return 0;

			return _Data[rowIndex * _NbColums + columnIndex];
		}
	};
	/** Container for data extracted from entity sheet.
	 *	This is used to store 'static' information like gender.
	 */
	struct TSheetInfo
	{
		/// The name of the entity model.
		std::string			SheetName;
		/// The race of the creature.
		std::string			Race;
		/// The gender of this entity model.
		GSGENDER::EGender	Gender;
		/// The display name
//		std::string			DisplayName;
		/// Creature profile (aka career)
		std::string			Profile;
		/// Creature chat profile
		std::string			ChatProfile;

		TSheetInfo() : Gender(GSGENDER::unknown)
		{}

		// load the values using the george sheet
		void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);

		// load/save the values using the serial system
		void serial (NLMISC::IStream &s)
		{
			s.serial(SheetName);
			s.serial(Race);
			s.serialEnum(Gender);
//			s.serial(DisplayName);
			s.serial(Profile);
			s.serial(ChatProfile);
		}

		// Event to implement any action when the sheet is no longer existent.
		// This method is call when a sheet have been read from the packed sheet
		// and the associated sheet file no more exist in the directories.
		void removed()
		{
			// any action that is needed if the sheet no more exist.
		}

		// return the version of this class, increments this value when the content hof this class changed
		static uint getVersion () { return 5; }
	};

	// configuration vars --------------------------
	/// Debug the string manager (default false)
	bool		ReadTranslationWork;
	/// Path to the work translation repository
	std::string	TranslationWorkPath;

	// Ring user item description
	struct TRingUserItemInfo
	{
		uint32	AIInstance;
		uint32	ItemNameId;
	};
	// Ring user item container
	typedef CHashMap<NLMISC::CSheetId, std::vector<TRingUserItemInfo>, NLMISC::CSheetIdHashMapTraits >	TRingUserItemInfos;

private:
	/// A name ordered container to store the phrases.
	typedef std::map<std::string, CPhrase>		TPhrasesContainer;
	/// A container to store string to id association
	typedef std::map<ucstring, uint32>			TMappedUStringContainer;
	/// A container to store the mapped string in order.
	typedef std::vector<ucstring>				TUStringContainer;
	/// A container to store the entity info.
	typedef std::map<NLMISC::CSheetId, TSheetInfo>		TSheetInfoContainer;

	/// All the mapped string with mapping index.
	TMappedUStringContainer			_StringIdx;
	/// All the string in order.
	TUStringContainer				_StringBase;
	NLMISC::CStringMapper			*_Mapper;
	/// An array of phrase container, one for each supported language.
	TPhrasesContainer				_AllPhrases[NB_LANGUAGES];

	/// A static array of language name.
	static std::string				_LanguageCode[NB_LANGUAGES];
	/// An array of entity words for each entity, each language.
	CEntityWords					_AllEntityWords[NB_LANGUAGES][STRING_MANAGER::NB_PARAM_TYPES];

	/// All entity info ordered by sheet Id
	TSheetInfoContainer		_SheetInfo;

	//@{
	//\name Cache related data
	/// Flag that indicate if the cache is loaded.
	bool							_CacheLoaded;
	/// Timestamp for cache
	uint32							_CacheTimestamp;
	/// full fill name for cache file.
	std::string						_CacheFilename;
	//@}

	/// Associations between user ids and languages
	TUserLanguagesContainer			_UsersLanguages;

	/** Storage for bot name translation table. NB : this is not
	*	by client language translation but a shard wide translation.
	*	The container only contains string id.
	*/
//	std::hash_map<uint32, uint32>	_BotNameTranslation;
	std::map<uint32, uint32>	_BotNameTranslation;

	/** Storage for event faction translation table. NB : this is not
	*	by client language translation but a shard wide translation.
	*	The container only contains string id.
	*/
	std::map<uint32, uint32>	_EventFactionTranslation;

	/** Storage for user defined ring item
	 *	NB : the container key is on the item sheetId and NOT on the AI instanceId,
	 *	this is to speedup lookup for renamed item in the phrase construction
	 *	code (witch is the more intensive part of this feature).
	 *	Otherwise, we should always check the user instance id before checking
	 *	if the item is translated or not.
	 *	Obviously, this choice will slow down a little the storage of a new
	 *	set of user item in the container (because we need to first erase the
	 *	previous set by parsing the complete container).
	 */
	TRingUserItemInfos			_RingUserItemInfos;

	/// Language used in the setPhrase command.
	/** NB_LANGUAGES for all.
	*/
	TLanguages _DefaultSetPhraseLanguage;

	bool	_TestOnly;

public:
	/// A temporary storage used to resolve clause strings indirection.
	std::map<std::string, ucstring> TempClauseStrings;

public:
	/// Constructor.
	CStringManager();

	void	setTestOnly()		{ _TestOnly = true;}

	/** Initialize the string manager.
	 *	Calling init while load all the translation files and build the internal data structure.
	 */
	void					init(NLMISC::CLog *log = NLMISC::InfoLog);

	/** Reload all the translation file.
	*/
	void					reload(NLMISC::CLog *log = NLMISC::InfoLog);

	/**
	 * Load Phrase file for a specified language
	 */
	void					loadPhraseFile(const std::string& filename, TLanguages language, const std::string& workfilename, NLMISC::CLog *log = NLMISC::InfoLog);

	/**
	 * Merge EntityWords
	 */
	void					mergeEntityWordsFile(const std::string& filename, TLanguages language, STRING_MANAGER::TParamType wordType, NLMISC::CLog *log = NLMISC::InfoLog);

	/**
	 * Load EntityWords file
	 */
	void					loadEntityWordsFile(const std::string& filename, const std::string &workfilename, CEntityWords& words, NLMISC::CLog *log = NLMISC::InfoLog);

	/**
	 * Merge EntityWords
	 */
	void					mergeEntityWords(CEntityWords& dest, const CEntityWords& source, NLMISC::CLog *log = NLMISC::InfoLog);

	/**
	 * display EntityWords
	 */
	void					displayEntityWords(TLanguages language, STRING_MANAGER::TParamType wordType, const std::string& wc, NLMISC::CLog *log = NLMISC::InfoLog);

	/**
	 * reset entity word
	 */
	void					setEntityWord(const std::string& path, const ucstring& value);

	/**
	 * Load bot names file
	 */
	void					loadBotNames(const std::string& filename, bool resetBotNames, NLMISC::CLog *log = NLMISC::InfoLog);

	/**
	 * Set bot name
	 * WARNING: this method is quite slow, because it remaps all bot names each time you call it!!
	 */
	void					setBotName(const ucstring& botname, const ucstring& translation);

	/**
	 * Remap bot names
	 */
	void					remapBotNames();

	/**
	 * read repository
	 */
	void					readRepository(const std::string& path, TLanguages language, NLMISC::CLog *log = NLMISC::InfoLog);

	/** Clear all the cache and reload transation files
	*/
	void					clearCache(NLMISC::CLog *log = NLMISC::InfoLog);

	uint32					getCacheTimestamp()	{	return _CacheTimestamp; }

	/**
	 * Reload event faction translation table from file
	 */
	void					reloadEventFactions(NLMISC::CLog * log = NLMISC::InfoLog, std::string fileName = "");

	/** Translate a bot name using the shard global translation table.
	*/
	uint32					translateShortName(uint32 shortNameIndex);
	/** Translate a bot name using the shard global translation table.
	*	This version take the unmapped string.
	*/
	uint32					translateShortName(const ucstring &shortName);

	/** Translate a title/function.*/
	uint32					translateTitle(const std::string &title, TLanguages language);

	/** Translate an event faction.*/
	uint32					translateEventFaction(uint32 eventFactionId);
	uint32					translateEventFaction(const ucstring &eventFaction);

	/** Return the entity word class for a given lang and a given parameter type.
	 */
	const CEntityWords		&getEntityWords(TLanguages lang, STRING_MANAGER::TParamType type) const;

	/** Receive a phrase id and parameters from a server
	 *	and forward the selected clause to the client.
	 */
	void					receivePhrase(NLNET::CMessage &message, bool debug, const std::string &serviceName);

	/** Receive a phrase id and parameters from a server
	 *	and forward the selected clause to a user. Useful when a user has no selected character
	 */
	void					receiveUserPhrase(NLNET::CMessage &message, bool debug);

	void					broadcastSystemMessage(NLNET::CMessage &message, bool debug);
	/**
	 * Helper used to fill a bitmemstream with all the parameters that define a string
	 */
	bool					buildPhraseStream( CCharacterInfos * charInfo, uint32 seqNum, NLNET::CMessage & message, bool debug, CStringManager::TLanguages lang, NLMISC::CBitMemStream & bmsOut);

	/// Build message stream for missing phrase.
	void					buildMissingPhraseStream(CCharacterInfos * charInfo, uint32 seqNum, NLMISC::CBitMemStream & bmsOut, const std::string &phraseName);


	/** The client request a string.*/
//	void					requestString(const NLMISC::CEntityId &client, uint32 stringId);

	/** The user request a string.*/
	void					requestString(uint32 userId, uint32 stringId);

	/** Convert a string language code into it's enumerated value.
	 *	If none of the language match the given language string, then the
	 *	first enumerated language is returned (english).
	 */
	TLanguages				checkLanguageCode(const std::string &languageCode);
	/** Return the string representing the language code.
	 */
	const std::string		&getLanguageCodeString(TLanguages language);
	/** Return the entity info for a given sheet Id
	*/
	const TSheetInfo		&getSheetInfo(const NLMISC::CSheetId &sheetId);

	/** Return the sheetId associated to an Entity via the mirrored data.
	 */
	NLMISC::CSheetId		getSheetId(const NLMISC::CEntityId &entityId);
	/** Return the sheetServerId associated to an Entity via the mirrored data.
	 */
	NLMISC::CSheetId		getSheetServerId(const NLMISC::CEntityId &entityId);

	/// Store the string in the string base en return an Id
	uint32	storeString(const ucstring &str);
	/// Retreive a string in the string base
	const ucstring &getString(uint32 stringId);

	// TODO : temp, remove when dyndb removed.
//	ucstring	getEntityDisplayName(const NLMISC::CEntityId &eid);

	/// Send the requested string
	void					sendString( uint32 nameIndex, NLNET::TServiceId serviceId );

	/// Send the names of all online entities
	void					retrieveEntityNames( NLNET::TServiceId serviceId );

	/// update the language of a user and send the cache timestamp to the client.
	void updateUserLanguage( uint32 userId, NLNET::TServiceId frontEndId, const std::string & lang );

	// remove a user language association
	inline void removeUserLanguage( uint32 userId )
	{
		_UsersLanguages.erase( userId );
	}

	void setPhrase(NLNET::CMessage &message);
	void setPhraseLang(NLNET::CMessage &message);
	void setPhrase(std::string const& phraseName, ucstring const& phraseContent);
	void setPhrase(std::string const& phraseName, ucstring const& phraseContent, TLanguages language);
	/// Returns the language used in the setPhrase command.
	/** NB_LANGUAGES for all.
	*/
	TLanguages getDefaultSetPhraseLanguage() { return _DefaultSetPhraseLanguage; }
	/// Sets the language used in the setPhrase command.
	/** NB_LANGUAGES for all.
	*/
	void setDefaultSetPhraseLanguage(TLanguages language)  { _DefaultSetPhraseLanguage = language; }

	/// Store a set of user named item associated with an AIInstance
	void storeItemNamesForAIInstance(uint32 aiInstance, const std::vector < R2::TCharMappedInfo > &itemInfo);
	const TRingUserItemInfos &getUserItems()	{ return _RingUserItemInfos; }

private:

	void loadCache();

	/** Search for a param named paramName int the phrase.
	 *	If the search succeed, it fill pparamId to point to the parameterId and return true,
	 *	otherwise, false is returned.
	 */
	bool findParam(const CPhrase &phrase, const std::string paramName, const TParamId *&pparamId);

	//@{
	//\name phrase parsing methods
	/// Parse the clauses doc that contain all labeled indirect clause text.
	bool parseClauseStrings(const ucstring &clausesStrings);
	/// Parse the phrase doc contained in a string for the specified language
	void parsePhraseDoc(ucstring &doc, uint langNum);
	/// Parse a block of the phrase doc. A bloc contain one phrase.
	bool parseBlock(const ucstring &block, CPhrase &phrase);
	/// Parse a string to extract the position, name and format of replacement ($xx$).
	bool extractReplacement(const CPhrase &phrase, const ucstring &str, std::vector<TReplacement> &result);
	/// Parse a replacement tag.
	bool parseTag(const CPhrase &phrase, const ucstring &tag, TReplacement &rep);
	/// Parse a clause
	bool parseClauses(const CPhrase &phrase, ucstring::const_iterator &it, ucstring::const_iterator &last, std::vector<CClause> &clauses);
	/// Parse a condition
	bool parseCondition(const CPhrase &phrase, const ucstring &str, std::vector<TCondition> &result);
	/// Parse a marked string, ie a string delimited by [ and ]
//	bool parseMarkedString(ucchar openMark, ucchar closeMark, ucstring::const_iterator &it, ucstring::const_iterator &last, ucstring &result);
	/// Parse the param list. Param list are delimited by ( and ), param are separated by ','
	bool parseParamList(ucstring::const_iterator &it, ucstring::const_iterator &last, std::vector<CParameterTraits*> &result);
	/// Advance iterator to next non white space (tab, space, cr or lf)
//	void skipWhiteSpace(ucstring::const_iterator &it, ucstring::const_iterator &last);
	/// Parse a label. Label are defined as in C.
//	bool parseLabel(ucstring::const_iterator &it, ucstring::const_iterator &last, ucstring &result);
	//@}

	CEntityWords parseEntityWords(const ucstring &str);

};


// the string manager instance.
extern	CStringManager *SM;

// Local sender
class CIosLocalSender : public STRING_MANAGER::ISender
{
	void send(NLNET::CMessage &message, bool debug)
	{
		if (SM)
		{
			message.invert();
			SM->receivePhrase(message, debug, NLNET::IService::getInstance()->getServiceShortName());
		}
	}
};
extern	CIosLocalSender	IosLocalSender;

#endif //STRING_MANAGER_H
