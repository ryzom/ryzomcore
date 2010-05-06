
#error "deprecated file"

#ifndef LOGIN_SERVICE_H
#define LOGIN_SERVICE_H

#include "stdpch.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_part.h"

#include "login_service_itf.h"

class CLoginService : 
	public NLNET::CEmptyModuleCommBehav<NLNET::NLNET::CModuleBase> > >
	public CLoginServiceSkel
{
public:
	CLoginService()
	{
		CLoginServiceSkel(this);
	}
};

#endif // LOGIN_SERVICE_H