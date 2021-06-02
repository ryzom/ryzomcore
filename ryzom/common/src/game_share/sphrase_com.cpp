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
#include "sphrase_com.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************
const CSPhraseCom	CSPhraseCom::EmptyPhrase;

// ***************************************************************************
bool	CSPhraseCom::operator==(const CSPhraseCom &p) const
{
	if(Bricks.size()!=p.Bricks.size())
		return false;

	for(uint i=0;i<Bricks.size();i++)
	{
		if(Bricks[i]!=p.Bricks[i])
			return false;
	}
	return true;
}

// ***************************************************************************
bool	CSPhraseCom::operator<(const CSPhraseCom &p) const
{
	if(Bricks.size()!=p.Bricks.size())
		return Bricks.size()<p.Bricks.size();

	for(uint i=0;i<Bricks.size();i++)
	{
		if(Bricks[i]!=p.Bricks[i])
			return Bricks[i]<p.Bricks[i];
	}
	return false;
}

// ***************************************************************************
void	CSPhraseCom::serial(NLMISC::IStream &impulse)
{
	string sTmp;

	if(!impulse.isReading())
		sTmp = Name.toUtf8();

	impulse.serial(sTmp);

	if(impulse.isReading())
		Name.fromUtf8(sTmp);

	// Get the type of .sbrick
	static	uint	sbrickType= 0;
	if(sbrickType==0)
	{
		sbrickType= CSheetId::typeFromFileExtension("sbrick");
	}

	// 16 bits compression of the Bricks
	static vector<uint16>	compBricks;	// static for speed
	if(impulse.isReading())
	{
		// read
		impulse.serialCont(compBricks);
		// uncompress
		contReset(Bricks);
		Bricks.resize(compBricks.size());
		for(uint i=0;i<Bricks.size();i++)
		{
			if(compBricks[i]==0)
				Bricks[i]= 0;
			else
				Bricks[i].buildSheetId(compBricks[i]-1, sbrickType);
		}
	}
	else
	{
		// fill default with 0.
		compBricks.clear();
		compBricks.resize(Bricks.size(), 0);
		// compress
		for(uint i=0;i<Bricks.size();i++)
		{
			// if not empty SheetId
			if(Bricks[i].asInt())
			{
				uint32	compId= Bricks[i].getShortId();
				// the sbrick SheetId must be <65535, else error!
				if(compId>=65535)
				{
					nlwarning("ERROR: found a .sbrick SheetId with SubId>=65535: %s", Bricks[i].toString().c_str());
					// and leave 0.
				}
				else
				{
					compBricks[i]= (uint16)(compId+1);
				}
			}
		}
		// write
		impulse.serialCont(compBricks);
	}
}

// ***************************************************************************
void	CSPhraseSlot::serial(NLMISC::IStream &impulse)
{
	impulse.serial(Phrase);
	impulse.serial(KnownSlot);
	impulse.serial(PhraseSheetId);
}

// ***************************************************************************
void	CSPhraseMemorySlot::serial(NLMISC::IStream &impulse)
{
	impulse.serial(MemoryLineId);
	impulse.serial(MemorySlotId);
	impulse.serial(PhraseId);
}

// ***************************************************************************
void	CFaberMsgItem::serial(NLMISC::IStream &impulse)
{
	impulse.serial(InvId);
	impulse.serial(IndexInInv);
	impulse.serial(Quantity);
}

