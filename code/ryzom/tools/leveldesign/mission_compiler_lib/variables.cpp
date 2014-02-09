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

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;


class IVarFactory
{
public:
	virtual IVar *createVar(CMissionData &md, IPrimitive *prim) = 0;

};

template <class VarClass>
class CVarFactory : public IVarFactory
{
	IVar *createVar(CMissionData &md, IPrimitive *prim)
	{
		return new VarClass(md, prim);
	}
};

#define REGISTER_VAR_INDIRECT(varClass, key)	typedef CVarFactory<varClass> TVarFactory##varClass; \
												NLMISC_REGISTER_OBJECT_INDIRECT(IVarFactory, TVarFactory##varClass, string, string(key));


//#define REGISTER_VARIABLE(className, varName) NLMISC_REGISTER_OBJECT(IVar, className, std::string, string(varName));

/* Class for npc variable */
class CVarNpc : public IVar
{
public:
	CVarNpc(CMissionData &md, IPrimitive *prim)
		: IVar(vt_npc, prim)
	{
		_NpcLabel = getPrimProperty(prim, "npc_name");
		_NpcFunction = getPrimProperty(prim, "npc_function");
		if (!_NpcFunction.empty())
		{
			_NpcFunction = "$"+_NpcFunction+"$";
		}

		IVar *nameVar = CFactoryIndirect<IVarFactory, string>::instance().getFactory("var_npc_name")->createVar(md, prim);
		md.addVariable(prim, nameVar);
	}
	
	const std::string getNpcLabel()
	{
		return _NpcLabel;
	}
	const std::string getNpcFunction()
	{
		return _NpcFunction;
	}
	const std::string getNpcFullName()
	{
		return _NpcLabel+_NpcFunction;
	}
	
	string evalVar(const string &subPart)
	{
		if (subPart == "fullname")
			return getNpcFullName();
		else if (subPart == "function")
			return _NpcFunction;
		else if (subPart == "")
			return _NpcLabel;

		throw EParseException(NULL, toString("var_npc don't have a subpart '%s'", subPart.c_str()).c_str());
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::bot;
	}

	string genDecl(CMissionData &md)
	{
		return "decl : bot : "+evalVar("")+NL;
	}
private:
	string		_NpcLabel;
	string		_NpcFunction;
};

REGISTER_VAR_INDIRECT(CVarNpc, "var_npc");

/** Var for npc name (aka bot_name) 
 *	This class is implicitly instancied by
 *	CVarNpc.
 */
class CVarNpcName : public IVar
{
public:
	CVarNpcName(CMissionData &md, IPrimitive *prim)
		: IVar(vt_npc, prim)
	{
		// Change the var name
		_VarName += "_name";
		_NpcLabel = getPrimProperty(prim, "npc_name");
		_NpcFunction = getPrimProperty(prim, "npc_function");
		if (!_NpcFunction.empty())
		{
			_NpcFunction = "$"+_NpcFunction+"$";
		}
	}
	
	const std::string getNpcLabel()
	{
		return _NpcLabel;
	}
	const std::string getNpcFunction()
	{
		return _NpcFunction;
	}
	const std::string getNpcFullName()
	{
		return _NpcLabel+_NpcFunction;
	}
	
	string evalVar(const string &subPart)
	{
		if (subPart == "")
			return string("\"")+_NpcLabel+"\"";

		throw EParseException(NULL, toString("var_npc_name don't have a subpart '%s'", subPart.c_str()).c_str());
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::bot_name;
	}

	string genDecl(CMissionData &md)
	{
		return string();
	}

private:
	string		_NpcLabel;
	string		_NpcFunction;
};
REGISTER_VAR_INDIRECT(CVarNpcName, "var_npc_name");

/* Class for npc variable */
class CVarGroup : public IVar
{
public:
	CVarGroup(CMissionData &md, IPrimitive *prim)
		: IVar(vt_npc, prim)
	{
		_GroupName = getPrimProperty(prim, "group_name");
	}
	
	string evalVar(const string &subPart)
	{
		if (subPart.empty())
			return _GroupName;
		else if (subPart == "quoted")
			return string("\"")+_GroupName+"\"";
		else
			nlassert(false);
		return "";
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::bot_name;
	}

	string genDecl(CMissionData &md)
	{
//		return "decl : bot : "+evalVar("no_quote")+NL;
		return "decl : bot : "+evalVar("")+NL;
	}
private:
	string		_GroupName;
};

REGISTER_VAR_INDIRECT(CVarGroup, "var_group");
//NLMISC_REGISTER_OBJECT(IVar, CVarGroup, std::string, string("var_group"));


/* Class for item */
class CVarItem : public IVar
{
public:
	CVarItem(CMissionData &md, IPrimitive *prim)
		: IVar(vt_item, prim)
	{
		_ItemSheet = getPrimProperty(prim, "item_sheet");
	}
	
	const std::string getItemSheet()
	{
		return _ItemSheet;
	}
	
	string evalVar(const string &subPart)
	{
		nlassert(subPart.empty());
		return _ItemSheet;
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::item;
	}

	string genDecl(CMissionData &md)
	{
		return "decl : item : "+evalVar("")+NL;
	}
private:
	string		_ItemSheet;
};

REGISTER_VAR_INDIRECT(CVarItem, "var_item");
//NLMISC_REGISTER_OBJECT(IVar, CVarItem, std::string, string("var_item"));


/* Class for race */
class CVarRace : public IVar
{
public:
	CVarRace(CMissionData &md, IPrimitive *prim)
		: IVar(vt_item, prim)
	{
		_Race = getPrimProperty(prim, "race");
	}
	
	string evalVar(const string &subPart)
	{
		nlassert(subPart.empty());
		return _Race;
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::race;
	}

	string genDecl(CMissionData &md)
	{
		return "decl : race : "+evalVar("")+NL;
	}
private:
	string		_Race;
};

REGISTER_VAR_INDIRECT(CVarRace, "var_race");

/* Class for sphrase */
class CVarSPhrase : public IVar
{
public:
	CVarSPhrase(CMissionData &md, IPrimitive *prim)
		: IVar(vt_item, prim)
	{
		_SPhrase = getPrimProperty(prim, "sphrase_sheet");
	}
	
	string evalVar(const string &subPart)
	{
		nlassert(subPart.empty());
		return _SPhrase;
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::sphrase;
	}

	string genDecl(CMissionData &md)
	{
		return "decl : sphrase : "+evalVar("")+NL;
	}
private:
	string		_SPhrase;
};

REGISTER_VAR_INDIRECT(CVarSPhrase, "var_sphrase");

/* Class for sbrick */
class CVarSBrick : public IVar
{
public:
	CVarSBrick(CMissionData &md, IPrimitive *prim)
		: IVar(vt_item, prim)
	{
		_SBrick = getPrimProperty(prim, "sbrick_sheet");
	}
	
	string evalVar(const string &subPart)
	{
		nlassert(subPart.empty());
		return _SBrick;
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::sbrick;
	}

	string genDecl(CMissionData &md)
	{
		return "decl : sbrick : "+evalVar("")+NL;
	}
private:
	string		_SBrick;
};

REGISTER_VAR_INDIRECT(CVarSBrick, "var_sbrick");


/* for special item */
const char	*SpecialItemProp[] =
{
	"Durability",
	"Weight",
	"SapLoad",
	"Dmg",
	"Speed",
	"Range",
	"DodgeModifier",
	"ParryModifier",
	"AdversaryDodgeModifier",
	"AdversaryParryModifier",
	"ProtectionFactor",
	"MaxSlashingProtection",
	"MaxBluntProtection",
	"MaxPiercingProtection",
	"HpBuff",
	"SapBuff",
	"StaBuff",
	"FocusBuff"
};

struct TItemProperty
{
	string	PropName;
	string	PropValue;
};

/* Class for special item */
class CVarSpecialItem : public IVar
{
public:
	CVarSpecialItem(CMissionData &md, IPrimitive *prim)
		: IVar(vt_item, prim)
	{
		static bool init = false;
		static set<string> propertyNames;

		if (!init)
		{
			for (uint i=0; i<sizeof(SpecialItemProp)/sizeof(char*); ++i)
				propertyNames.insert(SpecialItemProp[i]);

			init = true;
		}

		_ItemSheet = getPrimProperty(prim, "item_sheet");
		_ReqSkill = getPrimProperty(prim, "req_skill_level");
		vector<string>	vs;
		vs = getPrimPropertyArray(prim, "properties/values");
		// parse the strings vector
		for (uint i=0; i<vs.size(); ++i)
		{
			vector<string> parts;
			explode(vs[i], string(" "), parts, true);
			if (!parts.empty() && parts.size() != 2)
			{
				string s = toString("Invalid special item property at line %u", i+1);
				throw EParseException(prim, s.c_str());
			}

			if (parts.size() == 2)
			{
				TItemProperty ip;
				ip.PropName = parts[0];
				ip.PropValue = parts[1];

				if (propertyNames.find(ip.PropName) == propertyNames.end())
				{
					string s = toString("Invalid property name '%s'", ip.PropName.c_str());
					throw EParseException(prim, s.c_str());
				}

				_Properties.push_back(ip);
			}
		}
		_Action = getPrimProperty(prim, "item_action");
		vs.clear();
		vs = getPrimPropertyArray(prim, "phrase_item_name");
		_ItemPhrase.initPhrase(md, prim, vs);

		string s;
		s = getPrimProperty(prim, "no_drop");
		_NoDrop = (s == "true");
	}
	
//	const std::string getItemSheet()
//	{
//		return _ItemSheet;
//	}
	
	string evalVar(const string &subPart)
	{
		nlassert(subPart.empty());
		return _VarName;
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::item;
	}

	string genDecl(CMissionData &md)
	{
		string ret = string("decl_item : ")+_VarName+" : "+_ItemSheet+" : "+_ReqSkill;
		if (!_Properties.empty() ||!_Action.empty())
			ret += " : ";
		for (uint i=0; i<_Properties.size(); ++i)
		{
			TItemProperty &ip = _Properties[i];
			ret += ip.PropName+" "+ip.PropValue;
			if (i < _Properties.size()-1 || !_Action.empty())
				ret += "; ";
		}

		if (!_Action.empty())
			ret += _Action;

		ret += " : "+_ItemPhrase.genScript(md);

		if (_NoDrop)
			ret += " : no_drop";
	
		ret += NL;

		return ret;
	}

	std::string genPhrase()
	{
		return _ItemPhrase.genPhrase();
	}
		

private:
	/// the item sheet used as base for this special item
	string					_ItemSheet;
	/// The skill required to use the item
	string					_ReqSkill;
	/// The list of properties
	vector<TItemProperty>	_Properties;
	// Optional action (enchantement)
	string					_Action;
	// Name of the item
	CPhrase					_ItemPhrase;
	// No drop flag
	bool					_NoDrop;

	string		_Color;

};

REGISTER_VAR_INDIRECT(CVarSpecialItem, "var_special_item");
//NLMISC_REGISTER_OBJECT(IVar, CVarSpecialItem, std::string, string("var_special_item"));

/* Class for place variable */
class CVarPlace : public IVar
{
public:
	CVarPlace(CMissionData &md, IPrimitive *prim)
		: IVar(vt_npc, prim)
	{
		_PlaceLabel = getPrimProperty(prim, "place_name");
	}
	
/*	const std::string getPlaceLabel()
	{
		return _PlaceLabel;
	}
*/	
	string evalVar(const string &subPart)
	{
		nlassert(subPart.empty());
		return _PlaceLabel;
	}
	
	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::place;
	}

	string genDecl(CMissionData &md)
	{
		return "decl : place : "+evalVar("")+NL;
	}

private:
	string		_PlaceLabel;
};
REGISTER_VAR_INDIRECT(CVarPlace, "var_place");
//NLMISC_REGISTER_OBJECT(IVar, CVarPlace, std::string, string("var_place"));

/* Class for integer variable */
class CVarInteger : public IVar
{
public:
	CVarInteger(CMissionData &md, IPrimitive *prim)
		: IVar(vt_integer, prim)
	{
		if (prim->checkProperty("value"))
			_Value = getPrimProperty(prim, "value");
		else if (prim->checkProperty("quantity"))
			_Value = getPrimProperty(prim, "quantity");
		else if (prim->checkProperty("quality"))
			_Value = getPrimProperty(prim, "quality");
		else
		{
			string err = toString("Can't find a valid property for integer variable");
			throw EParseException(prim, err.c_str());
		}
	}
	
	const std::string getIntegerValue()
	{
		return _Value;
	}
	
	string evalVar(const string &subPart)
	{
		nlassert(subPart.empty());
		return _Value;;
	}
	
	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::integer;
	}

	string genDecl(CMissionData &md)
	{
		// nothing to declare for this pseudo var
		return string();
	}
private:
	string		_Value;
};
REGISTER_VAR_INDIRECT(CVarInteger, "var_integer");
//NLMISC_REGISTER_OBJECT(IVar, CVarInteger, std::string, string("var_integer"));
typedef CVarInteger CVarQuantity;
REGISTER_VAR_INDIRECT(CVarQuantity, "var_quantity");
//NLMISC_REGISTER_OBJECT(IVar, CVarQuantity, std::string, string("var_quantity"));
typedef CVarInteger CVarQuality;
REGISTER_VAR_INDIRECT(CVarQuality, "var_quality");
//NLMISC_REGISTER_OBJECT(IVar, CVarQuality, std::string, string("var_quality"));


/* Class for text var */
class CVarText : public IVar
{
public:
	CVarText(CMissionData &md, IPrimitive *prim)
		: IVar(vt_item, prim)
	{
		_TextValue = getPrimPropertyArray(prim, "text");
	}
	
	const vector<std::string> &getText()
	{
		return _TextValue;;
	}
	
	string evalVar(const string &subPart)
	{
		nlassert(subPart.empty());
		string t;
		return std::accumulate(_TextValue.begin(), _TextValue.end(), string(""));
		return t;
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
		nlassert(false);
		return STRING_MANAGER::NB_PARAM_TYPES;
	}

	string genDecl(CMissionData &md)
	{
		// nothing to declare for this one
		return string();
	}
private:
	vector<string>		_TextValue;
};
REGISTER_VAR_INDIRECT(CVarText, "var_text");
//NLMISC_REGISTER_OBJECT(IVar, CVarText, std::string, string("var_text"));

/* Class for creature var */
class CVarCreature : public IVar
{
public:
	CVarCreature(CMissionData &md, IPrimitive *prim)
		: IVar(vt_item, prim)
	{
		_CreatureSheet = getPrimProperty(prim, "creature_sheet");
	}
	
	string evalVar(const string &subPart)
	{
		nlassert(subPart.empty());
		return _CreatureSheet;
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
//		return STRING_MANAGER::creature;
		return STRING_MANAGER::creature_model;
	}

	string genDecl(CMissionData &md)
	{
		// declare a creature sheet
//		return "decl : creature : "+_CreatureSheet+NL;
		return "decl : creature_model : "+_CreatureSheet+NL;
	}
private:
	string				_CreatureSheet;
};
REGISTER_VAR_INDIRECT(CVarCreature, "var_creature");
//NLMISC_REGISTER_OBJECT(IVar, CVarCreature, std::string, string("var_creature"));

/* Class for faction var */
class CVarFaction : public IVar
{
public:
	CVarFaction(CMissionData &md, IPrimitive *prim)
		: IVar(vt_item, prim)
	{
		_FactionName = getPrimProperty(prim, "faction_name");
	}
	
	string evalVar(const string &subPart)
	{
		nlassert(subPart.empty());
		return _FactionName;
	}

	STRING_MANAGER::TParamType getStringManagerType()
	{
		return STRING_MANAGER::faction;
	}

	string genDecl(CMissionData &md)
	{
		// declare a creature sheet
		return "decl : faction : "+_FactionName+NL;
	}
private:
	string				_FactionName;
};
REGISTER_VAR_INDIRECT(CVarFaction, "var_faction");
//NLMISC_REGISTER_OBJECT(IVar, CVarFaction, std::string, string("var_faction"));

// Variable factory.
IVar *IVar::createVar(CMissionData &md, IPrimitive *prim)
{
	string *c;
	if (!prim->getPropertyByName("class", c))
		throw EParseException(prim, "Can't find property 'class' on primitive");

	return CFactoryIndirect<IVarFactory, string>::instance().getFactory(*c)->createVar(md, prim);

	return NULL;
};
//IVar *IVar::createVar(CMissionData &md, IPrimitive *prim)
//{
//	string *className;
//	if (!prim->getPropertyByName("class", className))
//		throw EParseException(prim, "Can't find property 'class' in primitive");
//
//	IVar *ret = NLMISC_GET_FACTORY_INDIRECT(IVar, std::string).createObject(md, *className, prim);
//
//	return ret;
//}
//
