/** \file cdb.cpp
 *
 * $Id$
 */



//#include "stdpch.h"

//////////////
// Includes //
//////////////
#include "cdb.h"
#include "cdb_branch.h"
#include <nel/misc/bit_mem_stream.h>

////////////////
// Namespaces //
////////////////
using namespace NLMISC;
using namespace std;


CStringMapper *ICDBNode::_DBSM = NULL;


std::string ICDBNode::getFullName()
{
	std::string sTmp = _DBSM->localUnmap(_Name);
	CCDBNodeBranch *pParent = getParent();
	if (pParent == NULL) return sTmp;
	while (pParent->getParent() != NULL)
	{
		sTmp = *pParent->getName() + ":" + sTmp;
		pParent = pParent->getParent();
	}
	return sTmp;
}
