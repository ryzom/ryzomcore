

#ifndef SESSION_BROWSER_SERVER_H
#define SESSION_BROWSER_SERVER_H

#include "nel/net/service.h"


class CSessionBrowserServer : public NLNET::IService
{
	
	void init();
	
	bool update();
	
	void release();
	
};


#endif //SESSION_BROWSER_SERVER_H

