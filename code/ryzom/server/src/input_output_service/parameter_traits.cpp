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
#include "nel/misc/variable.h"
#include "nel/misc/path.h"
#include "string_manager.h"
#include "input_output_service.h"
#include "game_share/skills.h"
#include "game_share/body.h"
#include "game_share/scores.h"
#include "game_share/characteristics.h"
#include "game_share/damage_types.h"
#include "game_share/power_types.h"
#include "game_share/ecosystem.h"
#include "game_share/people.h"
#include "game_share/roles.h"
#include "game_share/fame.h"
#include "game_share/ryzom_mirror_properties.h"


using namespace STRING_MANAGER;
using namespace NLMISC;
using namespace NLNET;
using namespace std;


extern CVariable<bool> VerboseStringManager;
#define LOG if (!VerboseStringManager) {} else nlinfo


const char *OperatorNames[] =
{
		"equal",
		"notEqual",
		"greater",
		"greaterEqual",
		"less",
		"lessEqual",
		"nop"
};


// --------------- ParameterTraits class -------------------

void CStringManager::CParameterTraits::fillBitMemStream( const CCharacterInfos *charInfo,TLanguages language, const TReplacement &rep, NLMISC::CBitMemStream &bms)
{
	const CStringManager::CEntityWords &ew = SM->getEntityWords(language, ParamId.Type);
	std::string rowName = NLMISC::strlwr(getParameterId());
	uint32 stringId;
	stringId = ew.getStringId(rowName, rep.Format);
	bms.serial(stringId);
}

bool CStringManager::CParameterTraits::eval(CStringManager::TLanguages lang,const CCharacterInfos *charInfo, const TCondition &cond) const
{
	// Default eval only check in words column as property
	const CStringManager::CEntityWords &ew = SM->getEntityWords(lang, ParamId.Type);
	uint32 colIndex = ew.getColumnId(cond.Property);
	if (colIndex == 0xffffffff)
	{
		nlwarning("The column %s is unknown in words file for %s", cond.Property.c_str(), getParameterId().c_str());
		return false;
	}

	// ok, this column exist, retrieve the row
	uint32 rowIndex = ew.getRowId(NLMISC::strlwr(getParameterId()));
	if (rowIndex == 0xffffffff)
	{
		nlwarning("The entry in table for %s is unknown in words.", getParameterId().c_str());
		return false;
	}
	
	uint32 stringId = ew.getStringId(rowIndex, colIndex);
	const std::string &str = SM->getString(stringId).toString();
	NLMISC::strlwr(str);

	LOG("SM : (paramTraits) eval condition for property %s [%s] %s [%s]", cond.Property.c_str(), str.c_str(), OperatorNames[cond.Operator], cond.ReferenceStr.c_str());

	switch(cond.Operator)
	{
	case equal:
		return str == cond.ReferenceStr;
	case notEqual:
		return str != cond.ReferenceStr;
	case greater:
		return str > cond.ReferenceStr;
	case greaterEqual:
		return str >= cond.ReferenceStr;
	case less:
		return str < cond.ReferenceStr;
	case lessEqual:
		return str <= cond.ReferenceStr;
	default:
		nlwarning("Operator %s is not applicable to property %s", OperatorNames[cond.Operator], cond.Property.c_str());
		return false;
	}
}


class CParameterTraitsEntity : public CStringManager::CParameterTraits
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsEntity(STRING_MANAGER::entity, "entity");
	}

	CParameterTraitsEntity(STRING_MANAGER::TParamType type, const std::string &typeName)
		: CParameterTraits(type, typeName)
	{}
	bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		TParam	param;
		if (param.serialParam(debug, message, ParamId.Type))
		{
			EId = param.getEId();
			_AIAlias = param.getAIAlias();
			return true;
		}
		else
		{
			EId = CEntityId::Unknown;
			_AIAlias = 0;
			return false;
		}
	}
	const std::string &getParameterId() const
	{
		static std::string empty;
		// special case for fauna entity 
		if (EId.getType() == RYZOMID::creature || EId.getType() == RYZOMID::npc)
		{
			const NLMISC::CSheetId &sheetId = SM->getSheetId(EId);
			if (sheetId == NLMISC::CSheetId::Unknown)
			{
				nlwarning("The sheet id for creature entity %s is unknown !", EId.toString().c_str());
				return empty;
			}

			const CStringManager::TSheetInfo &si = SM->getSheetInfo(sheetId);
			return si.SheetName;
		}

		// TODO : We need to found the name of the entity.
		return empty;
	}
	// just a wrapper to base class
	bool eval(CStringManager::TLanguages lang,const CCharacterInfos *charInfo, const CStringManager::TCondition &cond) const
	{
		// Test unknown entity
		if ((cond.ReferenceInt == 0) && (cond.Property.empty()))
		{
			if (cond.Operator == CStringManager::equal)
				return EId.isUnknownId();
			else if (cond.Operator == CStringManager::notEqual)
				return !EId.isUnknownId();
		}

		// test on race
		const NLMISC::CSheetId &sheetId = SM->getSheetId(EId);
		if (sheetId == NLMISC::CSheetId::Unknown)
		{
			nlwarning("The sheet id for entity entity %s is unknown !", EId.toString().c_str());
			return false;
		}
		const CStringManager::TSheetInfo &si = SM->getSheetInfo(sheetId);
		std::string op1;
			
		if (cond.Property == "race")
		{
			op1 = si.Race;
		}
		else if (cond.Property == "gender")
		{
			CCharacterInfos	*playerInfo = IOS->getCharInfos(EId);
			if (playerInfo != 0)
			{
				op1 = GSGENDER::toString(playerInfo->getGender());
			}
		}
		else
		{
			return CParameterTraits::eval(lang, charInfo, cond);
		}

		NLMISC::strlwr(op1);
		LOG("SM : (entity) eval condition for property %s [%s] %s [%s]", cond.Property.c_str(), op1.c_str(), OperatorNames[cond.Operator], cond.ReferenceStr.c_str());

		switch(cond.Operator)
		{
		case CStringManager::equal:
			return op1 == cond.ReferenceStr;
		case CStringManager::notEqual:
			return op1 != cond.ReferenceStr;
		case CStringManager::less:
			return op1 < cond.ReferenceStr;
		case CStringManager::lessEqual:
			return op1 <= cond.ReferenceStr;
		case CStringManager::greater:
			return op1 > cond.ReferenceStr;
		case CStringManager::greaterEqual:
			return op1 >= cond.ReferenceStr;
		default:
			nlwarning("Operator %s not applicable to creature property %s", OperatorNames[cond.Operator], cond.Property.c_str());
			return false;
		}
		nlerror("This point of code can never be reach !");
		return CParameterTraits::eval(lang, charInfo, cond);
	}

	void fillBitMemStream( const CCharacterInfos *charInfo,CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		if ( EId == CEntityId::Unknown && _AIAlias != 0)
		{							
			CAIAliasManager& aiAliasMgr = IOS->getAIAliasManager();
			if (aiAliasMgr.is(_AIAlias)) 
			{

				static const string NAME("name");
				CSString format = rep.Format;
				if (format.left(4) != NAME)
				{
					// can't replace this parameter, write a del char
					const static string s("\010");
					uint32 index = SM->storeString(s);
					bms.serial(index);
					return;
				}
				else
				{
					uint32 index ;
					index = aiAliasMgr.getShortNameIndex(_AIAlias);
					if (index)
					{
						bms.serial(index);
						return;
					}
					index = aiAliasMgr.getTitleIndex(_AIAlias, language);
					if (index)
					{
						bms.serial(index);
						return;
					}
				}
				// No translated title or translated phrase found send ''
				//		ucstring temp(EId.toString());
				const ucstring NoName("''");
				uint32 index = SM->storeString(NoName);
				bms.serial(index);
				return;
			}
		}

		if (EId.getType() == RYZOMID::player || EId.getType() == RYZOMID::npc)
		{	
			CCharacterInfos	*playerInfo = IOS->getCharInfos(EId);
			if (playerInfo != 0)
			{
				static const string NAME("name");
				CSString format = rep.Format;
				if (format.left(4) != NAME)
				{
					// can't replace this parameter, write a del char
					const static string s("\010");
					uint32 index = SM->storeString(s);
					bms.serial(index);
					return;
				}
				else
				{
					if (!playerInfo->ShortName.empty())
					{
						uint32 index = playerInfo->ShortNameIndex;
						bms.serial(index);
						return;
					}
					if (!playerInfo->Title.empty())
					{
						uint32 index = SM->translateTitle(playerInfo->Title, language);
						// we must match this index against the function table
						bms.serial(index);
						return;
					}
				}
			}
		}

		// special case fauna entity or unnamed npc (i.e. npc not registered with a character name, like fauna in npc state machine)
		if (EId.getType() == RYZOMID::creature || EId.getType() == RYZOMID::npc)
		{
			// a big FAKE
			STRING_MANAGER::TParamType realType = ParamId.Type;
			ParamId.Type = STRING_MANAGER::creature;
			CParameterTraits::fillBitMemStream(charInfo, language, rep, bms);

			// restore normal type
			ParamId.Type = realType;
			return;
		}

		// no info on the name, just send the EID as string.
//		ucstring temp(EId.toString());
		const ucstring NoName("''");
		uint32 index = SM->storeString(NoName);
		bms.serial(index);
	}

	void setDefaultValue()
	{
		EId = NLMISC::CEntityId::Unknown;
		_AIAlias = 0;
	}

};
class CParameterTraitsEnum : public CStringManager::CParameterTraits
{
public:
	CParameterTraitsEnum(STRING_MANAGER::TParamType type, const std::string &typeName)
		: CParameterTraits(type, typeName)
	{} 
	bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		TParam	param;
		if (param.serialParam(debug, message, ParamId.Type))
		{
			Enum = param.Enum;
			return true;
		}
		else
		{
			setDefaultValue();
			return false;
		}
	}
};

class CParameterTraitsIdentifier : public CStringManager::CParameterTraits
{
public:
	CParameterTraitsIdentifier(STRING_MANAGER::TParamType type, const std::string &typeName)
		: CParameterTraits(type, typeName)
	{} 
	bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		TParam	param;
		if (param.serialParam(debug, message, ParamId.Type))
		{
			Identifier = param.Identifier;
			return true;
		}
		else
		{
			setDefaultValue();
			return false;
		}
	}

	const std::string &getParameterId() const
	{
		return Identifier;
	}

	void setDefaultValue()
	{
		Identifier.clear();
	}
};

class CParameterTraitsSheet : public CStringManager::CParameterTraits
{
public:
	CParameterTraitsSheet(STRING_MANAGER::TParamType type, const std::string &typeName)
		: CParameterTraits(type, typeName)
	{} 
	bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		TParam	param;
		if (param.serialParam(debug, message, ParamId.Type))
		{
			SheetId = param.SheetId;
			return true;
		}
		else
		{
			setDefaultValue();
			return false;
		}
//		message.serial(SheetId);
	}

	const std::string &getParameterId() const
	{
//		const CStringManager::TSheetInfo &si = SM->getSheetInfo(SheetId);

		static string sheetName;
		sheetName = SheetId.toString();
		sheetName = CFile::getFilenameWithoutExtension(sheetName);

		return sheetName;
	}

	void setDefaultValue()
	{
		SheetId = NLMISC::CSheetId::Unknown;
	}

};


class CParameterTraitsItem : public CParameterTraitsSheet
{
public:
	CParameterTraitsItem() : CParameterTraitsSheet(STRING_MANAGER::item, "item")
	{}

	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsItem();
	}

	/// fill overloaded to deals with ring user defined item with custom names
	void fillBitMemStream( const CCharacterInfos *charInfo, CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		// check if SheetId if in the table or user item
		const CStringManager::TRingUserItemInfos &itemInfos = SM->getUserItems();
		CStringManager::TRingUserItemInfos::const_iterator it(itemInfos.find(SheetId));
		if (it != itemInfos.end() && !it->second.empty())
		{
			// this item has some user name, check the aiInstance
			const vector<CStringManager::TRingUserItemInfo> &userItemInfos = it->second;
			for (uint i=0; i<userItemInfos.size(); ++i)
			{
				if (userItemInfos[i].AIInstance == charInfo->AIInstance)
				{
					// ok, this item is renamed !

					// use the user name for a list a predefined column :
					//    name, named, nameda, p, pd
					if (	rep.Format == "name"
						||	rep.Format == "named"
						||	rep.Format == "nameda"
						||	rep.Format == "p"
						||	rep.Format == "pd")
					{
						nlWrite(bms, serial, userItemInfos[i].ItemNameId);
					}
					else
					{
						// not a valid replacement, return a 'backspace' character
						static uint32 noString = SM->storeString(ucstring()+ucchar(8));
						bms.serial(noString);
					}

					/// return here ---------------------
					return;
				}
			}
		}

		// normal item name, use the base function instead
		CParameterTraitsSheet::fillBitMemStream( charInfo, language, rep, bms);
	}

};

class CParameterTraitsSPhrase : public CParameterTraitsSheet
{
public:
	CParameterTraitsSPhrase() : CParameterTraitsSheet(STRING_MANAGER::sphrase, "sphrase")
	{}

	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsSPhrase();
	}
};

class CParameterTraitsBotName : public CParameterTraitsIdentifier
{
public:
	CParameterTraitsBotName() : CParameterTraitsIdentifier(STRING_MANAGER::bot_name, "bot_name")
	{}

	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsBotName();
	}

	void fillBitMemStream( const CCharacterInfos *charInfo, CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{

		uint32 nameId1 = SM->storeString(Identifier);
		uint32 nameId2 = SM->translateShortName(nameId1);
		const ucstring &name = SM->getString(nameId2);
		if (!name.empty() && name[0] == '$')
		{
			// this name is a generic name, translate the title
			ucstring title = name.substr(1, name.size()-2);
			nameId2 = SM->translateTitle(title.toString(), language);
		}
		// serial the string ID
		bms.serial(nameId2);
	}

};


class CParameterTraitsPlace : public CParameterTraitsIdentifier
{
public:
	CParameterTraitsPlace() : CParameterTraitsIdentifier(STRING_MANAGER::place, "place")
	{}

	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsPlace();
	}

};

class CParameterTraitsCreature : public CParameterTraitsEntity
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsCreature();
	}

	CParameterTraitsCreature() : CParameterTraitsEntity(STRING_MANAGER::creature, "creature")
	{}
	const std::string &getParameterId() const
	{
		const NLMISC::CSheetId &sheetId = SM->getSheetId(EId);
		if (sheetId == NLMISC::CSheetId::Unknown)
		{
			nlwarning("The sheet id for creature entity %s is unknown !", EId.toString().c_str());
			static std::string empty;
			return empty;
		}


		const CStringManager::TSheetInfo &si = SM->getSheetInfo(sheetId);

		return si.SheetName;
//		return si.Race;

	}
	bool eval(CStringManager::TLanguages lang, const CCharacterInfos *charInfo, const CStringManager::TCondition &cond) const
	{
		// Test unknown entity
		if ((cond.ReferenceInt == 0) && (cond.Property.empty()))
		{
			if (cond.Operator == CStringManager::equal)
				return EId.isUnknownId();
			else if (cond.Operator == CStringManager::notEqual)
				return !EId.isUnknownId();
		}

		// creature can test model name, model gender

		// 1st, I need to retreive the sheetId from the mirror
		const NLMISC::CSheetId &sheetId = SM->getSheetId(EId);
		if (sheetId == NLMISC::CSheetId::Unknown)
		{
			nlwarning("The sheet id for creature entity %s is unknown !", EId.toString().c_str());
			return false;
		}

		const CStringManager::TSheetInfo &si = SM->getSheetInfo(sheetId);
		std::string op1;
		if (cond.Property == "name")
		{
			op1 = si.SheetName;
		}
		else if (cond.Property == "gender")
		{
			op1 = si.Gender;
		}
		else
		{
			return CParameterTraitsEntity::eval(lang,charInfo, cond);
		}

		NLMISC::strlwr(op1);

		LOG("SM : (creature) eval condition for property %s [%s] %s [%s]", cond.Property.c_str(), op1.c_str(), OperatorNames[cond.Operator], cond.ReferenceStr.c_str());

		switch(cond.Operator)
		{
		case CStringManager::equal:
			return op1 == cond.ReferenceStr;
		case CStringManager::notEqual:
			return op1 != cond.ReferenceStr;
		case CStringManager::less:
			return op1 < cond.ReferenceStr;
		case CStringManager::lessEqual:
			return op1 <= cond.ReferenceStr;
		case CStringManager::greater:
			return op1 > cond.ReferenceStr;
		case CStringManager::greaterEqual:
			return op1 >= cond.ReferenceStr;
		default:
			nlwarning("Operator %s not applicable to creature property %s", OperatorNames[cond.Operator], cond.Property.c_str());
			return false;
		}
		nlerror("This point of code can never be reach !");
	}
/*	void fillBitMemStream(CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		// need to evaluate the name of the creature : name from the sheet id
		ucstring temp;
		NLMISC::CSheetId sid = SM->getSheetId(EId);
		if (sid != NLMISC::CSheetId::Unknown)
		{
			const CStringManager::TSheetInfo &si = SM->getSheetInfo(sid);

			temp =  si.DisplayName;
			if (!temp.empty())
			{
				uint32 index = SM->storeString(temp);
				bms.serial(index);
				return;
			}
		}
		// hum, no name available
		CParameterTraitsEntity::fillBitMemStream(charInfo, rep, bms);
	}
*/
};
class CParameterTraitsSkill : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsSkill();
	}
	CParameterTraitsSkill() : CParameterTraitsEnum(STRING_MANAGER::skill, "skill")
	{}
	const std::string &getParameterId() const
	{
		return SKILLS::toString(SKILLS::ESkills(Enum));
	}
	void setDefaultValue()
	{
		Enum = SKILLS::unknown;
	}
};

class CParameterTraitsBodyPart : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsBodyPart();
	}
	CParameterTraitsBodyPart() : CParameterTraitsEnum(STRING_MANAGER::body_part, "bodypart")
	{}
	const std::string &getParameterId() const
	{
		return BODY::toString(BODY::TBodyPart(Enum));
	}
	void setDefaultValue()
	{
		Enum = BODY::UnknownBodyPart;
	}
};

class CParameterTraitsScore : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsScore();
	}
	CParameterTraitsScore() : CParameterTraitsEnum(STRING_MANAGER::score, "score")
	{}
	const std::string &getParameterId() const
	{
		return SCORES::toString(SCORES::TScores(Enum));
	}
	void setDefaultValue()
	{
		Enum = SCORES::unknown;
	}
};

class CParameterTraitsCharacteristic : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsCharacteristic();
	}
	CParameterTraitsCharacteristic() : CParameterTraitsEnum(STRING_MANAGER::characteristic, "characteristic")
	{}
	const std::string &getParameterId() const
	{
		return CHARACTERISTICS::toString(CHARACTERISTICS::TCharacteristics(Enum));
	}
	void setDefaultValue()
	{
		Enum = CHARACTERISTICS::Unknown;
	}
};

class CParameterTraitsDamageType : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsDamageType();
	}
	CParameterTraitsDamageType() : CParameterTraitsEnum(STRING_MANAGER::damage_type, "damagetype")
	{}
	const std::string &getParameterId() const
	{
		return DMGTYPE::toString(DMGTYPE::EDamageType(Enum));
	}
	void setDefaultValue()
	{
		Enum = DMGTYPE::UNDEFINED;
	}
};

class CParameterTraitsClassificationType : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsClassificationType ();
	}
	CParameterTraitsClassificationType () : CParameterTraitsEnum(STRING_MANAGER::classification_type, "classificationtype")
	{}
	const std::string &getParameterId() const
	{
		return DMGTYPE::toString(DMGTYPE::EDamageType(Enum));
	}
	void setDefaultValue()
	{
		Enum = EGSPD::CClassificationType::Unknown;
	}
};


class CParameterTraitsPowerType : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsPowerType();
	}
	CParameterTraitsPowerType() : CParameterTraitsEnum(STRING_MANAGER::power_type, "powertype")
	{}
	const std::string &getParameterId() const
	{
		return POWERS::toString(POWERS::TPowerType(Enum));
	}
	void setDefaultValue()
	{
		Enum = POWERS::UnknownType;
	}
};

class CParameterTraitsRole : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsRole();
	}
	CParameterTraitsRole() : CParameterTraitsEnum(STRING_MANAGER::role, "role")
	{}
	const std::string &getParameterId() const
	{
		return ROLES::toString(ROLES::ERole(Enum));
	}
	void setDefaultValue()
	{
		// TODO : set a default value
		Enum = ROLES::role_unknown;
	}
};

/*
class CParameterTraitsCareer : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsCareer();
	}
	CParameterTraitsCareer() : CParameterTraitsEnum(STRING_MANAGER::career, "career")
	{}
	const std::string &getParameterId() const
	{
		// TODO : use the career enum when it exist !
		return ROLES::toString(ROLES::ERole(Enum));
	}
	void setDefaultValue()
	{
		Enum = ROLES::role_unknown;
	}
};
*/
/*
class CParameterTraitsJob : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsJob();
	}
	CParameterTraitsJob() : CParameterTraitsEnum(STRING_MANAGER::job, "job")
	{}
	const std::string &getParameterId() const
	{
		return JOBS::toString(JOBS::TJob(Enum));
	}
	void setDefaultValue()
	{
		Enum = JOBS::Unknown;
	}
};
*/
class CParameterTraitsEcosystem : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsEcosystem();
	}
	CParameterTraitsEcosystem() : CParameterTraitsEnum(STRING_MANAGER::ecosystem, "ecosystem")
	{}
	const std::string &getParameterId() const
	{
		return ECOSYSTEM::toString(ECOSYSTEM::EECosystem(Enum));
	}
	void setDefaultValue()
	{
		Enum = ECOSYSTEM::unknown;
	}
};
class CParameterTraitsRace : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsRace();
	}
	CParameterTraitsRace() : CParameterTraitsEnum(STRING_MANAGER::race, "race")
	{}
	const std::string &getParameterId() const
	{
		return EGSPD::CPeople::toString(EGSPD::CPeople::TPeople(Enum));
	}
	void setDefaultValue()
	{
		Enum = EGSPD::CPeople::EndPeople;
	}
};

class CParameterTraitsBrick : public CParameterTraitsSheet
{
public:
	CParameterTraitsBrick() : CParameterTraitsSheet(STRING_MANAGER::sbrick, "sbrick")
	{}

	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsBrick();
	}
};

class CParameterTraitsOutpostWord : public CParameterTraitsSheet
{
public:
	CParameterTraitsOutpostWord() : CParameterTraitsSheet(STRING_MANAGER::outpost, "outpost")
	{}
	
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsOutpostWord();
	}
	const std::string &getParameterId() const
	{
		static string sheetName;
		sheetName = SheetId.toString();
		return sheetName;
	}
};

/*
class CParameterTraitsBrick : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsBrick();
	}
	CParameterTraitsBrick() : CParameterTraitsEnum(STRING_MANAGER::sbrick, "sbrick")
	{}
	const std::string &getParameterId() const
	{
//		return BRICK_FAMILIES::toString(BRICK_FAMILIES::TBrickFamily(Enum));
		return SheetId.toString();
	}
	void setDefaultValue()
	{
		Enum = BRICK_FAMILIES::Unknown;
	}
};
*/
class CParameterTraitsFaction : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsFaction();
	}
	CParameterTraitsFaction() : CParameterTraitsEnum(STRING_MANAGER::faction, "faction")
	{}
	const std::string &getParameterId() const
	{
		return CStaticFames::getInstance().getFactionName(Enum);
	}
	void setDefaultValue()
	{
		// TODO : set a default value
		Enum = 0;
	}
};
class CParameterTraitsGuild : public CParameterTraitsEntity
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsGuild();
	}
	CParameterTraitsGuild() : CParameterTraitsEntity(STRING_MANAGER::guild, "guild")
	{}
};

class CParameterTraitsPlayer : public CParameterTraitsEntity
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsPlayer();
	}
	CParameterTraitsPlayer() : CParameterTraitsEntity(STRING_MANAGER::player, "player")
	{}

	bool eval(CStringManager::TLanguages lang, const CCharacterInfos *charInfo, const CStringManager::TCondition &cond) const
	{
		// Test unknown entity
		if ((cond.ReferenceInt == 0) && (cond.Property.empty()))
		{
			if (cond.Operator == CStringManager::equal)
				return EId.isUnknownId();
			else if (cond.Operator == CStringManager::notEqual)
				return !EId.isUnknownId();
		}

		// player can test model name, entity name, entity gender
		// 1st, I need to retrieve the sheetId from the mirror
		std::string op1;
		if (cond.Property == "name")
		{
			const NLMISC::CSheetId &sheetId = SM->getSheetId(EId);
			if (sheetId == NLMISC::CSheetId::Unknown)
			{
				nlwarning("The sheet id for creature entity %s is unknown !", EId.toString().c_str());
				return false;
			}
			const CStringManager::TSheetInfo &si = SM->getSheetInfo(sheetId);
			op1 = si.SheetName;
		}
		else if (cond.Property == "gender")
		{
			CCharacterInfos	*charInfo = IOS->getCharInfos(EId);
			if (charInfo == NULL)
			{
				nlwarning("Could not find character info for EId %s to check property %s", EId.toString().c_str(), cond.Property.c_str());
				return false;
			}
			op1 = GSGENDER::toString(charInfo->getGender());
		}
		else
		{
			return CParameterTraitsEntity::eval(lang,charInfo, cond);
		}

		NLMISC::strlwr(op1);

		LOG("SM : (player) eval condition for property %s [%s] %s [%s]", cond.Property.c_str(), op1.c_str(), OperatorNames[cond.Operator], cond.ReferenceStr.c_str());

		switch(cond.Operator)
		{
		case CStringManager::equal:
			return op1 == cond.ReferenceStr;
		case CStringManager::notEqual:
			return op1 != cond.ReferenceStr;
		case CStringManager::less:
			return op1 < cond.ReferenceStr;
		case CStringManager::lessEqual:
			return op1 <= cond.ReferenceStr;
		case CStringManager::greater:
			return op1 > cond.ReferenceStr;
		case CStringManager::greaterEqual:
			return op1 >= cond.ReferenceStr;
		default:
			nlwarning("Operator %s not applicable to player property %s", OperatorNames[cond.Operator], cond.Property.c_str());
			return false;
		}
		nlerror("This point of code can never be reach !");
	}
	void fillBitMemStream( const CCharacterInfos *charInfo,CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		ucstring temp;

		CCharacterInfos	*playerInfo = IOS->getCharInfos(EId);
		if (playerInfo != 0)
		{
			if (!playerInfo->ShortName.empty())
			{
				uint32 index = playerInfo->ShortNameIndex;
				bms.serial(index);
				return;
			}
		}
		// hum, no name available
		CParameterTraitsEntity::fillBitMemStream(charInfo,language, rep, bms);
	}
};
class CParameterTraitsBot : public CParameterTraitsEntity
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsBot();
	}
	CParameterTraitsBot() : CParameterTraitsEntity(STRING_MANAGER::bot, "bot")
	{}
	bool eval(CStringManager::TLanguages lang, const CCharacterInfos *charInfo, const CStringManager::TCondition &cond) const
	{
		// Test unknown entity
		if ((cond.ReferenceInt == 0) && (cond.Property.empty()))
		{
			if (cond.Operator == CStringManager::equal)
				return EId.isUnknownId();
			else if (cond.Operator == CStringManager::notEqual)
				return !EId.isUnknownId();
		}

		// bot can test career and role
		// 1st, I need to retrieve the sheetId from the mirror
		// TODO : reuse sheet server when AI fill in correctly !
		const NLMISC::CSheetId &sheetId = SM->getSheetServerId(EId);
		if (sheetId == NLMISC::CSheetId::Unknown)
		{
			nlwarning("The sheet id for creature bot %s is unknown !", EId.toString().c_str());
			return false;
		}

		LOG("Bot %s use server sheet %s", EId.toString().c_str(), sheetId.toString().c_str());
		const CStringManager::TSheetInfo &si = SM->getSheetInfo(sheetId);
		std::string op1;
		if (cond.Property == "career")
		{
			op1 = si.Profile;
		}
		else if (cond.Property == "role")
		{
			op1 = si.ChatProfile;
		}
		else if (cond.Property == "title")
		{
			// we need to retrieve the charInfo
			CCharacterInfos	*ci = IOS->getCharInfos(EId);
			if (ci != NULL)
				op1 = ci->Title;
			else
			{
				nlwarning("No character info for bot %s, can't test property 'title' !", EId.toString().c_str());
				return false;
			}
		}
		else
		{
			return CParameterTraitsEntity::eval(lang, charInfo, cond);
		}

		NLMISC::strlwr(op1);

		LOG("SM : (bot) eval condition for property %s [%s] %s [%s]", cond.Property.c_str(), op1.c_str(), OperatorNames[cond.Operator], cond.ReferenceStr.c_str());

		switch(cond.Operator)
		{
		case CStringManager::equal:
			return op1 == cond.ReferenceStr;
		case CStringManager::notEqual:
			return op1 != cond.ReferenceStr;
		case CStringManager::less:
			return op1 < cond.ReferenceStr;
		case CStringManager::lessEqual:
			return op1 <= cond.ReferenceStr;
		case CStringManager::greater:
			return op1 > cond.ReferenceStr;
		case CStringManager::greaterEqual:
			return op1 >= cond.ReferenceStr;
		default:
			nlwarning("Operator %s not applicable to player property %s", OperatorNames[cond.Operator], cond.Property.c_str());
			return false;
		}
		nlerror("This point of code can never be reach !");
	}

	void fillBitMemStream( const CCharacterInfos *charInfo,CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		// need to evaluate the name of the bot : should be in the charinfo
		ucstring temp;

		CCharacterInfos	*botInfo = IOS->getCharInfos(EId);
		if (botInfo != 0)
		{
			if (!botInfo->ShortName.empty())
			{
				uint32 index = botInfo->ShortNameIndex;
				bms.serial(index);
				return;
			}
			if (!botInfo->Title.empty())
			{
				uint32 index = SM->translateTitle(botInfo->Title, language);
				bms.serial(index);
				return;
			}
		}
		// hum, no name available
		CParameterTraitsEntity::fillBitMemStream(charInfo, language, rep, bms);
	}
};
class CParameterTraitsInteger : public CStringManager::CParameterTraits
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsInteger();
	}
	CParameterTraitsInteger() : CParameterTraits(STRING_MANAGER::integer, "integer")
	{}
	bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		TParam	param;
		if (param.serialParam(debug, message, ParamId.Type))
		{
			Int = param.Int;
			return true;
		}
		else
		{
			setDefaultValue();
			return false;
		}
//		message.serial(Int);
	}

	void fillBitMemStream( const CCharacterInfos *charInfo,CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		bms.serial(Int);
	}

	const std::string &getParameterId() const
	{
		nlstopex(("Never call this !"));
		static std::string temp = "";
		return temp;
	}

	bool eval(CStringManager::TLanguages lang, const CCharacterInfos *charInfo, const CStringManager::TCondition &cond) const
	{
		switch(cond.Operator)
		{
		case CStringManager::equal:
			return Int == cond.ReferenceInt;
		case CStringManager::notEqual:
			return Int != cond.ReferenceInt;
		case CStringManager::greater:
			return Int > cond.ReferenceInt;
		case CStringManager::greaterEqual:
			return Int >= cond.ReferenceInt;
		case CStringManager::less:
			return Int < cond.ReferenceInt;
		case CStringManager::lessEqual:
			return Int <= cond.ReferenceInt;
		default:
			nlwarning("Operator not valid (%d)", cond.Operator);
			return false;
		}
	}
	void setDefaultValue()
	{
		Int = 0;
	}
};
class CParameterTraitsTime : public CStringManager::CParameterTraits
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsTime();
	}
	CParameterTraitsTime() : CParameterTraits(STRING_MANAGER::time, "time")
	{}
	bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		TParam	param;
		if (param.serialParam(debug, message, ParamId.Type))
		{
			Time = param.Time;
			return true;
		}
		else
		{
			setDefaultValue();
			return false;
		}
//		message.serial(Time);
	}
	void fillBitMemStream( const CCharacterInfos *charInfo,CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		bms.serial(Time);
	}
	const std::string &getParameterId() const
	{
		nlstopex(("Never call this !"));
		static std::string temp = "";
		return temp;
	}
	bool eval(CStringManager::TLanguages lang, const CCharacterInfos *charInfo, const CStringManager::TCondition &cond) const
	{
		nlwarning("Time parameter can't be conditional");
		return false;
	}
	void setDefaultValue()
	{
		Time = 0;
	}
};
class CParameterTraitsMoney : public CStringManager::CParameterTraits
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsMoney();
	}
	CParameterTraitsMoney() : CParameterTraits(STRING_MANAGER::money, "money")
	{}
	bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		TParam	param;
		if (param.serialParam(debug, message, ParamId.Type))
		{
			Money = param.Money;
			return true;
		}
		else
		{
			setDefaultValue();
			return false;
		}
//		message.serial(Money);
	}
	void fillBitMemStream( const CCharacterInfos *charInfo,CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		// TODO : serial only 48 bits
		bms.serial(Money);
	}
	const std::string &getParameterId() const
	{
		nlstopex(("Never call this !"));
		static std::string temp = "";
		return temp;
	}
	bool eval(CStringManager::TLanguages lang, const CCharacterInfos *charInfo, const CStringManager::TCondition &cond) const
	{
		nlwarning("Money parameter can't be conditional");
		return false;
	}
	void setDefaultValue()
	{
		Money = 0;
	}
};
class CParameterTraitsCompass : public CParameterTraitsEnum
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsCompass();
	}
	CParameterTraitsCompass() : CParameterTraitsEnum(STRING_MANAGER::compass, "compass")
	{}
	const std::string &getParameterId() const
	{
		// TODO : add an enum for compass dir in game_share
		static std::string temp = "";
		return temp;
	}
	void setDefaultValue()
	{
		// TODO : set a default value
		Enum = 0;
	}
};
class CParameterTraitsStringId : public CStringManager::CParameterTraits
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsStringId();
	}
	CParameterTraitsStringId() : CParameterTraits(STRING_MANAGER::string_id, "string_id")
	{}
	bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		TParam	param;
		if (param.serialParam(debug, message, ParamId.Type))
		{
			StringId = param.StringId;
			return true;
		}
		else
		{
			setDefaultValue();
			return false;
		}
//		message.serial(StringId);
	}
	void fillBitMemStream( const CCharacterInfos *charInfo,CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		bms.serial(StringId);
	}
	const std::string &getParameterId() const
	{
		nlstop;
		static std::string temp = "";
		return temp;
	}
	bool eval(CStringManager::TLanguages lang, const CCharacterInfos *charInfo, const CStringManager::TCondition &cond) const
	{
		switch(cond.Operator)
		{
		case CStringManager::equal:
			return StringId == uint32(cond.ReferenceInt);
		case CStringManager::notEqual:
			return StringId != uint32(cond.ReferenceInt);
		default:
			nlwarning("Operator %s not applicable to self property %s", OperatorNames[cond.Operator], cond.Property.c_str());
			return false;
		}
	}
	void setDefaultValue()
	{
		StringId = 0;
	}
};
class CParameterTraitsdyn_string_id : public CStringManager::CParameterTraits
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsdyn_string_id();
	}
	CParameterTraitsdyn_string_id() : CParameterTraits(STRING_MANAGER::dyn_string_id, "dyn_string_id")
	{}
	bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		TParam	param;
		if (param.serialParam(debug, message, ParamId.Type))
		{
			StringId = param.StringId;
			return true;
		}
		else
		{
			setDefaultValue();
			return false;
		}
//		message.serial(StringId);
	}
	void fillBitMemStream( const CCharacterInfos *charInfo,CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		bms.serial(StringId);
	}
	const std::string &getParameterId() const
	{
		nlstop;
		static std::string temp = "";
		return temp;
	}
	bool eval(CStringManager::TLanguages lang, const CCharacterInfos *charInfo, const CStringManager::TCondition &cond) const
	{
		switch(cond.Operator)
		{
		case CStringManager::equal:
			return StringId == uint32(cond.ReferenceInt);
		case CStringManager::notEqual:
			return StringId != uint32(cond.ReferenceInt);
		default:
			nlwarning("Operator %s not applicable to self property %s", OperatorNames[cond.Operator], cond.Property.c_str());
			return false;
		}
	}
	void setDefaultValue()
	{
		StringId = 0;
	}
};
class CParameterTraitsSelf : public CParameterTraitsEntity
{
public:
	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsSelf();
	}
	CParameterTraitsSelf() : CParameterTraitsEntity(STRING_MANAGER::self, "self")
	{}
	bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		nlwarning("CParameterTraitsSelf can't be received !");
		return false;
	}
	const std::string &getParameterId() const
	{
		// TODO : retreive the entity name...
		static std::string temp = "";
		return temp;
	}
	bool eval(CStringManager::TLanguages lang, const CCharacterInfos *charInfo, const CStringManager::TCondition &cond) const
	{
		if ( !charInfo )
			return false;
		std::string value;

		// check if checked property is gender or name
		if (cond.Property == "gender")
		{
			value = GSGENDER::toString(charInfo->getGender());
		}
		else if (cond.Property == "name")
		{
			value = charInfo->ShortName.toString();
		}
		else
		{
			return CParameterTraitsEntity::eval(lang,charInfo, cond);
		}

		NLMISC::strlwr(value);
		LOG("SM : (self) eval condition for property %s [%s] %s [%s]", cond.Property.c_str(), value.c_str(), OperatorNames[cond.Operator], cond.ReferenceStr.c_str());

		switch(cond.Operator)
		{
		case CStringManager::equal:
			return value == cond.ReferenceStr;
		case CStringManager::notEqual:
			return value != cond.ReferenceStr;
		case CStringManager::less:
			return value < cond.ReferenceStr;
		case CStringManager::lessEqual:
			return value <= cond.ReferenceStr;
		case CStringManager::greater:
			return value > cond.ReferenceStr;
		case CStringManager::greaterEqual:
			return value >= cond.ReferenceStr;
		default:
			nlwarning("Operator %s not applicable to self property %s", OperatorNames[cond.Operator], cond.Property.c_str());
			return false;
		}
	}
	void fillBitMemStream( const CCharacterInfos *charInfo,CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		// need to evaluate the name of the bot : should be in the charinfo
		ucstring temp;

		if (charInfo != 0)
		{
			if (!charInfo->Name.empty())
			{
				uint32 index = charInfo->ShortNameIndex;
				bms.serial(index);
				return;
			}
		}
		// hum, no name available
		CParameterTraitsEntity::fillBitMemStream(charInfo, language, rep, bms);
	}
};

class CParameterTraitsCreatureModel : public CParameterTraitsSheet
{
public:
	CParameterTraitsCreatureModel() : CParameterTraitsSheet(STRING_MANAGER::creature_model, "creature")
	{}

	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsItem();
	}
};

class CParameterTraitsLiteral : public CStringManager::CParameterTraits
{
public:
	CParameterTraitsLiteral() : CParameterTraits(STRING_MANAGER::literal, "literal")
	{}

	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsLiteral();
	}

	/// Return parameter id, ie, the name that is use to identity the data row in the csv file.
	virtual const std::string &getParameterId() const
	{
		nlstop;
		static const string s;
		return s;
	}
	/// Extract the parameter value from a message.
	virtual bool extractFromMessage(NLNET::CMessage &message, bool debug)
	{
		TParam	param;
		if (param.serialParam(debug, message, ParamId.Type))
		{
			Literal = param.Literal;
			return true;
		}
		else
		{
			setDefaultValue();
			return false;
		}
	}
	/// Fill a bitmem strean with the parameter value.
	virtual void fillBitMemStream( const CCharacterInfos *charInfo, CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		uint32 id = SM->storeString(Literal);
		bms.serial(id);
	}
	/// Eval a condition with this parameter.
//	virtual bool eval(TLanguages lang, const CCharacterInfos *charInfo, const TCondition &cond) const;
	/// set a default value
	virtual void setDefaultValue()
	{
		Literal = ucstring();
	}

};

class CParameterTraitsTitle : public CParameterTraitsIdentifier
{
public:
	CParameterTraitsTitle() : CParameterTraitsIdentifier(STRING_MANAGER::title, "title")
	{}

	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsTitle();
	}

};

class CParameterTraitsEventFaction : public CParameterTraitsIdentifier
{
public:
	CParameterTraitsEventFaction() : CParameterTraitsIdentifier(STRING_MANAGER::event_faction, "event_faction")
	{}

	CStringManager::CParameterTraits *clone()
	{
		return new CParameterTraitsEventFaction();
	}

	void fillBitMemStream( const CCharacterInfos *charInfo, CStringManager::TLanguages language, const CStringManager::TReplacement &rep, NLMISC::CBitMemStream &bms)
	{
		uint32 eventFactionId = SM->translateEventFaction(Identifier);

		// serial the string ID
		bms.serial(eventFactionId);
	}

};

/**
 * WARNING: the order in this array is very important, and MUST match exactly
 * the order in the TParamType enum declared in string_manager_sender.h
 */
std::vector<CStringManager::CParameterTraits*>	CStringManager::CParameterTraits::_Models;

void	CStringManager::CParameterTraits::init()
{
	_Models.push_back(new CParameterTraitsItem());
	_Models.push_back(new CParameterTraitsPlace());
	_Models.push_back(new CParameterTraitsCreature());
	_Models.push_back(new CParameterTraitsSkill());
	_Models.push_back(new CParameterTraitsRole());
	_Models.push_back(new CParameterTraitsEcosystem());
	_Models.push_back(new CParameterTraitsRace());
	_Models.push_back(new CParameterTraitsBrick());
	_Models.push_back(new CParameterTraitsFaction());
	_Models.push_back(new CParameterTraitsGuild());
	_Models.push_back(new CParameterTraitsPlayer());
	_Models.push_back(new CParameterTraitsBot());
	_Models.push_back(new CParameterTraitsInteger());
	_Models.push_back(new CParameterTraitsTime());
	_Models.push_back(new CParameterTraitsMoney());
	_Models.push_back(new CParameterTraitsCompass());
	_Models.push_back(new CParameterTraitsStringId());
	_Models.push_back(new CParameterTraitsdyn_string_id());
	_Models.push_back(new CParameterTraitsSelf());
	_Models.push_back(new CParameterTraitsCreatureModel());
	_Models.push_back(new CParameterTraitsEntity(STRING_MANAGER::entity, "entity"));
	_Models.push_back(new CParameterTraitsBodyPart());
	_Models.push_back(new CParameterTraitsScore());
	_Models.push_back(new CParameterTraitsSPhrase());
	_Models.push_back(new CParameterTraitsCharacteristic());
	_Models.push_back(new CParameterTraitsDamageType());
	_Models.push_back(new CParameterTraitsBotName());
	_Models.push_back(new CParameterTraitsPowerType());
	_Models.push_back(new CParameterTraitsLiteral());
	_Models.push_back(new CParameterTraitsTitle());
	_Models.push_back(new CParameterTraitsEventFaction());
	_Models.push_back(new CParameterTraitsClassificationType());
	_Models.push_back(new CParameterTraitsOutpostWord());
}

CStringManager::CParameterTraits *CStringManager::CParameterTraits::createParameterTraits(STRING_MANAGER::TParamType type)
{
	for (uint i=0; i<_Models.size(); ++i)
	{
		if (_Models[i]->ParamId.Type == type)
			return _Models[i]->clone();
	}
	nlstop;

	return 0;
}

std::vector<std::pair<STRING_MANAGER::TParamType, std::string> >	CStringManager::CParameterTraits::getParameterTraitsNames()
{
	std::vector<std::pair<STRING_MANAGER::TParamType, std::string> > ret;
	for (uint i=0; i<_Models.size(); ++i)
	{
		ret.push_back(std::make_pair(_Models[i]->ParamId.Type, _Models[i]->TypeName));
	}
	return ret;
}

