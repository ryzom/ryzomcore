#include <gmock/gmock.h>
#include <gtest/gtest.h>

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
#include <nel/net/unified_network.h>

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
using ::NLNET::CCallbackClient;
using ::NLNET::CCallbackNetBase;
using ::NLNET::CInetAddress;
using ::NLNET::CInetHost;
using ::NLNET::CMessage;
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
	CNamingService instance;
	int port = 50000;

	void SetUp() override
	{
		CVar basePort;
		basePort.Type = NLMISC::CConfigFile::CVar::T_INT;
		basePort.setAsInt(51000);
		instance.ConfigFile.insertVar("BasePort", basePort);

		CVar uniqueOnShardServices;
		uniqueOnShardServices.Type = NLMISC::CConfigFile::CVar::T_STRING;
		uniqueOnShardServices.setAsString((vector<string>) {});
		instance.ConfigFile.insertVar("UniqueOnShardServices", uniqueOnShardServices);

		CVar uniqueByMachineServices;
		uniqueByMachineServices.Type = NLMISC::CConfigFile::CVar::T_STRING;
		uniqueByMachineServices.setAsString((vector<string>) {});
		instance.ConfigFile.insertVar("UniqueByMachineServices", uniqueByMachineServices);

		CVar nsPort;
		nsPort.Type = NLMISC::CConfigFile::CVar::T_INT;
		nsPort.setAsInt(port);
		instance.ConfigFile.insertVar("NSPort", nsPort);
		instance.init();

		CInetHost host("localhost");
		host.setPort(port);
		client.connect(host);
		ASSERT_THAT(client.connected(), IsTrue());
	}

	void TearDown() override
	{
		client.disconnect();
	}
};

TEST_F(CNamingServiceIT, shouldAnswerToRegistration)
{
	RGRequest request{
		.serviceId = TServiceId(123),
		.name = "test-service",
		.addresses = {"localhost:12345"}
	};
	CMessage msgout("RG");
	msgout.serial(request);

	RGResponse response;
	TCallbackItem callbackArray[] = {
		{ "RG", [&response](CMessage &msgin, TSockId from, CCallbackNetBase &netbase) {
		     msgin.serial(response);
		 } }
	};
	client.addCallbackArray(callbackArray, sizeof(callbackArray) / sizeof(callbackArray[0]));

	client.send(msgout);
	client.flush();
	instance.update();
	client.update2(-1, 100);

	EXPECT_THAT(response.success, IsTrue());
	EXPECT_THAT(response.sid, Optional(Eq(request.serviceId)));
	EXPECT_THAT(
	    response.content,
	    Optional(Field(&RGBResponse::items,
	        ElementsAre(
	            AllOf(
	                Field(&RGBResponseEntry::name, "test-service"),
	                Field(&RGBResponseEntry::sid, Eq(request.serviceId)),
	                Field(&RGBResponseEntry::addr, ElementsAre(Property(&CInetAddress::asString, "[::1]:12345"))))))));
}
