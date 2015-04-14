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



#include "stdpch.h"
#include "nel/misc/path.h"
#include "fame.h"
#include "nel/misc/diff_tool.h"
#include "ryzom_entity_id.h"
//#include "../entities_game_service/egs_variables.h"
//#include "pvp_clan.h"

using namespace std;
using namespace NLMISC;
using namespace STRING_MANAGER;

std::string RowIdToStringCb(void *value);

std::string FameToStringCb(void *value)
{
	sint32 &fame = *((sint32*)value);

	if (fame == NO_FAME)
		return "NO_FAME";
	else
		return toString("%.3f", fame/1000.0);
}


CStaticFames	*CStaticFames::_Instance = NULL;
CFameInterface	*CFameInterface::_Instance = NULL;

sint16	CFameInterface::TFameOwner::CivilisationPropIndex = 0;
sint16	CFameInterface::TFameOwner::GuildPropIndex = 0;
sint16	CFameInterface::TFameOwner::FameMemoryPropIndex = 0;
sint16	CFameInterface::TFameOwner::FirstFamePropIndex = 0;

CVariable<bool> UseAsymmetricStaticFames("variables", "UseAsymmetricStaticFames", "if 1, fame between faction A and B may be different that between B and A", false, 0, true);

//----------------------------------------------------------------------------
NLMISC_COMMAND(displayFactions, "Display a list of fame faction", "")
{
	CStaticFames &sf = CStaticFames::getInstance();

	const std::vector<NLMISC::TStringId> &names = sf.getFactionNames();

	log.displayNL("Listing %u factions:", names.size());

	for (uint i=0; i<names.size(); ++i)
	{
		log.displayNL("  %u : '%s'", i, CStringMapper::unmap(names[i]).c_str());
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(displayStaticFames, "Display the fame for each factions", "[<faction>]")
{
	if (args.size() > 1)
		return false;

	CStaticFames &sf = CStaticFames::getInstance();

	const std::vector<NLMISC::TStringId> &names = sf.getFactionNames();

	uint start =0;
	uint end = (uint)names.size();


	if (args.size() == 1)
	{
		uint32 factionIndex = sf.getFactionIndex(args[0]);
		if (factionIndex != CStaticFames::INVALID_FACTION_INDEX)
		{
			start = factionIndex;
			end = start+1;
		}
		else
		{
			log.displayNL("invalid faction name '%s'", args[0].c_str());
			return true;
		}
	}

	for (uint i=start; i<end; ++i)
	{
		for (uint j=0; j<names.size(); ++j)
		{
			sint32 fame = sf.getStaticFameIndexed(i, j);
			if (fame != 0)
				log.displayNL("Fame between %s and %s : %7.3f",
					sf.getFactionName(i).c_str(),
					sf.getFactionName(j).c_str(),
					fame/1000.0f);
		}
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(displayFame, "List the fame value between an entity and one or all faction.", "<entityId> [<detail>] [<factionIndex|factionName>]* ")
{
	if (args.size() < 1)
		return false;

	CFameInterface &fi = CFameInterface::getInstance();

	CEntityId	eid(args[0]);
	if (eid == CEntityId::Unknown)
	{
		log.display("Invalid entity id '%s'", args[0].c_str());
		return false;
	}
	TDataSetRow dsr = fi.getFameDataSet()->getDataSetRow(eid);
	if (!fi.getFameDataSet()->isAccessible(dsr))
	{
		log.display("Invalid entity id '%s', can't map in fame dataset", args[0].c_str());
		return false;
	}

	const vector<TStringId>	factionNames = CStaticFames::getInstance().getFactionNames();
//	uint32		staticIndex = CStaticFames::INVALID_FACTION_INDEX;

	bool detail = false;
	if (args.size() >= 2)
	{
		if (args[1] == "detail")
		{
			detail = true;
		}
	}

	// build the filter list
	vector<uint32>	factionFilter;
	for (uint i=(detail ? 2 : 1); i<args.size(); ++i)
	{
		uint32 factionIndex = CStaticFames::INVALID_FACTION_INDEX;
		if (isdigit(args[i][0]))
		{
			fromString(args[i], factionIndex);
		}
		else
		{
			factionIndex = CStaticFames::getInstance().getFactionIndex(args[i]);
		}
		if (factionIndex == CStaticFames::INVALID_FACTION_INDEX)
		{
			log.display("Invalid faction '%s'.\nTry 'displayFactions' to list the available factions.", args[i].c_str());
			return false;
		}
		factionFilter.push_back(factionIndex);
	}


	for (uint i=0; i<factionNames.size(); /*MAX_FAME_OWNER;*/ ++i)
	{

		if (factionFilter.empty() || find(factionFilter.begin(), factionFilter.end(), i) != factionFilter.end())
		{
			const string &factionName = CStringMapper::unmap(factionNames[i]);

			if (!detail)
			{
				sint32 fame = fi.getFameIndexed(eid, i);

				if (!factionNames.empty())
				{
					if (fame == NO_FAME)
						log.displayNL("NO Fame between \t%s and \t%s", eid.toString().c_str(), factionName.c_str());
					else
						log.displayNL("Fame between %s and %35s = %7.3f", eid.toString().c_str(), factionName.c_str(), fame/1000.0f);
				}
			}
			else
			{
				TDataSetRow gdsr = dsr;
				TDataSetRow cdsr = dsr;
				TDataSetRow mdsr = dsr;
/*				CEntityId geid = eid;
				CEntityId ceid = eid;
				CEntityId meid = eid;
*/				string fp, fg, fc, fm;
				fp = fg = fc = fm = "NA";


				switch (eid.getType())
				{
				case RYZOMID::player:
					{
						sint32 fame = fi.getFameIndexed(eid, i, false, true);

						if (fame != NO_FAME)
							fp = toString("%7.3f", fame/1000.0f);
						else
							fp = "no fame";

						gdsr = fi.getGuildIndex(eid);
						cdsr = fi.getCivilisationIndex(eid);
						mdsr = fi.getFameMemoryIndex(eid);

						fame = fi.getFameIndexed(fi.getFameDataSet()->getEntityId(mdsr), i, false, true);

						if (fame != NO_FAME)
							fm = toString("%7.3f", fame/1000.0f);
						else
							fm = "no fame";

					}
				case RYZOMID::guild:
					if (fi.getFameDataSet()->isAccessible(gdsr))
					{
						sint32 fame = fi.getFameIndexed(fi.getFameDataSet()->getEntityId(gdsr), i, false, true);

						if (fame != NO_FAME)
							fg = toString("%7.3f", fame/1000.0f);
						else
							fg = "no fame";

						if (eid.getType() == RYZOMID::guild)
							cdsr = fi.getCivilisationIndex(eid);
					}
				case RYZOMID::civilisation:
					if (fi.getFameDataSet()->isAccessible(cdsr))
					{
						sint32 fame = fi.getFameIndexed(fi.getFameDataSet()->getEntityId(cdsr), i, false, true);

						if (fame != NO_FAME)
							fc = toString("%7.3f", fame/1000.0f);
						else
							fc = "no fame";
					}
				}

				// display the result
				log.displayNL("Fame between \t%s and %35s = P: %7s, G: %7s, M: %7s, C: %7s",
					eid.toString().c_str(),
					factionName.c_str(),
					fp.c_str(),
					fg.c_str(),
					fm.c_str(),
					fc.c_str());
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(addFame, "Add some fame value between an entity and a faction.", "<entityId> <factionIndex|factionName> <fameDelta>")
{
	if (args.size() != 3)
		return false;

	CEntityId	eid(args[0]);
	if (eid == CEntityId::Unknown)
	{
		log.display("Invalid entity id '%s'", args[0].c_str());
		return false;
	}
	TDataSetRow dsr = CFameInterface::getInstance().getFameDataSet()->getDataSetRow(eid);
	if (!CFameInterface::getInstance().getFameDataSet()->isAccessible(dsr))
	{
		log.display("Invalid entity id '%s', can't map in fame dataset", args[0].c_str());
		return false;
	}

	uint32		factionIndex;

	if (isdigit(args[1][0]))
	{
		fromString(args[1], factionIndex);
	}
	else
	{
		factionIndex = CStaticFames::getInstance().getFactionIndex(args[1]);
	}
	if (factionIndex == CStaticFames::INVALID_FACTION_INDEX)
	{
		log.display("Invalid faction '%s'.\nTry displayFactions to list the available factions.", args[1].c_str());
		return false;
	}

	sint32 delta;
	fromString(args[2], delta);
//	clamp(delta, FameAbsoluteMin, FameAbsoluteMax);

	// finaly, retreive the

	CFameInterface::getInstance().addFameIndexed(eid, factionIndex, delta, true);

	return true;
}

//----------------------------------------------------------------------------
CStaticFames::CStaticFames()
{
	_FameTable = NULL;
	_PropagationFactorTable = NULL;
	loadStaticFame("static_fame.txt");
	loadTribeThreshold("fame_tribes_threshold.txt");
}

//----------------------------------------------------------------------------
CStaticFames::~CStaticFames()
{
	delete[] _FameTable;
	_FameTable = NULL;
	delete[] _PropagationFactorTable;
	_PropagationFactorTable = NULL;
}

//----------------------------------------------------------------------------
void CStaticFames::releaseInstance()
{
	if( _Instance )
	{
		delete _Instance;
		_Instance = NULL;
	}
}

//----------------------------------------------------------------------------
void CStaticFames::loadStaticFame( const string& filename )
{
	_FameTableSize = 0;
	_FirstTribeFameIndex = 0;
	_FameTable = NULL;

	string path = CPath::lookup(filename, false);
	if (path.empty())
	{
		nlwarning("FAME: Error can't find static fame file '%s' ! No static fame data available.",
			filename.c_str());
		return;
	}

	STRING_MANAGER::TWorksheet	ws;
	// load the static fame file
	if (loadExcelSheet(path, ws, false))
	{
		// ok, we can parse the worksheet
		// 1st, build the index table
		for (uint i=1; i<ws.size(); ++i)
		{
			string name = ws.getData(i, 0).toString();
			name = strlwr(name);
			if (name.empty())
				break;
			_FactionNameIndex.insert(make_pair(CStringMapper::map(name), i-1));
			_FactionNames.push_back(CStringMapper::map(name));

			CSheetId sheet( name + ".faction" );
			_FactionSheets.push_back(sheet);
			_FactionSheetIndex.insert(make_pair(sheet, i-1));

			//nldebug("FAME: Added faction %u as '%s'", i-1, name.c_str());
		}

		// 2nd check the table structure
		for (uint i=2; i<ws.ColCount; ++i)
		{
			string name = ws.getData(0, i).toString();
			name = strlwr(name);

			if (name.empty())
			{
				if (i-2 != _FactionNameIndex.size())
				{
					nlwarning("FAME: Error while reading fame file '%s' : missing column identifier, found only %u column, need %u !",
						path.c_str(),
						i,
						_FactionNameIndex.size());
					nlwarning("FAME: The faction data will not be available !");

					_FactionNameIndex.clear();
					_FactionNames.clear();
					_FactionSheets.clear();
					_FactionSheetIndex.clear();
					return;
				}

				break;
			}

			TFactionNameIndexList::iterator it(_FactionNameIndex.find(CStringMapper::map(name)));
			if (it == _FactionNameIndex.end())
			{
				nlwarning("FAME: Error while reading fame file '%s' : column identifier '%s' not found in row identifiers !",
					path.c_str(),
					name.c_str());
				nlwarning("FAME: The faction data will not be available !");

				_FactionNameIndex.clear();
				_FactionNames.clear();
				_FactionSheets.clear();
				_FactionSheetIndex.clear();
				return;
			}
			if (it->second != i-2)
			{
				nlwarning("FAME: Error while reading fame file '%s' : identifier '%s' is in row %u but in column %u, column and row index must match !",
					path.c_str(),
					name.c_str(),
					it->second,
					i-2
					);
				nlwarning("FAME: The faction data will not be available !");

				_FactionNameIndex.clear();
				_FactionNames.clear();
				_FactionSheets.clear();
				_FactionSheetIndex.clear();
				return;
			}
		}
		// 3rd, allocate fame table and load the data
		_FameTableSize = (uint)_FactionNameIndex.size();

		_FameTable = new sint32[_FameTableSize*_FameTableSize];
		_PropagationFactorTable = new float[_FameTableSize*_FameTableSize];
		_FameIndexToDatabaseIndex.resize( _FameTableSize );

		// Parse database indices
		for (uint i=1; i<_FameTableSize+1; ++i)
		{
			int iFaction = i-1;
			string s = ws.getData(i, 1).toString();

			// get rid of " characters
			string::size_type k;
			while ( (k = s.find_first_of('"')) != string::npos)
				s.erase(k,1);

			uint databaseIndex;
			fromString(s, databaseIndex);
			if( databaseIndex >= 1000 )
			{
				databaseIndex -= 1000;
				_FirstTribeFameIndex = iFaction;
			}
			_FameIndexToDatabaseIndex[iFaction] = databaseIndex;
		}
		// Parse static fames and propagation factors
		for (uint i=1; i<_FameTableSize+1; ++i)
		{
			int iFaction = i-1;
			for (uint j=2; j<_FameTableSize+2; ++j)
			{
				int jFaction = j-2;

				string s = ws.getData(i, j).toString();

				// get rid of " characters
				string::size_type k;
				while ( (k = s.find_first_of('"')) != string::npos)
					s.erase(k,1);
				// Extract "fame;factor"
				float factor = 0.0f;
				string::size_type sep = s.find_first_of(';');
				if (sep == string::npos)
					sep = s.size();
				else
					NLMISC::fromString(s.substr(sep+1, s.size()-sep-1), factor);
				// Fames in file are in [-600;600] so don't forget 1000 factor
				float fameFloat;
				NLMISC::fromString(s.substr(0, sep), fameFloat);
				sint32 fame = (sint32)(fameFloat * 1000.f);

				_FameTable[iFaction*_FameTableSize + jFaction] = fame;
				_PropagationFactorTable[iFaction*_FameTableSize + jFaction] = factor;

				if (fame != 0)
				{
/*					nldebug ("FAME: Setting fame between '%s' and '%s' to value %f, propagation factor %f",
						CStringMapper::unmap(_FactionNames[iFaction]).c_str(),
						CStringMapper::unmap(_FactionNames[jFaction]).c_str(),
						fame/1000.0f,
						factor
						);*/
				}
			}
		}
	}
	else
	{
		nlwarning("FAME: Error while reading the static fame file '%s' ! No static fame data available.",
			path.c_str());
		return;
	}

	nlassert(_FactionNames.size() == _FactionSheets.size() );
	nlinfo("FAME: Loaded fame for %u factions, total of %u fame values",
		_FameTableSize,
		_FameTableSize*_FameTableSize);
}

//----------------------------------------------------------------------------
void CStaticFames::loadTribeThreshold( const string& filename )
{
	string path = CPath::lookup(filename, false);
	if (path.empty())
	{
		nlwarning("FAME: Error can't find tribe threshold fame file '%s' ! No tribe threshold available.", filename.c_str());
		return;
	}

	STRING_MANAGER::TWorksheet	ws;
	// load the tribe threshold fame file
	if (loadExcelSheet(path, ws, false))
	{
		// ok, we can parse the worksheet
		// check table structure
		uint nbTribe = ws.size()-2;
		nlassert(nbTribe<=_FameTableSize);
		nlassert(ws.ColCount == 16); // 5 ( 4 people + neutral ) * 3 cult + 1 for tribe name

		_TribeCultThresholdPerCiv.resize(nbTribe);

		for (uint i=2; i<ws.size(); ++i)
		{
			string name = ws.getData(i, 0).toString();
			nlinfo("TRIBE THRESHOLD TABLE: %s", name.c_str() );

			uint index = getFactionIndex( ws.getData(i, 0).toString() );
			nlassert(index < _FameTableSize && index != INVALID_FACTION_INDEX);

			_TribeCultThresholdPerCiv[i-2].FameIndex = index;

			for( uint c=1; c<ws.ColCount; c+=3)
			{
				sint32 thresholdKami, thresholdKaravan, thresholdNeutral;
				fromString(ws.getData(i, c).toString(), thresholdKami);
				fromString(ws.getData(i, c+1).toString(), thresholdKaravan);
				fromString(ws.getData(i, c+2).toString(), thresholdNeutral);

				CTribeCultThreshold * tc;

				if( c < 4 )
				{
					tc = &_TribeCultThresholdPerCiv[i-2].Matis;
				}
				else if( c < 7 )
				{
					tc = &_TribeCultThresholdPerCiv[i-2].Fyros;
				}
				else if( c < 10 )
				{
					tc = &_TribeCultThresholdPerCiv[i-2].Tryker;
				}
				else if( c < 13 )
				{
					tc = &_TribeCultThresholdPerCiv[i-2].Zorai;
				}
				else
				{
					tc = &_TribeCultThresholdPerCiv[i-2].Neutral;
				}

				tc->setKami(thresholdKami*6000);
				tc->setKaravan(thresholdKaravan*6000);
				tc->setNeutral(thresholdNeutral*6000);

				// This message removed by Sadge because there is no context displayed, meaning that message must be useless
				// nldebug(" %s", ws.getData(i, c).toString().c_str() );
			}
		}
	}
}

//----------------------------------------------------------------------------
const std::string &CStaticFames::getFactionName(uint32 factionIndex)
{
	const static std::string emptyString;
	if (factionIndex < _FactionNames.size())
		return NLMISC::CStringMapper::unmap(_FactionNames[factionIndex]);
	else
		return emptyString;
}

//----------------------------------------------------------------------------
uint	CStaticFames::getFactionIndex(NLMISC::TStringId factionName)
{
	TFactionNameIndexList::iterator it(_FactionNameIndex.find(factionName));
	if (it != _FactionNameIndex.end())
	{
		return it->second;
	}
	return INVALID_FACTION_INDEX;
}

//----------------------------------------------------------------------------
uint	CStaticFames::getFactionIndex(const std::string &factionName)
{
	string n = strlwr(factionName);
	return getFactionIndex(CStringMapper::map(n));
}

//----------------------------------------------------------------------------
sint32	CStaticFames::getStaticFameIndexed(uint factionIndex1, uint factionIndex2)
{
	if ((factionIndex1 < factionIndex2) && !UseAsymmetricStaticFames)
		swap(factionIndex1, factionIndex2);

	if (factionIndex1 >= _FameTableSize || factionIndex2 >= _FameTableSize)
	{
		nlwarning("FAME: CStaticFames::getStaticFame invalid faction, return 0");
		return 0;
	}

#ifdef NL_DEBUG
	nlassert(factionIndex1 < _FameTableSize);
	nlassert(factionIndex2 < _FameTableSize);
#endif

	return _FameTable[factionIndex1*_FameTableSize + factionIndex2];
}

//----------------------------------------------------------------------------
float CStaticFames::getPropagationFactorIndexed(uint factionIndex1, uint factionIndex2) const
{
	if ((factionIndex1 < factionIndex2) && !UseAsymmetricStaticFames)
		swap(factionIndex1, factionIndex2);

	if (factionIndex1 >= _FameTableSize || factionIndex2 >= _FameTableSize)
	{
		nlwarning("FAME: CStaticFames::getPropagationFactorIndexed invalid faction, return 0");
		return 0.0f;
	}

#ifdef NL_DEBUG
	nlassert(factionIndex1 < _FameTableSize);
	nlassert(factionIndex2 < _FameTableSize);
#endif

	return _PropagationFactorTable[factionIndex1*_FameTableSize + factionIndex2];
}

//----------------------------------------------------------------------------
sint32 CStaticFames::getStaticFame(NLMISC::TStringId faction1, NLMISC::TStringId faction2)
{
	uint index1, index2;

	// retreive fame index
	index1 = getFactionIndex(faction1);
	index2 = getFactionIndex(faction2);

	return getStaticFameIndexed(index1, index2);
}

//----------------------------------------------------------------------------
sint32	CStaticFames::getStaticFame(const std::string &faction1, const std::string &faction2)
{
	string n1, n2;
	n1 = strlwr(faction1);
	n2 = strlwr(faction2);

	return getStaticFame(CStringMapper::map(n1), CStringMapper::map(n2));
}

//----------------------------------------------------------------------------
sint32	CFameInterface::getFameIndexed(const CEntityId &entityId, uint32 factionIndex, bool modulated, bool returnUnknownValue)
{
	if (_FameOverload)
	{
		return _FameOverload->getFameIndexed(entityId, factionIndex, modulated, returnUnknownValue);
	}
	else
	{
		// if no faction, just return 0
		if (factionIndex == CStaticFames::INVALID_FACTION_INDEX)
			return 0;

		// retrieve the faction
		TDataSetRow entityRow = _FameDataSet->getDataSetRow(entityId);
		TFameContainer::iterator it(_FamesOwners.find(entityRow));
		if (it == _FamesOwners.end())
		{
//			nlwarning("FAME: entity %x doesn't exist in fame owners containers!", entityIndex.getIndex());
			return 0;
		}


		if (factionIndex >=  MAX_FACTION)
		{
			nlwarning("FAME: entity %s : faction %u is out of limit (%u)", entityId.toString().c_str(), factionIndex, MAX_FACTION);
			return 0;
		}

		TFameOwner	*fo = it->second;
		sint32 fame = fo->Fames[factionIndex];

		// The return values *should* be clamped here, but the values are stored in the
		//  EGS.  Just return the raw value if we don't have the _FameOverload.

		if (!returnUnknownValue && fame == NO_FAME)
			fame = 0;

		return sint32(fame);
	}
}

//----------------------------------------------------------------------------
sint32 CFameInterface::getFactionIndex(TStringId faction)
{
	CStaticFames	&staticFame = CStaticFames::getInstance();
	return staticFame.getFactionIndex(faction);
}

//----------------------------------------------------------------------------
sint32 CFameInterface::getFame(const CEntityId &entityId, TStringId faction, bool modulated, bool returnUnknowValue)
{
	CStaticFames	&staticFame = CStaticFames::getInstance();
	// get the fame faction index
	uint	oi = staticFame.getFactionIndex(faction);
	if(oi == CStaticFames::INVALID_FACTION_INDEX)
		return NO_FAME;

	return getFameIndexed(entityId, oi, modulated, returnUnknowValue);

}

//----------------------------------------------------------------------------
void CFameInterface::addFame(const CEntityId &entityId, NLMISC::TStringId faction, sint32 deltaFame, bool propagate)
{
	// get the fame faction index
	CStaticFames &staticFame = CStaticFames::getInstance();
	uint32	oi = staticFame.getFactionIndex(faction);
	if (oi == CStaticFames::INVALID_FACTION_INDEX)
	{
		nlwarning("FAME:addFame(): trying to add fame to faction '%s'(stringId=%u) which is not a valid faction name !",
			CStringMapper::unmap(faction).c_str(),
			faction);

		return;
	}

	addFameIndexed(entityId, oi, deltaFame, propagate);
}

//----------------------------------------------------------------------------
void CFameInterface::addFameIndexed(const CEntityId &entityId, uint32 factionIndex, sint32 deltaFame, bool propagate)
{
	if (_FameOverload)
	{
		_FameOverload->addFameIndexed(entityId, factionIndex, deltaFame, propagate);
	}
	else
	{
		// retrieve the fame owner.
		TDataSetRow dsr = _FameDataSet->getDataSetRow(entityId);
		TFameContainer::iterator it(_FamesOwners.find(dsr));
		if (it == _FamesOwners.end())
		{
			nlwarning("FAME:addFameIndexed(): can't find info for entity %s(index=%u) in _FamesOwners, can't add fame",
				entityId.toString().c_str(),
				dsr.toString().c_str());
			return;
		}

		if (factionIndex >= MAX_FACTION)
		{
			nlwarning("FAME:addFameIndexed(): trying to add fame for entity %s to an invalid faction %u, can't add fame",
				entityId.toString().c_str(),
				factionIndex);
			return;
		}

		NLNET::CMessage msg("FAME_DELTA");
		// the entity index
		msg.serial(const_cast<CEntityId&>(entityId));
		// the other faction index
		msg.serial(factionIndex);
		// the delta value
		msg.serial(deltaFame);
		msg.serial(propagate);
		// send the message
		NLNET::CUnifiedNetwork::getInstance()->send("EGS", msg);
	}
}

//----------------------------------------------------------------------------
const TDataSetRow	&CFameInterface::getCivilisationIndex(const CEntityId &entityId)
{
	static const TDataSetRow invalidIndex;

	if (_FameOverload)
	{
		return _FameOverload->getCivilisationIndex(entityId);
	}
	else
	{
		TDataSetRow dsr = _FameDataSet->getDataSetRow(entityId);
		TFameContainer::iterator it(_FamesOwners.find(dsr));
		if (it == _FamesOwners.end())
			return	invalidIndex;

		return it->second->Civilisation;
	}
}

//----------------------------------------------------------------------------
const TDataSetRow	&CFameInterface::getGuildIndex(const CEntityId &entityId)
{
	static const TDataSetRow invalidIndex;

	if (_FameOverload)
	{
		return _FameOverload->getGuildIndex(entityId);
	}
	else
	{
		TDataSetRow dsr = _FameDataSet->getDataSetRow(entityId);
		TFameContainer::iterator it(_FamesOwners.find(dsr));
		if (it == _FamesOwners.end())
			return invalidIndex;

		return it->second->Guild;
	}
}

//----------------------------------------------------------------------------
const TDataSetRow	&CFameInterface::getFameMemoryIndex(const CEntityId &entityId)
{
	static const TDataSetRow invalidIndex;

	if (_FameOverload)
	{
		return _FameOverload->getFameMemoryIndex(entityId);
	}
	else
	{
		TDataSetRow dsr = _FameDataSet->getDataSetRow(entityId);
		TFameContainer::iterator it(_FamesOwners.find(dsr));
		if (it == _FamesOwners.end())
			return invalidIndex;

		return it->second->FameMemory;
	}
}

//----------------------------------------------------------------------------
void CFameInterface::setFameDataSet(CMirroredDataSet *fameDataSet, bool declareProperty)
{
	const static std::string civilisation("Civilisation");
	const static std::string guild("Guild");
	const static std::string fameMemory("FameMemory");
	const static std::string firstFame("Fame_0");

	_FameDataSet = fameDataSet;

	if (!_FameDataSet)
		return;

	if (declareProperty)
	{
		// declare the fame properties
		fameDataSet->declareProperty( civilisation, PSOReadOnly );
		fameDataSet->declareProperty( fameMemory,	PSOReadOnly);
		fameDataSet->declareProperty( guild,		PSOReadOnly);

		for (uint i=0; i<MAX_FACTION; ++i)
		{
			string propName = toString("Fame_%u", i);
			fameDataSet->declareProperty( propName,		PSOReadOnly );
		}
	}

	// initialise property index
	TFameOwner::CivilisationPropIndex = _FameDataSet->getPropertyIndex("Civilisation");
	TFameOwner::GuildPropIndex = _FameDataSet->getPropertyIndex("Guild");
	TFameOwner::FameMemoryPropIndex = _FameDataSet->getPropertyIndex("FameMemory");
	TFameOwner::FirstFamePropIndex = _FameDataSet->getPropertyIndex("Fame_0");

	// set display call backs
	_FameDataSet->setDisplayCallback( TFameOwner::CivilisationPropIndex, RowIdToStringCb );
	_FameDataSet->setDisplayCallback( TFameOwner::GuildPropIndex, RowIdToStringCb );
	_FameDataSet->setDisplayCallback( TFameOwner::FameMemoryPropIndex, RowIdToStringCb );

	for (uint i=0; i<MAX_FACTION; ++i)
		_FameDataSet->setDisplayCallback( TFameOwner::FirstFamePropIndex+i, FameToStringCb );
}
