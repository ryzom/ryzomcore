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




#ifndef STRING_MANAGER_CLIENT_H
#define STRING_MANAGER_CLIENT_H

#include "nel/misc/static_map.h"
#include "nel/misc/ucstring.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/i18n.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/diff_tool.h"
#include "game_share/brick_families.h"
#include "game_share/skills.h"


namespace STRING_MANAGER
{

class IStringWaiterRemover;
class IStringWaitCallback;

class CStringManagerClient
{
	friend class IStringWaiterRemover;
	friend class IStringWaitCallback;
public:
	// Singleton pattern implementation
	static CStringManagerClient	*instance();
	static void	release(bool mustReleaseStaticArrays);

	/** Prepare the string manager to use a persistent string cache.
	 *	There is one cache file for each language and for each encountered shard.
	 */
	void initCache(const std::string &shardId, const std::string &languageCode);
	/** Clear the current string table and load the content of the cache file.
	 *	This method is called after receiving the impulse RELOAD_CACHE from
	 *	IOS.
	 *	If the received timestamp and the file timestamp differ, the file cache
	 *	is reseted.
	 */
	void loadCache(uint32 timestamp);
	bool isCacheLoaded()	{return _CacheLoaded;};
	// Force the cache to be saved
	void flushStringCache();

	bool getString(uint32 stringId, ucstring &result);
	void waitString(uint32 stringId, const IStringWaiterRemover *premover, ucstring *result);
	void waitString(uint32 stringId, IStringWaitCallback *pcallback);
	bool getDynString(uint32 dynStringId, ucstring &result);
	void waitDynString(uint32 stringId, const IStringWaiterRemover *premover, ucstring *result);
	void waitDynString(uint32 stringId, IStringWaitCallback *pcallback);

	void receiveString(uint32 stringId, const ucstring &str);
	void receiveDynString(NLMISC::CBitMemStream &bms);

	void releaseDynString(uint32 stringId);

	// Yoyo: Append to the I18N some words (skills, bricks...)
	static void				initI18NSpecialWords(const std::string &languageCode);
	static void				specialWordsMemoryCompress();
	// Yoyo: Replace the Brick Name with Filled stats (CSBrickManager work). No-Op if not found
	static void				replaceSBrickName(NLMISC::CSheetId id, const ucstring &name, const ucstring &desc, const ucstring &desc2);
	static void				replaceDynString(const ucstring &name, const ucstring &text);

	// Get the Localized Name of the Places.
	static const ucchar *getPlaceLocalizedName(const std::string &placeNameID);
	// Get the Localized Name of the faction (for the fame)
	static const ucchar *getFactionLocalizedName(const std::string &factionNameID);
	// Get the Localized Name of the Skill.
	static const ucchar *getSkillLocalizedName(SKILLS::ESkills e);
	// Get the Localized Name of the Item.
	static const ucchar *getItemLocalizedName(NLMISC::CSheetId id);
	// Get the Localized Name of the Creature.
	static const ucchar *getCreatureLocalizedName(NLMISC::CSheetId id);
	// Get the Localized Name of the SBrick.
	static const ucchar *getSBrickLocalizedName(NLMISC::CSheetId id);
	// Get the Localized Name of the SPhrase.
	static const ucchar *getSPhraseLocalizedName(NLMISC::CSheetId id);

	// Get the Localized Description of the Skill.
	static const ucchar *getSkillLocalizedDescription(SKILLS::ESkills e);
	// Get the Localized Descriptionof the Item.
	static const ucchar *getItemLocalizedDescription(NLMISC::CSheetId id);
	// Get the Localized Description of the SBrick.
	static const ucchar *getSBrickLocalizedDescription(NLMISC::CSheetId id);
	// Get the Localized Composition Description of the SBrick.
	static const ucchar *getSBrickLocalizedCompositionDescription(NLMISC::CSheetId id);
	// Get the Localized Description of the SPhrase.
	static const ucchar *getSPhraseLocalizedDescription(NLMISC::CSheetId id);

	// Get the Localized Title name
	static const ucchar *getTitleLocalizedName(const ucstring &titleId, bool women);
	static std::vector<ucstring> getTitleInfos(const ucstring &titleId, bool women);

	// Get the Localized name of a classification type
	static const ucchar *getClassificationTypeLocalizedName(EGSPD::CClassificationType::TClassificationType type);

	// Outpost name
	static const ucchar *getOutpostLocalizedName(NLMISC::CSheetId id);
	// Outpost description
	static const ucchar *getOutpostLocalizedDescription(NLMISC::CSheetId id);
	// Outpost building name
	static const ucchar *getOutpostBuildingLocalizedName(NLMISC::CSheetId id);
	// Outpost building description
	static const ucchar *getOutpostBuildingLocalizedDescription(NLMISC::CSheetId id);
	// Squad name
	static const ucchar *getSquadLocalizedName(NLMISC::CSheetId id);
	// Squad description
	static const ucchar *getSquadLocalizedDescription(NLMISC::CSheetId id);

private:
	// constructor.
	CStringManagerClient();
	// destructor.
	~CStringManagerClient();

	/// A string waiter is destoyed, remove any waiting string reference from it.
	void removeStringWaiter(const IStringWaiterRemover *remover);
	/// A string wait callback  is destoyed, remove any waiting string reference from it.
	void removeStringWaiter(const IStringWaitCallback *callback);

	enum TParamType
	{
		string_id,
		integer,
		time,
		money,
		dyn_string_id,
		sheet_id
	};

	struct TParamValue
	{
		TParamType				Type;
		ucstring::size_type		ReplacementPoint;
		union
		{
			uint32	StringId;
			sint32	Integer;
			uint32	Time;
			uint64	Money;
			uint32	DynStringId;
		};
	};

	struct TDynStringInfo
	{
		enum TStatus
		{
			received,
			serialized,
			complete
		};
		TStatus						Status;
		NLMISC::CBitMemStream		Message;
		uint32						StringId;
		std::vector<TParamValue>	Params;
		ucstring					String;
	};

	enum
	{
		EmptyStringId= 0,
		EmptyDynStringId= 0xffffffff
	};

	struct TStringWaiter
	{
		/// Pointer to the ucstring to fill
		ucstring					*Result;
		/// Pointer to the remover that contains this string reference
		const IStringWaiterRemover	*Remover;
	};

	bool	buildDynString(TDynStringInfo &dynInfo);


	/// Container for simple strings
	typedef CHashMap<uint, ucstring>			TStringsContainer;
	/// Container for dyn strings
	typedef CHashMap<uint, TDynStringInfo>		TDynStringsContainer;
	/// Container of string reference waiting for value.
	typedef CHashMultiMap<uint, TStringWaiter>	TStringWaitersContainer;
	/// Container of string reference to string callback object.
	typedef CHashMultiMap<uint, IStringWaitCallback*>	TStringCallbacksContainer;


	TStringsContainer		_ReceivedStrings;
	CHashSet<uint>	    	_WaitingStrings;

	TDynStringsContainer	_ReceivedDynStrings;
	TDynStringsContainer	_WaitingDynStrings;

	/// String waiting the string value from the server.
	TStringWaitersContainer		_StringsWaiters;
	// String waiting the dyn string value from the server.
	TStringWaitersContainer		_DynStringsWaiters;

	// Callback for string value from the server
	TStringCallbacksContainer	_StringsCallbacks;
	// Callback for dyn string value from the server
	TStringCallbacksContainer	_DynStringsCallbacks;

	// Return value for waiting string..
	static ucstring			_WaitString;

	// Singleton pattern implementation
	static CStringManagerClient	*_Instance;

	//@{
	//\name Cache management
	/// Flag for cache management initialisation done.
	bool			_CacheInited;
	/// Shard id is used to identify the cache file to use.
	std::string		_ShardId;
	/// Language code is used to identify the cache file to use.
	std::string		_LanguageCode;
	/// Timestamp (unix date) of the corrently loaded cache file.
	uint32			_Timestamp;
	/// Fullpath name of the current cache file.
	std::string		_CacheFilename;
	/// Flag to incate that the cache is loaded.
	bool			_CacheLoaded;
	/// Waiting string to be saved in cache
	struct CCacheString
	{
		uint32		StringId;
		ucstring	String;
	};
	std::vector<CCacheString>	_CacheStringToSave;
	//@}

	// SpecialItems.
	class	CItem
	{
	public:
		// The Name of the item
		ucstring		Name;
		// The Women Name of the item
		ucstring		WomenName;
		// Description of the item
		ucstring		Desc;
		// Optional Second description (For SBrick composition for example)
		ucstring		Desc2;

		void	serial(NLMISC::IStream &f)
		{
			sint	ver= f.serialVersion(1);
			f.serial(Name);
			if (ver >= 1)
				f.serial(WomenName);
			f.serial(Desc);
			f.serial(Desc2);
		}
	};

	static bool _SpecItem_MemoryCompressed;

	static	std::map<std::string, CItem> _SpecItem_TempMap;
	static std::vector<ucstring> _TitleWords;
	static std::map<ucstring, ucstring> _DynStrings;


	static char *_SpecItem_Labels;
	static ucchar *_SpecItem_NameDesc;
	struct CItemLight
	{
		char	*Label;
		ucchar	*Name;
		ucchar	*WomenName;
		ucchar	*Desc;
		ucchar	*Desc2;
	};
	struct CItemLightComp
	{
		bool operator()(const CItemLight &x, const CItemLight &y) const
		{
			return strcmp(x.Label, y.Label) < 0;
		}
	};

	static std::vector<CItemLight> _SpecItems;

	static	const ucchar *getSpecialWord(const std::string &label, bool women = false);
	static	const ucchar *getSpecialDesc(const std::string &label);
	static	const ucchar *getSpecialDesc2(const std::string &label);

	// Check Files for the Packed string.
	class CFileCheck
	{
	public:
		uint32		ReferenceDate;
		uint32		AdditionDate;

		CFileCheck()
		{
			ReferenceDate= 0;
			AdditionDate= 0;
		}

		bool operator==(const CFileCheck &o) const
		{
			return ReferenceDate==o.ReferenceDate && AdditionDate==o.AdditionDate;
		}

		void	serial(NLMISC::IStream &f)
		{
			f.serialVersion(0);
			f.serial(ReferenceDate);
			f.serial(AdditionDate);
		}
	};

	// The Header for the Packed String Client
	class CPackHeader
	{
	public:
		// The language code used
		std::string		LanguageCode;
		// The Modification Dates of each word files
		std::vector<CFileCheck>		FileChecks;
		// If must rebuild the packed version for any reason.
		uint32			PackedVersion;

		void	serial(NLMISC::IStream &f)
		{
			f.serialCheck(NELID("_RTS"));
			f.serialCheck(NELID("KCAP"));
			f.serialVersion(0);
			f.serial(PackedVersion);
			f.serial(LanguageCode);
			f.serialCont(FileChecks);
		}
	};

	// return true if OK (same).
	static bool checkWordFileDates(std::vector<CFileCheck> &fileChecks, const std::vector<std::string> &fileNames, const std::string &languageCode);

};


/** Class that want to register to wait for string
 *	need to derive from this class.
 */
class IStringWaiterRemover
{
public:
	virtual ~IStringWaiterRemover()
	{
		// signal the string manager that this waiter is destroyed
		CStringManagerClient::instance()->removeStringWaiter(this);
	}
};

/** Implement this class if you want to wait for
 *	string to be delivered.
 */
class IStringWaitCallback
{
public:
	/// Overide this method to receive callback for string.
	virtual void onStringAvailable(uint /* stringId */, const ucstring &/* value */) {}
	/// Overide this method to receive callback for dynamic string.
	virtual void onDynStringAvailable(uint /* stringId */, const ucstring &/* value */) {}

	virtual ~IStringWaitCallback()
	{
		// signal the string manager that this waiter is destroyed
		CStringManagerClient::instance()->removeStringWaiter(this);
	}

};


/** A proxy file loader for CI18N
 *	The implementation will check if there is any
 *	new content in the translation/work directory of ryzom
 *	then overwrite the value found in data/gamedev/language
 */
class CLoadProxy : public NLMISC::CI18N::ILoadProxy, public TStringDiff::IDiffCallback
{
	void loadStringFile(const std::string &filename, ucstring &text);

	void onEquivalent(uint addIndex, uint refIndex, TStringDiffContext &context);
	void onAdd(uint addIndex, uint refIndex, TStringDiffContext &context);
	void onRemove(uint addIndex, uint refIndex, TStringDiffContext &context);
	void onChanged(uint addIndex, uint refIndex, TStringDiffContext &context);
	void onSwap(uint newIndex, uint refIndex, TStringDiffContext &context);

};

} // namespace STRING_MANAGER

#endif //STRING_MANAGER_CLIENT_H
