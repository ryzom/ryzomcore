/*

	Sabrina_test project: player manager

*/

#include "nel/misc/sstring.h"
#include "sabrina/entity_base.h"
#include "sabrina/sabrina_actor_creature.h"

#ifdef NL_OS_WINDOWS
#pragma warning (disable : 4355)	//	warning C4355: 'this' : used in base member initializer list
#endif

//------------------------------------------------------------------------
// 	CNPCRecord / CCreatureRecord
//------------------------------------------------------------------------

class CNPCRecord: public CEntityBase
{
public:
	CNPCRecord(const CSString& name);

	ISabrinaActor* getSabrinaActor() { return const_cast<CSabrinaActorCreature*>(getSabAct()); }

	const CSString& getName() const { return _Name; }
	const CSabrinaActorCreature* getSabAct() const { return &_SabrinaActor; }

private:
	CSString _Name;
	CSabrinaActorCreature _SabrinaActor;
};

class CCreatureRecord: public CEntityBase
{
public:
	CCreatureRecord(const CSString& name);

	ISabrinaActor* getSabrinaActor() { return const_cast<CSabrinaActorCreature*>(getSabAct()); }

	const CSString& getName() const { return _Name; }
	const CSabrinaActorCreature* getSabAct() const { return &_SabrinaActor; }

private:
	CSString _Name;
	CSabrinaActorCreature _SabrinaActor;
};

//------------------------------------------------------------------------
// CNPCRecord / CCreatureRecord inlines
//------------------------------------------------------------------------

CNPCRecord::CNPCRecord(const CSString& name): _SabrinaActor(this)
{
	_Name=name;
}

CCreatureRecord::CCreatureRecord(const CSString& name): _SabrinaActor(this)
{
	_Name=name;
}

//------------------------------------------------------------------------
// 	CBotManager
//------------------------------------------------------------------------

class CBotManager
{
public:
	void addNPC(const CSString& name);
	void removeNPC(const CSString& name);
	void listNPCs() const;
	CNPCRecord* getNPC(const CSString& name);

	void addCreature(const CSString& name,const NLMISC::CSheetId sheet);
	void removeCreature(const CSString& name);
	void listCreatures() const;
	CCreatureRecord* getCreature(const CSString& name);

	CEntityBase* getBot(const CSString& name);

private:
	std::vector<CNPCRecord*> _NPCs;
	std::vector<CCreatureRecord*> _Creatures;
};

//------------------------------------------------------------------------
// CBotManager inlines - Creature
//------------------------------------------------------------------------

inline void CBotManager::addCreature(const CSString& name,const NLMISC::CSheetId sheet)
{
	removeCreature(name);
	_Creatures.push_back(new CCreatureRecord(name));
}

inline void CBotManager::removeCreature(const CSString& name)
{
	for (uint i=_Creatures.size();i--;)
	{
		if (_Creatures[i]->getName()==name)
		{
			delete _Creatures[i];
			_Creatures[i]=_Creatures[_Creatures.size()-1];
			_Creatures.pop_back();
		}
	}
}

inline void CBotManager::listCreatures() const
{
	for (uint i=_Creatures.size();i--;)
	{
		CSString s="Creature";
		s+=": "+_Creatures[i]->getName();
		s+=": "+_Creatures[i]->getSabAct()->stateString();
		nlinfo("%s",s.c_str());
	}
}

inline CCreatureRecord* CBotManager::getCreature(const CSString& name)
{
	for (uint i=_Creatures.size();i--;)
	{
		if (_Creatures[i]->getName()==name)
		{
			return _Creatures[i]; 
		}
	}
	return NULL;
}

//------------------------------------------------------------------------
// CBotManager inlines - NPC
//------------------------------------------------------------------------

inline void CBotManager::addNPC(const CSString& name)
{
	removeNPC(name);
	_NPCs.push_back(new CNPCRecord(name));
}

inline void CBotManager::removeNPC(const CSString& name)
{
	for (uint i=_NPCs.size();i--;)
	{
		if (_NPCs[i]->getName()==name)
		{
			delete _NPCs[i];
			_NPCs[i]=_NPCs[_NPCs.size()-1];
			_NPCs.pop_back();
		}
	}
}

inline void CBotManager::listNPCs()	const
{
	for (uint i=_NPCs.size();i--;)
	{
		CSString s="NPC";
		s+=": "+_NPCs[i]->getName();
		s+=": "+_NPCs[i]->getSabAct()->stateString();
		nlinfo("%s",s.c_str());
	}
}

inline CNPCRecord* CBotManager::getNPC(const CSString& name)
{
	for (uint i=_NPCs.size();i--;)
	{
		if (_NPCs[i]->getName()==name)
		{
			return _NPCs[i]; 
		}
	}
	return NULL;
}

//------------------------------------------------------------------------
// CBotManager inlines
//------------------------------------------------------------------------

inline CEntityBase* CBotManager::getBot(const CSString& name)
{
	CEntityBase* result= getNPC(name);
	if (result!=NULL)
		return result;
	else
		return getCreature(name);
}

