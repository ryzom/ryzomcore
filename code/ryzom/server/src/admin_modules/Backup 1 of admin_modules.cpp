/** This file declare a pure nel module library */


#include "nel/net/module_manager.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern void as_forceLink();
extern void aes_forceLink();
extern void aesclient_forceLink();

void admin_modules_forceLink()
{
	as_forceLink();
	aes_forceLink();
	aesclient_forceLink();
}

//NLMISC_DECL_PURE_LIB(CNelModuleLibrary);

