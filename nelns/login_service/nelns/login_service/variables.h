#ifndef NELNS_LOGIN_SERVICE_VARIABLES_H
#define NELNS_LOGIN_SERVICE_VARIABLES_H

#include <vector>

#include <nel/misc/displayer.h>
#include <nel/misc/log.h>

#include <nelns/login_service/login_service.h>

//
// Variables
//
extern NLMISC::CFileDisplayer *Fd;
extern NLMISC::CStdDisplayer Sd;
extern NLMISC::CLog*		Output;

extern std::vector<CShard>	Shards;



#endif // NELNS_LOGIN_SERVICE_VARIABLES_H
