

#ifndef MODULE_SECURITY_H
#define MODULE_SECURITY_H

#include "nel/misc/entity_id.h"
#include "nel/net/module.h"

enum TRyzomModuleSecurityTag
{
	rmst_client_info,
};


struct TClientInfo : public NLNET::TSecurityData
{
	TClientInfo(const NLNET::TSecurityData::TCtorParam &param)
		: TSecurityData(param)
	{
	}

	uint32				UserId;
	NLMISC::CEntityId	ClientEid;
	std::string			UserPriv;
	std::string			ExtendedPriv;

	void serial(NLMISC::CMemStream &s)
	{
		s.serial(UserId);
		s.serial(ClientEid);
		s.serial(UserPriv);
		s.serial(ExtendedPriv);
	}

};

NLMISC_REGISTER_OBJECT(NLNET::TSecurityData, TClientInfo, uint8, rmst_client_info);

#endif // MODULE_SECURITY_H
