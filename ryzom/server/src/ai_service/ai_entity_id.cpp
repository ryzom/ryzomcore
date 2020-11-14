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



/*
#include "stdpch.h"


//--------------------------------------------------------------------------
//   CAIEntityId creation from string methods
//--------------------------------------------------------------------------

CAIEntityId	CAIEntityId::entityId(const std::string &name)
{	
	CAIEntityId	eid;
	
	eid	=	CAIEntityId::mgrId(name);
	if (!eid.isInvalid())
		return	(eid);
		
	eid	=	CAIEntityId::grpId(name);
	if (!eid.isInvalid())
		return	(eid);
	
	eid	=	CAIEntityId::botId(name);
	if (!eid.isInvalid())
		return	(eid);

	return CAIEntityId();
}

CAIEntityId CAIEntityId::botId(const std::string &name)
{
	CAIEntityId id;
	
	// see if we have a CAIEntityId.toString()
	id=CAIEntityId(name);
	if (id.isBot() && id.exists())
		return id;
	
	// see if we have a NLMISC::CEntityId.toStirng()
	if (name[0]=='(')
	{
		id=CAIEntityId(NLMISC::CEntityId(name.c_str()));
		if (id.isBot() && id.exists())
			return id;
	}
	
	// see if we have a bot number, group number and manager number
	uint mgridx, grpidx, botidx;
	char s[30];
	
	sscanf(name.c_str(),"%i:%i:%i",&mgridx,&grpidx,&botidx);
	sprintf(s,"%i:%i:%i",mgridx,grpidx,botidx);
	if (name==std::string(s))
		return CAIEntityId::botId(mgridx,grpidx,botidx);
	
	sscanf(name.c_str(),"%i,%i,%i",&mgridx,&grpidx,&botidx);
	sprintf(s,"%i,%i,%i",mgridx,grpidx,botidx);
	if (name==std::string(s))
		return CAIEntityId::botId(mgridx,grpidx,botidx);
	
	sscanf(name.c_str(),"%i.%i.%i",&mgridx,&grpidx,&botidx);
	sprintf(s,"%i.%i.%i",mgridx,grpidx,botidx);
	if (name==std::string(s))
		return CAIEntityId::botId(mgridx,grpidx,botidx);
	
	// we're out of options so give up
	return	CAIEntityId();
}



CAIEntityId CAIEntityId::grpId(const std::string &name)
{
	CAIEntityId id;
	
	// see if we have a CAIEntityId.toStirng()
	id=CAIEntityId(name);
	if (id.isGrp() && id.exists())
		return id;
	
	// see if we have a NLMISC::CEntityId.toStirng()
	if (name[0]=='(')
	{
		id=CAIEntityId(NLMISC::CEntityId(name.c_str()));
		if (id.isGrp() && id.exists())
			return id;
	}
	
	// see if name corresponds to the name of one of the groups
	for (id=CAIEntityId::firstMgr().firstGrp();!id.isInvalid();id=id.nextGrp())
		if (id.exists())
			if (id.grpPtr()->getName()==name || 
				id.grpPtr()->getName()+"."+id.grpPtr()->getName()==name ||
				id.grpPtr()->getName()+":"+id.grpPtr()->getName()==name	)
				return id;
			
	// see if we have a group number and manager number
	uint mgridx, grpidx;
	char s[30];
	
	sscanf(name.c_str(),"%i:%i",&mgridx,&grpidx);
	sprintf(s,"%i:%i",mgridx,grpidx);
	if (name==std::string(s))
		return CAIEntityId::grpId(mgridx,grpidx);
	
	sscanf(name.c_str(),"%i,%i",&mgridx,&grpidx);
	sprintf(s,"%i,%i",mgridx,grpidx);
	if (name==std::string(s))
		return CAIEntityId::grpId(mgridx,grpidx);
	
	sscanf(name.c_str(),"%i.%i",&mgridx,&grpidx);
	sprintf(s,"%i.%i",mgridx,grpidx);
	if (name==std::string(s))
		return CAIEntityId::grpId(mgridx,grpidx);
	
	// we're out of options so give up
	return	CAIEntityId();
}



CAIEntityId CAIEntityId::mgrId(const std::string &name)
{
	CAIEntityId id;
	
	// see if we have a CAIEntityId.toString()
	id=CAIEntityId(name);
	if (id.isMgr() && id.exists())
		return id;
	
	// see if we have a NLMISC::CEntityId.toString()
	if (name[0]=='(')
	{
		id=CAIEntityId(NLMISC::CEntityId(name.c_str()));
		if (id.isMgr() && id.exists())
			return id;
	}
	
	// see if name corresponds to the name of one of the manager
	for (id=CAIEntityId::firstMgr();!id.isInvalid();id=id.nextMgr())
		if (id.exists())
			if (id.mgrPtr()->getName()==name)
				return id;
			
	// see if we have a manager number
	uint	idx;
	char	s[256];
	sscanf(name.c_str(),"%i",&idx);
	sprintf(s,"%i",idx);
	if (name==std::string(s))
		return CAIS::getMgr(idx)->id();
	
	//	toString no more accepts to compile and not have the time to look at, so its remplaced by the upper piece of code ..
	//	uint idx=atoi(name.c_str());
	//	if (toString(idx)==name)
	//		return CAIS::getMgr(idx)->id();
			
	// we're out of options so give up
	return CAIEntityId();
}
*/
