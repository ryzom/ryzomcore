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

#include "client.h"
#include "mirrors.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern std::map<TYPE_NAME_STRING_ID, std::string>	StringMap;
extern std::set<TYPE_NAME_STRING_ID>				StringAsked;

// ***************************************************************************

// Must be a pointer to control when to start listening socket and when stop it
extern CCallbackServer *Server;

uint32		StillToAdd = 0;
uint32		StillToRemove = 0;
uint32		Added = 0;
uint32		Removed = 0;
uint32		CurrentlyInVision = 0;
uint32		SentPos = 0;
uint32		SentMiscProp = 0;
uint32		SentStr = 0;

NLMISC_VARIABLE(uint32, StillToAdd, "");
NLMISC_VARIABLE(uint32, StillToRemove, "");
NLMISC_VARIABLE(uint32, Added, "");
NLMISC_VARIABLE(uint32, Removed, "");
NLMISC_VARIABLE(uint32, CurrentlyInVision, "");
NLMISC_VARIABLE(uint32, SentPos, "");
NLMISC_VARIABLE(uint32, SentMiscProp, "");
NLMISC_VARIABLE(uint32, SentStr, "");

// ***************************************************************************

vector<NLMISC::CSmartPtr<CMonitorClient> > Clients;

// ***************************************************************************

CMonitorClient::CMonitorClient(TSockId sock)
{
	_Sock = sock;
	setWindow(0,0,0,0);
	StartOffset = 0;
	AllowedUploadBandwidth = 2048;	// 2kB per second for a start
	// counter = 0;

	AddWeight = 3.0f;
	RemoveWeight = 1.0f;
	PosWeight = 1.0f;
	StrWeight = 1.0f;
	MiscPropWeight = 0.5f;
	Authentificated = false;
	BadLogin = false;
}

// ***************************************************************************

CMonitorClient::~CMonitorClient()
{
}

// ***************************************************************************

void CMonitorClient::setWindow(float xmin, float ymin, float xmax, float ymax)
{
	if (xmin > xmax)
	{
		swap(xmin, xmax);
	}
	if (ymin > ymax)
	{
		swap(ymin, ymax);
	}
	_WindowTopLeft = CVector(xmin, ymin, 0);
	_WindowBottomRight = CVector(xmax, ymax, 0);
}

// ***************************************************************************

void	CMonitorClient::add (const TDataSetRow &entity)
{
	uint32	index = entity.getIndex();

	if (index >= Entites.size())
		Entites.resize(index+1);

	// don't add twice
	if ( (Entites[index].Flags & CEntityEntry::Present) != 0)
		return;

	// if pending in removal, just remove from PendingRemove list
	if ( (Entites[index].Flags & CEntityEntry::Pending) != 0)
	{
		vector<uint32>::iterator	it = find(PendingRemove.begin(), PendingRemove.end(), entity.getIndex());
		if (it != PendingRemove.end())
			PendingRemove.erase(it);
		Entites[index].Flags &= (~CEntityEntry::Pending);
		Entites[index].Flags |= CEntityEntry::Present;
		return;
	}

	// set entity as present
	Entites[index].Flags |= (CEntityEntry::Present | CEntityEntry::DirtyAll | CEntityEntry::Pending);

	// add to pending to addition entities
	PendingAdd.push_back(index);
}

// ***************************************************************************

void	CMonitorClient::remove (const TDataSetRow &entity)
{
	uint32	index = entity.getIndex();

	nlassert( index < Entites.size() );

	// don't remove twice
	if ( (Entites[index].Flags & CEntityEntry::Present) == 0)
		return;

	// if pending in addition, just remove from PendingAdd list
	if ( (Entites[index].Flags & CEntityEntry::Pending) != 0)
	{
		vector<uint32>::iterator	it = find(PendingAdd.begin(), PendingAdd.end(), entity.getIndex());
		if (it != PendingAdd.end())
			PendingAdd.erase(it);
		Entites[index].Flags &= (~(CEntityEntry::Pending | CEntityEntry::Present));
		return;
	}

	// set entity as not present
	Entites[index].Flags &= (~CEntityEntry::Present);
	Entites[index].Flags |= CEntityEntry::Pending;

	PendingRemove.push_back(index);
}

// ***************************************************************************

void	CMonitorClient::resetVision()
{
	uint	i;

	const CVector	&topleft = getTopLeft();
	const CVector	&bottomright = getBottomRight();

	for (i=0; i<GlobalEntites.size(); ++i)
	{
		if ((GlobalEntites[i].Flags & CGlobalEntityEntry::Present) != 0)
		{
			TDataSetRow	entityIndex = TDataSetRow::createFromRawIndex (i);
			CMirrorPropValueRO<TYPE_POSX> valueX( TheDataset, entityIndex, DSPropertyPOSX );
			CMirrorPropValueRO<TYPE_POSY> valueY( TheDataset, entityIndex, DSPropertyPOSY );

			CVector		pos((float)valueX / 1000.f, (float)valueY / 1000.f, 0.0f);

			// is entity in client window
			//nlassert(entityIndex.getIndex() < client.Entites.size());
			bool	wasPresent = (i < Entites.size() && (Entites[i].Flags & CEntityEntry::Present) != 0);
			if (pos.x > topleft.x && pos.x < bottomright.x && pos.y > topleft.y && pos.y < bottomright.y)
			{
				if (!wasPresent)
				{
					// Add
					add( entityIndex );
				}

				Entites[i].Flags |= CEntityEntry::DirtyAll;
			}
			else if (wasPresent)
			{
				// Rmv
				remove( entityIndex );
			}
		}
	}
}

// ***************************************************************************

void CMonitorClient::update ()
{
	StillToAdd = 0;
	StillToRemove = 0;
	Added = 0;
	Removed = 0;
	CurrentlyInVision = 0;
	SentPos = 0;
	SentStr = 0;

	// compute bandwidth spaces allowed for this cycle
	float		cycleBandw = (float)AllowedUploadBandwidth / 10.0f;
	float		totalRatio = AddWeight + RemoveWeight + PosWeight + StrWeight + MiscPropWeight;
	uint		maxAddSize = (uint)(cycleBandw*AddWeight/totalRatio);
	uint		maxRmvSize = (uint)(cycleBandw*RemoveWeight/totalRatio);
	uint		maxPosSize = (uint)(cycleBandw*PosWeight/totalRatio);
	uint		maxStrSize = (uint)(cycleBandw*StrWeight/totalRatio);
	uint		maxMiscPropSize  = (uint)(cycleBandw*MiscPropWeight/totalRatio);

	const uint	addSize = 4+4+8+4+4+4;
	const uint	rmvSize = 4;
	const uint	posSize = 4+4+4+4;
	const uint	miscPropSize  = 4+4+4+1+1;

	CConfigFile::CVar *var = IService::getInstance()->ConfigFile.getVarPtr ("UpdatePerTick");
	// here we suppose that position is changing often, whereas other less important
	// properties are updated less frequently
	uint		posToSend = 0;
	uint		miscPropToSend = 0;
	{
		uint poscount = 10;
		if (var && (var->Type == CConfigFile::CVar::T_INT))
			poscount = var->asInt();

		uint	startOffset = StartOffset;
		if (startOffset >= InVision.size())
			startOffset = 0;

		uint	entity = startOffset;
		while (poscount > 0 && !InVision.empty())
		{
			// is entity dirty ?
			uint32	entityRawIndex = InVision[entity];
			if ((Entites[entityRawIndex].Flags & CEntityEntry::PosDirty) != 0)
			{
				if ((Entites[entityRawIndex].Flags & CEntityEntry::Pending) == 0)
				{
					++posToSend;
				}
			}
			if ((Entites[entityRawIndex].Flags & CEntityEntry::MiscPropDirty) != 0 && (Entites[entityRawIndex].Flags & CEntityEntry::Pending) == 0)
				++miscPropToSend;

			++entity;
			if (entity >= InVision.size())
				entity = 0;
			if (entity == startOffset)
				break;
		}
	}

	uint		addTotalSize = (uint)PendingAdd.size()*addSize;
	uint		rmvTotalSize = (uint)PendingRemove.size()*rmvSize;
	uint		posTotalSize = posToSend*posSize;
	uint		miscPropTotalSize = miscPropToSend*miscPropSize;
	uint		strTotalSize = 0;

	uint		i;
	vector< pair<TYPE_NAME_STRING_ID, string*> >	strToSend;
	for (i=0; i<Str.size() && strTotalSize < (uint)cycleBandw; ++i)
	{
		TYPE_NAME_STRING_ID id = Str[i];
		std::map<TYPE_NAME_STRING_ID, std::string>::iterator ite = StringMap.find (id);
		nlassert (ite != StringMap.end());
		strToSend.push_back( make_pair<TYPE_NAME_STRING_ID, string*>(id, &((*ite).second)) );
		strTotalSize += 4+4+(uint)(*ite).second.size();
	}

	bool		restrictAdd =      (addTotalSize > maxAddSize);
	bool		restrictRmv =      (rmvTotalSize > maxRmvSize);
	bool		restrictPos =      (posTotalSize > maxPosSize);
	bool		restrictStr =      (strTotalSize > maxStrSize);
	bool        restrictMiscProp = (miscPropTotalSize > maxMiscPropSize);

	float		remainingBandw = cycleBandw;

	if (!restrictAdd)	    { totalRatio -= AddWeight; remainingBandw -= addTotalSize; }
	if (!restrictRmv)	    { totalRatio -= RemoveWeight; remainingBandw -= rmvTotalSize; }
	if (!restrictPos)	    { totalRatio -= PosWeight; remainingBandw -= posTotalSize; }
	if (!restrictStr)	    { totalRatio -= StrWeight; remainingBandw -= strTotalSize; }
	if (!restrictMiscProp)	{ totalRatio -= MiscPropWeight; remainingBandw -= miscPropTotalSize; }

	if (restrictAdd)	    addTotalSize = (uint)(remainingBandw*AddWeight/totalRatio);
	if (restrictRmv)	    rmvTotalSize = (uint)(remainingBandw*RemoveWeight/totalRatio);
	if (restrictPos)	    posTotalSize = (uint)(remainingBandw*PosWeight/totalRatio);
	if (restrictStr)	    strTotalSize = (uint)(remainingBandw*StrWeight/totalRatio);
	if (restrictMiscProp)	miscPropTotalSize = (uint)(remainingBandw*MiscPropWeight/totalRatio);


	// update pending lists
	uint	pendingAddCount = (addTotalSize/addSize);
	while (!PendingAdd.empty() && pendingAddCount > 0)
	{
		uint32	entityRawIndex = PendingAdd.back();
		PendingAdd.pop_back();
		TDataSetRow	entity = TDataSetRow::createFromRawIndex (entityRawIndex);
		CMirrorPropValueRO<TYPE_NAME_STRING_ID>	stringId( TheDataset, entity, DSPropertyNAME_STRING_ID);
		CMirrorPropValueRO<TYPE_SHEET>	sheetId( TheDataset, entity, DSPropertySHEET);
		CAddData	addData;
		addData.Id = entity.getIndex();
		addData.StringId = stringId;
		addData.EntityId = TheDataset.getEntityId (entity);
		addData.SheetId = sheetId;
		Add.push_back (addData);

		Entites[entityRawIndex].Flags &= (~CEntityEntry::Pending);

		InVision.push_back(entity.getIndex());

		--pendingAddCount;
		++Added;
	}

	uint	pendingRemoveCount = (rmvTotalSize/rmvSize);
	while (!PendingRemove.empty() && pendingRemoveCount > 0)
	{
		uint32	entityRawIndex = PendingRemove.back();

		vector<uint32>::iterator	it = find(InVision.begin(), InVision.end(), entityRawIndex);
		if (it != InVision.end())
			InVision.erase(it);

		PendingRemove.pop_back();
		Rmv.push_back (entityRawIndex);

		Entites[entityRawIndex].Flags &= (~CEntityEntry::Pending);

		--pendingRemoveCount;
		++Removed;
	}

	StillToAdd = (uint32)PendingAdd.size();
	StillToRemove = (uint32)PendingRemove.size();
	CurrentlyInVision = (uint32)InVision.size();

	if (StartOffset >= InVision.size())
		StartOffset = 0;

	uint	entity = StartOffset;
	uint	pendingPosCount = (posTotalSize/posSize);
	uint	pendingMiscPropCount = (miscPropTotalSize/posSize);
	while ((pendingPosCount > 0 || pendingMiscPropCount > 0) && !InVision.empty())
	{
		// is entity dirty ?
		uint32	entityRawIndex = InVision[entity];
		if ((Entites[entityRawIndex].Flags & CEntityEntry::PosDirty) != 0 && (Entites[entityRawIndex].Flags & CEntityEntry::Pending) == 0)
		{
			TDataSetRow	entityIndex = TDataSetRow::createFromRawIndex (entityRawIndex);
			CMirrorPropValueRO<TYPE_POSX>			valueX( TheDataset, entityIndex, DSPropertyPOSX );
			CMirrorPropValueRO<TYPE_POSY>			valueY( TheDataset, entityIndex, DSPropertyPOSY );
			CMirrorPropValueRO<TYPE_ORIENTATION>	valueT( TheDataset, entityIndex, DSPropertyORIENTATION );

			CMonitorClient::CPosData posData;
			posData.X = (float)valueX / 1000.f;
			posData.Y = (float)valueY / 1000.f;
			posData.Id = entityRawIndex;
			posData.Tetha = valueT;
			Pos.push_back (posData);

			Entites[entityRawIndex].Flags &= (~CEntityEntry::PosDirty);

			--pendingPosCount;
			++SentPos;
		}
		if ((Entites[entityRawIndex].Flags & CEntityEntry::MiscPropDirty) != 0 && (Entites[entityRawIndex].Flags & CEntityEntry::Pending) == 0)
		{
			TDataSetRow	entityIndex = TDataSetRow::createFromRawIndex (entityRawIndex);
			CMirrorPropValueRO<TYPE_CURRENT_HIT_POINTS>	    valueCurrentHP( TheDataset, entityIndex, DSPropertyCURRENT_HIT_POINTS);
			CMirrorPropValueRO<TYPE_MAX_HIT_POINTS>			valueMaxHP( TheDataset, entityIndex, DSPropertyMAX_HIT_POINTS);
			CMirrorPropValueRO<TYPE_MODE>					valueMode( TheDataset, entityIndex, DSPropertyMODE);
			CMirrorPropValueRO<TYPE_BEHAVIOUR>				valueBehaviour( TheDataset, entityIndex, DSPropertyBEHAVIOUR);

			CMonitorClient::CMiscPropData miscPropData;
			miscPropData.Id = entityRawIndex;
			miscPropData.CurrentHP = valueCurrentHP;
			miscPropData.MaxHP = valueMaxHP;
			miscPropData.Mode = (uint8) valueMode;
			miscPropData.Behaviour = (uint8) valueBehaviour;
			MiscProp.push_back (miscPropData);

			Entites[entityRawIndex].Flags &= (~CEntityEntry::MiscPropDirty);

			--pendingMiscPropCount;
			++SentMiscProp;
		}

		++entity;
		if (entity >= InVision.size())
			entity = 0;
		if (entity == StartOffset)
			break;
	}

	StartOffset = entity;

	// Send a ADD message
	if (!Add.empty())
	{
		CMessage msgout;
		msgout.setType("ADD");
		// version 1 : added sheet ID of the entity
		msgout.serialVersion(1);
		uint32 count = (uint32)Add.size();
		msgout.serial(count);
		uint i;
		for (i=0; i<count; i++)
		{
			msgout.serial (Add[i].Id);
			msgout.serial (Add[i].StringId);
			msgout.serial (Add[i].EntityId);
			msgout.serial (Add[i].SheetId);

			Entites[Add[i].Id].Flags |= CEntityEntry::Present;
		}
		Server->send(msgout, getSock());
		Add.clear ();
	}

	// Send a RMV message
	if (!Rmv.empty())
	{
		CMessage msgout;
		msgout.setType("RMV");
		msgout.serialVersion(0);
		uint32 count = (uint32)Rmv.size();
		msgout.serial(count);
		uint i;
		for (i=0; i<count; i++)
		{
			msgout.serial (Rmv[i]);
			Entites[Rmv[i]].Flags &= ~CEntityEntry::Present;
		}
		Server->send(msgout, getSock());
		Rmv.clear ();
	}

	// Send a POS message
	if (!Pos.empty())
	{
		CMessage msgout;
		msgout.setType("POS");
		msgout.serialVersion(0);
		uint32 count = (uint32)Pos.size();
		msgout.serial(count);
		uint i;
		for (i=0; i<count; i++)
		{
			msgout.serial (Pos[i].Id);
			msgout.serial (Pos[i].X);
			msgout.serial (Pos[i].Y);
			msgout.serial (Pos[i].Tetha);
		}
		Server->send(msgout, getSock());
		Pos.clear ();
	}

	// Send a MISC_PROP message
	if (!MiscProp.empty())
	{
		CMessage msgout;
		msgout.setType("MISC_PROP");
		msgout.serialVersion(0);
		uint32 count = (uint32)MiscProp.size();
		msgout.serial(count);
		uint i;
		for (i=0; i<count; i++)
		{
			msgout.serial (MiscProp[i].Id);
			msgout.serial (MiscProp[i].CurrentHP);
			msgout.serial (MiscProp[i].MaxHP);
			msgout.serial (MiscProp[i].Mode);
			msgout.serial (MiscProp[i].Behaviour);
		}
		Server->send(msgout, getSock());
		MiscProp.clear ();
	}


	// Send the strings
	if (!Str.empty() && strTotalSize > 0)
	{
		CMessage msgout;
		msgout.setType("STR");
		msgout.serialVersion(0);

		uint	sentStrSize = 0;
		uint	i;
		for (i=0; i<strToSend.size() && sentStrSize < strTotalSize; ++i)
			sentStrSize += 4+4+(uint)(strToSend[i].second)->size();

		uint32 count = i;
		msgout.serial(count);
		for (i=0; i<count; i++)
		{
			msgout.serial(strToSend[i].first);
			msgout.serial(*(strToSend[i].second));
		}

		Str.erase(Str.begin(), Str.begin()+count);

		Server->send(msgout, getSock());
		SentStr = count;
	}
}

// ***************************************************************************
