#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <optional>
#include <string>
#include <vector>

#include <nel/misc/config_file.h>
#include <nel/misc/stream.h>
#include <nel/net/buf_net_base.h>
#include <nel/net/callback_client.h>
#include <nel/net/callback_net_base.h>
#include <nel/net/inet_address.h>
#include <nel/net/inet_host.h>
#include <nel/net/message.h>
#include <nel/net/naming_client.h>
#include <nel/net/unified_network.h>

#include <nelns/naming_service/service_entry.h>
#include <nelns/naming_service/naming_service.h>
#include <nelns/naming_service/variables.h>

using ::std::nullopt;
using ::std::optional;
using ::std::string;
using ::std::vector;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Field;
using ::testing::FieldsAre;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::Not;
using ::testing::NotNull;
using ::testing::Optional;
using ::testing::Property;
using ::testing::SizeIs;
using ::testing::StrEq;

using CVar = ::NLMISC::CConfigFile::CVar;
using ::NLMISC::IStream;
using ::NLMISC::nlSleep;
using ::NLNET::CCallbackClient;
using ::NLNET::CCallbackNetBase;
using ::NLNET::CInetAddress;
using ::NLNET::CInetHost;
using ::NLNET::CMessage;
using ::NLNET::CNamingClient;
using ::NLNET::TCallbackItem;
using ::NLNET::TServiceId;
using ::NLNET::TSockId;

struct RGBResponseEntry
{
	string name;
	TServiceId sid;
	vector<CInetAddress> addr;
	void serial(IStream &stream)
	{
		stream.serial(name);
		stream.serial(sid);
		stream.serialCont(addr);
	}
};

struct RGBResponse
{
	vector<RGBResponseEntry> items;
	void serial(IStream &stream)
	{
		TServiceId::size_type size;
		stream.serial(size);
		for (TServiceId::size_type i = 0; i < size; ++i)
		{
			RGBResponseEntry entry;

			stream.serial(entry);
			items.push_back(entry);
		}
	}
};

struct RGResponse
{
	bool success;
	optional<TServiceId> sid;
	optional<RGBResponse> content;
	optional<string> reason;
	void serial(IStream &stream)
	{
		stream.serial(success);

		if (success)
		{
			TServiceId sidIn;
			stream.serial(sidIn);
			sid = sidIn;
			RGBResponse contentIn;
			stream.serial(contentIn);
			content = contentIn;
		}
		else
		{
			string reasonIn;
			stream.serial(reasonIn);
			reason = reasonIn;
		}
	}
};

struct RGRequest
{
	TServiceId serviceId;
	string name;
	vector<CInetAddress> addresses;

	void serial(IStream &stream)
	{
		stream.serial(name);
		stream.serialCont(addresses);
		stream.serial(serviceId);
	}
};

class CNamingServiceIT : public testing::Test
{
protected:
	CCallbackClient client;
	CNamingService namingService;
	CInetHost host = CInetHost("localhost");
	int port = 50000;
	int minPort = 51000;
	std::chrono::seconds defaultTimeout = std::chrono::seconds(10);

	void SetUp() override
	{
		CVar basePort;
		basePort.Type = NLMISC::CConfigFile::CVar::T_INT;
		basePort.setAsInt(minPort);
		namingService.ConfigFile.insertVar("BasePort", basePort);

		CVar uniqueOnShardServices;
		uniqueOnShardServices.Type = NLMISC::CConfigFile::CVar::T_STRING;
		uniqueOnShardServices.setAsString((vector<string>) {});
		namingService.ConfigFile.insertVar("UniqueOnShardServices", uniqueOnShardServices);

		CVar uniqueByMachineServices;
		uniqueByMachineServices.Type = NLMISC::CConfigFile::CVar::T_STRING;
		uniqueByMachineServices.setAsString((vector<string>) {});
		namingService.ConfigFile.insertVar("UniqueByMachineServices", uniqueByMachineServices);

		CVar nsPort;
		nsPort.Type = NLMISC::CConfigFile::CVar::T_INT;
		nsPort.setAsInt(port);
		namingService.ConfigFile.insertVar("NSPort", nsPort);
		namingService.init();

		host.setPort(port);
		client.connect(host);
		ASSERT_THAT(client.connected(), IsTrue());
	}

	void TearDown() override
	{
		client.disconnect();
		client.update2(-1, 200);
		namingService.release();
		RegisteredServices.clear();
	}
};

TEST_F(CNamingServiceIT, shouldAnswerToRegistration)
{
	RGRequest request {
		.serviceId = TServiceId(123),
		.name = "test register service",
		.addresses = { "localhost:12345" }
	};
	CMessage msgout("RG");
	msgout.serial(request);

	std::promise<RGResponse> response_promise;
	std::future<RGResponse> response = response_promise.get_future();
	TCallbackItem callbackArray[] = {
		{ "RG", [&response_promise](CMessage &msgin, TSockId from, CCallbackNetBase &netbase) {
		     RGResponse response;
		     msgin.serial(response);
		     response_promise.set_value(response);
		 } }
	};
	client.addCallbackArray(callbackArray, sizeof(callbackArray) / sizeof(callbackArray[0]));

	client.send(msgout);
	auto updateClient = std::async(std::launch::async, [=, &response]() {
		while (response.valid())
		{
			client.update();
			nlSleep(1);
		}
	});
	auto updateNamingService = std::async(std::launch::async, [=, &response]() {
		while (response.valid())
		{
			namingService.update();
			nlSleep(1);
		}
	});

	auto state = response.wait_for(defaultTimeout);
	ASSERT_THAT(state, Eq(std::future_status::ready));
	EXPECT_THAT(response.get(),
	    AllOf(
	        Field(&RGResponse::success, IsTrue()),
	        Field(&RGResponse::sid, Optional(Eq(request.serviceId))),
	        Field(
	            &RGResponse::content,
	            Optional(Field(&RGBResponse::items,
	                ElementsAre(
	                    AllOf(
	                        Field(&RGBResponseEntry::name, Eq(request.name)),
	                        Field(&RGBResponseEntry::sid, Eq(request.serviceId)),
	                        Field(&RGBResponseEntry::addr, ElementsAre(Property(&CInetAddress::asString, "[::1]:12345"))))))))));
}

TEST_F(CNamingServiceIT, shouldUpdateServiceRegistry)
{
	RGRequest request {
		.serviceId = TServiceId(123),
		.name = "test re-register service",
		.addresses = { "localhost:12345" }
	};
	CMessage msgout("RRG");
	msgout.serial(request);

	client.send(msgout);
	client.flush();
	client.update2(-1, 200);
	namingService.update();
	client.update2(-1, 200);

	EXPECT_THAT(
	    RegisteredServices,
	    ElementsAre(
	        AllOf(
	            Field(&CServiceEntry::Name, Eq(request.name)),
	            Field(&CServiceEntry::SId, Eq(request.serviceId)),
	            Field(&CServiceEntry::Addr, ElementsAre(Property(&CInetAddress::asString, "[::1]:12345"))))));
}

TEST_F(CNamingServiceIT, shouldAnswerToQueryPort)
{
	vector<CInetAddress> addresses { "localhost:12345" };
	CNamingClient::connect(host, NLNET::CCallbackNetBase::Off, addresses);

	auto response = std::async(std::launch::async, &CNamingClient::queryServicePort);
	auto update = std::async(std::launch::async, [=, &response]() {
		while (response.valid())
		{
			namingService.update();
			nlSleep(1);
		}
	});

	auto state = response.wait_for(defaultTimeout);
	ASSERT_THAT(state, Eq(std::future_status::ready));
	EXPECT_THAT(response.get(), Eq(minPort));
}
