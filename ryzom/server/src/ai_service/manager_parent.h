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



#ifndef MANAGER_PARENT_H
#define MANAGER_PARENT_H

class CAIInstance;
class CCellZone;
//class IBot;
class CGroup;
class CManager;

class IManagerParent : public NLMISC::CDbgRefCount<IManagerParent>
{
public:
	// define some virtual to be implemented by parent

	/// Return a reference to the ai instance that contains this manager (directly or indirectly)
	virtual CAIInstance * getAIInstance() const = 0;
	/** Return a pointeur to the cell zone that contain this mananger. 
	 *	Can be null if the manager is is a AIInstance object.
	 */
	virtual CCellZone	* getCellZone() = 0;

	/// return a display index string
	virtual std::string getIndexString() const =0;
	/// child manager ask parent to build is index string (the one for the manager, not for the parent)
	virtual std::string getManagerIndexString(const CManager *manager) const =0;

	virtual	void	addEnergy		(uint32	energy)	{}
	virtual	void	removeEnergy	(uint32	energy)	{}

	/// Signal that a group is now dead (no more bots spawned)
	virtual void	groupDead(CGroup *grp)	=	0;
};


#endif // MANAGER_PARENT_H
