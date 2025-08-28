#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <set>
#include <vector>

#include <nel/misc/app_context.h>
#include <nel/misc/command.h>
#include <nel/misc/debug.h>
#include <nel/net/module.h>
#include <nel/net/module_gateway.h>
#include <nel/net/module_manager.h>

using std::string;
using std::set;
using std::vector;

using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::NotNull;
using ::testing::StartsWith;
using ::testing::StrEq;

class CModuleType0 : public NLNET::CModuleBase
{
public:
	uint PingCount;
	uint ResponseReceived;

	set<NLNET::TModuleProxyPtr> ModuleType0;

	uint32 ModuleUpCalled;
	uint32 ModuleDownCalled;
	uint32 ProcessMessageCalled;
	uint32 SecurityUpdateCalled;

	CModuleType0()
	    : PingCount(0)
	    , ResponseReceived(0)
	{
		ModuleUpCalled = 0;
		ModuleDownCalled = 0;
		ProcessMessageCalled = 0;
		SecurityUpdateCalled = 0;
	}

	std::string buildModuleManifest() const
	{
		return "CModuleType0";
	}

	bool initModule(const NLNET::TParsedCommandLine &param)
	{
		bool ret = CModuleBase::initModule(param);
		if (param.getParam("FAIL") != nullptr)
			return false;

		return ret;
	}

	void onServiceUp(const std::string &serviceName, NLNET::TServiceId serviceId)
	{
	}
	/// A nel layer 5 service has stopped.
	void onServiceDown(const std::string &serviceName, NLNET::TServiceId serviceId)
	{
	}
	void onModuleUpdate()
	{
	}
	/** The service main loop is terminating it job', all module will be
	 *	disconnected and removed after this callback.
	 */
	void onApplicationExit()
	{
	}

	void onModuleUp(NLNET::IModuleProxy *moduleProxy)
	{
		ModuleUpCalled++;

		if (moduleProxy->getModuleClassName() == getModuleClassName())
			ModuleType0.insert(moduleProxy);
	}
	/** Called by a socket to inform this module that another
	 *	module has been deleted OR has been no more accessible (due to
	 *	some gateway disconnection).
	 */
	void onModuleDown(NLNET::IModuleProxy *moduleProxy)
	{
		ModuleDownCalled++;

		if (moduleProxy->getModuleClassName() == getModuleClassName())
			ModuleType0.erase(moduleProxy);
	}

	bool onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message)
	{
		ProcessMessageCalled++;

		if (message.getName() == "DEBUG_MOD_PING")
		{
			PingCount++;
			return true;
		}
		else if (message.getName() == "HELLO")
		{
			NLNET::CMessage ping("DEBUG_MOD_PING");
			senderModuleProxy->sendModuleMessage(this, NLNET::CMessage(ping));
			senderModuleProxy->sendModuleMessage(this, NLNET::CMessage(ping));
			{
				NLNET::CMessage resp;
				resp.setType("HELLO_RESP", NLNET::CMessage::Response);

				senderModuleProxy->sendModuleMessage(this, resp);
			}
			senderModuleProxy->sendModuleMessage(this, NLNET::CMessage(ping));
			senderModuleProxy->sendModuleMessage(this, NLNET::CMessage(ping));
			return true;
		}
		else if (message.getName() == "HELLO2")
		{
			// the response for the life, the universe and all other things...
			throw 42;

			return true;
		}

		return false;
	}

	void onModuleSecurityChange(NLNET::IModuleProxy *moduleProxy)
	{
		SecurityUpdateCalled++;
	}

	void onModuleSocketEvent(NLNET::IModuleSocket *moduleSocket, IModule::TModuleSocketEvent eventType)
	{
	}

	void startTaskA()
	{
		// start a task on module
		NLNET_START_MODULE_TASK(CModuleType0, taskA);
	}

	// test task A
	void taskA()
	{
		// use the first like me in the list
		nlassert(!ModuleType0.empty());

		NLNET::TModuleProxyPtr proxy = *ModuleType0.begin();

		NLNET::CMessage msg;
		msg.setType("HELLO", NLNET::CMessage::Request);
		NLNET::CMessage resp;
		invokeModuleOperation(proxy, msg, resp);

		nlassert(resp.getName() == "HELLO_RESP");

		ResponseReceived++;
	}

	void startTaskB()
	{
		// start a task on module
		NLNET_START_MODULE_TASK(CModuleType0, taskB);
	}

	// test task B
	void taskB()
	{
		// use the first like me in the list
		nlassert(!ModuleType0.empty());

		NLNET::TModuleProxyPtr proxy = *ModuleType0.begin();

		NLNET::CMessage msg;
		msg.setType("HELLO2", NLNET::CMessage::Request);
		NLNET::CMessage resp;

		try
		{
			invokeModuleOperation(proxy, msg, resp);
		}
		catch (const IModule::EInvokeFailed &)
		{
			ResponseReceived++;
		}
	}
};

NLNET_REGISTER_MODULE_FACTORY(CModuleType0, "ModuleType0");

// A module that doesn't support immediate dispatching
class CModuleAsync : public CModuleType0
{
public:
	bool isImmediateDispatchingSupported() const
	{
		return false;
	}
};
NLNET_REGISTER_MODULE_FACTORY(CModuleAsync, "ModuleAsync");


class CUTNetModule : public testing::Test
{
protected:
	NLMISC::CApplicationContext context;

	void SetUp() override
	{
		context = NLMISC::CApplicationContext();

		NLMISC::createDebug(nullptr);
	}

	void TearDown() override
	{
	}

	// utility to look for a specified proxy in a vector of proxy
	// return true if the proxy if found
	bool lookForModuleProxy(const vector<NLNET::IModuleProxy *> proxList, const std::string &modName)
	{
		for (uint i = 0; i < proxList.size(); ++i)
		{
			if (proxList[i]->getModuleName().find(modName) == (proxList[i]->getModuleName().size() - modName.size()))
				return true;
		}

		return false;
	}

	NLNET::IModuleProxy *retrieveModuleProxy(NLNET::IModuleGateway *gw, const std::string &modName)
	{
		vector<NLNET::IModuleProxy *> proxList;
		gw->getModuleProxyList(proxList);

		for (uint i = 0; i < proxList.size(); ++i)
		{
			if (proxList[i]->getModuleName().find(modName) == (proxList[i]->getModuleName().size() - modName.size()))
				return proxList[i];
		}

		return nullptr;
	}
};

TEST_F(CUTNetModule, localMessageQueing)
{
	NLNET::IModuleManager &mm = NLNET::IModuleManager::getInstance();
	NLMISC::CCommandRegistry &cr = NLMISC::CCommandRegistry::getInstance();

	NLNET::IModule *mods = mm.createModule("StandardGateway", "gws", "");
	ASSERT_TRUE(mods != nullptr);
	NLNET::IModuleGateway *gws = dynamic_cast<NLNET::IModuleGateway *>(mods);
	ASSERT_TRUE(gws != nullptr);

	// get the socket interface of the gateway
	NLNET::IModuleSocket *socketGws = mm.getModuleSocket("gws");
	ASSERT_TRUE(socketGws != nullptr);

	// create two modules that will communicate localy
	NLNET::IModule *m1 = mm.createModule("ModuleType0", "m1", "");
	ASSERT_TRUE(m1 != nullptr);
	NLNET::IModule *m2 = mm.createModule("ModuleAsync", "m2", "");
	ASSERT_TRUE(m2 != nullptr);

	m1->plugModule(socketGws);
	m2->plugModule(socketGws);

	// update the networks
	for (uint i = 0; i < 4; ++i)
	{
		mm.updateModules();
		NLMISC::nlSleep(50);
	}

	// retrieve module proxy and send one ping to each other
	vector<NLNET::IModuleProxy *> proxiesC;
	gws->getModuleProxyList(proxiesC);
	ASSERT_TRUE(proxiesC.size() == 2);
	ASSERT_TRUE(lookForModuleProxy(proxiesC, "m2"));
	NLNET::IModuleProxy *pm2 = retrieveModuleProxy(gws, "m2");
	ASSERT_TRUE(pm2 != nullptr);
	NLNET::CMessage aMessage("DEBUG_MOD_PING");
	pm2->sendModuleMessage(m1, aMessage);

	proxiesC.clear();
	gws->getModuleProxyList(proxiesC);
	ASSERT_TRUE(proxiesC.size() == 2);
	ASSERT_TRUE(lookForModuleProxy(proxiesC, "m1"));
	NLNET::IModuleProxy *pm1 = retrieveModuleProxy(gws, "m1");
	ASSERT_TRUE(pm1 != nullptr);
	aMessage = NLNET::CMessage("DEBUG_MOD_PING");
	pm1->sendModuleMessage(m2, aMessage);

	// check received ping count
	CModuleType0 *mod1 = dynamic_cast<CModuleType0 *>(m1);
	ASSERT_TRUE(mod1 != nullptr);
	ASSERT_TRUE(mod1->PingCount == 1);
	CModuleType0 *mod2 = dynamic_cast<CModuleType0 *>(m2);
	ASSERT_TRUE(mod2 != nullptr);
	ASSERT_TRUE(mod2->PingCount == 0);

	// update the networks
	for (uint i = 0; i < 4; ++i)
	{
		mm.updateModules();
		NLMISC::nlSleep(50);
	}

	// check received ping count
	ASSERT_TRUE(mod1->PingCount == 1);
	ASSERT_TRUE(mod2->PingCount == 1);

	// update the networks
	for (uint i = 0; i < 4; ++i)
	{
		mm.updateModules();
		NLMISC::nlSleep(50);
	}

	// check received ping count
	ASSERT_TRUE(mod1->PingCount == 1);
	ASSERT_TRUE(mod2->PingCount == 1);

	// cleanup
	mm.deleteModule(m1);
	mm.deleteModule(m2);
	mm.deleteModule(mods);
}
