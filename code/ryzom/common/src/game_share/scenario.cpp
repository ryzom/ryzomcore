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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"

#include "scenario.h"
#include "object.h"



#include "nel/misc/string_common.h"
#include "nel/misc/sstring.h"
#include "nel/misc/debug.h"
#include "nel/misc/file.h"
#include "nel/misc/algo.h"

using namespace R2;
//----------------------------------------------------------------
namespace R2
{


}

void CScenario::serial( NLMISC::IStream &f)
{
	// _Palette is not backuped because only on client side
	// There is no characters connected so _CurrentChars is not need,
	// _InstanceMap is set dynamically

	if ( f.isReading())
	{
		_Clean = false;
	}
	f.serialEnum(_SessionType);
	f.serial(_Mode);
	f.serial(_InitialActIndex);

	if (!f.isReading())
	{

		CObjectSerializerServer hl(_HighLevel);
		f.serial(hl);
		CObjectSerializerServer bb(_BasicBricks);
		f.serial(bb);
	}
	else
	{
		CObjectSerializerServer hl;
		f.serial(hl);
		CObjectSerializerServer bb;
		f.serial(bb);
		setHighLevel( hl.getData() ) ;	// Set instance Map
		delete _BasicBricks;
		_BasicBricks = bb.getData();
	}
}

CScenario::CScenario(CObject* object, TScenarioSessionType sessionType)
{
	_HighLevel = object;
	_BasicBricks = 0;
	_InstanceMap = new CInstanceMap("InstanceId");
	if (object){_InstanceMap->add(object);}
	_Palette = 0;

	_Mode = 1;
	_SessionType = sessionType;
	_InitialActIndex = 1;
	_Clean = false;

}

void CScenario::setHighLevel(CObject* highLevel)
{
	_Clean = false;
	if (highLevel != _HighLevel)
	{
		delete _HighLevel;
		_HighLevel = highLevel;
	}

	_InstanceMap->set(highLevel);

}


CObject *CScenario::find(const std::string& instanceId, const std::string & attrName, sint32 position, const std::string &key)
{
	CObject *src = _InstanceMap->find(instanceId);
	if (!src)
	{
		nlwarning("Can't find object with id %s", instanceId.c_str());
		return NULL;
	}
	if (!attrName.empty())
	{
		CObject *subObj = src->getAttr(attrName);
		if (!subObj)
		{
			nlwarning("Can't find attribute %s inside object with InstanceId =  %s", attrName.c_str(), instanceId.c_str());
			return NULL;
		}
		src = subObj;
	}
	if (position != -1)
	{
		CObject *subObj = src->getValue(position);
		if (!subObj)
		{
			nlwarning("Can't find attribute %s[%d] inside object with InstanceId =  %s", attrName.c_str(), (int) position, instanceId.c_str());
			return NULL;
		}
		src = subObj;
	}
	if (!key.empty())
	{
		CObject *subObj = src->getAttr(key);
		if (!subObj)
		{
			nlwarning("Can't find attribute %s['%s'] inside object with InstanceId =  %s", attrName.c_str(), key.c_str(), instanceId.c_str());
			return NULL;
		}
		src = subObj;
	}
	return src;
}

CScenario::~CScenario()
{
	delete _HighLevel;
	delete _BasicBricks;
	delete _InstanceMap;
	delete _Palette;
}



bool CScenario::setNode( const std::string& instanceId, const std::string& attrName, CObject* value)
{
	_Clean = false;
	CObject* found= _InstanceMap->find(instanceId);

	if (!found)
	{
		nlwarning("CScenario::setNode : couldn't find object with id = %s", instanceId.c_str());
		return false;
	}
	return found->setObject(attrName, value);

}

bool  CScenario::insertNode(const std::string&  instanceId, const std::string & attrName, sint32 position,
	const std::string& key, CObject* value)
{
	_Clean = false;
	CObject* found= _InstanceMap->find(instanceId);

	if (!found)
	{
		nlwarning("CScenario::insertNode : couldn't find object with id = %s", instanceId.c_str());
		return false;
	}
	if (!attrName.empty())
	{
		found=found->getAttr(attrName);
	}
	if (!found)
	{
		nlwarning("CScenario::insertNode : couldn't find attribute '%s' in object with id = %s", attrName.c_str(), instanceId.c_str());
		return false;
	}

	bool inserted = found->insert(key, value, position);
	if (inserted)
	{
		_InstanceMap->add(value);
	}
	return inserted;

}

bool CScenario::eraseNode(const std::string&  instanceId, const std::string & attrName, sint32 position)
{
	_Clean = false;
	CObject* found= _InstanceMap->find(instanceId);
	if (!found)
	{
		nlwarning("CScenario::eraseNode : couldn't find object with id = %s", instanceId.c_str());
		return false;
	}

	if (!attrName.empty()) { found=found->getAttr(attrName); }
	if (!found)
	{
		nlwarning("CScenario::eraseNode : couldn't find attribute '%s' in object with id = %s", attrName.c_str(), instanceId.c_str());
		return false;
	}

	CObject* removed = NULL;
	if (found->isTable())
	{
		bool canRemove = found->canTake(position);
		if (!canRemove)
		{
			nlwarning("CScenario::eraseNode : can not remove a non-existing object from a table (%s, %s, %d)", instanceId.c_str(), attrName.c_str(), position );
			return false;
		}

		removed = found->take(position);
		if (!removed)
		{
			nlwarning("CScenario::eraseNode remove a NULL object (%s, %s, %d)", instanceId.c_str(), attrName.c_str(), position );
			return true;
		}
	}
	else if (position == -1)
	{
		// removing a property
		// this may happen in an undo if the property is redefined in the base
		sint32 index = found->getParent()->findIndex(found);
		nlassert(index != -1);
		removed = found->getParent()->take(index);
	}
	else
	{
		nlwarning("CScenario::eraseNode when removing (%s, %s) : position must be -1, because object is not a table", instanceId.c_str(), attrName.c_str());
		return true;
	}
	_InstanceMap->remove(removed);
	delete removed;
	return true;
}

bool CScenario::moveNode( const std::string& instanceId1, const std::string& attrName1, int position1,
	const std::string& instanceId2, const std::string& attrName2, int position2)
{
	_Clean = false;

	static volatile bool verboseMoveNode = false;
	CObject* from= _InstanceMap->find(instanceId1);
	if (from && !attrName1.empty()) { from=from->getAttr(attrName1); }
	if (!from)
	{
		nlwarning("<CScenario::moveNode> 'from' node with id (%s, %s) not found", instanceId1.c_str(), attrName1.c_str());
		return false;
	}
	CObject* to = _InstanceMap->find(instanceId2);
	if (to && verboseMoveNode)
	{
		nlwarning("Insertion object is : ");
		to->dump();
	}
	if (to && !attrName2.empty())
	{
		to=to->getAttr(attrName2);
		if (to && verboseMoveNode)
		{
			nlwarning("Insertion object sub element is : ");
			to->dump();
		}
	}
	if (!to)
	{
		nlwarning("<CScenario::moveNode> 'to' node with id (%s, %s) not found", instanceId2.c_str(), attrName2.c_str());
		return false;
	}


	if ( ! (to->isTable() && -1<= position2 && position2 <= static_cast<sint32>(to->getSize())) )
	{
		nlwarning("<CScenario::moveNode> Can not move node to (%s, %s, %d)", instanceId2.c_str(), attrName2.c_str(), position2);
		return false;
	}


	if (! from->canTake(position1))
	{
		nlwarning("<CScenario::moveNode> Can not move node from (%s, %s, %d)", instanceId1.c_str(), attrName1.c_str(), position1);
		return false;
	}

	CObject* removed = from->take(position1);

	if (!removed)
	{
		nlwarning("<CScenario::moveNode> Error: try to move a non-existing object from a table (%s, %s, %d)", instanceId1.c_str(), attrName1.c_str(), position1 );
		return false;
	}

	if (verboseMoveNode)
	{
		nlwarning("Removed object is : ");
		removed->dump();
	}

	if (verboseMoveNode)
	{
		nlwarning("After removal, 'from' object is  : ");
		from->dump();
	}

	bool inserted = to->insert("", removed, position2);


	if (verboseMoveNode)
	{
		nlwarning("After insertion, destination object is  : ");
		to->dump();
	}

	return inserted;
}

CObject* CScenario::getHighLevel() const { return _HighLevel;}

void CScenario::setRtData(CObject* rtScenario)
{
	_Clean = false;
	if (_BasicBricks) { delete _BasicBricks; }
	this->_BasicBricks = rtScenario;
}

CObject* CScenario::getRtData() const { return _BasicBricks;}
sint32 CScenario::getMaxId(const std::string& eid)
{
	return _InstanceMap->getMaxId(eid);
}

void CScenario::setMaxId(const std::string& eid, sint32 maxId)
{
	_InstanceMap->setMaxId(eid, maxId);

}

//----------------------------------------------------------------


void CInstanceMap::add(CObject* root)
{
	if (root->isTable())
	{
		if ( root->isString(_IdName) )
		{
			std::string instanceId = root->toString(_IdName);
			if ( _Map.find(instanceId) != _Map.end())
			{
				nlwarning("Trying to add object to the instance map but object already exist. Objec t is :");
				root->dump();
				return;
			}
			_Map[instanceId] = root;
			if (_IdName == "InstanceId")
			{
				std::string user;
				sint32 maxId;
				NLMISC::CSString str(instanceId);
				user=str.strtok("_");
				std::string tmp=str.strtok("_");
				NLMISC::fromString(tmp,maxId);
				std::map<std::string,sint32>::iterator it=_MapEids.find(user);
				if(it==_MapEids.end() || it->second<maxId)
				{
					_MapEids[user]=maxId;
				}

			}

		}
		sint32 size = root->getSize() ;
		sint32 first = 0;
		for (first = 0 ; first != size ; ++first)
		{
			CObject* value = root->getValue(first);
			add(value);
		}

	}
}


void CInstanceMap::remove(CObject* root)
{
	if (root->isTable())
	{
		if ( root->isString("InstanceId") )
		{
			std::string instanceId = root->toString(_IdName);
			std::map<std::string, CObject*>::iterator found(_Map.find(instanceId));
			if ( found == _Map.end())
			{
				nlwarning("Trying to remove object from instance map but object is not found. Objec t is :");
				root->dump();
				return;
			}
			_Map.erase(found);
		}
		sint32 size = root->getSize() ;
		sint32 first = 0;
		for (first = 0 ; first != size ; ++first)
		{
			CObject* value = root->getValue(first);
			remove(value);
		}

	}
}

void CInstanceMap::set(CObject* root)
{
	_Map.clear();
	if(root)
		add(root);
}

CObject* CInstanceMap::find (const std::string& instanceId)
{
	std::map< std::string , CObject*>::const_iterator found = _Map.find(instanceId);
	if (found != _Map.end()) { return found->second; }

	return 0;
}

sint32 CInstanceMap::getMaxId(const std::string& eid)
{
	std::map<std::string, sint32>::iterator it=_MapEids.find(eid);
	if(it!=_MapEids.end())
	{
		return it->second;
	}
	else
	{
		return 0;
	}
}


void CInstanceMap::setMaxId(const std::string& eid, sint32 maxId)
{
	std::map<std::string, sint32>::iterator it=_MapEids.find(eid);
	if(it!=_MapEids.end())
		it->second = maxId;
}


bool CScenario::isEditing() const
{
	return _SessionType == st_edit && (_Mode != 2 && _Mode != 3);
}

bool CScenario::isRunning() const
{
	return !isEditing();
}

bool CScenario::isWaiting() const
{
	return false;
}



//--------------------------------------------------------------------------------

CUserComponent::CUserComponent()
{
	UncompressedData = 0;
	UncompressedDataLength = 0;
	CompressedData = 0;
	CompressedDataLength = 0;
}

CUserComponent::CUserComponent(const std::string &filename,
	uint8* uncompressedData,  uint32 uncompressedDataLength,
	uint8* compressedData,  uint32 compressedDataLength
	)
:Filename(filename), UncompressedData(uncompressedData), UncompressedDataLength(uncompressedDataLength),
	CompressedData(compressedData), CompressedDataLength(compressedDataLength)
{
	computeMd5();
}


CUserComponent::~CUserComponent()
{
	if (UncompressedData) { delete [] UncompressedData; }
	if (CompressedData) { delete [] CompressedData; }
}

void CUserComponent::computeMd5()
{
	Md5 =  NLMISC::getMD5(UncompressedData, UncompressedDataLength);
}

void  CUserComponent::serial(NLMISC::IStream &f)
{
	f.serial(Md5);
	f.serial(Md5Id);
	f.serialEnum(ComponentType);

	f.serial(Name);
	f.serial(Description);

	f.serial(Filename);
	f.serial(CompressedDataLength);
	f.serial(UncompressedDataLength);

	if (f.isReading())
	{
		delete [] CompressedData;
		CompressedData = new uint8[CompressedDataLength];
	}

	f.serialBuffer(CompressedData, CompressedDataLength);

}

uint8* CUserComponent::getUncompressedData() const { return UncompressedData; }

uint32 CUserComponent::getUncompressedDataLength() const { return UncompressedDataLength; }

std::string getBehavior(const std::string& emoteId);


CEmoteBehavior::CEmoteBehavior()
{
}

std::string CEmoteBehavior::get(const std::string& emoteId) const
{
	if (_EmotesMap.empty()) { load(); } //lazy initialization
	std::map<std::string, std::string>::const_iterator found( _EmotesMap.find( emoteId));
	if (found == _EmotesMap.end()) return "";
	return found->second;
}

void CEmoteBehavior::load() const
{

		_EmotesMap["Absentminded"] =  "afk" ;
		_EmotesMap["Adventurous"] =  "impatient" ;
		_EmotesMap["Aggressive"] =  "roar" ;
		_EmotesMap["Agree"] =  "agree" ;
		_EmotesMap["Alert"] =  "alert" ;
		_EmotesMap["Altruist"] =  "smile" ;
		_EmotesMap["Amazed"] =  "cheer" ;
		_EmotesMap["Ambivalent"] =  "apologize" ;
		_EmotesMap["Amused"] =  "laugh" ;
		_EmotesMap["Angry"] =  "angry" ;
		_EmotesMap["Annoyed"] =  "angry" ;
		_EmotesMap["Apathetic"] =  "afk" ;
		_EmotesMap["Approve"] =  "agree" ;
		_EmotesMap["Arrogant"] =  "giggle" ;
		_EmotesMap["Ashamed"] =  "ashamed_disgusted_desillusioned_loathing_pitying" ;
		_EmotesMap["Belligerent"] =  "gesture" ;
		_EmotesMap["Bitter"] =  "unhappy" ;
		_EmotesMap["Bloodthirsty"] =  "gesture" ;
		_EmotesMap["Bored"] =  "afk" ;
		_EmotesMap["Bow"] =  "bow" ;
		_EmotesMap["Brave"] =  "brave_great" ;
		_EmotesMap["Bubbly"] =  "laugh" ;
		_EmotesMap["Burp"] =  "burp" ;
		_EmotesMap["Calm"] =  "calm" ;
		_EmotesMap["Calmdown"] =  "calm" ;
		_EmotesMap["Careful"] =  "calm" ;
		_EmotesMap["Careless"] =  "relieved_sigh" ;
		_EmotesMap["Casual"] =  "relaxed" ;
		_EmotesMap["Chaotic"] =  "unhappy" ;
		_EmotesMap["Cheer"] =  "cheer" ;
		_EmotesMap["Clinical"] =  "follow" ;
		_EmotesMap["Cold"] =  "cold_contemptuous_disdainful_haughty_megalomaniac_obnoxious" ;
		_EmotesMap["Compassionate"] =  "smile" ;
		_EmotesMap["Condescending"] =  "smile" ;
		_EmotesMap["Confident"] =  "smile" ;
		_EmotesMap["Confused"] =  "unhappy" ;
		_EmotesMap["Contemptuous"] =  "cold_contemptuous_disdainful_haughty_megalomaniac_obnoxious" ;
		_EmotesMap["Content"] =  "smile" ;
		_EmotesMap["Courageous"] =  "roar" ;
		_EmotesMap["Courtly"] =  "smile" ;
		_EmotesMap["Coward"] =  "blush" ;
		_EmotesMap["Crazy"] =  "dance" ;
		_EmotesMap["Crude"] =  "gesture" ;
		_EmotesMap["Cruel"] =  "giggle" ;
		_EmotesMap["Curious"] =  "curious" ;
		_EmotesMap["Cynical"] =  "smile" ;
		_EmotesMap["Dainty"] =  "apologize" ;
		_EmotesMap["Dance"] =  "dance" ;
		_EmotesMap["Defensive"] =  "calm" ;
		_EmotesMap["Depressed"] =  "sad" ;
		_EmotesMap["Desire"] =  "kiss" ;
		_EmotesMap["Despaired"] =  "cry" ;
		_EmotesMap["Destructive"] =  "angry" ;
		_EmotesMap["Die"] =  "" ;
		_EmotesMap["Dignified"] =  "relaxed" ;
		_EmotesMap["Diplomatic"] =  "bow" ;
		_EmotesMap["Disappointed"] =  "unhappy" ;
		_EmotesMap["Discreet"] =  "discreet_hardsilence" ;
		_EmotesMap["Disdainful"] =  "cold_contemptuous_disdainful_haughty_megalomaniac_obnoxious" ;
		_EmotesMap["Disgruntled"] =  "roar" ;
		_EmotesMap["Disgusted"] =  "ashamed_disgusted_desillusioned_loathing_pitying" ;
		_EmotesMap["Disillusioned"] =  "ashamed_disgusted_desillusioned_loathing_pitying" ;
		_EmotesMap["Dismayed"] =  "sad" ;
		_EmotesMap["Disoriented"] =  "puzzled_thoughtful_troubled" ;
		_EmotesMap["Distracted"] =  "afk" ;
		_EmotesMap["Doubtful"] =  "doubtful" ;
		_EmotesMap["Dramatic"] =  "dramatic" ;
		_EmotesMap["Dreamy"] =  "afk" ;
		_EmotesMap["Drunk"] =  "drunk" ;
		_EmotesMap["Dutiful"] =  "kneel" ;
		_EmotesMap["Eager"] =  "impatient" ;
		_EmotesMap["Earnest"] =  "firm" ;
		_EmotesMap["Ecstatic"] =  "applaud" ;
		_EmotesMap["Egoistic"] =  "pompous" ;
		_EmotesMap["Embarrassed"] =  "blush" ;
		_EmotesMap["Emotional"] =  "" ;
		_EmotesMap["Emotionless"] =  "discreet_hardsilence" ;
		_EmotesMap["Emphatic"] =  "cheer" ;
		_EmotesMap["Encouraging"] =  "cheer" ;
		_EmotesMap["Enraged"] =  "roar" ;
		_EmotesMap["Enthusiastic"] =  "dance" ;
		_EmotesMap["Envious"] =  "" ;
		_EmotesMap["Evil"] =  "gesture" ;
		_EmotesMap["Excited"] =  "impatient" ;
		_EmotesMap["Exercise"] =  "exercise" ;
		_EmotesMap["Exhausted"] =  "calm" ;
		_EmotesMap["Exuberant"] =  "lol" ;
		_EmotesMap["Faithful"] =  "" ;
		_EmotesMap["Fanatical"] =  "" ;
		_EmotesMap["Fastidious"] =  "" ;
		_EmotesMap["FBT"] =  "FBT" ;
		_EmotesMap["Fearful"] =  "fearful_insecure_nervous_panic_scared" ;
		_EmotesMap["Firm"] =  "firm" ;
		_EmotesMap["Forgive"] =  "forgive" ;
		_EmotesMap["Fraternal"] =  "cheer" ;
		_EmotesMap["Friendly"] =  "cheer" ;
		_EmotesMap["Frustrated"] =  "sad" ;
		_EmotesMap["Funny"] =  "laugh" ;
		_EmotesMap["Generous"] =  "smile" ;
		_EmotesMap["Gimme5"] =  "" ;
		_EmotesMap["Gloomy"] =  "sad" ;
		_EmotesMap["Goofy"] =  "" ;
		_EmotesMap["Great"] =  "brave_great" ;
		_EmotesMap["Grin"] =  "smile" ;
		_EmotesMap["Grumpy"] =  "unhappy" ;
		_EmotesMap["Guilty"] =  "guilty" ;
		_EmotesMap["Happy"] =  "smile" ;
		_EmotesMap["Hardsilence"] =  "discreet_hardsilence" ;
		_EmotesMap["Haughty"] =  "cold_contemptuous_disdainful_haughty_megalomaniac_obnoxious" ;
		_EmotesMap["Helpful"] =  "" ;
		_EmotesMap["Heroic"] =  "heroic" ;
		_EmotesMap["Hiha"] =  "wave" ;
		_EmotesMap["Honest"] =  "" ;
		_EmotesMap["Hopeful"] =  "" ;
		_EmotesMap["Hopeless"] =  "cry" ;
		_EmotesMap["Humble"] =  "bow" ;
		_EmotesMap["Hungry"] =  "hungry" ;
		_EmotesMap["Hurried"] =  "impatient" ;
		_EmotesMap["Hurry"] =  "impatient" ;
		_EmotesMap["Hysterical"] =  "roar" ;
		_EmotesMap["Imploring"] =  "kneel" ;
		_EmotesMap["Indifferent"] =  "indifferent_neutral_noclue_resigned" ;
		_EmotesMap["Indignant"] =  "shocked" ;
		_EmotesMap["Indulgent"] =  "" ;
		_EmotesMap["Innocent"] =  "sincerely" ;
		_EmotesMap["Insecure"] =  "fearful_insecure_nervous_panic_scared" ;
		_EmotesMap["Interested"] =  "interested" ;
		_EmotesMap["Jealous"] =  "angry" ;
		_EmotesMap["Joyful"] =  "laugh" ;
		_EmotesMap["Kind"] =  "smile" ;
		_EmotesMap["Lazy"] =  "none" ;
		_EmotesMap["Loathing"] =  "loathing" ;
		_EmotesMap["Logical"] =  "" ;
		_EmotesMap["Lonely"] =  "sad" ;
		_EmotesMap["Loud"] =  "pompous" ;
		_EmotesMap["Love"] =  "kiss" ;
		_EmotesMap["Loyal"] =  "agree" ;
		_EmotesMap["Lustful"] =  "" ;
		_EmotesMap["Malevolent"] =  "cold_contemptuous_disdainful_haughty_megalomaniac_obnoxious" ;
		_EmotesMap["Malicious"] =  "giggle" ;
		_EmotesMap["Mean"] =  "" ;
		_EmotesMap["Megalomaniac"] =  "cold_contemptuous_disdainful_haughty_megalomaniac_obnoxious" ;
		_EmotesMap["Merciful"] =  "" ;
		_EmotesMap["Mischievous"] =  "smile" ;
		_EmotesMap["Mocking"] =  "giggle" ;
		_EmotesMap["Nervous"] =  "fearful_insecure_nervous_panic_scared" ;
		_EmotesMap["Neutral"] =  "indifferent_neutral_noclue_resigned" ;
		_EmotesMap["Nice"] =  "smile" ;
		_EmotesMap["Noclue"] =  "indifferent_neutral_noclue_resigned" ;
		_EmotesMap["None"] =  "none" ;
		_EmotesMap["Nostalgic"] =  "" ;
		_EmotesMap["Obnoxious"] =  "obnoxious" ;
		_EmotesMap["Obscure"] =  "" ;
		_EmotesMap["Obsessed"] =  "" ;
		_EmotesMap["Offended"] =  "angry" ;
		_EmotesMap["Optimistic"] =  "smile" ;
		_EmotesMap["Over"] =  "over" ;
		_EmotesMap["Pacific"] =  "cheer" ;
		_EmotesMap["Painful"] =  "cry" ;
		_EmotesMap["Panick"] =  "fearful_insecure_nervous_panic_scared" ;
		_EmotesMap["Patient"] =  "calm" ;
		_EmotesMap["Patriotic"] =  "" ;
		_EmotesMap["Pedantic"] =  "" ;
		_EmotesMap["Perturbed"] =  "puzzled_thoughtful_troubled" ;
		_EmotesMap["Pessimistic"] =  "unhappy" ;
		_EmotesMap["Petulant"] =  "petulant" ;
		_EmotesMap["Philosophical"] =  "" ;
		_EmotesMap["Pitying"] =  "pitying" ;
		_EmotesMap["Playful"] =  "playful" ;
		_EmotesMap["Pleased"] =  "smile" ;
		_EmotesMap["Point"] =  "point" ;
		_EmotesMap["Pointback"] =  "pointback" ;
		_EmotesMap["Pointfront"] =  "point" ;
		_EmotesMap["Pointleft"] =  "pointleft" ;
		_EmotesMap["Pointright"] =  "pointright" ;
		_EmotesMap["Polite"] =  "smile" ;
		_EmotesMap["Pompous"] =  "pompous" ;
		_EmotesMap["Powerful"] =  "" ;
		_EmotesMap["Praying"] =  "praying" ;
		_EmotesMap["Proud"] =  "" ;
		_EmotesMap["Provocative"] =  "point" ;
		_EmotesMap["Puzzled"] =  "puzzled_thoughtful_troubled" ;
		_EmotesMap["Quiet"] =  "calm" ;
		_EmotesMap["Ready"] =  "impatient" ;
		_EmotesMap["Reassured"] =  "" ;
		_EmotesMap["Rebellious"] =  "" ;
		_EmotesMap["Reckless"] =  "impatient" ;
		_EmotesMap["Regretful"] =  "apologize" ;
		_EmotesMap["Relaxed"] =  "relaxed" ;
		_EmotesMap["Relieved"] =  "relieved" ;
		_EmotesMap["Reluctant"] =  "sad" ;
		_EmotesMap["Remorseful"] =  "sad" ;
		_EmotesMap["Resigned"] =  "indifferent_neutral_noclue_resigned" ;
		_EmotesMap["Respectful"] =  "bow" ;
		_EmotesMap["Revengeful"] =  "revengeful_spitful" ;
		_EmotesMap["Rice"] =  "rice" ;
		_EmotesMap["Ridicule"] =  "giggle" ;
		_EmotesMap["Righteous"] =  "righteous" ;
		_EmotesMap["Romantic"] =  "kiss" ;
		_EmotesMap["Rude"] =  "gesture" ;
		_EmotesMap["Sad"] =  "sad" ;
		_EmotesMap["Sarcastic"] =  "smile" ;
		_EmotesMap["Scared"] =  "fearful_insecure_nervous_panic_scared" ;
		_EmotesMap["Scolding"] =  "angry" ;
		_EmotesMap["Sedate"] =  "calm" ;
		_EmotesMap["Selfish"] =  "" ;
		_EmotesMap["Serious"] =  "serious" ;
		_EmotesMap["Shameless"] =  "giggle" ;
		_EmotesMap["Sheepish"] =  "blush" ;
		_EmotesMap["Shifty"] =  "shifty" ;
		_EmotesMap["Shocked"] =  "shocked" ;
		_EmotesMap["Shutup"] =  "angry" ;
		_EmotesMap["Shy"] =  "blush" ;
		_EmotesMap["Sigh"] =  "sigh" ;
		_EmotesMap["Silence"] =  "calm" ;
		_EmotesMap["Silly"] =  "silly" ;
		_EmotesMap["Sincerely"] =  "sincerely" ;
		_EmotesMap["Sleepy"] =  "sleepy_yawn_tired" ;
		_EmotesMap["Sly"] =  "smile" ;
		_EmotesMap["Smack"] =  "smack" ;
		_EmotesMap["Smug"] =  "smile" ;
		_EmotesMap["Sorry"] =  "apologize" ;
		_EmotesMap["Spiteful"] =  "" ;
		_EmotesMap["Squeamish"] =  "squeamish" ;
		_EmotesMap["Stop"] =  "disagree" ;
		_EmotesMap["Strong"] =  "roar" ;
		_EmotesMap["Stubborn"] =  "" ;
		_EmotesMap["Suffering"] =  "cry" ;
		_EmotesMap["Surprised"] =  "surprised" ;
		_EmotesMap["Suspicious"] =  "suspicious" ;
		_EmotesMap["Taunting"] =  "gesture" ;
		_EmotesMap["Terrified"] =  "unhappy" ;
		_EmotesMap["Thankful"] =  "thank" ;
		_EmotesMap["Thirsty"] =  "thirsty" ;
		_EmotesMap["Thoughtful"] =  "puzzled_thoughtful_troubled";
		_EmotesMap["Tired"] =  "sleepy_yawn_tired" ;
		_EmotesMap["Tolerant"] =  "agree" ;
		_EmotesMap["Troubled"] =  "" ;
		_EmotesMap["Uncertain"] =  "apologize" ;
		_EmotesMap["Unhappy"] =  "unhappy" ;
		_EmotesMap["Unwilling"] =  "unwilling" ;
		_EmotesMap["Vengeful"] =  "gesture" ;
		_EmotesMap["Wait"] =  "wait" ;
		_EmotesMap["Warm"] =  "smile" ;
		_EmotesMap["Wary"] =  "" ;
		_EmotesMap["Wave"] =  "wave" ;
		_EmotesMap["Whine"] =  "cry" ;
		_EmotesMap["Wicked"] =  "wicked" ;
		_EmotesMap["Wise"] =  "" ;
		_EmotesMap["Wistful"] =  "" ;
		_EmotesMap["Worried"] =  "puzzled_thoughtful_troubled" ;
		_EmotesMap["Wounded"] =  "" ;
		_EmotesMap["Yawn"] =  "sleepy_yawn_tired" ;
		_EmotesMap["Youandme"] =  "youandme";
}






bool CScenarioValidator::setScenarioToLoad( const std::string& filename, CScenarioValidator::TValues& values, std::string& md5,  std::string& signature, bool checkMD5)
{

	values.clear();

	_Filename = filename;
	_Values.clear();
	// open our input file
	NLMISC::CIFile inf;
	if (!inf.open(_Filename, true) )
	{
		nlwarning("Can't load scenario %s", _Filename.c_str());
		return false;
	}

	try
	{
		static const char * header = "---- Header\n";
		static const char * slashheader = "---- /Header\n\n";
		static const char * comment = "-- ";

		static const uint headerLen = (uint)strlen(header);
		static const uint slasheaderLen = (uint)strlen(slashheader);
		static const uint commentLen = (uint)strlen(comment);


		NLMISC::CSString tmp;
		tmp.resize( inf.getFileSize() );
		inf.serialBuffer((uint8*)&tmp[0], (uint)tmp.size());
		_ScenarioBody = tmp.replace("\r\n", "\n");





		// Scenario without header
		if (_ScenarioBody.size() < headerLen ||_ScenarioBody.substr(0, headerLen) != header )
		{
			md5 = "";
			signature = "";
			inf.close();
			return true;
		}

		std::string::size_type endHeader = _ScenarioBody.find(slashheader, headerLen);
		if (endHeader == std::string::npos ) {  inf.close(); return false; }
		std::string::size_type startHeader = headerLen;

		std::vector<std::string> lines;
		NLMISC::splitString( _ScenarioBody.substr(startHeader, endHeader - startHeader), "'\n", lines);
		std::vector<std::string>::const_iterator firstLine(lines.begin()), lastLine(lines.end());
		std::vector<std::string> result;

		for (; firstLine != lastLine ; ++firstLine)
		{
			result.clear();
			NLMISC::splitString(*firstLine, " = '", result);
			if (result.size() == 1)
			{
				result.push_back("");
			}
			if (result.size() == 2)
			{
				if (result[0].find(comment) != std::string::npos)
				{
					//>result[1]"\\n" => "\n"
					NLMISC::CSString tmp = result[1];
					tmp = tmp.replace("\\n", "\n");

					values.push_back( std::make_pair( result[0].substr(commentLen), tmp));
				}
			}
		}

		if (values.size() >=2
			&& values[0].first == "Version"
			&& values[1].first == "Signature"
			&& values[2].first == "HeaderMD5" && values[3].first =="BodyMD5")
		{
			std::string headerBodyMd5;
			std::string::size_type subHeader = _ScenarioBody.find("-- BodyMD5", startHeader);
			if (checkMD5)
			{
				std::string md5Id1 = NLMISC::getMD5((uint8*)(_ScenarioBody.data() + subHeader), (uint32)(endHeader - subHeader)).toString();
				if (values[2].second != md5Id1 )
				{
					return false;
				}
				std::string md5Id2 = NLMISC::getMD5((uint8*)(_ScenarioBody.data() + endHeader + slasheaderLen), (uint32)(_ScenarioBody.size() - (endHeader + slasheaderLen))).toString();
				if (values[3].second != md5Id2)
				{
					return false;
				}

			}
			md5 = values[2].second;
			signature = values[1].second;

		}


	}
	catch(...)
	{
		_Values = values;
		nlwarning("Can't load scenario %s", _Filename.c_str());
		return false;
	}

	_Values = values;
	// close the file
	inf.close();
	return true;

}


void CScenarioValidator::applyLoad(std::string& filename, std::string& body, CScenarioValidator::TValues& values)
{
	filename = _Filename;
	body = _ScenarioBody;
	values = _Values;
	if (_LoadCb)
	{
		_LoadCb->doOperation(filename, body, values);
	}
}

bool CScenarioValidator::setScenarioToSave(const std::string& filename, CObject* scenario, const CScenarioValidator::TValues& values, std::string& headerMD5)
{
	_Filename = filename;
	//std::ostringstream out2;
	//out2.str("");
	std::string out2;

	if (!scenario)
	{
		return false;
	}

	//out2 <<"scenario = "<< *scenario ;
	out2 += "scenario = ";
	scenario->serialize(out2);

	_ScenarioBody = out2;
	{
		NLMISC::CHashKeyMD5 md5Id = NLMISC::getMD5((uint8*)_ScenarioBody.data(),(uint32) _ScenarioBody.size());
		_BodyMd5 = md5Id.toString().c_str();
	}


	out2.clear();
	//out2.str("");
	out2 += NLMISC::toString("-- BodyMD5 = '%s'\n", _BodyMd5.c_str() );
	TValues::const_iterator first(values.begin()), last(values.end());
	for (; first != last; ++first)
	{
		//>first->second.c_str()) "\n" => "\\n"
		NLMISC::CSString tmp = first->second.c_str();
		tmp = tmp.replace("\n", "\\n");

		out2 += NLMISC::toString("-- %s = '%s'\n", first->first.c_str(), tmp.c_str());
	}

	_HeaderBody =out2;
	std::string headerBodyMd5;
	{
		NLMISC::CHashKeyMD5 md5Id = NLMISC::getMD5((uint8*)_HeaderBody.data(), (uint32)_HeaderBody.size());
		_HeaderMd5 = md5Id.toString().c_str();
		headerMD5 = _HeaderMd5;
	}
	return true;
}

std::string CScenarioValidator::AutoSaveSignature;

bool CScenarioValidator::applySave(const std::string& signature)
{
	NLMISC::COFile out;

	std::string::size_type index = NLMISC::CFile::getLastSeparator(_Filename);
	if (index != std::string::npos)
	{
		std::string dir = _Filename.substr(0, index);
		if (NLMISC::CFile::isExists(dir) == false )
		{
			if (NLMISC::CFile::createDirectory(dir) == false)
			{
				return false;
			}
		}
	}

	if (!out.open(_Filename, false, false, true))
	{

		nlwarning("Can't save: the file %s can not be saved.", _Filename.c_str());
		return false;
	}

	try
	{


		//std::stringstream out2;

		//out2.str("");
		//out2 << "---- Header\n";
		//out2 << NLMISC::toString("-- Version = '%u'\n", 1 );
		//out2 << NLMISC::toString("-- Signature = '%s'\n", signature.c_str() );
		//out2 << NLMISC::toString("-- HeaderMD5 = '%s'\n", _HeaderMd5.c_str() );
		//out2 << _HeaderBody;
		//out2 << "---- /Header\n\n";
		//out2 << _ScenarioBody;

		std::string out2;

		out2 += "---- Header\n";
		out2 += NLMISC::toString("-- Version = '%u'\n", 1 );
		out2 += NLMISC::toString("-- Signature = '%s'\n", signature.c_str() );
		out2 += NLMISC::toString("-- HeaderMD5 = '%s'\n", _HeaderMd5.c_str() );
		out2 += _HeaderBody;
		out2 += "---- /Header\n\n";
		out2 += _ScenarioBody;

		out.serialBuffer((uint8*)out2.c_str(), (uint)out2.size());
		if (_Filename != std::string("save/r2_buffer.dat") )
		{
			nlinfo("Scenario %s saved", _Filename.c_str());
		}
		else
		{
			AutoSaveSignature = signature;
		}

		out.close();

		return true;
	}
	catch (...)
	{
		nlwarning("Can't save: the file %s can not be saved.", _Filename.c_str());
		out.close();

		return false;
	}

	return true;

}

CScenarioValidator::CScenarioValidator(CScenarioValidatorLoadSuccededCallback* loadCb ):_LoadCb(loadCb){}

CScenarioValidator::~CScenarioValidator(){ delete _LoadCb; }


/**********************************************************/
/******* USER COMPONENT MD5 MANAGEMENT ********************/
/**********************************************************/
bool CUserComponentValidator::setUserComponentToLoad( const std::string& filename, CScenarioValidator::TValues& values, std::string& md5,  std::string& signature, bool checkMD5)
{

	values.clear();

	_Filename = filename;
	_Values.clear();
	// open our input file
	NLMISC::CIFile inf;
	if (!inf.open(_Filename, true) )
	{
		nlwarning("Can't load usercomponent in file %s", _Filename.c_str());
		return false;
	}

	try
	{
		static const char * header = "---- Header\n";
		static const char * slashheader = "---- /Header\n\n";
		static const char * comment = "-- ";

		static const uint headerLen = (uint)strlen(header);
		static const uint slasheaderLen = (uint)strlen(slashheader);
		static const uint commentLen = (uint)strlen(comment);


		NLMISC::CSString tmp;
		tmp.resize( inf.getFileSize() );
		inf.serialBuffer((uint8*)&tmp[0], (uint)tmp.size());
		_UserComponentBody = tmp.replace("\r\n", "\n");

		// Scenario without header
		if (_UserComponentBody.size() < headerLen ||_UserComponentBody.substr(0, headerLen) != header )
		{
			md5 = "";
			signature = "";
			inf.close();
			return true;
		}

		std::string::size_type endHeader = _UserComponentBody.find(slashheader, headerLen);
		if (endHeader == std::string::npos ) {  inf.close(); return false; }
		std::string::size_type startHeader = headerLen;

		std::vector<std::string> lines;
		NLMISC::splitString( _UserComponentBody.substr(startHeader, endHeader - startHeader), "'\n", lines);
		std::vector<std::string>::const_iterator firstLine(lines.begin()), lastLine(lines.end());
		std::vector<std::string> result;

		for (; firstLine != lastLine ; ++firstLine)
		{
			result.clear();
			NLMISC::splitString(*firstLine, " = '", result);
			if (result.size() == 1)
			{
				result.push_back("");
			}
			if (result.size() == 2)
			{
				if (result[0].find(comment) != std::string::npos)
				{
					//>result[1]"\\n" => "\n"
					NLMISC::CSString tmp = result[1];
					tmp = tmp.replace("\\n", "\n");

					values.push_back( std::make_pair( result[0].substr(commentLen), tmp));
				}
			}
		}

		if (values.size() >=2
			&& values[0].first == "Version"
			&& values[1].first == "Signature"
			&& values[2].first == "HeaderMD5" && values[3].first =="BodyMD5")
		{
			std::string headerBodyMd5;
			std::string::size_type subHeader = _UserComponentBody.find("-- BodyMD5", startHeader);
			if (checkMD5)
			{
				std::string md5Id1 = NLMISC::getMD5((uint8*)(_UserComponentBody.data() + subHeader), (uint32)(endHeader - subHeader)).toString();
				if (values[2].second != md5Id1 )
				{
					return false;
				}
				std::string md5Id2 = NLMISC::getMD5((uint8*)(_UserComponentBody.data() + endHeader + slasheaderLen), (uint32)(_UserComponentBody.size() - (endHeader + slasheaderLen))).toString();
				if (values[3].second != md5Id2)
				{
					return false;
				}

			}
			md5 = values[2].second;
			signature = values[1].second;

		}


	}
	catch(...)
	{
		_Values = values;
		nlwarning("Can't load UserComponent defined in %s", _Filename.c_str());
		return false;
	}

	_Values = values;
	// close the file
	inf.close();
	return true;

}


void CUserComponentValidator::applyLoad(std::string& filename, std::string& body, CScenarioValidator::TValues& values)
{
	filename = _Filename;
	body = _UserComponentBody;
	values = _Values;
	if (_LoadCb)
	{
		_LoadCb->doOperation(filename, body, values);
	}
}

bool CUserComponentValidator::setUserComponentToSave(const std::string& filename, const CUserComponentValidator::TValues& values, std::string& headerMD5, const std::string &body)
{
	_Filename = filename;
//	std::ostringstream out2;
//	out2.str(body);

//	_UserComponentBody =out2.str();

	_UserComponentBody = body;

	{
		NLMISC::CHashKeyMD5 md5Id = NLMISC::getMD5((uint8*)_UserComponentBody.data(), (uint32)_UserComponentBody.size());
		_BodyMd5 = md5Id.toString().c_str();
	}

//	out2.str("");
//	out2 << NLMISC::toString("-- BodyMD5 = '%s'\n", _BodyMd5.c_str() );
//	TValues::const_iterator first(values.begin()), last(values.end());
//	for (; first != last; ++first)
//	{
		//>first->second.c_str()) "\n" => "\\n"
//		NLMISC::CSString tmp = first->second.c_str();
//		tmp = tmp.replace("\n", "\\n");

//		out2 << NLMISC::toString("-- %s = '%s'\n", first->first.c_str(), tmp.c_str());
//	}


//	_HeaderBody =out2.str();


	_HeaderBody = NLMISC::toString("-- BodyMD5 = '%s'\n", _BodyMd5.c_str() );

	TValues::const_iterator first(values.begin()), last(values.end());
	for (; first != last; ++first)
	{
		//>first->second.c_str()) "\n" => "\\n"
		NLMISC::CSString tmp = first->second.c_str();
		tmp = tmp.replace("\n", "\\n");

		_HeaderBody += NLMISC::toString("-- %s = '%s'\n", first->first.c_str(), tmp.c_str());
	}


	std::string headerBodyMd5;
	{
		NLMISC::CHashKeyMD5 md5Id = NLMISC::getMD5((uint8*)_HeaderBody.data(), (uint32)_HeaderBody.size());
		_HeaderMd5 = md5Id.toString().c_str();
		headerMD5 = _HeaderMd5;
	}

	return true;
}

std::string CUserComponentValidator::AutoSaveSignature;

bool CUserComponentValidator::applySave(const std::string& signature)
{
	NLMISC::COFile out;

	std::string::size_type index = NLMISC::CFile::getLastSeparator(_Filename);
	if (index != std::string::npos)
	{
		std::string dir = _Filename.substr(0, index);
		if (NLMISC::CFile::isExists(dir) == false )
		{
			if (NLMISC::CFile::createDirectory(dir) == false)
			{
				return false;
			}
		}
	}

	if (!out.open(_Filename, false, false, true))
	{

		nlwarning("Can't save: the file %s can not be saved.", _Filename.c_str());
		return false;
	}

	try
	{


		//std::stringstream out2;

		//out2.str("");
		//out2 << "---- Header\n";
		//out2 << NLMISC::toString("-- Version = '%u'\n", 1 );
		//out2 << NLMISC::toString("-- Signature = '%s'\n", signature.c_str() );
		//out2 << NLMISC::toString("-- HeaderMD5 = '%s'\n", _HeaderMd5.c_str() );
		//out2 << _HeaderBody;
		//out2 << "---- /Header\n\n";
		//out2 << _UserComponentBody;


		//out.serialBuffer((uint8*)out2.str().c_str(),out2.str().size());

		std::string out2;

		out2 += "---- Header\n";
		out2 += NLMISC::toString("-- Version = '%u'\n", 1 );
		out2 += NLMISC::toString("-- Signature = '%s'\n", signature.c_str() );
		out2 += NLMISC::toString("-- HeaderMD5 = '%s'\n", _HeaderMd5.c_str() );
		out2 += _HeaderBody;
		out2 += "---- /Header\n\n";
		out2 += _UserComponentBody;


		out.serialBuffer((uint8*)out2.c_str(),(uint)out2.size());
		if (_Filename != std::string("save/r2_buffer.dat") )
		{
			nlinfo("UserComponent %s saved", _Filename.c_str());
		}

		out.close();

		return true;
	}
	catch (...)
	{
		nlwarning("Can't save user component: the file %s can not be saved.", _Filename.c_str());
		out.close();

		return false;
	}

	return true;

}

CUserComponentValidator::CUserComponentValidator(CUserComponentValidatorLoadSuccededCallback* loadCb ):_LoadCb(loadCb){}

CUserComponentValidator::~CUserComponentValidator(){ delete _LoadCb; }
/********************************************************************/
