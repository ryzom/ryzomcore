/*

	Sabrina_test project: player manager

*/

#include "nel/misc/sstring.h"
#include "sabrina/entity_base.h"
#include "sabrina/sabrina_actor_player.h"

#ifdef NL_OS_WINDOWS
#pragma warning (disable : 4355)	//	warning C4355: 'this' : used in base member initializer list
#endif

//------------------------------------------------------------------------
// 	CPlayerRecord
//------------------------------------------------------------------------

class CPlayerRecord: public CEntityBase
{
public:
	CPlayerRecord(const CSString& name);

	ISabrinaActor* getSabrinaActor() { return const_cast<CSabrinaActorPlayer*>(getSabAct()); }

	const CSString& getName() const { return _Name; }
	const CSabrinaActorPlayer* getSabAct() const { return &_SabrinaActor; }
	CSabrinaActorPlayer* getSabAct() { return &_SabrinaActor; }

private:
	CSString _Name;
	CSabrinaActorPlayer _SabrinaActor;
};

//------------------------------------------------------------------------
// CPlayerRecord inlines
//------------------------------------------------------------------------

CPlayerRecord::CPlayerRecord(const CSString& name): _SabrinaActor(this)
{
	_Name=name;
}

//------------------------------------------------------------------------
// CPlayerManager
//------------------------------------------------------------------------

class CPlayerManager
{
public:
	CPlayerManager();

	void addPlayer(const CSString& name);
	void removePlayer(const CSString& name);
	void listPlayers() const;
	CPlayerRecord* getPlayer(const CSString& name);

private:
	std::vector<CPlayerRecord*> _Players;
	uint32 _UniquePlayerId;
};

//------------------------------------------------------------------------
// CPlayerManager inlines
//------------------------------------------------------------------------

CPlayerManager::CPlayerManager()
{
	_UniquePlayerId=0;
}

inline void CPlayerManager::addPlayer(const CSString& name)
{
	removePlayer(name);
	_Players.push_back(new CPlayerRecord(name));
	_Players[_Players.size()-1]->setEntityRowId(_UniquePlayerId++);
	_Players[_Players.size()-1]->getSabAct()->addMemoryBank("default");
}

inline void CPlayerManager::removePlayer(const CSString& name)
{
	for (uint i=_Players.size();i--;)
	{
		if (_Players[i]->getName()==name)
		{
			delete _Players[i];
			_Players[i]=_Players[_Players.size()-1];
			_Players.pop_back();
		}
	}
}

inline void CPlayerManager::listPlayers() const
{
	for (uint i=_Players.size();i--;)
	{
		CSString s="Player";
		s+=": "+_Players[i]->getName();
		s+=": "+_Players[i]->getSabAct()->stateString();
		nlinfo("%s",s.c_str());
	}
}

inline CPlayerRecord* CPlayerManager::getPlayer(const CSString& name)
{
	for (uint i=_Players.size();i--;)
	{
		if (_Players[i]->getName()==name)
		{
			return _Players[i]; 
		}
	}
	return NULL;
}
