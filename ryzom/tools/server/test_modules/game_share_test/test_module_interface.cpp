#include <gtest/gtest.h>

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <nel/misc/dynloadlib.h>
#include <nel/misc/path.h>

#include <nel/net/unified_network.h>
#include <nel/net/module.h>
#include <nel/net/module_manager.h>
#include <nel/net/module_builder_parts.h>

#include "test_module_itf.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace TST_MOD_ITF;

// class A
//{
// public:
//	void getParentFromA()
//	{
//		getParent("A");
//	}
//	virtual void getParent(const char*) =0;
// };
//
// class B
//{
// public:
//	void getParentFromB()
//	{
//		getParent("B");
//	}
//	virtual void getParent(const char*) =0;
// };
//
//
// class C
//: public A, B
//{
// public:
//
//	C()
//	{
//		getParentFromA();
//		getParentFromB();
//	}
//
//	virtual void getParent(const char*childName)
//	{
//		printf("Child %s called getParent", childName);
//	}
//};
//
// C	c;
//
// class D
//{
//	virtual void getParent(const char*childName)
//	{
//		printf("Child %s called getParent", childName);
//	}
//};

class CModuleServant
    : public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase>>>,
      public CTestModuleInterfaceSkel
{
	bool _ImmediateDispatching;

public:
	CModuleServant()
	{
		CTestModuleInterfaceSkel::init(this);
		_ImmediateDispatching = false;
	}

	bool isImmediateDispatchingSupported() const
	{
		return _ImmediateDispatching;
	}

	//	void onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
	//	{
	//		if (CTestModuleInterfaceSkel::onDispatchMessage(sender, message))
	//			return;
	//
	//		nlassert(false);
	//	}

	void noParam(NLNET::IModuleProxy *sender)
	{
	}

	uint32 twoWayInvoke(NLNET::IModuleProxy *sender, uint32 value1, uint32 value2)
	{
		return value1 + value2;
	}

	void setImmediateDispatching(bool value)
	{
		_ImmediateDispatching = value;
	}
};

NLNET_REGISTER_MODULE_FACTORY(CModuleServant, "ModuleServant");

class CModuleClient
    : public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase>>>
{
public:
	TModuleProxyPtr Servant;
	bool TaskRunning;

	uint32 Result;

	void startTask()
	{
		TaskRunning = true;
		NLNET_START_MODULE_TASK(CModuleClient, taskFunc);
	}

	void onModuleUp(IModuleProxy *proxy)
	{
		if (proxy->getModuleClassName() == "ModuleServant")
		{
			Servant = proxy;
		}
	}

	void onModuleDown(IModuleProxy *proxy)
	{
		if (proxy == Servant)
			Servant = NULL;
	}

	void taskFunc()
	{
		while (Servant == NULL)
		{
			if (getActiveModuleTask()->isTerminationRequested())
				return;

			getActiveModuleTask()->yield();
		}
		CTestModuleInterfaceProxy tmi(Servant);

		Result = tmi.twoWayInvoke(this, 32, 4);
		TaskRunning = false;
	}

	void callImmediat()
	{
		nlassert(Servant != NULL);
		CTestModuleInterfaceProxy tmi(Servant);
		Result = tmi.twoWayInvoke(this, 32, 4);
	}
};

NLNET_REGISTER_MODULE_FACTORY(CModuleClient, "ModuleClient");

class CModuleInterfaceTS : public testing::Test
{

protected:
	void SetUp() override
	{
		ASSERT_TRUE(NLMISC::INelContext::getInstance().isContextInitialised());
		NLMISC::createDebug(nullptr);
	}

	void TearDown() override
	{
	}
};

TEST_F(CModuleInterfaceTS, testTwoWay)
{
	// create two modules and test the two way invocation protocol

	IModuleManager::getInstance();
	CCommandRegistry &cr = CCommandRegistry::getInstance();

	cr.execute("moduleManager.createModule StandardGateway gw", InfoLog());
	cr.execute("moduleManager.createModule ModuleServant srv", InfoLog());
	cr.execute("moduleManager.createModule ModuleClient clt", InfoLog());

	// plug the modules
	cr.execute("srv.plug gw", InfoLog());
	cr.execute("clt.plug gw", InfoLog());

	// retreive the client module
	IModule *mod = IModuleManager::getInstance().getLocalModule("clt");
	ASSERT_NE(mod, nullptr);
	CModuleClient *client = dynamic_cast<CModuleClient *>(mod);
	ASSERT_NE(client, nullptr);
	CModuleServant *servant = dynamic_cast<CModuleServant *>(IModuleManager::getInstance().getLocalModule("srv"));
	ASSERT_NE(servant, nullptr);

	client->startTask();

	// update the modules
	for (uint i = 0; i < 10; ++i)
	{
		IModuleManager::getInstance().updateModules();
		nlSleep(10);
	}

	// check that the client have received the response
	ASSERT_FALSE(client->TaskRunning);
	// check that the result is ok
	ASSERT_EQ(client->Result, 36);

	// Retry with immediate dispatching and direct call thank to the interface gernerator
	client->Result = 0;
	servant->setImmediateDispatching(true);

	client->callImmediat();
	// check that the result is ok
	ASSERT_EQ(client->Result, 36);

	// cleanup
	// detroy the modules
	cr.execute("moduleManager.deleteModule gw", InfoLog());
	cr.execute("moduleManager.deleteModule srv", InfoLog());
	cr.execute("moduleManager.deleteModule clt", InfoLog());
}
