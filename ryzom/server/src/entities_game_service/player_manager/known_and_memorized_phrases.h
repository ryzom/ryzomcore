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



#ifndef RY_KNOWN_MEMORIZED_PHRASES_H
#define RY_KNOWN_MEMORIZED_PHRASES_H

// game share
#include "game_share/sphrase_com.h"
#include "game_share/memorization_set_types.h"

// the max number of sentence that can be memorized in one set (must match client interface)
const uint8 MaxNbSentencesPerSet			= 20;

class CCharacter;

/// What keeps a character from executing a phrase
enum TPhraseRefusal
{
	PhraseAccepted = 0,
	/// the character does not know one of the bricks
	PhraseUnknownBrick,
	/// the bricks do not follow the phrase grammar: no phrase the action composer of the client builds
	PhraseAgainstGrammar,
	/// a brick excludes another one, or the credit does not cover the cost
	PhraseUnbalanced
};

/// Checks the bricks of a phrase a character learns, memorizes or executes. refusal receives what is wrong, for the log.
TPhraseRefusal checkPhraseOfCharacter(const std::vector<NLMISC::CSheetId> &bricks, const std::set<NLMISC::CSheetId> &knownBricks, std::string &refusal);

/**
 * struct for Known Phrases
 */
struct CKnownPhrase
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	CSPhraseCom			PhraseDesc;
	NLMISC::CSheetId	PhraseSheetId;

	bool empty() const;
	void clear();
	void serial(NLMISC::IStream &f);
};


/**
 * struct for a memorized phrase
 */
struct CMemorizedPhrase
{
	NL_INSTANCE_COUNTER_DECL(CMemorizedPhrase);
public:

	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	CMemorizedPhrase();
	explicit CMemorizedPhrase(const std::vector<NLMISC::CSheetId> &bricks, uint16 id);

	void clear();
	void serial(NLMISC::IStream &f);

	// clang-format off
	std::vector<NLMISC::CSheetId>	Bricks;
	uint16							PhraseId;
	/// The phrase is checked before it is executed: it was refused at its last check (a brick the character does not
	/// know, or bricks that make no valid phrase), or it was not checked yet. A refused phrase keeps its slot and is
	/// saved as it is; this mark is not saved, the phrase is checked again at every login and when the player tries it.
	bool							Refused;
	// clang-format on
};

/**
 * struct for memorization sets
 */
struct CMemorizationSet
{
	NL_INSTANCE_COUNTER_DECL(CMemorizationSet);
public:

	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	CMemorizationSet();
	~CMemorizationSet();

	void clear();

	/// returns whether the phrase could be built and was memorized
	bool memorize(uint8 i, const std::vector<NLMISC::CSheetId> &bricks, uint16 id, const TDataSetRow &rowId);

	void memorizeWithoutCheck(uint8 i, const std::vector<NLMISC::CSheetId> &bricks, uint16 id);

	/// memorize a phrase the character may not execute for now, without any check: it keeps its slot and is saved
	void memorizeRefused(uint8 i, const std::vector<NLMISC::CSheetId> &bricks, uint16 id);

	/// memorize starter phrases in easy order for newcomers
	bool memorizeStarterPhrase(const std::vector<NLMISC::CSheetId> &bricks, uint16 id);

	/// memorize in first found empty slot, returns true if successful, make no check
	bool memorizeInFirstEmptySlot(const std::vector<NLMISC::CSheetId> &bricks, uint16 id);
	
	void executePhrase(uint8 i, CCharacter *actor, const TDataSetRow &target, bool cyclic, bool enchant);

	void forget(uint8 i);

	/// forget All phrases
	void forgetAll();

	/// Re-memorise phrases to reconstruct them in case bricks inside had changed.
	void fixPhrases(const std::vector<CKnownPhrase> &knownPhrases, const std::set<NLMISC::CSheetId> &knownBricks, const TDataSetRow &rowId);

	/// have every phrase checked again before its next execution
	void checkAgainBeforeExecution();
	
	const std::vector<CMemorizedPhrase*>	&getMemorizedPhrases() const { return Phrases; }

	void serial(NLMISC::IStream &f);

private:
	std::vector<CMemorizedPhrase*>	Phrases;

private:
	// prevent copy constructor
	CMemorizationSet(const CMemorizationSet &other) {}
	// prevent copy operator
	CMemorizationSet &operator =(const CMemorizationSet&other) { return *this;}
};


/**
 * struct for all memorized phrase for a player
 */
struct CPlayerPhraseMemory
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	CPlayerPhraseMemory();

	~CPlayerPhraseMemory();

	void clear();

	void memorize(uint8 memorizationSet, uint8 index, const std::vector<NLMISC::CSheetId> &bricks, uint16 id, const TDataSetRow &entityRowId);

	/// memorize starter phrases in easy order for newcomers
	bool memorizeStarterPhrase(const std::vector<NLMISC::CSheetId> &bricks, uint16 id);
	
	/// memorize in first found empty slot, returns true if successful, make no check on phrase validity
	bool memorizeInFirstEmptySlot(const std::vector<NLMISC::CSheetId> &bricks, uint16 id);

	// memorize in given set without any check on phrase validity
	void memorizeWithoutCheck(uint8 memorizationSet, uint8 i, const std::vector<NLMISC::CSheetId> &bricks, uint16 id );

	/// memorize in given set a phrase the character may not execute for now, without any check
	void memorizeRefused(uint8 memorizationSet, uint8 i, const std::vector<NLMISC::CSheetId> &bricks, uint16 id);
		
	void executePhrase(uint8 memorizationSet, uint8 i, CCharacter *actor, const TDataSetRow &target, bool cyclic, bool enchant);

	void forget(uint8 memorizationSet, uint8 i);

	/// forget all phrases
	void forgetAll();

	/// Re-memorise phrases to reconstruct them in case bricks inside had changed.
	void fixPhrases(const std::vector<CKnownPhrase> &knownPhrases, const std::set<NLMISC::CSheetId> &knownBricks, const TDataSetRow &rowId);

	/// have every phrase checked again before its next execution (the character lost a brick)
	void checkAgainBeforeExecution();

	const std::vector<CMemorizationSet*>	&getMemorizationSets() const { return _MemSets; }

	void serial(NLMISC::IStream &f);

private:
	CMemorizationSet* getMemSet(uint32 idx);
	std::vector<CMemorizationSet*>	_MemSets;


private:
	// prevent copy constructor
	CPlayerPhraseMemory(const CMemorizationSet &other) {}
	// prevent copy operator
	CPlayerPhraseMemory &operator =(const CMemorizationSet&other) { return *this;}
};




#endif //RY_KNOWN_MEMORIZED_PHRASES_H


/* End of known_and_memorized_phrases.h */
