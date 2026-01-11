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



#ifndef DYN_GRP_H
#define DYN_GRP_H

#include "continent.h"

//////////////////////////////////////////////////////////////////////////////
// CDynGrpBase                                                              //
//////////////////////////////////////////////////////////////////////////////

class CDynGrpBase
{
public:
	CDynGrpBase();
	virtual ~CDynGrpBase();
	
	void initDynGrp(IGroupDesc const* const gd, CFamilyBehavior const* const familyBehavior);
	
	void setDiscardable(bool discardable) const;
	bool getDiscardable() const;
	
	NLMISC::CSmartPtr<IGroupDesc const> const& getGroupDesc() const;
	
	NLMISC::CDbgPtr<CFamilyBehavior> const& getFamilyBehavior() const;
	
	float getEnergyCoef() const;
	
	bool getCountMultiplierFlag() const;
		
protected:	
	/** Flag for group discardability.
	 *	If this flag is set, then the group can be despawn when
	 *	the family spawned energy is to high.
	 *	When cleared, the group cannot be despawned automaticaly
	 *	either for enegy reason nor for unadequate energy level.
	*/
	mutable	bool							_Discardable;
	/// The dynamic group model
	NLMISC::CSmartPtr<IGroupDesc const>		_GroupDesc;

	/// The family this group belong to
	NLMISC::CDbgPtr<CFamilyBehavior>		_FamilyBehavior;
};

//////////////////////////////////////////////////////////////////////////////
// CDynBot                                                                  //
//////////////////////////////////////////////////////////////////////////////

class CDynBot
{
public:
	CDynBot()	: _BotEnergyValue(0)
	{}
	virtual	~CDynBot()
	{}
	/// The energy value of this bot.

	const	uint32	&botEnergyValue	()	const
	{
		return	_BotEnergyValue;
	}

	void	setBotEnergyValue	(const	uint32	&energyValue)
	{
		_BotEnergyValue=energyValue;
	}

	virtual	void	addEnergy()		const	=	0;
	virtual	void	removeEnergy()	const	=	0;

private:
	uint32	_BotEnergyValue;
};

class CDynSpawnBot
{
public:
	CDynSpawnBot(const	CDynBot	&dynBot)	:	_DynBot(dynBot)
	{
		_DynBot.addEnergy();
	}

	virtual	~CDynSpawnBot()
	{
		_DynBot.removeEnergy();
	}

private:
	const	CDynBot	&_DynBot;
};


#endif
