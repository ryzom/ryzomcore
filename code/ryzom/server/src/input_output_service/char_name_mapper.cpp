
#include "stdpch.h"
#include "nel/net/module_builder_parts.h"

#include "server_share/char_name_mapper_itf.h"

#include "string_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace CNM;

class CCharNameMapper : 
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public CCharNameMapperSkel
{


public:

	CCharNameMapper()
	{
		CCharNameMapperSkel::init(this);
	}

	////////////////////////////////////////////////////////////////
	// Virtual overrides from char name mapper
	////////////////////////////////////////////////////////////////
	virtual void mapCharNames(NLNET::IModuleProxy *sender, const std::vector < TCharNameInfo > &charNameInfos)
	{
		// we receive a list of character name, map them in the IOS string table

		vector<TCharMappedInfo> result(charNameInfos.size());

		for (uint i=0; i<charNameInfos.size(); ++i)
		{
			const TCharNameInfo &cni = charNameInfos[i];
			uint32 stringId = SM->storeString(cni.getCharName());

			result[i].setCharEid(cni.getCharEid());
			result[i].setStringId(stringId);
		}

		// return the result to the client
		CCharNameMapperClientProxy cnmc(sender);
		cnmc.charNamesMapped(this, result);
	}


};


NLNET_REGISTER_MODULE_FACTORY(CCharNameMapper, "CharNameMapper");
