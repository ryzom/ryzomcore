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

TEST(CNamingService, shouldAnswerToRegistration)
{
	CCallbackClient client;
	CNamingService instance;
	CVar basePort;
	basePort.Type = NLMISC::CConfigFile::CVar::T_INT;
	basePort.setAsInt(51000);
	CVar uniqueOnShardServices;
	uniqueOnShardServices.Type = NLMISC::CConfigFile::CVar::T_STRING;
	uniqueOnShardServices.setAsString((vector<string>) {});
	CVar uniqueByMachineServices;
	uniqueByMachineServices.Type = NLMISC::CConfigFile::CVar::T_STRING;
	uniqueByMachineServices.setAsString((vector<string>) {});
	CVar nsPort;
	nsPort.Type = NLMISC::CConfigFile::CVar::T_INT;
	int port = 50001;
	nsPort.setAsInt(port);
	TServiceId serviceId(123);
	CMessage msgout("RG");
	string name("test-service");
	vector<CInetAddress> addresses = { "localhost:12345" };
	msgout.serial(name);
	msgout.serialCont(addresses);
	msgout.serial(serviceId);

	instance.ConfigFile.insertVar("BasePort", basePort);
	instance.ConfigFile.insertVar("NSPort", nsPort);
	instance.ConfigFile.insertVar("UniqueOnShardServices", uniqueOnShardServices);
	instance.ConfigFile.insertVar("UniqueByMachineServices", uniqueByMachineServices);

	instance.init();

	CInetHost host("localhost");
	host.setPort(port);
	client.connect(host);
	ASSERT_THAT(client.connected(), IsTrue());
	TCallbackItem callbackArray[] = {
		{ "RG", [](CMessage &msgin, TSockId from, CCallbackNetBase &netbase) {
		     RGResponse response;
		     msgin.serial(response);

		     EXPECT_THAT(response.success, IsTrue());
		     EXPECT_THAT(response.sid, Optional(Property(&TServiceId::get, Eq(123))));
		     EXPECT_THAT(
		         response.content,
		         Optional(Field(&RGBResponse::items,
		             ElementsAre(
		                 AllOf(
		                     Field(&RGBResponseEntry::name, "test-service"),
		                     Field(&RGBResponseEntry::sid, Property(&TServiceId::get, 123)),
		                     Field(&RGBResponseEntry::addr, ElementsAre(Property(&CInetAddress::asString, "[::1]:12345"))))))));
		 } }
	};

	client.addCallbackArray(callbackArray, sizeof(callbackArray) / sizeof(callbackArray[0]));
	client.send(msgout);
	client.update(1000);
	instance.update();
	client.update(1000);
}
