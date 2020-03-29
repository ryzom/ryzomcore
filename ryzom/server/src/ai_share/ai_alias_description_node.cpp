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
//----------------------------------------------------------------------------

#include "ai_alias_description_node.h"

using namespace NLMISC;
//using namespace NLNET;
using namespace std;

std::vector<NLMISC::CSmartPtr<CAIAliasDescriptionNode> >	CAIAliasDescriptionNode::_aliasDescriptionList;

//---------------------------------------------------------------------------------------
// Stuff used for management of log messages

bool VerboseAliasDescriptionNodeLog=false;


//---------------------------------------------------------------------------------------
// Control over verbose nature of logging
//---------------------------------------------------------------------------------------

NLMISC_COMMAND(verboseAliasNodeTreeParserLog,"Turn on or off or check the state of verbose .primitive parser logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
		StrToBool	(VerboseAliasDescriptionNodeLog, args[0]);

	nlinfo("verbose Logging is %s",VerboseAliasDescriptionNodeLog?"ON":"OFF");
	return true;
}

//---------------------------------------------------------------------------------------
