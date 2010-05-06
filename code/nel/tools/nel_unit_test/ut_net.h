#ifndef UT_NET
#define UT_NET

#include <nel/net/message.h>

using namespace NLNET;

#include "ut_net_layer3.h"
#include "ut_net_message.h"
#include "ut_net_module.h"
// Add a line here when adding a new test CLASS

struct CUTNet : public Test::Suite
{
	CUTNet()
	{
		add(auto_ptr<Test::Suite>(new CUTNetLayer3));
		add(auto_ptr<Test::Suite>(new CUTNetMessage));
		add(auto_ptr<Test::Suite>(new CUTNetModule));
		// Add a line here when adding a new test CLASS
	}
};

#endif
