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



/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
#include "player_manager/known_and_memorized_phrases.h"
#include "player_manager/character.h"
#include "phrase_manager/s_phrase.h"
#include "phrase_manager/phrase_manager.h"
#include "egs_sheets/egs_sheets.h"

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CMemorizedPhrase);
NL_INSTANCE_COUNTER_IMPL(CMemorizationSet);

//-----------------------------------------------
// CKnownPhrase::empty
//-----------------------------------------------
bool CKnownPhrase::empty() const
{
	return (PhraseDesc.empty() && PhraseSheetId == NLMISC::CSheetId::Unknown);
}

//-----------------------------------------------
// CKnownPhrase::clear
//-----------------------------------------------
void CKnownPhrase::clear()
{
	PhraseDesc.clear();
	PhraseSheetId = NLMISC::CSheetId::Unknown;
}

//-----------------------------------------------
// CKnownPhrase::serial
//-----------------------------------------------
void CKnownPhrase::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(PhraseDesc);
	f.serial(PhraseSheetId);
}


//-----------------------------------------------
// CMemorizedPhrase::CMemorizedPhrase
//-----------------------------------------------
CMemorizedPhrase::CMemorizedPhrase()
{
	clear();
}

//-----------------------------------------------
// CMemorizedPhrase::CMemorizedPhrase
//-----------------------------------------------
CMemorizedPhrase::CMemorizedPhrase(const std::vector<NLMISC::CSheetId> &bricks, uint16 id) : Bricks(bricks), PhraseId(id)
{
}

//-----------------------------------------------
// CMemorizedPhrase::clear
//-----------------------------------------------
void CMemorizedPhrase::clear()
{
	Bricks.clear();
	PhraseId=0;
}

//-----------------------------------------------
// CMemorizedPhrase::serial
//-----------------------------------------------
void CMemorizedPhrase::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(Bricks);
	f.serial(PhraseId);
}


//-----------------------------------------------
// CMemorizationSet::CMemorizationSet
//-----------------------------------------------
CMemorizationSet::CMemorizationSet()
{
	Phrases.resize(MaxNbSentencesPerSet, NULL);
}

//-----------------------------------------------
// CMemorizationSet::~CMemorizationSet
//-----------------------------------------------
CMemorizationSet::~CMemorizationSet()
{
	for (uint i = 0 ; i < Phrases.size() ; ++i)
	{
		if (Phrases[i] != NULL)
			delete Phrases[i];
	}
}

//-----------------------------------------------
// CMemorizationSet::clear
//-----------------------------------------------
void CMemorizationSet::clear()
{
	for (uint i = 0 ; i < Phrases.size() ; ++i)
	{
		if (Phrases[i] != NULL)
			Phrases[i]->clear();
	}
}

//-----------------------------------------------
// CMemorizationSet::memorize
//-----------------------------------------------
void CMemorizationSet::memorize(uint8 i, const vector<CSheetId> &bricks, uint16 id, const TDataSetRow &rowId)
{
	if (i >= Phrases.size() )
	{
		nlwarning("<CMemorizationSet::setPhrase> Error, tried to memorized in slot %u while there is %u slots", i, Phrases.size());
		return;
	}

	CSPhrasePtr phrase = CPhraseManager::getInstance().buildSabrinaPhrase(rowId, TDataSetRow(), bricks, 0 , 0, false);
	if (!phrase)
	{
		return;
	}
	if ( !phrase->evaluate() ) //|| !phrase->validate() )
	{
		nlwarning("<CMemorizationSet::setPhrase> phrase is invalid");
		return;
	}

	if (Phrases[i] != NULL)
		delete Phrases[i];

	Phrases[i] = new CMemorizedPhrase(bricks, id);
}

//-----------------------------------------------
// CMemorizationSet::memorizeStarterPhrase
//-----------------------------------------------
bool CMemorizationSet::memorizeStarterPhrase(const std::vector<NLMISC::CSheetId> &bricks, uint16 id)
{
	const CStaticBrick * brick = CSheets::getSBrickForm( bricks[0] );
	if( brick )
	{
		uint n = (uint)Phrases.size() / 2;
		BRICK_TYPE::EBrickType phraseType = BRICK_FAMILIES::brickType(brick->Family);
		if (phraseType == BRICK_TYPE::FORAGE_PROSPECTION || phraseType == BRICK_TYPE::FORAGE_EXTRACTION)
		{
			// Top-Right
			for (uint i = 0 ; i < n ; ++i)
			{
				if (Phrases[n-i-1] == NULL)
				{
					Phrases[n-i-1] = new CMemorizedPhrase(bricks, id);
					return true;
				}
			}
		}
		else if (phraseType == BRICK_TYPE::MAGIC)
		{
			// Bottom-Left
			for (uint i = 0 ; i < n ; ++i)
			{
				if (Phrases[n+i] == NULL)
				{
					Phrases[n+i] = new CMemorizedPhrase(bricks, id);
					return true;
				}
			}
		}
		else if (phraseType == BRICK_TYPE::COMBAT || phraseType == BRICK_TYPE::SPECIAL_POWER)
		{
			// Top-Left
			return memorizeInFirstEmptySlot(bricks, id);
		}
		else
		{
			// Bottom-Right
			for (uint i = 0 ; i < n ; ++i)
			{
				if (Phrases[2*n-i-1] == NULL)
				{
					Phrases[2*n-i-1] = new CMemorizedPhrase(bricks, id);
					return true;
				}
			}
		}
	}
	else
	{
		return memorizeInFirstEmptySlot(bricks, id);
	}

	return false;
}


//-----------------------------------------------
// CMemorizationSet::memorizeInFirstEmptySlot
//-----------------------------------------------
bool CMemorizationSet::memorizeInFirstEmptySlot(const vector<CSheetId> &bricks, uint16 id)
{
	for (uint i = 0 ; i < Phrases.size() ; ++i)
	{
		if (Phrases[i] == NULL)
		{
			Phrases[i] = new CMemorizedPhrase(bricks, id);
			return true;
		}
	}
	
	return false;
} // CMemorizationSet::memorizeWithoutCheck //


//-----------------------------------------------
// CMemorizationSet::memorizeWithoutCheck
//-----------------------------------------------
void CMemorizationSet::memorizeWithoutCheck(uint8 i, const vector<CSheetId> &bricks, uint16 id)
{
	if (i >= Phrases.size() )
	{
		nlwarning("<CMemorizationSet::memorizeWithoutCheck> Error, tried to memorized in slot %u while there is %u slots", i, Phrases.size());
		return;
	}
	
	Phrases[i] = new CMemorizedPhrase(bricks, id);
} // CMemorizationSet::memorizeWithoutCheck //


//-----------------------------------------------
// CMemorizationSet::executePhrase
//-----------------------------------------------
void CMemorizationSet::executePhrase(uint8 i, CCharacter *actor, const TDataSetRow &target, bool cyclic, bool enchant)
{
	if (!actor) return;

	if (i >= Phrases.size() ) return;

	if (Phrases[i] == NULL)
	{
		nlwarning("<CMemorizationSet::executePhrase> phrase %u is NULL", i);
		return;
	}
	if ( CPhraseManager::getInstance().executePhrase( actor->getEntityRowId(), target, Phrases[i]->Bricks, cyclic, Phrases[i]->PhraseId, actor->nextCounter(),enchant) == NULL)
	{
		// error
//		actor->_PropertyDatabase.setProp( "EXECUTE_PHRASE:NEXT_COUNTER", actor->nextCounter() );

		CPhraseManager::getInstance().updateNextCounterValue( actor->getEntityRowId(), actor->nextCounter());

		if (cyclic)
		{
//			actor->writeCycleCounterInDB();
			actor->sendPhraseExecAck( cyclic, actor->cycleCounter(), false);
		}
		else
			actor->sendPhraseExecAck( cyclic, actor->nextCounter(), false);
	}
	else
	{
		if (cyclic)
			actor->sendPhraseExecAck( cyclic, actor->cycleCounter(), true);
		else
			actor->sendPhraseExecAck( cyclic, actor->nextCounter(), true);
	}
} // CMemorizationSet::executePhrase //


//-----------------------------------------------
// fixPhrases :
// Re-memorise phrases to reconstruct them in case bricks inside had changed.
// \warning actor must not be NULL.
//-----------------------------------------------
void CMemorizationSet::fixPhrases(const std::vector<CKnownPhrase> &knownPhrases, const TDataSetRow &rowId)
{
	const uint nbPhrases = (uint)Phrases.size();
	for(uint phraseIndex=0; phraseIndex<nbPhrases; ++phraseIndex)
	{
		if(Phrases[phraseIndex]!=0)
		{
			uint16 phraseId = Phrases[phraseIndex]->PhraseId;
			forget(phraseIndex);
			if(phraseId < knownPhrases.size())
			{
				// check phrase validity and memorize it
				if ( CPhraseManager::getInstance().checkPhraseValidity(knownPhrases[phraseId].PhraseDesc.Bricks) )
					memorize(phraseIndex, knownPhrases[phraseId].PhraseDesc.Bricks, phraseId, rowId);
			}
		}
	}
}// fixPhrases //

//-----------------------------------------------
// CMemorizationSet::forget
//-----------------------------------------------
void CMemorizationSet::forget(uint8 i)
{
	BOMB_IF( (i >= Phrases.size()), "phrase index is out of range", return );

	if (Phrases[i] != NULL)
	{
		delete Phrases[i];
		Phrases[i] = NULL;
	}
}

//-----------------------------------------------
// CMemorizationSet::forgetAll :
//-----------------------------------------------
void CMemorizationSet::forgetAll()
{
	const uint nbPhrases = (uint)Phrases.size();
	for(uint phraseIndex=0; phraseIndex<nbPhrases; ++phraseIndex)
	{
		if(Phrases[phraseIndex]!=0)
		{
			delete Phrases[phraseIndex];
			Phrases[phraseIndex] = NULL;
		}
	}
} // forgetAll //


//-----------------------------------------------
// CMemorizationSet::serial
//-----------------------------------------------
void CMemorizationSet::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialContPtr(Phrases);
}


//-----------------------------------------------
// CPlayerPhraseMemory::CPlayerPhraseMemory
//-----------------------------------------------
CPlayerPhraseMemory::CPlayerPhraseMemory()
{
	_MemSets.resize(MEM_SET_TYPES::NumMemories, NULL);
}

//-----------------------------------------------
// CPlayerPhraseMemory::~CPlayerPhraseMemory
//-----------------------------------------------
CPlayerPhraseMemory::~CPlayerPhraseMemory()
{
	clear();
}

//-----------------------------------------------
// CPlayerPhraseMemory::clear
//-----------------------------------------------
void CPlayerPhraseMemory::clear()
{
	for (uint i = 0 ; i < _MemSets.size() ; ++i)
	{
		if (_MemSets[i] != NULL)
		{
			_MemSets[i]->clear();
			delete _MemSets[i];
		}
	}
}

//-----------------------------------------------
// CPlayerPhraseMemory::getMemSet
//-----------------------------------------------
CMemorizationSet* CPlayerPhraseMemory::getMemSet(uint32 idx)
{
	nlassert(idx<_MemSets.size());
	if (_MemSets[idx]==NULL)
		_MemSets[idx]= new CMemorizationSet;
	return _MemSets[idx];
}

//-----------------------------------------------
// CPlayerPhraseMemory::memorize
//-----------------------------------------------
void CPlayerPhraseMemory::memorize(uint8 memorizationSet, uint8 i, const vector<CSheetId> &bricks, uint16 id, const TDataSetRow &rowId )
{
	BOMB_IF (memorizationSet >= _MemSets.size(), "Invlaid memory set bank id", return);

	getMemSet(memorizationSet)->memorize(i, bricks, id, rowId);
}

//-----------------------------------------------
// CPlayerPhraseMemory::memorizeStarterPhrase
//-----------------------------------------------
bool CPlayerPhraseMemory::memorizeStarterPhrase(const vector<CSheetId> &bricks, uint16 id)
{
	for (uint i = 0 ; i < _MemSets.size() ; ++i)
	{
		if (getMemSet(i)->memorizeStarterPhrase(bricks,id))
			return true;
	}
	
	return false;
}

//-----------------------------------------------
// CPlayerPhraseMemory::memorizeInFirstEmptySlot
//-----------------------------------------------
bool CPlayerPhraseMemory::memorizeInFirstEmptySlot(const vector<CSheetId> &bricks, uint16 id)
{
	for (uint i = 0 ; i < _MemSets.size() ; ++i)
	{
		if (getMemSet(i)->memorizeInFirstEmptySlot(bricks,id))
			return true;
	}
	
	return false;
}

//-----------------------------------------------
// CPlayerPhraseMemory::memorizeWithoutCheck
//-----------------------------------------------
void CPlayerPhraseMemory::memorizeWithoutCheck(uint8 memorizationSet, uint8 i, const vector<CSheetId> &bricks, uint16 id )
{
	BOMB_IF (memorizationSet >= _MemSets.size(), "Invlaid memory set bank id", return);

	getMemSet(memorizationSet)->memorizeWithoutCheck(i, bricks, id);
}


//-----------------------------------------------
// CPlayerPhraseMemory::executePhrase
//-----------------------------------------------
void CPlayerPhraseMemory::executePhrase(uint8 memorizationSet, uint8 i, CCharacter *actor, const TDataSetRow &target, bool cyclic, bool enchant)
{
	if( actor )
		actor->checkCharacterStillValide("start CPlayerPhraseMemory::executePhrase");

	if (memorizationSet >= _MemSets.size() ) return;

	if ( !_MemSets[memorizationSet] )
	{
		return;
	}

	_MemSets[memorizationSet]->executePhrase(i, actor, target, cyclic, enchant );

	if( actor )
		actor->checkCharacterStillValide("end CPlayerPhraseMemory::executePhrase");
}


//-----------------------------------------------
// CPlayerPhraseMemory::fixPhrases :
// Re-memorise phrases to reconstruct them in case bricks inside had changed.
// \warning actor must not be NULL.
//-----------------------------------------------
void CPlayerPhraseMemory::fixPhrases(const std::vector<CKnownPhrase> &knownPhrases, const TDataSetRow &rowId)
{
	const uint nbSet = (uint)_MemSets.size();
	for(uint i=0; i<nbSet; ++i)
	{
		// Re-build phrases in cases bricks inside had changed.
		if(_MemSets[i] != 0)
			_MemSets[i]->fixPhrases(knownPhrases, rowId);
	}
}// fixPhrases //

//-----------------------------------------------
// CPlayerPhraseMemory::forget
//-----------------------------------------------
void CPlayerPhraseMemory::forget(uint8 memorizationSet, uint8 i)
{
	if (memorizationSet >= _MemSets.size()) return;

	if (_MemSets[memorizationSet] != NULL)
	{
		_MemSets[memorizationSet]->forget(i);
	}
}

//-----------------------------------------------
// CPlayerPhraseMemory::forgetAll :
//-----------------------------------------------
void CPlayerPhraseMemory::forgetAll()
{
	const uint nbSet = (uint)_MemSets.size();
	for(uint i=0; i<nbSet; ++i)
	{
		if(_MemSets[i] != 0)
			_MemSets[i]->forgetAll();
	}
} // forgetAll //

//-----------------------------------------------
// CPlayerPhraseMemory::serial
//-----------------------------------------------
void CPlayerPhraseMemory::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialContPtr(_MemSets);
}

