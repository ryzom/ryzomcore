#include <nelns/naming_service/naming_service.h>

#include <nel/misc/common.h>
#include <nel/misc/config_file.h>
#include <nel/misc/string_common.h>

#include <nel/net/callback_net_base.h>
#include <nel/net/callback_server.h>

#include <nelns/naming_service/functions.h>
#include <nelns/naming_service/variables.h>

using std::map;
using std::set;
using std::string;
using std::vector;

using NLMISC::CConfigFile;
using NLMISC::Exception;
using NLMISC::toString;
using NLNET::CCallbackNetBase;
using NLNET::CCallbackServer;
using NLNET::CInetAddress;
using NLNET::CMessage;
using NLNET::CUnifiedNetwork;
using NLNET::TCallbackItem;
using NLNET::TServiceId;
using NLNET::TSockId;

/**
 * Init
 */
void CNamingService::init()
{
	// if a baseport is available in the config file, get it
	CConfigFile::CVar *var;
	if ((var = ConfigFile.getVarPtr("BasePort")) != NULL)
	{
		uint16 newBasePort = var->asInt();
		nlinfo("Changing the MinBasePort number from %hu to %hu", MinBasePort, newBasePort);
		sint32 delta = MaxBasePort - MinBasePort;
		nlassert(delta > 0);
		MinBasePort = newBasePort;
		MaxBasePort = MinBasePort + uint16(delta);
	}

	// Parameters for the service instance manager
	try
	{
		CConfigFile::CVar &uniqueServices = ConfigFile.getVar("UniqueOnShardServices");
		for (uint i = 0; i != uniqueServices.size(); ++i)
		{
			_ServiceInstances.addUniqueService(uniqueServices.asString(i), true);
		}
	}
	catch (Exception &)
	{
	}
	try
	{
		CConfigFile::CVar &uniqueServicesM = ConfigFile.getVar("UniqueByMachineServices");
		for (uint i = 0; i != uniqueServicesM.size(); ++i)
		{
			_ServiceInstances.addUniqueService(uniqueServicesM.asString(i), false);
		}
	}
	catch (Exception &)
	{
	}

	/*
	        // we don't try to associate message from client
	        CNetManager::getNetBase ("NS")->ignoreAllUnknownId (true);

	        // add the callback in case of disconnection
	        CNetManager::setConnectionCallback ("NS", cbConnect, NULL);

	        // add the callback in case of disconnection
	        CNetManager::setDisconnectionCallback ("NS", cbDisconnect, NULL);
	*/
	// DEBUG
	// DebugLog->addDisplayer( new CStdDisplayer() );

	vector<CInetAddress> v = CInetAddress::localAddresses();
	nlinfo("%d detected local addresses:", v.size());
	for (uint i = 0; i < v.size(); i++)
	{
		nlinfo(" %d - '%s'", i, v[i].asString().c_str());
	}

	uint16 nsport = 50000;
	if ((var = ConfigFile.getVarPtr("NSPort")) != NULL)
	{
		nsport = var->asInt();
	}

	TCallbackItem CallbackArray[] = {
		{ "RG", [=](auto &msgin, auto from, auto &netbase) { cbRegister(msgin, from); } },
		{ "RRG", [=](auto &msgin, auto from, auto &netbase) { cbResendRegistration(msgin, from); } },
		{ "QP", [=](auto &msgin, auto from, auto &netbase) { cbQueryPort(msgin, from); } },
		{ "UNI", cbUnregisterSId },
		{ "ACK_UNI", cbACKUnregistration },
		//	{ "RS", cbRegisteredServices },
	};
	CallbackServer = new CCallbackServer;
	CallbackServer->init(nsport);
	CallbackServer->addCallbackArray(CallbackArray, sizeof(CallbackArray) / sizeof(CallbackArray[0]));
	CallbackServer->setConnectionCallback(cbConnect, NULL);
	CallbackServer->setDisconnectionCallback(cbDisconnect, NULL);
}

/**
 * Update
 */
bool CNamingService::update()
{
	checkWaitingUnregistrationServices();

	CallbackServer->update();

	return true;
}

void CNamingService::release()
{
	if (CallbackServer != NULL)
		delete CallbackServer;
	CallbackServer = NULL;
}

void CNamingService::cbRegister(CMessage &msgin, TSockId from)
{
	string name;
	vector<CInetAddress> addr;
	TServiceId sid;
	msgin.serial(name);
	msgin.serialCont(addr);
	msgin.serial(sid);

	doRegister(name, addr, sid, from, *CallbackServer);
}

void CNamingService::cbResendRegistration(NLNET::CMessage &msgin, TSockId from)
{
	string name;
	vector<CInetAddress> addr;
	TServiceId sid;
	msgin.serial(name);
	msgin.serialCont(addr);
	msgin.serial(sid);

	doRegister(name, addr, sid, from, *CallbackServer, true);
}

void CNamingService::cbQueryPort(CMessage &msgin, TSockId from)
{
	// Allocate port
	uint16 port = doAllocatePort(CallbackServer->hostAddress(from));

	// Send port back
	CMessage msgout("QP");
	msgout.serial(port);
	CallbackServer->send(msgout, from);

	nlinfo("The service got port %hu", port);
}
