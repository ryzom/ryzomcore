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

#include "mission_compiler.h"
#include "step.h"

#include "nel/ligo/primitive_utils.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;


// ************
// IStepContent
// ************

IStepContent *IStepContent::createStepContent(CMissionData &md, NLLIGO::IPrimitive *prim)
{
	string className;
	prim->getPropertyByName("class", className);

	IStepContentFactory *factory = CFactoryIndirect<IStepContentFactory, string>::instance().getFactory(className);
	if (factory == NULL)
	{
		string err = toString("Can't find factory for class '%s'", className.c_str());
		throw EParseException(prim, err.c_str());
	}

	IStepContent *content = factory->createStepContent(md, prim);
	return content;
}

void IStepContent::init(CMissionData &md, NLLIGO::IPrimitive *prim)
{
	prim->getPropertyByName("name", _ContentName);

	// parse the post actions
	for (uint i=0; i<prim->getNumChildren(); ++i)
	{
		IPrimitive *child;
		prim->getChild(child, i);
		if (child)
		{
			// ok, we got one
			IStepContent *sc = IStepContent::createStepContent(md, child);
			if (sc)
				_PostActions.push_back(sc);
		}
	}
}

std::string IStepContent::genStepContentCode(CMissionData &md)
{
	string ret;
	// call the derived class code generation
	ret += genCode(md);

	// generate the code for the post action
	for (uint i=0; i<_PostActions.size(); ++i)
	{
		ret += _PostActions[i]->genCode(md);
	}

	return ret;
}

std::string IStepContent::genStepContentPhrase() 
{ 
	string ret;	

	// call ther derived class phrase generation
	ret += genPhrase();

	// generate the phrase for the post action
	for (uint i=0; i<_PostActions.size(); ++i)
	{
		ret += _PostActions[i]->genPhrase();
	}

	return ret;
}

void IStepContent::fillJump(CMissionData &md, std::set<TJumpInfo> &jumpPoints)
{
	for (uint i=0; i<_PostActions.size(); ++i)
	{
		_PostActions[i]->fillJump(md, jumpPoints);
	}
}


////////////////////// Actions and objectives /////////////////////

class CActionJumpTo : public IStepContent
{
	string	_JumpTo;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_JumpTo = md.getProperty(prim, "target", true, false);
	}

	string genCode(CMissionData &md)
	{
		if (!_JumpTo.empty())
			return "jump : "+_JumpTo+NL;
		else
			return string();
	}

	void fillJump(CMissionData &md, std::set<TJumpInfo> &jumpPoints)
	{
		IStepContent::fillJump(md, jumpPoints);
		jumpPoints.insert(TJumpInfo(_JumpTo, "", false));
	}

};
REGISTER_STEP_CONTENT(CActionJumpTo, "jump_to");


// ---------------------------------------------------------------------------
class CActionRecvMoney : public IStepContent
{
	string	_Amount;
	bool	_Guild;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_Amount = md.getProperty(prim, "amount", true, false);

		_Guild = md.getProperty(prim, "guild", false, true) == "true";
		// Check: if _Guild is true then check if we are in a guild mission
		if (_Guild && !md.isGuildMission())
		{
			string err = toString("primitive(%s): 'guild' option true 1 for non guild mission.", prim->getName().c_str());
			throw EParseException(prim, err.c_str());
		}
	}
	
	string genCode(CMissionData &md)
	{
		if (!_Amount.empty())
		{
			string ret;
			ret = "recv_money : "+_Amount;
			if (_Guild)
				ret += ": guild";
			ret += NL;
			return ret;
		}
		else
			return string();
	}
	
};
REGISTER_STEP_CONTENT(CActionRecvMoney, "recv_money");


// ---------------------------------------------------------------------------
class CActionRecvChargePoint : public IStepContent
{
	string	_Amount;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_Amount = md.getProperty(prim, "charge_points", true, false);
	}

	string genCode(CMissionData &md)
	{
		if (!_Amount.empty())
		{
			string ret;
			ret = "recv_charge_point : "+_Amount;
			ret += NL;
			return ret;
		}
		else
			return string();
	}

};
REGISTER_STEP_CONTENT(CActionRecvChargePoint, "recv_charge_point");


// ---------------------------------------------------------------------------
class CActionGiveOutpostControl : public IStepContent
{
	string	_OutpostName;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_OutpostName = md.getProperty(prim, "outpost_name", true, false);
	}

	string genCode(CMissionData &md)
	{
		if (!_OutpostName.empty())
		{
			string ret;
			ret = "give_control : "+_OutpostName;
			ret += NL;
			return ret;
		}
		else
			return string();
	}

};
REGISTER_STEP_CONTENT(CActionGiveOutpostControl, "give_control");


// ---------------------------------------------------------------------------
class CActionSpawnMission : public IStepContent
{
protected:
	string	_MissionName;
	string	_GiverName;
	bool	_Guild;
private:
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_MissionName = md.getProperty(prim, "mission_name", true, false);
		_GiverName = md.getProperty(prim, "giver_name", true, false);
		if (_GiverName.empty())
		{
			throw EParseException(prim, "giver_name is empty !");
		}

		_Guild = md.getProperty(prim, "guild", false, true) == "true";
		// Check: if _Guild is true then check if we are in a guild mission
		if (_Guild && !md.isGuildMission())
		{
			string err = toString("primitive(%s): 'guild' option true 1 for non guild mission.", prim->getName().c_str());
			throw EParseException(prim, err.c_str());
		}
	}
	
	string genCode(CMissionData &md)
	{
		string ret;
		if (!_MissionName.empty())
		{
			ret =  "spawn_mission : " + _MissionName + " : " + _GiverName;
			if (_Guild)
				ret += " : guild";
			ret += NL;
		}
		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionSpawnMission, "spawn_mission");


// ---------------------------------------------------------------------------
class CActionChainMission : public CActionSpawnMission
{
public:

	string genCode(CMissionData &md)
	{
		
		if (!_MissionName.empty())
		{
		

			return "chain_mission : " + _MissionName + " : " + _GiverName + NL;
		}
		else
			return string();
	}

};
REGISTER_STEP_CONTENT(CActionChainMission, "chain_mission");


// ---------------------------------------------------------------------------
class CActionEncycloUnlock : public IStepContent
{
	string	_AlbumThema;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_AlbumThema = md.getProperty(prim, "album_thema", true, false);
	}

	string genCode(CMissionData &md)
	{
		if (!_AlbumThema.empty())
			return "encyclo_unlock : " + _AlbumThema + NL;
		else
			return string();
	}

};
REGISTER_STEP_CONTENT(CActionEncycloUnlock, "encyclo_unlock");


// ---------------------------------------------------------------------------
class CActionHandleCreate : public IStepContent
{
protected:
	string	_Group;
	string	_DespawnTimer;
private:
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_Group = md.getProperty(prim, "group", true, false);
		_DespawnTimer = md.getProperty(prim, "despawn_timer", true, false);
	}
	
	string genCode(CMissionData &md)
	{
		if (!_Group.empty())
		{
			if (!_DespawnTimer.empty())
				return "handle_create : " + _Group + " : " + _DespawnTimer + NL;
			else
				return "handle_create : " + _Group + NL;
		}
		else
			return string();
	}
	
};
REGISTER_STEP_CONTENT(CActionHandleCreate, "handle_create");


// ---------------------------------------------------------------------------
class CActionHandleRelease : public IStepContent
{
protected:
	string	_Group;

private:
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_Group = md.getProperty(prim, "group", true, false);
	}
	
	string genCode(CMissionData &md)
	{
		if (!_Group.empty())
			return "handle_release : " + _Group + NL;
		else
			return string();
	}
	
};
REGISTER_STEP_CONTENT(CActionHandleRelease, "handle_release");


// ---------------------------------------------------------------------------
class CActionSetEventFaction : public IStepContent
{
	string	_EventFaction;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_EventFaction = md.getProperty(prim, "event_faction", true, false);
	}

	string genCode(CMissionData &md)
	{
		if (!_EventFaction.empty())
			return "set_event_faction : " + _EventFaction + NL;
		else
			return string();
	}

};
REGISTER_STEP_CONTENT(CActionSetEventFaction, "set_event_faction");


// ---------------------------------------------------------------------------
class CActionSetRespawnPoints : public IStepContent
{
	string			_Continent;
	vector<string>	_RespawnPoints;
	bool			_HideOthers;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_Continent = md.getProperty(prim, "continent", true, false);

		vector<string> vs;
		vs = md.getPropertyArray(prim, "respawn_points", true, false);

		for (uint i=0; i<vs.size(); ++i)
		{
			if (!vs[i].empty())
				_RespawnPoints.push_back(vs[i]);
		}

		string s;
		s = md.getProperty(prim, "hide_others", true, false);
		_HideOthers = (NLMISC::toLower(s) == "true");
	}

	string genCode(CMissionData &md)
	{
		string ret;

		if (_Continent.empty() || _RespawnPoints.empty())
			return ret;

		ret = "set_respawn_points : " + _Continent + " : ";

		for (uint i = 0; i < _RespawnPoints.size(); i++)
		{
			ret += _RespawnPoints[i];
			if (i < _RespawnPoints.size() - 1)
				ret += "; ";
		}

		ret += string(" : ") + (_HideOthers?"true":"false");
		ret += NL;

		return ret;
	}

};
REGISTER_STEP_CONTENT(CActionSetRespawnPoints, "set_respawn_points");


// ---------------------------------------------------------------------------
class CActionRecvFactionPoint : public IStepContent
{
	string	_Faction;
	string	_Point;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_Faction = md.getProperty(prim, "faction", true, false);
		_Point = md.getProperty(prim, "point", true, false);
	}

	string genCode(CMissionData &md)
	{
		if (!_Faction.empty() && !_Point.empty())
			return string("recv_faction_point : ")+_Faction+" "+_Point+NL;
		else
			return string();
	}
};
REGISTER_STEP_CONTENT(CActionRecvFactionPoint, "recv_faction_point");


// ---------------------------------------------------------------------------
class CActionSDBSet : public IStepContent
{
	string	_SDBPath;
	string	_SDBValue;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_SDBPath = md.getProperty(prim, "sdb_path", true, false);
		_SDBValue = md.getProperty(prim, "sdb_value", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		if (_SDBPath.empty() || _SDBValue.empty())
			return ret;

		ret = "sdb_set : " + _SDBPath + " : " + _SDBValue + NL;
		return ret;
	}

};
REGISTER_STEP_CONTENT(CActionSDBSet, "sdb_set");


// ---------------------------------------------------------------------------
class CActionSDBAdd : public IStepContent
{
	string	_SDBPath;
	string	_SDBValue;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_SDBPath = md.getProperty(prim, "sdb_path", true, false);
		_SDBValue = md.getProperty(prim, "sdb_value", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		if (_SDBPath.empty() || _SDBValue.empty())
			return ret;

		ret = "sdb_add : " + _SDBPath + " : " + _SDBValue + NL;
		return ret;
	}

};
REGISTER_STEP_CONTENT(CActionSDBAdd, "sdb_add");


// ---------------------------------------------------------------------------
class CActionSDBPlayerAdd : public IStepContent
{
	string	_SDBPath;
	string	_SDBValue;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_SDBPath = md.getProperty(prim, "sdb_path", true, false);
		_SDBValue = md.getProperty(prim, "sdb_value", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		if (_SDBPath.empty() || _SDBValue.empty())
			return ret;

		ret = "sdb_player_add : " + _SDBPath + " : " + _SDBValue + NL;
		return ret;
	}

};
REGISTER_STEP_CONTENT(CActionSDBPlayerAdd, "sdb_player_add");


// ---------------------------------------------------------------------------
class CActionSDBSetPvPPath : public IStepContent
{
	string	_SDBPath;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_SDBPath = md.getProperty(prim, "sdb_path", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		if (_SDBPath.empty())
			return ret;

		ret = "sdb_set_pvp_path : " + _SDBPath + NL;
		return ret;
	}

};
REGISTER_STEP_CONTENT(CActionSDBSetPvPPath, "sdb_set_pvp_path");


// ---------------------------------------------------------------------------
class CActionSDBClearPvPPath : public IStepContent
{
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret = "sdb_clear_pvp_path" + NL;
		return ret;
	}

};
REGISTER_STEP_CONTENT(CActionSDBClearPvPPath, "sdb_clear_pvp_path");


// ---------------------------------------------------------------------------
class CActionRecvFame : public IStepContent
{
	string	_Faction;
	string	_Fame;
	bool	_Guild;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_Faction = md.getProperty(prim, "faction", true, false);
		_Fame = md.getProperty(prim, "value", true, false);

		_Guild = md.getProperty(prim, "guild", false, true) == "true";
		// Check: if _Guild is true then check if we are in a guild mission
		if (_Guild && !md.isGuildMission())
		{
			string err = toString("primitive(%s): 'guild' option true 1 for non guild mission.", prim->getName().c_str());
			throw EParseException(prim, err.c_str());
		}
	}

	string genCode(CMissionData &md)
	{
		if (!_Faction.empty() && !_Fame.empty())
		{
			string ret;
			ret = "recv_fame : "+_Faction+" "+_Fame;
			if (_Guild)
				ret += ": guild";
			ret += NL;
			return ret;
		}
		else
			return string();
	}
};
REGISTER_STEP_CONTENT(CActionRecvFame, "recv_fame");

// ---------------------------------------------------------------------------
struct TItemDesc
{
	string	ItemName;
	string	ItemQuant;
	string	ItemQual;
};

class CActionRecvItem : public IStepContent
{
	string				_BotGiver;
	vector<TItemDesc>	_Items;
	bool				_QualSpec;
	bool				_Group;
	bool				_Guild;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		_BotGiver =md.getProperty(prim, "npc_name", true, false);
		vector<string> vs;
		vs = md.getPropertyArray(prim, "item/quantity/quality", true, false);
		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> args;
			if (!vs[i].empty())
			{
				explode(vs[i], string(" "), args, false);
				if (args.size() != 3 && args.size() != 2)
				{
					string err = toString("can't find 2 or 3 part (<item> <quant> [<qual>]) in item line '%s', found %u instead", vs[i].c_str(), args.size());
					throw EParseException(prim, err.c_str());
				}

				if (i == 0)
				{
					_QualSpec = (args.size() == 3);
				}
				else if (args.size() == 3 && !_QualSpec)
				{
					string err = toString("can't mix item with quality and item without quality in item line '%s'", vs[i].c_str());
					throw EParseException(prim, err.c_str());
				}

				TItemDesc item;
				item.ItemName = args[0];
				item.ItemQuant = args[1];
				if (_QualSpec)
					item.ItemQual = args[2];

				_Items.push_back(item);
			}
		}

		string s;
		s = md.getProperty(prim, "group", true, false);
		_Group = (NLMISC::toLower(s) == "true");

		_Guild = md.getProperty(prim, "guild", false, true) == "true";
		// Check: if _Guild is true then check if we are in a guild mission
		if (_Guild && !md.isGuildMission())
		{
			string err = toString("primitive(%s): 'guild' option true 1 for non guild mission.", prim->getName().c_str());
			throw EParseException(prim, err.c_str());
		}

		IStepContent::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		for (uint i=0; i<_Items.size(); ++i)
		{
			TItemDesc &item = _Items[i];
			
			ret += "recv_item : "+item.ItemName+" "+item.ItemQuant;
			if (_QualSpec)
				ret +=" "+item.ItemQual;
			if (!_BotGiver.empty())
				ret += " : "+_BotGiver;
			if (_Group)
				ret += " : group";
			if (_Guild)
				ret += ": guild";
			ret += NL;
		}

		return ret;

	}
};
REGISTER_STEP_CONTENT(CActionRecvItem, "recv_item");

// ---------------------------------------------------------------------------
class CActionRecvNamedItem : public IStepContent
{
	vector<TItemDesc>	_Items;
	bool				_Group;
	bool				_Guild;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		vector<string> vs;
		vs = md.getPropertyArray(prim, "item/quantity", true, false);
		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> args;
			if (!vs[i].empty())
			{
				explode(vs[i], string(" "), args, false);
				if (args.size() != 1 && args.size() != 2)
				{
					string err = toString("can't find 1 or 2 part (<item> [<quant>]) in item line '%s', found %u instead", vs[i].c_str(), args.size());
					throw EParseException(prim, err.c_str());
				}
				
				TItemDesc item;
				item.ItemName = args[0];
				item.ItemQuant = "1";
				if(args.size()>=2 && atoi(args[1].c_str())>0)
					item.ItemQuant = args[1];
				
				_Items.push_back(item);
			}
		}
		
		string s;
		s = md.getProperty(prim, "group", true, false);
		_Group = (NLMISC::toLower(s) == "true");

		_Guild = md.getProperty(prim, "guild", false, true) == "true";
		// Check: if _Guild is true then check if we are in a guild mission
		if (_Guild && !md.isGuildMission())
		{
			string err = toString("primitive(%s): 'guild' option true 1 for non guild mission.", prim->getName().c_str());
			throw EParseException(prim, err.c_str());
		}

		IStepContent::init(md, prim);
	}
	
	string genCode(CMissionData &md)
	{
		string ret;
		for (uint i=0; i<_Items.size(); ++i)
		{
			TItemDesc &item = _Items[i];
			
			ret += "recv_named_item : "+item.ItemName+" "+item.ItemQuant;
			if (_Group)
				ret += " : group";
			if (_Guild)
				ret += ": guild";
			ret += NL;
		}
		
		return ret;
		
	}
};
REGISTER_STEP_CONTENT(CActionRecvNamedItem, "recv_named_item");

// ---------------------------------------------------------------------------
class CActionDestroyItem : public IStepContent
{
	struct CItemDesc
	{
		TItemDesc	Desc;
		bool		QuantSpec;
		bool		QualSpec;
	};

	string				_BotDestroyer;
	vector<CItemDesc>	_Items;
	bool				_Guild;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		// get the bot who destroys the item
		_BotDestroyer =md.getProperty(prim, "npc_name", true, false);

		// read list of items to destroy
		vector<string> vs;
		vs = md.getPropertyArray(prim, "item/quantity/quality", true, false);
		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> args;
			if (!vs[i].empty())
			{
				// parse
				explode(vs[i], string(" "), args, false);
				if (args.size()<1 || args.size()>3)
				{
					string err = toString("can't find 1, 2 or 3 part (<item> [<quant>] [<qual>]) in item line '%s', found %u instead", vs[i].c_str(), args.size());
					throw EParseException(prim, err.c_str());
				}
				
				// add the item in desc
				CItemDesc item;
				item.Desc.ItemName = args[0];
				if(args.size()>=2)
				{
					item.QuantSpec= true;
					item.Desc.ItemQuant = args[1];
				}
				if(args.size()>=3)
				{
					item.QualSpec= true;
					item.Desc.ItemQual = args[2];
				}
				
				_Items.push_back(item);
			}
		}

		_Guild = md.getProperty(prim, "guild", false, true) == "true";
		// Check: if _Guild is true then check if we are in a guild mission
		if (_Guild && !md.isGuildMission())
		{
			string err = toString("primitive(%s): 'guild' option true 1 for non guild mission.", prim->getName().c_str());
			throw EParseException(prim, err.c_str());
		}
		
		IStepContent::init(md, prim);
	}
	
	string genCode(CMissionData &md)
	{
		string ret;
		for (uint i=0; i<_Items.size(); ++i)
		{
			CItemDesc &item = _Items[i];
			
			ret += "destroy_item : "+item.Desc.ItemName;
			if (item.QuantSpec)
				ret +=" "+item.Desc.ItemQuant;
			if (item.QualSpec)
				ret +=" "+item.Desc.ItemQual;
			if (!_BotDestroyer.empty())
				ret += " : "+_BotDestroyer;
			if (_Guild)
				ret += ": guild";
			ret += NL;
		}
		
		return ret;
		
	}
};
REGISTER_STEP_CONTENT(CActionDestroyItem, "destroy_item");

// ---------------------------------------------------------------------------
class CActionRecvXp : public IStepContent
{
	string			_Skill;
	sint32			_Value;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:

	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_Skill = md.getProperty(prim, "skill", true, false);
		_Value = atoi(md.getProperty(prim, "value", true, false).c_str());
		if(_Skill.empty() || tolower(_Skill[0])!='s' || _Value<=0)
		{
			string err = toString("Bad skill name or value: '%s' '%d' ", _Skill.c_str(), _Value);
			throw EParseException(prim, err.c_str());
		}
	}
	
	string genCode(CMissionData &md)
	{
		if (!_Skill.empty())
			return toString("recv_xp : %s %d", _Skill.c_str(), _Value) + NL;
		else
			return string();
	}
};
REGISTER_STEP_CONTENT(CActionRecvXp, "recv_xp");

// ---------------------------------------------------------------------------
class CActionLearnAction : public IStepContent
{
	string				_BotGiver;
	vector<string>		_Actions;
	bool				_Group;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		_BotGiver = md.getProperty(prim, "npc_name", true, false);
		vector<string> vs;
		vs = md.getPropertyArray(prim, "actions", true, false);

		for (uint i=0; i<vs.size(); ++i)
		{
			if (!vs[i].empty())
				_Actions.push_back(vs[i]);
		}

		string s;
		s = md.getProperty(prim, "group", true, false);
		_Group = (NLMISC::toLower(s) == "true");

		IStepContent::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		if (_Actions.empty())
			return ret;

		ret = "learn_action : ";
		for (uint i=0; i<_Actions.size(); ++i)
		{
			ret += _Actions[i];
			if (i < _Actions.size()-1)
				ret += "; ";
		}

		if (!_BotGiver.empty())
			ret += " : "+_BotGiver;
		if (_Group)
			ret += " : group";
		ret += NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionLearnAction, "learn_action");

// ---------------------------------------------------------------------------
class CActionLearnBrick : public IStepContent
{
	string				_BotGiver;
	vector<string>		_Bricks;
	bool				_Group;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		_BotGiver = md.getProperty(prim, "npc_name", true, false);
		vector<string> vs;
		vs = md.getPropertyArray(prim, "bricks", true, false);

		for (uint i=0; i<vs.size(); ++i)
		{
			if (!vs[i].empty())
				_Bricks.push_back(vs[i]);
		}

		string s;
		s = md.getProperty(prim, "group", true, false);
		_Group = (NLMISC::toLower(s) == "true");

		IStepContent::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;

//		if (_Bricks.empty())
//			return ret;

		ret = "learn_brick : ";
		for (uint i=0; i<_Bricks.size(); ++i)
		{
			ret += _Bricks[i];
			if (i < _Bricks.size()-1)
				ret += "; ";
		}

		if (!_BotGiver.empty())
			ret += " : "+_BotGiver;
		if (_Group)
			ret += " : group";
		ret += NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionLearnBrick, "learn_brick");

// ---------------------------------------------------------------------------
class CActionBotChat : public IStepContent
{
protected:
	string				_BotName;
	CPhrase				_Phrase;
	string				_ChatMode;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_BotName = md.getProperty(prim, "npc_name", true, false);
		if (_BotName.empty())
			throw EParseException(prim, "npc_name is empty !");
		vector<string> vs;
		vs = md.getPropertyArray(prim, "phrase", false, false);
		_Phrase.initPhrase(md, prim, vs);

		_ChatMode = md.getProperty(prim, "chat_type", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "bot_chat : "+_ChatMode+" : "+_BotName+" : "+_Phrase.genScript(md)+NL;
		return ret;
	}

	string genPhrase()
	{
		return _Phrase.genPhrase();
	}
};
REGISTER_STEP_CONTENT(CActionBotChat, "bot_chat");

// ---------------------------------------------------------------------------
class CActionBotEmote : public IStepContent
{
	string				_BotName;
	string				_Emote;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_BotName = md.getProperty(prim, "npc_name", true, false);
		_Emote = md.getProperty(prim, "emote", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "bot_emot : "+_BotName+" "+_Emote+NL;
		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionBotEmote, "bot_emote");

// ---------------------------------------------------------------------------
class CActionAIEvent : public IStepContent
{
	string				_GroupName;
	string				_EventNumber;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_GroupName = md.getProperty(prim, "group_name", true, false);
		_EventNumber = md.getProperty(prim, "event_number", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "ai_event : "+_GroupName+"; "+_EventNumber+NL;
		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionAIEvent, "ai_event");

// ---------------------------------------------------------------------------
class CActionSetTeleport : public CActionBotChat
{
	string				_WorldPosition;
	bool				_Once;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		CActionBotChat::init(md, prim);

		_WorldPosition = md.getProperty(prim, "world_position", true, false);
		string s;
		prim->getPropertyByName("once", s);
		_Once = (NLMISC::toLower(s) == "true");
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "set_teleport : "+_BotName+" : "+_WorldPosition+" : ";

		if (_Once)
			ret += "once : ";
		ret += _Phrase.genScript(md) + NL;
		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionSetTeleport, "set_teleport");

// ---------------------------------------------------------------------------
class CActionTeleport : public IStepContent
{
	string				_WorldPosition;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_WorldPosition = md.getProperty(prim, "world_position", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "teleport : "+_WorldPosition+NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionTeleport, "teleport");

// ---------------------------------------------------------------------------
class CActionSetCult : public IStepContent
{
	string				_Cult;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_Cult = md.getProperty(prim, "cult", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "set_cult : " + _Cult + NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionSetCult, "set_cult");

// ---------------------------------------------------------------------------
class CActionSetCiv : public IStepContent
{
	string				_Civ;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_Civ = md.getProperty(prim, "civ", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "set_civ : " + _Civ + NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionSetCiv, "set_civ");

// ---------------------------------------------------------------------------
class CActionSetGuildCult : public IStepContent
{
	string				_Cult;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_Cult = md.getProperty(prim, "cult", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "set_guild_cult : " + _Cult + NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionSetGuildCult, "set_guild_cult");

// ---------------------------------------------------------------------------
class CActionSetGuildCiv : public IStepContent
{
	string				_Civ;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_Civ = md.getProperty(prim, "civ", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "set_guild_civ : " + _Civ + NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionSetGuildCiv, "set_guild_civ");

// ---------------------------------------------------------------------------
class CActionAddCompass : public IStepContent
{
	string				_NpcName;
	string				_PlaceName;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_NpcName = md.getProperty(prim, "npc_to_add", true, false);
		_PlaceName = md.getProperty(prim, "place_to_add", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		if (!_NpcName.empty())
			ret = "add_compass_npc : "+_NpcName+NL;
		if (!_PlaceName.empty())
			ret += "add_compass_place : "+_PlaceName+NL;
		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionAddCompass, "add_compass");

// ---------------------------------------------------------------------------
class CActionRemoveCompass : public IStepContent
{
	string				_NpcName;
	string				_PlaceName;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_NpcName = md.getProperty(prim, "npc_to_remove", true, false);
		_PlaceName = md.getProperty(prim, "place_to_remove", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		if (!_NpcName.empty())
			ret = "remove_compass_npc : "+_NpcName+NL;
		if (!_PlaceName.empty())
			ret += "remove_compass_place : "+_PlaceName+NL;
		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionRemoveCompass, "remove_compass");

// ---------------------------------------------------------------------------
class CActionFail : public IStepContent
{

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		return string("fail")+NL;
	}
};
REGISTER_STEP_CONTENT(CActionFail, "fail");

// ---------------------------------------------------------------------------
class CActionFailIfSDB : public IStepContent
{

	string _Condition;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_Condition = md.getProperty(prim, "condition", true, false);
	}

	string genCode(CMissionData &md)
	{
		return string("fail_if_sdb : ")+_Condition+NL;
	}
};
REGISTER_STEP_CONTENT(CActionFailIfSDB, "fail_if_sdb");

// ---------------------------------------------------------------------------
class CActionFailMissionCat : public IStepContent
{

	string _MissionCategory;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_MissionCategory = md.getProperty(prim, "category", true, false);
	}

	string genCode(CMissionData &md)
	{
		return string("fail_mission_cat : ")+_MissionCategory+NL;
	}
};
REGISTER_STEP_CONTENT(CActionFailMissionCat, "fail_mission_cat");

// ---------------------------------------------------------------------------
class CActionSystemMsg : public IStepContent
{
	CPhrase		_Message;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		vector<string>	vs = md.getPropertyArray(prim, "msg_to_display", false, false);

		_Message.initPhrase(md, prim, vs);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "system_msg : "+_Message.genScript(md)+NL;
		return ret;
	}

	string genPhrase()
	{
		return _Message.genPhrase();
	}
};
REGISTER_STEP_CONTENT(CActionSystemMsg, "system_msg");

// ---------------------------------------------------------------------------
class CActionPopupMsg : public IStepContent
{
	CPhrase		_Title;
	CPhrase		_Message;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		
		vector<string>	vst = md.getPropertyArray(prim, "title_to_display", false, false);
		vector<string>	vsm = md.getPropertyArray(prim, "msg_to_display", false, false);
		
		_Title.initPhrase(md, prim, vst);
		_Message.initPhrase(md, prim, vsm);
	}
	
	string genCode(CMissionData &md)
	{
		string ret;
		
		ret = "popup_msg : " + _Title.genScript(md) + " : " + _Message.genScript(md) + NL;
		return ret;
	}
	
	string genPhrase()
	{
		return _Title.genPhrase() + _Message.genPhrase();
	}
};
REGISTER_STEP_CONTENT(CActionPopupMsg, "popup_msg");

// ---------------------------------------------------------------------------
class CActionDeclareWar : public IStepContent
{

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		return string("declare_war")+NL;
	}
};
REGISTER_STEP_CONTENT(CActionDeclareWar, "declare_war");

// ---------------------------------------------------------------------------
class CActionSetConstrains : public IStepContent
{
	string	_Timer;
	string	_TimePeriod;
	string	_InsidePlace;
	string	_InsidePlaceDelay;
	string	_OutsidePlace;
	string	_OutsidePlaceDelay;
	vector<string>	_Wears;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		_Timer = md.getProperty(prim, "timer", true, false);
		_TimePeriod = md.getProperty(prim, "time_period", true, false);
		_InsidePlace = md.getProperty(prim, "inside_place", true, false);
		_InsidePlaceDelay = md.getProperty(prim, "inside_place_delay", true, false);
		_OutsidePlace = md.getProperty(prim, "outside_place", true, false);
		_OutsidePlaceDelay = md.getProperty(prim, "outside_place_delay", true, false);
		_Wears = md.getPropertyArray(prim, "wear", true, false);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		if (!_Timer.empty())
			ret += "timer : "+_Timer+NL;
		if (!_TimePeriod.empty())
		{
			if (	_TimePeriod == "winter"
				||	_TimePeriod == "spring"
				||	_TimePeriod == "summer"
				||	_TimePeriod == "autumn"
				||	_TimePeriod == "none")
			{
				ret += "season : "+_TimePeriod+NL;
			}
			else
				ret += "day_period : "+_TimePeriod+NL;
		}
		if (!_InsidePlace.empty())
		{
			if (_InsidePlace != "none")
			{
				ret += "inside : "+_InsidePlace;
				if (!_InsidePlaceDelay.empty())
					ret += " : "+_InsidePlaceDelay;
			}
			else
				ret += "inside";
			ret += NL;
		}
		if (!_OutsidePlace.empty())
		{
			if (_OutsidePlace != "none")
			{
				ret += "outside : "+_OutsidePlace;
				if (!_OutsidePlaceDelay.empty())
					ret += " : "+_OutsidePlaceDelay;
			}
			else
				ret += "outside";
			ret += NL;
		}
		if (!_Wears.empty())
			nlwarning("wear constraint not implemented yet");
		return ret;
	}
};
REGISTER_STEP_CONTENT(CActionSetConstrains, "set_constrains");

// ---------------------------------------------------------------------------
class CActionSetDesc : public IStepContent
{
	CPhrase		_Description;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);

		std::vector<string> vs = md.getPropertyArray(prim, "mission_description", false, false);

		_Description.initPhrase(md, prim, vs);
	}

	string genCode(CMissionData &md)
	{
		string ret;

		ret = "set_desc : "+_Description.genScript(md)+NL;
		return ret;
	}

	string genPhrase()
	{
		return _Description.genPhrase();
	}

};
REGISTER_STEP_CONTENT(CActionSetDesc, "set_desc");

// *****************
// CContentObjective
// *****************
// base class for step content objectives
// ---------------------------------------------------------------------------
void CContentObjective::init(CMissionData &md, IPrimitive *prim)
{
	IStepContent::init(md, prim);

	_HideObj = md.getProperty(prim, "hide_obj", true, false) == "true";
	_OverloadObj = md.getPropertyArray(prim, "overload_objective", false, false);
	_RoleplayObj = md.getPropertyArray(prim, "roleplay_objective", false, false);
	uint32 numEntry;
	CPhrase::TPredefParams params;
	// ask derived class for predefined params
	getPredefParam(numEntry, params);
	// init the overload phrase
	_OverloadPhrase.initPhrase(md, prim, _OverloadObj, numEntry, params);
	// init the roleplay phrase
	_RoleplayPhrase.initPhrase(md, prim, _RoleplayObj, numEntry, params);

	// check for the 'nb_guild_members_needed' option and see if it's correct for this mission
	/*string nbGuildMembersNeeded = md.getProperty(prim, "nb_guild_members_needed", false, true);
	if (nbGuildMembersNeeded.empty())
		nbGuildMembersNeeded = "1";
	if (!fromString(nbGuildMembersNeeded.c_str(), _NbGuildMembersNeeded))
		_NbGuildMembersNeeded = 1;

	// Check:
	if (!md.isGuildMission() && _NbGuildMembersNeeded != 1)
	{
		string err = toString("primitive(%s): nb_guild_members_needed != 1 for non guild mission.", prim->getName().c_str());
		throw EParseException(prim, err.c_str());
	}*/
}

// ---------------------------------------------------------------------------
string CContentObjective::genCode(CMissionData &md)
{
	string ret;
	if (_HideObj)
		ret = "hide_obj"+NL;
	if (!_OverloadObj.empty())
	{
		ret += "set_obj : " + _OverloadPhrase.genScript(md)+NL;
	}
	if (!_RoleplayObj.empty())
	{
		ret += "set_obj_rp : " + _RoleplayPhrase.genScript(md)+NL;
	}
	return ret;
}

// ---------------------------------------------------------------------------
/*std::string CContentObjective::genNbGuildMembersNeededOption(CMissionData &md)
{
	string ret;
	// If we are in a guild mission we add the 'nb_guild_members_needed' option to the script
	if (md.isGuildMission())
	{
		ret = ": nb_guild_members_needed ";
		ret += toString(_NbGuildMembersNeeded);
	}

	return ret;
}*/

// ---------------------------------------------------------------------------
string CContentObjective::genPhrase()
{
	if (!_OverloadObj.empty())
		return _OverloadPhrase.genPhrase();
	else
		return _RoleplayPhrase.genPhrase();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

struct TKillFaunaInfo
{
	TCompilerVarName	SheetName;
	TCompilerVarName	Quantity;
};

struct TKillRaceInfo
{
	TCompilerVarName	RaceName;
	TCompilerVarName	Quantity;
};


class CContentKill : public CContentObjective
{
	vector<TKillFaunaInfo>	_KillFaunas;
	vector<TKillRaceInfo>	_KillRaces;
	TCompilerVarName		_KillGroup;
	vector<TCompilerVarName> _KillNpcs;
	TCompilerVarName		_KillNpcByNames;
	TCompilerVarName		_KillNpcByNamesQuantity;
	TCompilerVarName		_KillFactionName;
	TCompilerVarName		_KillFactionQuantity;
	vector<string>			_PredefVarName;
	TCompilerVarName		_Place;


	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		if (!_KillFaunas.empty())
		{
			numEntry = (uint32)_KillFaunas.size();
			predef.resize(numEntry);
			for (uint i=0; i<_KillFaunas.size(); ++i)
			{
				predef[i].resize(2*(i+1));
				for (uint j=0; j<i+1; ++j)
				{
					predef[i][2*j] = _KillFaunas[j].SheetName;
					predef[i][2*j+1] = _KillFaunas[j].Quantity;
				}
			}
		}
		else if (!_KillRaces.empty())
		{
			numEntry = (uint32)_KillRaces.size();
			predef.resize(numEntry);
			for (uint i=0; i<_KillRaces.size(); ++i)
			{
				predef[i].resize(2*(i+1));
				for (uint j=0; j<i+1; ++j)
				{
					predef[i][2*j] = _KillRaces[j].RaceName;
					predef[i][2*j+1] = _KillRaces[j].Quantity;
				}
			}
		}
		else if (!_KillGroup.empty())
		{				
			numEntry = 0;
			predef.resize(1);
			predef[0].resize(1);
			predef[0][0] = _KillGroup;			
		}
		else if (!_KillNpcs.empty())
		{
			numEntry = (uint32)_KillNpcs.size();
			predef.resize(numEntry);
			for (uint i=0; i<_KillNpcs.size(); ++i)
			{
				predef[i].resize(i+1);
				for (uint j=0; j<i+1; ++j)
				{
					predef[i][j] = _KillNpcs[j];
				}
			}
		}
		else if (!_KillNpcByNames._VarValue.empty())
		{
			numEntry = 0;
			predef.resize(1);
			predef[0].resize(2);
			predef[0][0] = _KillNpcByNames;
			predef[0][1] =_KillNpcByNamesQuantity;
		}
		else if (!_KillFactionName._VarValue.empty())
		{
			numEntry = 0;
			predef.resize(1);
			predef[0].resize(2);
			predef[0][0] = _KillFactionName;
			predef[0][1] = _KillFactionQuantity;

		}

		// add optional place
		if (!_Place._VarValue.empty())
		{
			for (uint i=0; i<predef.size(); ++i)
			{
				predef[i].push_back(_Place);

			}
		}
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		vector<string> vs;
		vs = md.getPropertyArray(prim, "fauna/quantity", false, false);
		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string>	args;
			explode(vs[i], string(" "), args, true);
			if (args.size() != 2)
			{
				string err = toString("Syntax error in kill fauna : '%s', need <fauna_sheet_name> <quantity>", vs[i].c_str());
				throw EParseException (prim, err.c_str());
			}
			TKillFaunaInfo kfi;
			kfi.SheetName.initWithText(toString("c%u", i+1), STRING_MANAGER::creature_model, md, prim, args[0]);
			kfi.Quantity.initWithText(toString("q%u", i+1), STRING_MANAGER::integer, md, prim, args[1]);
			_KillFaunas.push_back(kfi);
		}

		vs = md.getPropertyArray(prim, "race/quantity", false, false);
		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string>	args;
			explode(vs[i], string(" "), args, true);
			if (args.size() != 2)
			{
				string err = toString("Syntax error in kill race : '%s', need <race_name> <quantity>", vs[i].c_str());
				throw EParseException (prim, err.c_str());
			}
			TKillRaceInfo kri;
			kri.RaceName.initWithText(toString("r%u", i+1), STRING_MANAGER::creature_model, md, prim, args[0]);
			kri.Quantity.initWithText(toString("q%u", i+1), STRING_MANAGER::integer, md, prim, args[1]);
			_KillRaces.push_back(kri);
		}
		string s;
		//s = md.getProperty(prim, "group", false, true);
		vs = md.getPropertyArray(prim, "group", false, false);
		if (!vs.empty())
		{
			if (vs.size() != 1)
			{
				string err = "Syntax error in kill group";
				throw EParseException (prim, err.c_str());
			}

			_KillGroup.initWithText("group_name", STRING_MANAGER::bot_name, md, prim, vs[0] );

		}
		

		vs = md.getPropertyArray(prim, "npc", false, false);
		for (uint i=0; i<vs.size(); ++i)
		{
			TCompilerVarName npcname;
			npcname.initWithText(toString("npc%u", i+1), STRING_MANAGER::bot, md, prim, vs[i]);
			_KillNpcs.push_back(npcname);
		}

		s = md.getProperty(prim, "npc_by_name/quantity", false, false);
		if (!s.empty())
		{
			vector<string> args;
			explode(s, string(" "), args, true);
			if (args.size() != 2)
			{
				string err = toString("Syntax error in kill npc by name : '%s', need <npc_name> <quantity>", s.c_str());
				throw EParseException(prim, err.c_str());
			}
			_KillNpcByNames.initWithText("npc", STRING_MANAGER::bot_name, md, prim, args[0]);
			_KillNpcByNamesQuantity.initWithText("qt", STRING_MANAGER::integer, md, prim, args[1]);
		}

		s = md.getProperty(prim, "faction/quantity", false, false);
		if (!s.empty())
		{
			vector<string>	args;
			explode(s, string(" "), args, true);
			if (args.size() != 2)
			{
				string err = toString("Syntax error in kill faction : '%s', need <faction_name> <quantity>", s.c_str());
				throw EParseException (prim, err.c_str());
			}
			_KillFactionName.initWithText("faction", STRING_MANAGER::faction, md, prim, args[0]);
			_KillFactionQuantity.initWithText("qt", STRING_MANAGER::integer, md, prim, args[1]);
		}

		bool check = false;
		if (!_KillFaunas.empty())
			check = true;
		if (!_KillRaces.empty())
		{
			if (check)
				throw EParseException(prim, "Merging of multiple kill mode is forbidden !");
			check = true;
		}
		if (!_KillGroup.empty())
		{
			if (check)
				throw EParseException(prim, "Merging of multiple kill mode is forbidden !");

			check = true;
		}
		if (!_KillNpcs.empty())
		{
			if (check)
				throw EParseException(prim, "Merging of multiple kill mode is forbidden !");
			check = true;
		}
		if (!_KillNpcByNames._VarValue.empty())
		{
			if (check)
				throw EParseException(prim, "Merging of multiple kill mode is forbidden !");
			check = true;
		}
		if (!_KillFactionName.empty())
		{
			if (check)
				throw EParseException(prim, "Merging of multiple kill mode is forbidden !");
			check = true;
		}
		
		_Place.init("p", STRING_MANAGER::place, md, prim, "place");

		if (!_Place.empty() && !_KillGroup.empty())
		{
			throw EParseException(prim, "Using location with kill group instruction is forbidden!");
		}
		// init base class 
		CContentObjective::init(md, prim);

	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		if (!_KillFaunas.empty())
		{
			ret += "kill_fauna : ";
			for (uint i=0; i<_KillFaunas.size(); ++i)
			{
				ret += _KillFaunas[i].SheetName+" "+_KillFaunas[i].Quantity;
				if (i < _KillFaunas.size()-1)
					ret += "; ";
			}
		}
		else if (!_KillRaces.empty())
		{
			ret += "kill_race : ";
			for (uint i=0; i<_KillRaces.size(); ++i)
			{
				ret += _KillRaces[i].RaceName+" "+_KillRaces[i].Quantity;
				if (i < _KillRaces.size()-1)
					ret += "; ";
			}
		}
		else if (!_KillGroup.empty())
		{
			ret += "kill_group : " + _KillGroup; 			
		}
		else if (!_KillNpcs.empty())
		{
			ret += "kill_npc : ";
			for (uint i=0; i<_KillNpcs.size(); ++i)
			{
				ret += _KillNpcs[i];
				if (i < _KillNpcs.size()-1)
					ret += "; ";
			}
		}
		else if (!_KillNpcByNames.empty())
		{
			ret += "kill_npc_by_name : "+_KillNpcByNames+" "+_KillNpcByNamesQuantity;
		}
		else if (!_KillFactionName.empty())
		{
			ret += "kill_faction : "+_KillFactionName+" "+_KillFactionQuantity;
		}

		if (!_Place.empty())
			ret += " : "+_Place;

		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentKill, "kill");

class CContentTalkTo : public CContentObjective
{
	TCompilerVarName		_BotName;
	CPhrase		_Phrase;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
		predef.resize(1);
		predef[0].resize(1);
		predef[0][0] = _BotName;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{


		_BotName.init("npc", STRING_MANAGER::bot, md, prim, "npc_name" );	

		CPhrase::TPredefParams pp(1);
		pp[0].push_back(_BotName.getParamInfo());
		
		vector<string> vs;
		vs = md.getPropertyArray(prim, "phrase", false, false);
		_Phrase.initPhrase(md, prim, vs, 0, pp);
 
//		_Phrase.initPhrase(md, prim, vs);

//		if (_Phrase.asAdditionnalParams())
//		{
//			// we need to remove the default 'npc' parameter if add params are found
//			CPhrase temp;
//			temp.initPhrase(md, prim, vs);
//
//			_Phrase = temp;
//		}
		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "talk_to : "+_BotName;
		
		if (!_Phrase.isEmpty())
			ret += " : "+_Phrase.genScript(md);
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;
		
		return ret;
	}

	string genPhrase()
	{
		string ret;
		ret = CContentObjective::genPhrase();
		ret += _Phrase.genPhrase();

		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentTalkTo, "talk_to");

class CContentCast : public CContentObjective
{
	vector<TCompilerVarName>	_Actions;
	TCompilerVarName			_Place;

	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	
	{
		numEntry = (uint32)_Actions.size();
		predef.resize(numEntry);
		for (uint i=0; i<numEntry; ++i)
		{
			predef[i].resize(i+1);

			for (uint j=0; j<i+1; ++j)
			{
				predef[i][j] = _Actions[j];
					
			}
		}

		if (!_Place.empty())
		{
			for (uint i=0; i<_Actions.size(); ++i)
			{
				predef[i].push_back(_Place);				
			}
		}
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		_Actions=TCompilerVarName::getPropertyArrayWithTextStaticDefaultName("action", STRING_MANAGER::sphrase, md, prim, "action");
		_Place.init("p",  STRING_MANAGER::place, md, prim, "place");

		

		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "cast : ";

		for (uint i=0; i<_Actions.size(); ++i)
		{
			ret += _Actions[i];
			if (i < _Actions.size()-1)
				ret += "; ";
		}

		if (!_Place.empty())
		{
			ret += ": "+_Place;
		}
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;
		
		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentCast, "cast");

struct TForageInfo
{
	TCompilerVarName	Item;
	TCompilerVarName	Qt;
	TCompilerVarName	Ql;
};

class CContentForage : public CContentObjective
{
	vector<TForageInfo>	_Mps;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = (uint32)_Mps.size();
		predef.resize(numEntry);
		for (uint i=0; i<numEntry; ++i)
		{
			predef[i].resize(3*(i+1));
			for (uint j=0; j<i+1; ++j)
			{
				predef[i][j*3]   = _Mps[j].Item;
				predef[i][j*3+1] = _Mps[j].Qt;
				predef[i][j*3+2] = _Mps[j].Ql;
			}
		}
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		vector<string> vs;
		vs = md.getPropertyArray(prim, "item/quantity/quality", true, false);

		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> args;
			explode(vs[i], string(" "), args, true);
			if (args.size() != 3)
			{
				string err = toString("Invalid forage item in '%s', need <item> <quantity> <quality>", vs[i].c_str());
				throw EParseException(prim, err.c_str());
			}

			TForageInfo fi;
			fi.Item.initWithText(toString("i%u", i+1), STRING_MANAGER::item, md, prim, args[0]);			
			fi.Qt.initWithText(toString("qt%u", i+1), STRING_MANAGER::integer, md, prim, args[1]);						
			fi.Ql.initWithText(toString("qual%u", i+1), STRING_MANAGER::integer, md, prim, args[2]);
			
			_Mps.push_back(fi);
		}

		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "forage : ";

		for (uint i=0; i<_Mps.size(); ++i)
		{
			ret += _Mps[i].Item+" "+_Mps[i].Qt+" "+_Mps[i].Ql;
			if (i < _Mps.size()-1)
				ret += "; ";
		}
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;
		
		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentForage, "forage");

struct TLootInfo
{
	TCompilerVarName	Item;
	TCompilerVarName	Qt;
	TCompilerVarName	Ql;
};

class CContentLoot : public CContentObjective
{
	enum TLootMode
	{
		lm_unknown,
		lm_item,
		lm_mp
	};

	TLootMode			_Mode;
	vector<TLootInfo>	_Items;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		if (_Mode == lm_item)
		{
			numEntry = (uint32)_Items.size();
			predef.resize(numEntry);
			for (uint i=0; i<numEntry; ++i)
			{
				predef[i].resize(i+1);
				for (uint j=0; j<i+1; ++j)
				{
					predef[i][j] = _Items[j].Item;
				}
			}
		}
		else if (_Mode == lm_mp)
		{
			numEntry = (uint32)_Items.size();
			predef.resize(numEntry);
			for (uint i=0; i<numEntry; ++i)
			{
				predef[i].resize(3*(i+1));
				for (uint j=0; j<i+1; ++j)
				{
					predef[i][j*3] = _Items[j].Item;
					predef[i][j*3+1] = _Items[j].Qt;
					predef[i][j*3+2] = _Items[j].Ql;
				}
			}
		}
		else
			numEntry = 0;

	}

public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		_Mode = lm_unknown;
		vector<string> vs;
		vs = md.getPropertyArray(prim, "item/quantity/quality", true, false);

		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> args;
			explode(vs[i], string(" "), args, true);
			if (args.size() != 1 && args.size() != 3)
			{
				string err = toString("Invalid loot item in '%s', need <item> or <item> <quantity> <quality>", vs[i].c_str());
				throw EParseException(prim, err.c_str());
			}

			if (args.size() == 1)
			{
				if (_Mode == lm_mp)
					throw EParseException(prim, "Can't mix item and mps loot");
				_Mode = lm_item;
			}
			else if (args.size() == 3)
			{
				if (_Mode == lm_item)
					throw EParseException(prim, "Can't mix item and mps loot");
				_Mode = lm_mp;
			}

			TLootInfo li;
			li.Item.initWithText(toString("i%u", i+1), STRING_MANAGER::item, md, prim, args[0]);

			if (args.size() > 1)
			{
				li.Qt.initWithText(toString("qt%u", i+1), STRING_MANAGER::integer, md, prim, args[1]);
				
				li.Ql.initWithText(toString("qual%u", i+1), STRING_MANAGER::integer, md, prim, args[2]);
				
			}
			_Items.push_back(li);
		}

		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		if (_Mode == lm_item)
			ret += "loot_item : ";
		else
			ret += "loot_mp : ";

		for (uint i=0; i<_Items.size(); ++i)
		{
			ret += _Items[i].Item;
			if (_Mode == lm_mp)
				ret += " "+_Items[i].Qt+" "+_Items[i].Ql;
			if (i < _Items.size()-1)
				ret += "; ";
		}
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;
		
		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentLoot, "loot");

struct TCraftInfo
{
	TCompilerVarName	Item;
	TCompilerVarName	Qt;
	TCompilerVarName	Ql;
};

class CContentCraft : public CContentObjective
{
	vector<TCraftInfo>	_Items;


	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = (uint32)_Items.size();
		predef.resize(numEntry);
		for (uint i=0; i<numEntry; ++i)
		{
			predef[i].resize(3*(i+1));
			for (uint j=0; j<i+1; ++j)
			{
				predef[i][j*3]   = _Items[j].Item;
				predef[i][j*3+1] = _Items[j].Qt;
				predef[i][j*3+2] = _Items[j].Ql;
			}
		}
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		vector<string> vs;
		vs = md.getPropertyArray(prim, "item/quantity/quality", true, false);

		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> args;
			explode(vs[i], string(" "), args, true);
			if (args.size() != 3)
			{
				string err = toString("Invalid craft item in '%s', need <item> <quantity> <quality>", vs[i].c_str());
				throw EParseException(prim, err.c_str());
			}

			TCraftInfo ci;			
			ci.Item.initWithText(toString("i%u", i+1), STRING_MANAGER::item, md, prim, args[0]);			
			ci.Qt.initWithText(toString("qt%u", i+1), STRING_MANAGER::integer, md, prim, args[1]);						
			ci.Ql.initWithText(toString("qual%u", i+1), STRING_MANAGER::integer, md, prim, args[2]);
			_Items.push_back(ci);
		}

		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "craft : ";

		for (uint i=0; i<_Items.size(); ++i)
		{
			ret += _Items[i].Item+" "+_Items[i].Qt+" "+_Items[i].Ql;
			if (i < _Items.size()-1)
				ret += "; ";
		}
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;
		
		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentCraft, "craft");

class CContentTarget : public CContentObjective
{
	
	vector<TCompilerVarName>	_Npcs;
	vector<TCompilerVarName>	_Faunas;
	vector<TCompilerVarName>	_Races;
	TCompilerVarName _Place;
	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		// phrase 0 - 1 parameter
		// phrase 1 - 2 parameters
		// etc ... as many (bots,faunas,race) as specified


		if (!_Npcs.empty())
		{
			numEntry = (uint32)_Npcs.size();
			predef.resize(numEntry);
			for (uint i=0; i<numEntry; ++i)
			{
				predef[i].resize(1*(i+1));
				for (uint j=0; j<i+1; ++j)
				{					
					predef[i][j] = _Npcs[j];
				}
			}
		}
		else if (!_Faunas.empty())
		{
			numEntry = (uint32)_Faunas.size();
			predef.resize(numEntry);
			for (uint i=0; i<numEntry; ++i)
			{
				predef[i].resize(1*(i+1));
				for (uint j=0; j<i+1; ++j)
				{					
					predef[i][j] = _Faunas[j];					
				}
			}
		}
		else if (!_Races.empty())
		{
			numEntry = (uint32)_Races.size();
			predef.resize(numEntry);
			for (uint i=0; i<numEntry; ++i)
			{
				predef[i].resize(1*(i+1));
				for (uint j=0; j<i+1; ++j)
				{					
					predef[i][j] = _Races[j];
				}
			}
		}

		// If there is a place add the param for text to all phrases (predef[x] is a phrase)
		if (!_Place.empty())
		{
			for (uint i= 0; i < predef.size(); ++i)
			{
				predef[i].push_back( _Place );
			}
		}
	}

public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		_Npcs = TCompilerVarName::getPropertyArrayWithText("npc", STRING_MANAGER::bot, md, prim, "npcs_to_target");			
		_Faunas = TCompilerVarName::getPropertyArrayWithText("fauna", STRING_MANAGER::creature_model, md, prim, "faunas_to_target" );						
		_Races = TCompilerVarName::getPropertyArrayWithText("race", STRING_MANAGER::race, md, prim, "races_to_target" );					
		_Place.init("place", STRING_MANAGER::place, md, prim, "place" );			
			
					
		if (!_Npcs.empty() && !_Faunas.empty())
			throw EParseException(prim, "Can't mix npc and fauna target");

		if (!_Npcs.empty() && !_Races.empty())
			throw EParseException(prim, "Can't mix npc and race target");

		if (!_Faunas.empty() && !_Races.empty())
			throw EParseException(prim, "Can't mix fauna and race target");
		
		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		if (!_Npcs.empty())
		{
			ret += "target_npc : ";
			for (uint i=0; i<_Npcs.size(); ++i)
			{
				ret += _Npcs[i];
				if (i < _Npcs.size()-1)
					ret += "; ";
			}
		}
		else if (!_Faunas.empty())
		{
			ret += "target_fauna : ";
			for (uint i=0; i<_Faunas.size(); ++i)
			{
				ret += _Faunas[i];
				if (i < _Faunas.size()-1)
					ret += "; ";
			}
		}
		else if (!_Races.empty())
		{
			ret += "target_race : ";
			for (uint i=0; i<_Races.size(); ++i)
			{
				ret += _Races[i];
				if (i < _Races.size()-1)
					ret += "; ";
			}
		}

		if (!_Place.empty())
			ret += " : " + _Place;
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentTarget, "target");

struct TSellInfo
{
	TCompilerVarName	Item;
	TCompilerVarName	Qt;
	TCompilerVarName	Ql;
};

class CContentSell : public CContentObjective
{
	vector<TSellInfo>	_Items;
	TCompilerVarName				_Npc;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = (uint32)_Items.size();
		predef.resize(numEntry);
		for (uint i=0; i<numEntry; ++i)
		{
			predef[i].resize(3*(i+1));
			for (uint j=0; j<i+1; ++j)
			{
				predef[i][j*3] = _Items[j].Item;
				predef[i][j*3+1] = _Items[j].Qt;
				predef[i][j*3+2] = _Items[j].Ql;
			}

			if (!_Npc.empty())
			{				
					predef[i].push_back(_Npc);				
			}
		}
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		vector<string> vs;
		vs = md.getPropertyArray(prim, "item/quantity/quality", true, false);

		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> args;
			explode(vs[i], string(" "), args, true);
			if (args.size() != 3)
			{
				string err = toString("Invalid sell item in '%s', need <item> <quantity> <quality>", vs[i].c_str());
				throw EParseException(prim, err.c_str());
			}

			TSellInfo si;
			si.Item.initWithText(toString("i%u", i+1), STRING_MANAGER::item, md, prim, args[0]);				
			si.Qt.initWithText(toString("qt%u", i+1), STRING_MANAGER::integer, md, prim, args[1]);				
			si.Ql.initWithText(toString("qual%u", i+1), STRING_MANAGER::integer, md, prim, args[2]);				
			_Items.push_back(si);
		}

		_Npc.init("npc", STRING_MANAGER::bot, md, prim, "npc_name");		
		
		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "sell : ";

		for (uint i=0; i<_Items.size(); ++i)
		{
			ret += _Items[i].Item+" "+_Items[i].Qt+" "+_Items[i].Ql;
			if (i < _Items.size()-1)
				ret += "; ";
		};
		if (!_Npc.empty())
		{
			ret += " : "+_Npc;
		}
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;
		
		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentSell, "sell");

struct TBuyInfo
{	
	TCompilerVarName	Item;
	TCompilerVarName	Qt;
	TCompilerVarName	Ql;
};

class CContentBuy : public CContentObjective
{
	vector<TBuyInfo>	_Items;
	TCompilerVarName				_Npc;


	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = (uint32)_Items.size();
		predef.resize(numEntry);
		for (uint i=0; i<numEntry; ++i)
		{
			predef[i].resize(3*(i+1));
			for (uint j=0; j<i+1; ++j)
			{
				predef[i][j*3]   = _Items[j].Item;
				predef[i][j*3+1] = _Items[j].Qt;
				predef[i][j*3+2] = _Items[j].Ql;
			}
			predef[i].push_back(_Npc);
		}	
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		vector<string> vs;
		vs = md.getPropertyArray(prim, "item/quantity/quality", true, false);

		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> args;
			explode(vs[i], string(" "), args, true);
			if (args.size() != 3)
			{
				string err = toString("Invalid buy item in '%s', need <item> <quantity> <quality>", vs[i].c_str());
				throw EParseException(prim, err.c_str());
			}

			TBuyInfo bi;			
			bi.Item.initWithText(toString("i%u", i+1), STRING_MANAGER::item, md, prim, args[0]);			
			bi.Qt.initWithText(toString("qt%u", i+1), STRING_MANAGER::integer, md, prim, args[1]);						
			bi.Ql.initWithText(toString("qual%u", i+1), STRING_MANAGER::integer, md, prim, args[2]);
			_Items.push_back(bi);
		}

		_Npc.init("npc", STRING_MANAGER::bot, md, prim, "npc_name");
			
		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "buy : ";

		for (uint i=0; i<_Items.size(); ++i)
		{
			ret += _Items[i].Item+" "+_Items[i].Qt+" "+_Items[i].Ql;
			if (i < _Items.size()-1)
				ret += "; ";
		};
		if (!_Npc.empty())
		{
			ret += " : "+_Npc;
		}
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;
		
		return ret;
	}
};

REGISTER_STEP_CONTENT(CContentBuy, "buy");

struct TGiveInfo
{
	TCompilerVarName	Item;
	TCompilerVarName	Qt;
	TCompilerVarName	Ql;
};

class CContentGive : public CContentObjective
{
	vector<TGiveInfo>	_Items;	
	TCompilerVarName	_Npc;
	bool				_QualSpec;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = (uint32)_Items.size();
		predef.resize(numEntry);
		for (uint i=0; i<numEntry; ++i)
		{
			if (_QualSpec)
			{
				predef[i].resize(3*(i+1));
				for (uint j=0; j<i+1; ++j)
				{
					predef[i][j*3]   = _Items[j].Item;
					predef[i][j*3+1] = _Items[j].Qt;
					predef[i][j*3+2] = _Items[j].Ql;
				}
			}
			else
			{
				predef[i].resize(2*(i+1));
				for (uint j=0; j<i+1; ++j)
				{
					predef[i][j*2]   = _Items[j].Item;
					predef[i][j*2+1] = _Items[j].Qt;				
				}
			}
			
			predef[i].push_back(_Npc);
			
		}
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		vector<string> vs;
		vs = md.getPropertyArray(prim, "item/quantity/quality", true, false);

		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> args;
			explode(vs[i], string(" "), args, true);
			if (args.size() != 3 && args.size() != 2)
			{
				string err = toString("Invalid give item in '%s', need <item> <quantity> [<quality>]", vs[i].c_str());
				throw EParseException(prim, err.c_str());
			}

			if (i == 0)
			{
				_QualSpec = args.size() == 3;
			}
			else
			{
				if (_QualSpec && args.size() == 2)
				{
					string err = toString("Invalid give item in '%s', mixing of item with quality and item without quality is not supported", vs[i].c_str());
					throw EParseException(prim, err.c_str());
				}
			}

			TGiveInfo gi;
			gi.Item.initWithText(toString("i%u", i+1), STRING_MANAGER::item, md, prim, args[0]);			
			gi.Qt.initWithText(toString("qt%u", i+1), STRING_MANAGER::integer, md, prim, args[1]);						

			if (_QualSpec)
				gi.Ql.initWithText(toString("qual%u", i+1), STRING_MANAGER::integer, md, prim, args[2]);

			_Items.push_back(gi);
		}

		_Npc.init("npc", STRING_MANAGER::bot, md, prim, "npc_name");

		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "give_item : ";

		for (uint i=0; i<_Items.size(); ++i)
		{
			ret += _Items[i].Item+" "+_Items[i].Qt;
			if (_QualSpec)
				ret += " "+_Items[i].Ql;
			if (i < _Items.size()-1)
				ret += "; ";
		};
		ret += " : "+_Npc;
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;
		
		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentGive, "give_item");

class CContentGiveMoney : public CContentObjective
{
	TCompilerVarName _Amount;
	TCompilerVarName _Npc;	

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
		predef.resize(1);
		predef[0].resize(2);

		predef[0][0] = _Amount;
		predef[0][1] = _Npc;
		
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		_Amount.init("amount", STRING_MANAGER::integer, md, prim, "amount");
		_Npc.init("npc", STRING_MANAGER::bot, md, prim, "npc_name");
		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "give_money : "+_Amount+" : "+_Npc;
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentGiveMoney, "give_money");

class CContentVisit : public CContentObjective
{
	TCompilerVarName			_Place;
	TCompilerVarName			_PlaceVar;
	vector<TCompilerVarName>	_Items;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
		predef.resize(1);
		predef[0].resize(1+_Items.size());
		predef[0][0] = _Place;
		
		for (uint i=0; i<_Items.size(); ++i)
			predef[0][i+1] = _Items[i];
		
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		
		_Place.init("place", STRING_MANAGER::place, md, prim, "place");
		_Items = TCompilerVarName::getPropertyArrayWithText("i", STRING_MANAGER::item, md, prim, "items_worn" );			 
		
		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "visit : "+_Place;

		if (!_Items.empty())
		{
			ret += " : ";
			for (uint i=0; i<_Items.size(); ++i)
			{
				ret += _Items[i];
				if (i < _Items.size()-1)
					ret += "; ";
			}
		}
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentVisit, "visit");

class CContentEscort : public CContentObjective
{
	string			_GroupName;
	bool			_SaveAll;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init (CMissionData &md, IPrimitive *prim)
	{
		_GroupName = md.getProperty(prim, "group_to_escort", true, false);
		string s = md.getProperty(prim, "save_all", true, false);
		_SaveAll = (NLMISC::toLower(s) == "true");

		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "escort : "+_GroupName;

		if (_SaveAll)
			ret += " : save_all";
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentEscort, "escort");

struct TSkillInfo
{
	TCompilerVarName	_SkillName;
	TCompilerVarName	_Level;
};

class CContentSkill: public CContentObjective
{
	vector<TSkillInfo>	_Skills;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = (uint32)_Skills.size();
		predef.resize(numEntry);

		for (uint i=0; i<numEntry; ++i)
		{
			predef[i].resize(2*(i+1));
			for (uint j=0; j<i+1; ++j)
			{
				predef[i][j*2]   = _Skills[j]._SkillName;
				predef[i][j*2+1] = _Skills[j]._Level;

			}
		}
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		vector<string>	vs;
		vs = md.getPropertyArray(prim, "skill_name/level", true, false);

		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> args;
			explode(vs[i], string(" "), args, true);

			if (args.size() != 2)
			{
				string err = toString("Invalid skill in '%s', need <skill_name> <skill_level>", vs[i].c_str());
				throw EParseException(prim, err.c_str());
			}

			TSkillInfo si;
			si._SkillName.initWithText(toString("s%u", i+1), STRING_MANAGER::skill, md, prim, args[0]);			
			si._Level.initWithText(toString("level%u", i+1), STRING_MANAGER::integer, md, prim, args[1]);	
			_Skills.push_back(si);
		}

		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "skill : ";

		for (uint i=0; i<_Skills.size(); ++i)
		{
			ret += _Skills[i]._SkillName+" "+_Skills[i]._Level;
			if (i < _Skills.size()-1)
				ret += "; ";
		}
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentSkill, "skill");

class CContentMission: public CContentObjective
{
	vector<string>	_Missions;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}

public:
	CContentMission(): _Prim(0) {}

	void init(CMissionData &md, IPrimitive *prim)
	{
		_Missions = md.getPropertyArray(prim, "mission_names", true, false);
		_Prim = prim;

		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "mission : ";

		for (uint i=0; i<_Missions.size(); ++i)
		{
			ret += _Missions[i];
			if (i < _Missions.size()-1)
				ret += "; ";

			// We check to see if we specified a number after the mission name. If so, we check if it's a guild mission
			std::size_t pos = _Missions[i].find_first_of(" \t");
			if (pos != std::string::npos && !md.isGuildMission())
			{
				string err = toString("primitive(%s): CContentMission: Number of members needed to complete the mission specified but the mission is not a guild mission.", _Prim->getName().c_str());
				throw EParseException(_Prim, err.c_str());
			}
		}
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;

		return ret;
	}

	IPrimitive *_Prim;
};
REGISTER_STEP_CONTENT(CContentMission, "do_mission");

class CContentWaitAIMsg : public CContentObjective
{
	vector<string>	_MsgContent;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}

public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		_MsgContent = md.getPropertyArray(prim, "msg_content", true, false);

		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "wait_msg : ";

		for (uint i=0; i<_MsgContent.size(); ++i)
		{
			ret += _MsgContent[i];
			if (i < _MsgContent.size()-1)
				ret += " ";
		}
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentWaitAIMsg, "wait_ai_msg");

// ***********
// queue_start
// ***********

class CContentQueueStart : public CContentObjective
{
	string			_StepName;
	string			_QueueName;
	string			_Timer;
	vector<string>	_AIInstances;
	vector<string>	_GroupsToSpawn;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
	
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		_StepName = md.getProperty(prim, "step_name", true, true);
		_QueueName = md.getProperty(prim, "queue_name", true, false);
		if (_StepName.empty())
			_StepName = "queue_start_" + _QueueName;

		_Timer = md.getProperty(prim, "timer", true, false);
		_GroupsToSpawn = md.getPropertyArray(prim, "groups_to_spawn", true, true);
		if (_GroupsToSpawn.empty())
			_AIInstances = md.getPropertyArray(prim, "ai_instances", true, true);
		else
			_AIInstances = md.getPropertyArray(prim, "ai_instances", true, false);

		CContentObjective::init(md, prim);
	}
	
	string genCode(CMissionData &md)
	{
		uint32 i;
		string ret;
		ret = CContentObjective::genCode(md);
		
		ret += "jump_point : " + _StepName + NL;
		ret += "queue_start : " + _QueueName + " : " + _Timer + NL;
		ret += NL;
		
		if (!_GroupsToSpawn.empty())
		{
			// crash : ai_instance_1 : ai_instance_2 : ...
			ret += "crash";
			nlassert(_AIInstances.size() > 0);
			for (i = 0; i < _AIInstances.size(); ++i)
				ret += " : " + _AIInstances[i];
			ret += NL;
			
			for (i = 0; i < _GroupsToSpawn.size(); ++i)
				ret += "handle_release : " + _GroupsToSpawn[i] + NL;
			
			ret += "/crash" + NL;
			ret += NL;
			
			ret += "failure" + NL;
			for (i = 0; i < _GroupsToSpawn.size(); ++i)
				ret += "handle_release : " + _GroupsToSpawn[i] + NL;
			ret += "/failure" + NL;
			ret += NL;		
		}

		ret += "player_reconnect" + NL;
		for (i = 0; i < _GroupsToSpawn.size(); ++i)
			ret += "handle_release : " + _GroupsToSpawn[i] + NL;
		ret += "jump : " + _StepName + NL;
		ret += "/player_reconnect" + NL;
		ret += NL;
		
		for (i = 0; i < _GroupsToSpawn.size(); ++i)
			ret += "handle_create : " + _GroupsToSpawn[i] + NL;
		
		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentQueueStart, "queue_start");

// *********
// queue_end
// *********

class CActionQueueEnd : public IStepContent
{
	string			_ActionName;
	string			_QueueName;
	// Find the following info in the queue_start node
	vector<string>	_AIInstances;
	vector<string>	_GroupsToSpawn;
	
	void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}
public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		IStepContent::init(md, prim);
		_ActionName = md.getProperty(prim, "action_name", true, true);
		_QueueName = md.getProperty(prim, "queue_name", true, false);
		if (_ActionName.empty())
			_ActionName = "queue_end_" + _QueueName;

		// Search in the queue_start node
		IPrimitive *primParent = prim;
		while (primParent->getParent() != NULL)
		{
			string className;
			if (primParent->getPropertyByName("class", className))
				if (className == "mission_tree")
					break;
			primParent = primParent->getParent();
		}
		TPrimitiveClassPredicate condClass("queue_start");
		CPrimitiveSet<TPrimitiveClassPredicate> s;
		TPrimitiveSet ps;
		s.buildSet(primParent, condClass, ps);

		TPrimitivePropertyPredicate condProp("queue_name", _QueueName);
		CPrimitiveSetFilter<TPrimitivePropertyPredicate> filter;
		TPrimitiveSet psFinal;
		filter.filterSet(ps, condProp, psFinal);

		if (psFinal.size() == 0)
		{
			string err = toString("Can't find queue_start for queue_end : '%s'", _QueueName.c_str());
			throw EParseException(prim, err.c_str());
		}
		if (psFinal.size() > 1)
		{
			string err = toString("More than 1 queue_start for queue_end : '%s'", _QueueName.c_str());
			throw EParseException(prim, err.c_str());
		}
		
		_GroupsToSpawn = md.getPropertyArray(psFinal[0], "groups_to_spawn", true, true);
		if (_GroupsToSpawn.empty())
			_AIInstances = md.getPropertyArray(psFinal[0], "ai_instances", true, true);
		else
			_AIInstances = md.getPropertyArray(psFinal[0], "ai_instances", true, false);
		
	}
	
	string genCode(CMissionData &md)
	{
		uint32 i;
		string ret;

		if (_GroupsToSpawn.size() > 0)
		{
			for (i = 0; i < _GroupsToSpawn.size(); ++i)
				ret += "handle_release : " + _GroupsToSpawn[i] + NL;
			ret += NL;
		}
		
		if (_AIInstances.size() > 0)
		{
			ret += "crash";
			for (i = 0; i < _AIInstances.size(); ++i)
				ret += " : " + _AIInstances[i];
			ret += NL;
			ret += "/crash" + NL;
			ret += NL;
		}

		ret += "failure" + NL;
		ret += "/failure" + NL;
		ret += NL;

		ret += "player_reconnect" + NL;
		ret += "/player_reconnect" + NL;
		ret += NL;
		ret += "queue_end : " +_QueueName + NL;
		
		return ret;
	}
	
};
REGISTER_STEP_CONTENT(CActionQueueEnd, "queue_end");


class CContentRingScenario: public CContentObjective
{
	string		_ScenarioTag;

	virtual void getPredefParam(uint32 &numEntry, CPhrase::TPredefParams &predef)
	{
		numEntry = 0;
	}

public:
	void init(CMissionData &md, IPrimitive *prim)
	{
		_ScenarioTag = md.getProperty(prim, "scenario_tag", true, false);

		CContentObjective::init(md, prim);
	}

	string genCode(CMissionData &md)
	{
		string ret;
		ret = CContentObjective::genCode(md);

		ret += "ring_scenario : ";
		ret += _ScenarioTag;
		// Add the 'nb_guild_members_needed' parameter if needed
		//ret += CContentObjective::genNbGuildMembersNeededOption(md);
		ret += NL;

		return ret;
	}
};
REGISTER_STEP_CONTENT(CContentRingScenario, "ring_scenario");

