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


#include "nel/misc/types_nl.h"

#include "nel/misc/singleton.h"
#include "nel/misc/string_mapper.h"
#include "nel/net/message.h"
#include "cdb_synchronised.h"


class CDBStringUpdater : public NLMISC::CSingleton<CDBStringUpdater>
{

	typedef uint32				TIOSStringId;
	typedef NLMISC::TStringId	TLocalStringId;

	// Identifier for one string mapped into one data container
	struct TBDStringLeaf
	{
		CCDBSynchronised		*ClientDB;
		ICDBStructNode			*Node;

		TBDStringLeaf()
			:	ClientDB(NULL),
				Node(NULL)
		{}

		TBDStringLeaf(CCDBSynchronised *clientDB, ICDBStructNode *node)
			: ClientDB(clientDB), Node(node)
		{}

		bool operator ==(const TBDStringLeaf &other) const
		{
			return ClientDB == other.ClientDB && Node == other.Node;
		}
	};

	// hasher for the identifier
	struct THashDBStringLeaf
	{
		size_t operator()(const TBDStringLeaf &stringLeaf) const
		{
			return ((size_t)stringLeaf.ClientDB>>4) ^ ((size_t)stringLeaf.Node>>4);
		}
	};

	// info for each string leaf
	struct TStringLeafInfo
	{
		TLocalStringId	LocalStringId;
		bool			ForceSending;
	};

	typedef CHashMap<TBDStringLeaf, TStringLeafInfo, THashDBStringLeaf> TStringLeafs;

	// All the string leaf with non empty string
	TStringLeafs			_StringLeafs;


	typedef std::map<TLocalStringId, TIOSStringId>	TMappedIOSStrings;

	// The local table of already mapped IOS id
	TMappedIOSStrings		_MappedIOSStrings;

	bool		_IOSIsUp;



private:
	void	storeAStringInIOS(const ucstring &str);

	void	storeStringResult(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
public:

	CDBStringUpdater();

	void onIOSUp();
	void onIOSDown();

	void onClientDatabaseDeleted(CCDBSynchronised *clientDB);
	void setStringLeaf(CCDBSynchronised *clientDB, ICDBStructNode *node, const std::string &str, bool forceSending);
	void setStringLeaf(CCDBSynchronised *clientDB, ICDBStructNode *node, const ucstring &str, bool forceSending);
	ucstring getUcstringLeaf(CCDBSynchronised *clientDB, ICDBStructNode *node) const;
	const std::string &getStringLeaf(CCDBSynchronised *clientDB, ICDBStructNode *node) const;

	static void	cbStoreStringResult(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
};



