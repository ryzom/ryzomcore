

#ifndef SHARD_UNIFIER_SERVICE_H
#define SHARD_UNIFIER_SERVICE_H

#include "nel/misc/sstring.h"
#include "nel/net/service.h"

extern NLMISC::CVariable<uint32>	TotalConcurentUser;

class CShardUnifier : public NLNET::IService
{

	void init();

	bool update();

	void release();

	std::string		getServiceStatusString() const
	{
		// add the TotalConcurentPlayer counter to the status string
		NLMISC::CSString ret;
		ret << "TotalConcurentUser="<<TotalConcurentUser.get();

		return ret;
	}

};


#endif //SHARD_UNIFIER_SERVICE_H

