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
#include "sbrick_manager.h"
#include "interface_manager.h"
#include "../sheet_manager.h"
#include "dbgroup_build_phrase.h"
#include "skill_manager.h"


using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


CSBrickManager* CSBrickManager::_Instance = NULL;

// ***************************************************************************
void CSBrickManager::releaseInstance()
{
	if( _Instance )
	{
		delete _Instance;
		_Instance = NULL;
	}
}

// ***************************************************************************
CSBrickManager::CSBrickManager() : _NbFamily(0), _SabrinaCom(&_BrickContainer)
{
	_BrickFamilyObs.Owner= this;
}

// ***************************************************************************
void CSBrickManager::init()
{

	// Read the Bricks from the SheetMngr.
	const CSheetManager::TEntitySheetMap &sheetMap=  SheetMngr.getSheets();
	for(CSheetManager::TEntitySheetMap::const_iterator it= sheetMap.begin(); it!=sheetMap.end(); it++)
	{
		// it's a brick?
		CSBrickSheet	*br= dynamic_cast<CSBrickSheet*>(it->second.EntitySheet);
		if(br)
		{
			// ok, add it
			_Bricks[it->first]= br;
		}
	}

	// Process Bricks
	if (_Bricks.empty()) return;

	map<CSheetId,CSBrickSheet*>::iterator itb;

	//build the vector of family bit fields, and the vector of existing bricks
	for (itb = _Bricks.begin();itb != _Bricks.end();itb++)
	{
		//resize our vectors if necessary
		if ( itb->second->BrickFamily >= (sint32)_NbFamily )
		{
			_SheetsByFamilies.resize(itb->second->BrickFamily+1);
			_FamiliesBits.resize(itb->second->BrickFamily+1, 0);
			_NbBricksPerFamily.resize(itb->second->BrickFamily+1, 0);

			_NbFamily = itb->second->BrickFamily+1;
		}
	}

	// Since _SheetsByFamilies is a vector of vector, avoid long reallocation by building it in 2 pass
	for (itb = _Bricks.begin();itb != _Bricks.end();itb++)
	{
		//resize our vectors if necessary
		//the index in familly must be decremented because the values start at 1 in the sheets
		if (itb->second->IndexInFamily<1)
		{
			nlwarning("CSBrickManager::CSBrickManager(): Reading file: %s: IndexInFamily==0 but should be >=1 - entry ignored",itb->first.toString().c_str());
			continue;
		}
		if (_NbBricksPerFamily[itb->second->BrickFamily] < (uint)(itb->second->IndexInFamily) )
		{
			_SheetsByFamilies[itb->second->BrickFamily].resize(itb->second->IndexInFamily);
			_NbBricksPerFamily[itb->second->BrickFamily]=itb->second->IndexInFamily;
		}
		_SheetsByFamilies[itb->second->BrickFamily][itb->second->IndexInFamily-1] = itb->first;
	}

	// check brick content for client.
	checkBricks();

	// make root list
	makeRoots();

	// Make interface bricks
	makeVisualBrickForSkill();

	// compile brick properties
	compileBrickProperties();

	// see getInterfaceRemoveBrick
	_InterfaceRemoveBrick.buildSheetId("big_remove.sbrick");
}

// ***************************************************************************
void CSBrickManager::initInGame()
{
	CInterfaceManager* pIM = CInterfaceManager::getInstance();

	// shortcut to the node
	char buf[100];
	for(uint i=0;i<_FamiliesBits.size();i++)
	{
		//get the known brick entries in the database
		sprintf(buf,"SERVER:BRICK_FAMILY:%d:BRICKS",i);
		_FamiliesBits[i] = NLGUI::CDBManager::getInstance()->getDbProp(buf);
	}

	// Add a branch observer on brick family
	NLGUI::CDBManager::getInstance()->addBranchObserver( "SERVER:BRICK_FAMILY", &_BrickFamilyObs);
}

// ***************************************************************************
void CSBrickManager::uninitInGame()
{
	CInterfaceManager* pIM = CInterfaceManager::getInstance();

	// remove shortcuts to the node
	for(uint i=0;i<_FamiliesBits.size();i++)
	{
		_FamiliesBits[i] = NULL;
	}

	// remove branch observer on brick family
	CCDBNodeBranch	*branch= NLGUI::CDBManager::getInstance()->getDbBranch("SERVER:BRICK_FAMILY");
	if(branch)
		branch->removeBranchObserver(&_BrickFamilyObs);
}

// ***************************************************************************
CSBrickManager::~CSBrickManager()
{
}


// ***************************************************************************
CSheetId CSBrickManager::getBrickSheet(uint family, uint index) const
{
	if (family >= _NbFamily)
		return NLMISC::CSheetId(0);
	if ( index >= _NbBricksPerFamily[family] )
		return NLMISC::CSheetId(0);
	return _SheetsByFamilies[family][index];
}


// ***************************************************************************
sint64 CSBrickManager::getKnownBrickBitField(uint family) const
{
	if (family < _NbFamily && _FamiliesBits[family])
		return _FamiliesBits[family]->getValue64();
	return 0;
}

// ***************************************************************************
CCDBNodeLeaf*	CSBrickManager::getKnownBrickBitFieldDB(uint family) const
{
	if (family < _NbFamily && _FamiliesBits[family])
		return _FamiliesBits[family];
	return NULL;
}



// ***************************************************************************
void			CSBrickManager::makeRoots()
{
	_Roots.clear();

	map<CSheetId,CSBrickSheet*>::iterator it = _Bricks.begin();

	while (it != _Bricks.end())
	{
		const CSheetId &rSheet = it->first;
		const CSBrickSheet &rBR = *it->second;

		// List only the Roots
		if ( !rBR.isRoot() )
		{ it++; continue; }

		_Roots.push_back(rSheet);

		it++;
	}

}

// ***************************************************************************
const std::vector<NLMISC::CSheetId>		&CSBrickManager::getFamilyBricks(uint family) const
{
	static	std::vector<NLMISC::CSheetId>	empty;

	if (family >= _NbFamily)
		return empty;
	return _SheetsByFamilies[family];
}

// ***************************************************************************
void			CSBrickManager::checkBricks()
{
	map<CSheetId,CSBrickSheet*>::iterator it = _Bricks.begin();

	while (it != _Bricks.end())
	{
		CSBrickSheet &rBR = *it->second;

		if(rBR.ParameterFamilies.size()>CDBGroupBuildPhrase::MaxParam)
		{
			nlwarning("The Sheet %s has too many parameters for Client Composition: %d/%d",
				rBR.Id.toString().c_str(), rBR.ParameterFamilies.size(), CDBGroupBuildPhrase::MaxParam);

			// reset them... don't crahs client, but won't work.
			rBR.ParameterFamilies.clear();
		}

		it++;
	}
}

// ***************************************************************************
sint32		CSBrickManager::CBrickContainer::getSabrinaCost(NLMISC::CSheetId id) const
{
	// get the true container
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*brick= pBM->getBrick(id);
	if(brick)
	{
		return brick->SabrinaCost;
	}
	else
	{
		return 0;
	}
}

// ***************************************************************************
float		CSBrickManager::CBrickContainer::getSabrinaRelativeCost(NLMISC::CSheetId id) const
{
	// get the true container
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*brick= pBM->getBrick(id);
	if(brick)
	{
		return brick->SabrinaRelativeCost;
	}
	else
	{
		return 0.f;
	}
}

// ***************************************************************************
sint32		CSBrickManager::CBrickContainer::getNumParameters(NLMISC::CSheetId id) const
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*brick= pBM->getBrick(id);
	if(brick)
	{
		return (sint32)brick->ParameterFamilies.size();
	}
	else
	{
		return 0;
	}
}


// ***************************************************************************
BRICK_FAMILIES::TBrickFamily	CSBrickManager::CBrickContainer::getBrickFamily(NLMISC::CSheetId id, uint& indexInFamily ) const
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*brick= pBM->getBrick(id);
	if(brick)
	{
		indexInFamily = brick->IndexInFamily;
		return brick->BrickFamily;
	}
	else
	{
		return BRICK_FAMILIES::Unknown;
	}
}

// ***************************************************************************
BRICK_TYPE::EBrickType	CSBrickManager::CBrickContainer::getBrickType(NLMISC::CSheetId id) const
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*brick= pBM->getBrick(id);
	if(brick)
	{
		return BRICK_FAMILIES::brickType(brick->BrickFamily);
	}
	else
	{
		return BRICK_TYPE::UNKNOWN;
	}
}

// ***************************************************************************
TOOL_TYPE::TCraftingToolType	CSBrickManager::CBrickContainer::getFaberPlanToolType(NLMISC::CSheetId id) const
{
	CSBrickManager	*pBM= CSBrickManager::getInstance();
	CSBrickSheet	*brick= pBM->getBrick(id);
	if(brick)
	{
		return brick->FaberPlan.ToolType;
	}
	else
	{
		return TOOL_TYPE::Unknown;
	}
}


// ***************************************************************************
void			CSBrickManager::makeVisualBrickForSkill()
{
	map<CSheetId,CSBrickSheet*>::iterator it = _Bricks.begin();

	// clear
	for(uint i=0;i<SKILLS::NUM_SKILLS;i++)
	{
		_VisualBrickForSkill[i]= CSheetId();
	}

	// fill with interface bricks
	while (it != _Bricks.end())
	{
		const CSheetId &rSheet = it->first;
		const CSBrickSheet &rBR = *it->second;

		// List only bricks with family == BIF
		if ( rBR.BrickFamily == BRICK_FAMILIES::BIF )
		{
			if(rBR.getSkill()<SKILLS::NUM_SKILLS)
				_VisualBrickForSkill[rBR.getSkill()]= rSheet;
		}

		it++;
	}
}

// ***************************************************************************
CSheetId		CSBrickManager::getVisualBrickForSkill(SKILLS::ESkills s)
{
	if(s<SKILLS::NUM_SKILLS)
		return _VisualBrickForSkill[s];
	else
		return CSheetId();
}


// ***************************************************************************
void			CSBrickManager::compileBrickProperties()
{
	map<CSheetId,CSBrickSheet*>::iterator it = _Bricks.begin();

	// clear
	_BrickPropIdMap.clear();
	uint	NumIds= 0;

	// **** for all bricks, compile props
	while (it != _Bricks.end())
	{
//		const CSheetId &rSheet = it->first;
		CSBrickSheet &rBR = *it->second;

		// For all properties of this brick, compile
		for(uint i=0;i<rBR.Properties.size();i++)
		{
			CSBrickSheet::CProperty	&prop= rBR.Properties[i];
			string::size_type pos = prop.Text.find(':');
			if(pos!=string::npos)
			{
				string	key= prop.Text.substr(0, pos);
				strlwr(key);
				string	value= prop.Text.substr(pos+1);
				// get key id.
				if(_BrickPropIdMap.find(key)==_BrickPropIdMap.end())
				{
					// Inc before to leave 0 as "undefined"
					_BrickPropIdMap[key]= ++NumIds;
				}
				prop.PropId= _BrickPropIdMap[key];
				fromString(value, prop.Value);
				pos= value.find(':');
				if(pos!=string::npos)
					fromString(value.substr(pos+1), prop.Value2);
			}
		}

		it++;
	}

	// Get usual PropIds
	HpPropId= getBrickPropId("hp");
	SapPropId= getBrickPropId("sap");
	StaPropId= getBrickPropId("sta");
	StaWeightFactorId = getBrickPropId("sta_weight_factor");
	FocusPropId= getBrickPropId("focus");
	CastTimePropId= getBrickPropId("ma_casting_time");
	RangePropId= getBrickPropId("ma_range");


	// **** for all bricks, recompute localized text with formated version
	it= _Bricks.begin();
	ucstring	textTemp;
	textTemp.reserve(1000);
	while (it != _Bricks.end())
	{
		const CSheetId &rSheet = it->first;
		const CSBrickSheet &rBR = *it->second;

		// Get the Brick texts
		ucstring	texts[3];
		texts[0]= STRING_MANAGER::CStringManagerClient::getSBrickLocalizedName(rSheet);
		texts[1]= STRING_MANAGER::CStringManagerClient::getSBrickLocalizedDescription(rSheet);
		texts[2]= STRING_MANAGER::CStringManagerClient::getSBrickLocalizedCompositionDescription(rSheet);

		// For alls texts, parse format
		for(uint i=0;i<3;i++)
		{
			ucstring	&text= texts[i];
			textTemp.erase();

			// Parse the text
			uint	textSize= (uint)text.size();
			for(uint j=0;j<textSize;)
			{
				// Property parsing?
				if(text[j]=='$')
				{
					// double $ ??
					if(j+1<textSize && text[j+1]=='$')
					{
						textTemp+= '$';
						j+= 2;
					}
					// else this is a key desc.
					else
					{
						string	key;
						uint	k= j+1;
						bool	doAbs= false;
						uint	paramId= 0;
						// read option formating option
						for(;;)
						{
							// Abs Value?
							if(k<textSize && text[k]=='|')
							{
								doAbs= true;
								k++;
							}
							// Param Id modifier? (ie read not the 0th value, but the 1th etc... up to 9)
							else if(k<textSize && isdigit(text[k]))
							{
								char	tp[2];
								tp[0]= (char)text[k];
								tp[1]= 0;
								fromString(tp, paramId);
								k++;
							}
							else
								break;
						}
						// find the key end.
						while(k<textSize && (isalpha((char)text[k]) || text[k]=='_'))
						{
							key+= (char)text[k];
							k++;
						}
						// get the key and replace text with value
						if(key.size())
						{
							// Parse all the brick properties if match the key
							float	value= 0.f;
							// get the wanted prop id
							strlwr(key);
							uint	propId= getBrickPropId(key);
							// if propid exist
							if(propId)
							{
								for(uint p=0;p<rBR.Properties.size();p++)
								{
									const CSBrickSheet::CProperty	&prop= rBR.Properties[p];
									if(prop.PropId==propId)
									{
										if(paramId==0)
											value= rBR.Properties[p].Value;
										else
										{
											// must parse the initial text/ skip the identifier
											string::size_type	pos= prop.Text.find(':');
											uint	id= 0;
											while(pos!=string::npos && id<paramId)
											{
												// skip the ':', and next
												pos= prop.Text.find(':', pos+1);
												id++;
											}
											if(pos!=string::npos)
											{
												// skip the :, and read the value
												fromString(prop.Text.substr(pos+1), value);
											}
										}
										break;
									}
								}
							}
							// abs value?
							if(doAbs)
								value= (float)fabs(value);

							// append to text wisely
							sint32	floorVal= (sint32)floorf(value);
							if( floorVal==value )
								textTemp+= toString("%d", floorVal);
							else
								textTemp+= toString("%.3f", value);
						}
						// skip the keyword
						j= k;
					}
				}
				// no, std letter
				else
				{
					textTemp+= text[j];
					j++;
				}
			}

			// repalce with result
			text= textTemp;
		}

		// reset
		STRING_MANAGER::CStringManagerClient::replaceSBrickName(rSheet, texts[0], texts[1], texts[2]);

		it++;
	}
}

// ***************************************************************************
uint			CSBrickManager::getBrickPropId(const std::string &name)
{
	std::map<std::string, uint>::const_iterator		it;
	it= _BrickPropIdMap.find(name);
	if(it!=_BrickPropIdMap.end())
		return it->second;

	return 0;
}

// ***************************************************************************
NLMISC::CSheetId	CSBrickManager::getInterfaceRemoveBrick()
{
	return _InterfaceRemoveBrick;
}

// ***************************************************************************
void				CSBrickManager::filterKnownBricks(std::vector<CSheetId> &bricks)
{
	std::vector<CSheetId>	res;
	res.reserve(bricks.size());

	// keep only known ones
	for(uint i=0;i<bricks.size();i++)
	{
		if(isBrickKnown(bricks[i]))
			res.push_back(bricks[i]);
	}

	// replace with filtered one
	bricks= res;
}

// ***************************************************************************
void				CSBrickManager::appendBrickLearnedCallback(IBrickLearnedCallback *cb)
{
	if(cb)
		_BrickLearnedCallbackSet.insert(cb);
}

// ***************************************************************************
void				CSBrickManager::removeBrickLearnedCallback(IBrickLearnedCallback *cb)
{
	if(cb)
		_BrickLearnedCallbackSet.erase(cb);
}

// ***************************************************************************
void				CSBrickManager::CBrickFamilyObs::update(ICDBNode * /* node */)
{
	CSBrickManager::TBLCBSet::iterator	it;
	for(it=Owner->_BrickLearnedCallbackSet.begin();it!=Owner->_BrickLearnedCallbackSet.end();it++)
	{
		(*it)->onBrickLearned();
	}
}
