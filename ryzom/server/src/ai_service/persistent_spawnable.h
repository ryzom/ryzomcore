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



#ifndef _PERSISTENT_SPAWNABLE_
#define	_PERSISTENT_SPAWNABLE_

//	SpawnObject
template <class T>
class CSpawnable :
	public	NLMISC::CDbgRefCount< CSpawnable<T> >,
	public NLMISC::CRefCount
{
public:
	CSpawnable(T&	owner)	:	_Owner(owner)
	{
	}
	virtual	~CSpawnable()
	{
	}
	
	inline	T&	getPersistent	()	const
	{
		return	_Owner;
	}
	
private:
	T&	_Owner;
};

//	SpawnObject
template	<class T>
class	CPersistent
{
public:
	CPersistent	(	)
	{
		_Spawnable	=	NULL;
	}
	virtual ~CPersistent	(	)
	{
		_Spawnable	=	NULL;
	}
	
	bool	isSpawned	()	const
	{
		return	!_Spawnable.isNull();
	}
	void	setSpawn	(const	NLMISC::CSmartPtr<T>	&spawnable)
	{
		_Spawnable=spawnable;
#if !FINAL_VERSION
	nlassert((!spawnable) || (static_cast<CPersistent<T>*>(&spawnable->getPersistent())==static_cast<CPersistent<T>*>(this)));
#endif	
	}
	
	inline	T	*getSpawnObj()	const	//	you must overload this access to cast the objet with a custom more proper type. ( you know what i mean ? )
	{
		return	_Spawnable;
	}
	
protected:
	
//	virtual	bool	spawn	()	=	0;
//	virtual	void	despawn	()	=	0;
	
private:
	NLMISC::CSmartPtr<T>	_Spawnable;
};

#endif
