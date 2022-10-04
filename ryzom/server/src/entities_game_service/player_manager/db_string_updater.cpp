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
#include "nel/net/unified_network.h"
#include "db_string_updater.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

TUnifiedCallbackItem	StringUpdateCbArray[] =
{
	{"STORE_STRING_RESULT", &CDBStringUpdater::cbStoreStringResult },
};


CDBStringUpdater::CDBStringUpdater()
	: _IOSIsUp(false)
{
	// register the callback from IOS
	NLNET::CUnifiedNetwork::getInstance()->addCallbackArray(StringUpdateCbArray, sizeofarray(StringUpdateCbArray));

	// store the empty string
	_MappedIOSStrings.insert(make_pair(CStringMapper::map(""), 0));
}

void CDBStringUpdater::onIOSUp()
{
	nlassert(_IOSIsUp == false);
	// we need to remap all string
	_IOSIsUp = true;

	// cleanup the old mapping talbe
	_MappedIOSStrings.clear();
	// insert the 'empty string' mapping
	_MappedIOSStrings.insert(make_pair(CStringMapper::map(""), 0));

	// send all string to IOS
	TStringLeafs::iterator first(_StringLeafs.begin()), last(_StringLeafs.end());
	for (; first != last; ++first)
	{
		const TBDStringLeaf &ident = first->first;

		ucstring str;

		str.fromUtf8(CStringMapper::unmap(first->second.LocalStringId));

		storeAStringInIOS(str);
	}

}

void CDBStringUpdater::onIOSDown()
{
	nlassert(_IOSIsUp == true);
	_IOSIsUp = false;
}

void CDBStringUpdater::onClientDatabaseDeleted(CCDBSynchronised *clientDB)
{
	static vector<TBDStringLeaf>	entryToRemove;

	entryToRemove.clear();

	// search any record about this container.
	TStringLeafs::iterator first(_StringLeafs.begin()), last(_StringLeafs.end());
	for (; first != last; ++first)
	{
		const TBDStringLeaf &ident = first->first;
		if (ident.ClientDB == clientDB)
		{
			// remove this one
			entryToRemove.push_back(ident);
		}
	}

	// remove the selected entry
	while (!entryToRemove.empty())
	{
		_StringLeafs.erase(entryToRemove.back());
		entryToRemove.pop_back();
	}
}

void CDBStringUpdater::setStringLeaf(CCDBSynchronised *clientDB, ICDBStructNode *node, const ucstring &str, bool forceSending)
{
	setStringLeaf(clientDB, node, str.toUtf8(), forceSending);
}

void CDBStringUpdater::setStringLeaf(CCDBSynchronised *clientDB, ICDBStructNode *node, const std::string &str, bool forceSending)
{
	TBDStringLeaf ident(clientDB, node);

	TLocalStringId localStringId = CStringMapper::map(str);

	// check if we already have a mapping for this entry
	TStringLeafs::iterator it(_StringLeafs.find(ident));
	if (it == _StringLeafs.end())
	{
		// create a new entry
		TStringLeafInfo sli;
		sli.LocalStringId = localStringId;
		sli.ForceSending = forceSending;
		_StringLeafs.insert(make_pair(ident, sli));
	}

	// check if we have the string already mapped
	TMappedIOSStrings::iterator it2(_MappedIOSStrings.find(localStringId));
	if (it2 != _MappedIOSStrings.end())
	{
		// we already have this string mapped, just set the value
		clientDB->x_setProp(node, it2->second);
	}
	else
	{
		// this is a new mapping, just record it and send the message to IOS
		storeAStringInIOS(str);
	}
}

ucstring CDBStringUpdater::getUcstringLeaf(CCDBSynchronised *clientDB, ICDBStructNode *node) const
{
	static const ucstring emptyStr;
	TBDStringLeaf ident(clientDB, node);

	// check if we already have a mapping for this entry
	TStringLeafs::const_iterator it(_StringLeafs.find(ident));
	if (it == _StringLeafs.end())
		return emptyStr;
	
	return ucstring::makeFromUtf8(CStringMapper::unmap(it->second.LocalStringId));
}

const std::string & CDBStringUpdater::getStringLeaf(CCDBSynchronised *clientDB, ICDBStructNode *node) const
{
	static const std::string emptyStr;
	TBDStringLeaf ident(clientDB, node);

	// check if we already have a mapping for this entry
	TStringLeafs::const_iterator it(_StringLeafs.find(ident));
	if (it == _StringLeafs.end())
		return emptyStr;
	
	return CStringMapper::unmap(it->second.LocalStringId);
}


void CDBStringUpdater::storeAStringInIOS(const ucstring &str)
{
	if (_IOSIsUp && !str.empty())
	{
		CMessage msgios("STORE_STRING");
		nlWrite(msgios, serial, str);
		CUnifiedNetwork::getInstance()->send("IOS", msgios);
	}
}


void	CDBStringUpdater::cbStoreStringResult(CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	CDBStringUpdater::getInstance().storeStringResult(msgin, serviceName, serviceId);
}

void	CDBStringUpdater::storeStringResult(CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	ucstring			str;
	TIOSStringId		iosStringId;

	msgin.serial(str);
	msgin.serial(iosStringId);

	TLocalStringId localStringId = CStringMapper::map(str.toUtf8());

	// store the mapping for later uses
	_MappedIOSStrings.insert(make_pair(localStringId, iosStringId));

	// for now, do a brute force attack : iterate over all strings and update
	// those that match the local string id
	TStringLeafs::iterator first(_StringLeafs.begin()), last(_StringLeafs.end());
	for (; first != last; ++first)
	{
		TStringLeafInfo &sli = first->second;
		if (sli.LocalStringId == localStringId)
		{
			const TBDStringLeaf &ident = first->first;
			// this one match, update the database with ios id
			ident.ClientDB->x_setProp(ident.Node, iosStringId, sli.ForceSending);

			// clear force sending flag
			sli.ForceSending = false;
		}
	}
}



