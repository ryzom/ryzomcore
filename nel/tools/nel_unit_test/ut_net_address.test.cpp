// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <gtest/gtest.h>

#include <set>
#include <algorithm>

#include <nel/misc/common.h>
#include <nel/misc/wang_hash.h>

#include <nel/net/ipv6_address.h>
#include <nel/net/inet_address.h>
#include <nel/net/inet_host.h>
#include <nel/net/sock.h>

class CUTNetAddressTest : public testing::Test
{
protected:
	void SetUp() override
	{
	}

	void TearDown() override
	{
	}
};

TEST_F(CUTNetAddressTest, validAddress)
{
	EXPECT_FALSE(NLNET::CIPv6Address().isValid());
	EXPECT_FALSE(NLNET::CIPv6Address("").isValid());
	EXPECT_FALSE(NLNET::CIPv6Address("null").isValid());
	EXPECT_FALSE(NLNET::CIPv6Address("nil").isValid());
	EXPECT_FALSE(NLNET::CIPv6Address("true").isValid());
	EXPECT_FALSE(NLNET::CIPv6Address("false").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("0").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("0.0.0.0").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("127.0.0.1").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("127.0.1.1").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("::1").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("::").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("::ffff:127.0.0.1").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("127.155.116.122").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("18.246.240.121").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("167.32.5.207").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("104.74.179.168").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("95.218.145.185").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("52.201.187.18").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("82.180.47.14").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("151.5.14.166").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("169.147.94.200").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("74.155.131.35").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("b78b:0046:a89c:0585:b7a4:7301:03f0:ab92").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("9af5:687c:4f42:a610:5ab1:14e8:9d3b:d201").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("83cc:afb5:2499:e6ee:eb73:9c8f:b5f9:ab84").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("800c:aebf:bdec:e2b9:24c3:548f:34a5:1128").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("7cb7:5f97:269b:fc25:948c:072d:2d9a:0e5a").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("bee3:7def:05f0:42a9:2d45:c59e:cf35:d57e").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("e39b:45c9:1522:17e5:7318:2ad2:c501:2263").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("5397:a2c8:26e9:bf7a:a2b8:0d15:4e83:973a").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("c3e8:6e2a:7b80:c18d:b1da:be23:eaed:2edb").isValid());
	EXPECT_TRUE(NLNET::CIPv6Address("3416:a1ed:5052:8882:6c85:3e8d:ef33:a7b1").isValid());
	EXPECT_FALSE(NLNET::CIPv6Address(":").isValid());
	EXPECT_FALSE(NLNET::CIPv6Address(".1").isValid());
	EXPECT_FALSE(NLNET::CIPv6Address("1.").isValid());
	EXPECT_FALSE(NLNET::CIPv6Address("127.0.0.1:ffff::").isValid());
}

TEST_F(CUTNetAddressTest, addressType)
{
	EXPECT_EQ(NLNET::CIPv6Address().getType(), NLNET::CIPv6Address::Invalid);
	EXPECT_EQ(NLNET::CIPv6Address("0").getType(), NLNET::CIPv6Address::Any);
	EXPECT_EQ(NLNET::CIPv6Address("0.0.0.0").getType(), NLNET::CIPv6Address::Any);
	EXPECT_EQ(NLNET::CIPv6Address("::").getType(), NLNET::CIPv6Address::Any);
	EXPECT_EQ(NLNET::CIPv6Address("127.0.0.1").getType(), NLNET::CIPv6Address::Loopback);
	EXPECT_EQ(NLNET::CIPv6Address("127.0.1.1").getType(), NLNET::CIPv6Address::Loopback);
	EXPECT_EQ(NLNET::CIPv6Address::loopbackIPv4().getType(), NLNET::CIPv6Address::Loopback);
	EXPECT_EQ(NLNET::CIPv6Address::loopbackIPv6().getType(), NLNET::CIPv6Address::Loopback);
	EXPECT_EQ(NLNET::CIPv6Address::loopback().getType(), NLNET::CIPv6Address::Loopback);
	EXPECT_EQ(NLNET::CIPv6Address::anyIPv4().getType(), NLNET::CIPv6Address::Any);
	EXPECT_EQ(NLNET::CIPv6Address::anyIPv6().getType(), NLNET::CIPv6Address::Any);
	EXPECT_EQ(NLNET::CIPv6Address::any().getType(), NLNET::CIPv6Address::Any);
	EXPECT_EQ(NLNET::CIPv6Address("::1").getType(), NLNET::CIPv6Address::Loopback);
	EXPECT_EQ(NLNET::CIPv6Address("8.8.8.8").getType(), NLNET::CIPv6Address::Internet);
	EXPECT_EQ(NLNET::CIPv6Address("192.168.9.9").getType(), NLNET::CIPv6Address::SiteLocal);
	EXPECT_EQ(NLNET::CIPv6Address("192.168.100.254").getType(), NLNET::CIPv6Address::SiteLocal);
	EXPECT_EQ(NLNET::CIPv6Address("10.0.0.1").getType(), NLNET::CIPv6Address::SiteLocal);
	EXPECT_EQ(NLNET::CIPv6Address("172.16.0.1").getType(), NLNET::CIPv6Address::SiteLocal);
	EXPECT_EQ(NLNET::CIPv6Address("2001:0db8:85a3:0000:0000:8a2e:0370:7334").getType(), NLNET::CIPv6Address::Internet);
	EXPECT_EQ(NLNET::CIPv6Address("2001:0db8:0000:0000:0000:ff00:0042:8329").getType(), NLNET::CIPv6Address::Internet);
	EXPECT_EQ(NLNET::CIPv6Address("2001:db8:1234:5678::1").getType(), NLNET::CIPv6Address::Internet);
	EXPECT_EQ(NLNET::CIPv6Address("2607:f8b0:4006:81a::200e").getType(), NLNET::CIPv6Address::Internet);
	EXPECT_EQ(NLNET::CIPv6Address("2a02:c7d:41f6:e400:1234:5678:90ab:cdef").getType(), NLNET::CIPv6Address::Internet);
	EXPECT_EQ(NLNET::CIPv6Address("fe80::1").getType(), NLNET::CIPv6Address::LinkLocal);
	EXPECT_EQ(NLNET::CIPv6Address("fe80::1234:5678:9abc:def0").getType(), NLNET::CIPv6Address::LinkLocal);
	EXPECT_EQ(NLNET::CIPv6Address("fe80::dc72:7c1f:a73c:b7a1").getType(), NLNET::CIPv6Address::LinkLocal);
	EXPECT_EQ(NLNET::CIPv6Address("fc00:1234:5678::1").getType(), NLNET::CIPv6Address::UniqueLocal);
	EXPECT_EQ(NLNET::CIPv6Address("fd00:1234:5678::1").getType(), NLNET::CIPv6Address::UniqueLocal);
	EXPECT_EQ(NLNET::CIPv6Address("fdc2:4c24:1e4a:f832::1").getType(), NLNET::CIPv6Address::UniqueLocal);
	EXPECT_EQ(NLNET::CIPv6Address("fc00:1:2:3:4:5:6:7").getType(), NLNET::CIPv6Address::UniqueLocal);
}

TEST_F(CUTNetAddressTest, standardAddress)
{
	EXPECT_EQ(NLNET::CIPv6Address::loopbackIPv4(), NLNET::CIPv6Address("127.0.0.1"));
	EXPECT_EQ(NLNET::CIPv6Address::loopbackIPv6(), NLNET::CIPv6Address("::1"));
	EXPECT_EQ(NLNET::CIPv6Address::anyIPv4(), NLNET::CIPv6Address("0.0.0.0"));
	EXPECT_EQ(NLNET::CIPv6Address::anyIPv6(), NLNET::CIPv6Address("::"));
}

TEST_F(CUTNetAddressTest, compareAddress)
{
	static const int count = 1000;
	for (int i = 0; i < count; ++i)
	{
		uint64 uniqueRandomA[2] = { NLMISC::wangHash64(i), NLMISC::wangHash64(i + count) };
		uint64 uniqueRandomB[2] = { NLMISC::wangHash64(i + (count * 2)), NLMISC::wangHash64(i + (count * 3)) };
		NLNET::CIPv6Address a = NLNET::CIPv6Address((uint8 *)uniqueRandomA, 16);
		NLNET::CIPv6Address b = NLNET::CIPv6Address((uint8 *)uniqueRandomB, 16);
		EXPECT_TRUE(a != b) << NLMISC::toString("a(%s) != b(%s)", a.toString().c_str(), b.toString().c_str());
		EXPECT_TRUE(!(a == b)) << NLMISC::toString("!(a(%s) == b(%s))", a.toString().c_str(), b.toString().c_str());
		EXPECT_TRUE(a < b || a > b) << NLMISC::toString("a(%s) < b(%s) || a > b", a.toString().c_str(), b.toString().c_str());
	}
	for (int i = 0; i < count; ++i)
	{
		uint32 uniqueRandomA = NLMISC::wangHash(i);
		uint32 uniqueRandomB = NLMISC::wangHash(i + count);
		NLNET::CIPv6Address a = NLNET::CIPv6Address((uint8 *)(&uniqueRandomA), 4);
		NLNET::CIPv6Address b = NLNET::CIPv6Address((uint8 *)(&uniqueRandomB), 4);
		EXPECT_TRUE(a != b) << NLMISC::toString("a(%s) != b(%s)", a.toString().c_str(), b.toString().c_str());
		EXPECT_TRUE(!(a == b)) << NLMISC::toString("!(a(%s) == b(%s))", a.toString().c_str(), b.toString().c_str());
		EXPECT_TRUE(a < b || a > b) << NLMISC::toString("a(%s) < b(%s) || a > b", a.toString().c_str(), b.toString().c_str());
	}
	for (int i = 0; i < count; ++i)
	{
		uint64 uniqueRandomA[2] = { NLMISC::wangHash64(i), NLMISC::wangHash64(i + count) };
		NLNET::CIPv6Address a = NLNET::CIPv6Address((uint8 *)uniqueRandomA, 16);
		NLNET::CIPv6Address b = NLNET::CIPv6Address((uint8 *)uniqueRandomA, 16);
		EXPECT_TRUE(a == b) << NLMISC::toString("a(%s) == b(%s)", a.toString().c_str(), b.toString().c_str());
		EXPECT_TRUE(!(a != b)) << NLMISC::toString("!(a(%s) != b(%s))", a.toString().c_str(), b.toString().c_str());
		EXPECT_TRUE(!(a < b) && !(a > b)) << NLMISC::toString("!(a(%s) < b(%s)) && !(a > b)", a.toString().c_str(), b.toString().c_str());
	}
	for (int i = 0; i < count; ++i)
	{
		uint32 uniqueRandomA = NLMISC::wangHash(i);
		NLNET::CIPv6Address a = NLNET::CIPv6Address((uint8 *)(&uniqueRandomA), 4);
		NLNET::CIPv6Address b = NLNET::CIPv6Address((uint8 *)(&uniqueRandomA), 4);
		EXPECT_TRUE(a == b) << NLMISC::toString("a(%s) == b(%s)", a.toString().c_str(), b.toString().c_str());
		EXPECT_TRUE(!(a != b)) << NLMISC::toString("!(a(%s) != b(%s))", a.toString().c_str(), b.toString().c_str());
		EXPECT_TRUE(!(a < b) && !(a > b)) << NLMISC::toString("!(a(%s) < b(%s)) && !(a > b)", a.toString().c_str(), b.toString().c_str());
	}
}

TEST_F(CUTNetAddressTest, addressSet)
{
	static const int count = 1000;
	std::set<NLNET::CIPv6Address> addresses;
	for (int i = 0; i < count; ++i)
	{
		uint64 uniqueRandomA[2] = { NLMISC::wangHash64(i), NLMISC::wangHash64(i + count) };
		uniqueRandomA[1] &= 0xFFFF7FFFFFF7FFFFULL; // Ensure no IPv4
		NLNET::CIPv6Address a = NLNET::CIPv6Address((uint8 *)uniqueRandomA, 16);
		EXPECT_FALSE(a.isIPv4());
		addresses.insert(a);
	}
	EXPECT_TRUE(addresses.size() == count);
	for (int i = 0; i < count; ++i)
	{
		uint64 uniqueRandomA[2] = { NLMISC::wangHash64(i), NLMISC::wangHash64(i + count) };
		uniqueRandomA[1] &= 0xFFFF7FFFFFF7FFFFULL; // Ensure no IPv4
		NLNET::CIPv6Address a = NLNET::CIPv6Address((uint8 *)uniqueRandomA, 16);
		EXPECT_FALSE(a.isIPv4());
		addresses.insert(a);
	}
	EXPECT_TRUE(addresses.size() == count);
	for (int i = 0; i < count; ++i)
	{
		uint32 uniqueRandomA = NLMISC::wangHash(i);
		NLNET::CIPv6Address a = NLNET::CIPv6Address((uint8 *)(&uniqueRandomA), 4);
		EXPECT_TRUE(a.isIPv4());
		addresses.insert(a);
	}
	EXPECT_TRUE(addresses.size() == count * 2);
	for (int i = 0; i < count; ++i)
	{
		uint32 uniqueRandomA = NLMISC::wangHash(i);
		NLNET::CIPv6Address a = NLNET::CIPv6Address((uint8 *)(&uniqueRandomA), 4);
		EXPECT_TRUE(a.isIPv4());
		addresses.insert(a);
	}
	EXPECT_EQ(addresses.size(), count * 2);
}

TEST_F(CUTNetAddressTest, stringCompare)
{
	EXPECT_EQ(NLNET::CIPv6Address("::ffff:127.0.0.1").toString(), "127.0.0.1");
	EXPECT_EQ(NLNET::CIPv6Address("0000::0001").toString(), "::1");
	EXPECT_EQ(NLNET::CIPv6Address("www.microsoft.com").toString(), "null");
	EXPECT_EQ(NLNET::CIPv6Address("0").toString(), "0.0.0.0");
	EXPECT_EQ(NLNET::CIPv6Address("1").toString(), "0.0.0.1");
	EXPECT_EQ(NLNET::CIPv6Address("127").toString(), "0.0.0.127");
	EXPECT_EQ(NLNET::CIPv6Address("32639").toString(), "0.0.127.127");
}

TEST_F(CUTNetAddressTest, netParse)
{
	EXPECT_NE(NLNET::CInetAddress("8.8.8.8").asString(), NLNET::CInetAddress("::").asString()); // Wrong constructor
	EXPECT_EQ(NLNET::CInetAddress("8.8.8.8").asString(), "8.8.8.8:0");
	EXPECT_EQ(NLNET::CInetAddress("8.8.8.8"), NLNET::CInetAddress("8.8.8.8:0"));
	EXPECT_EQ(NLNET::CInetAddress("::").asString(), "[::]:0");
}

TEST_F(CUTNetAddressTest, hostParse)
{
	EXPECT_EQ(NLNET::CInetHost("2606:4700:4700::1111").address().getAddress(), NLNET::CIPv6Address("2606:4700:4700::1111"));
	EXPECT_EQ(NLNET::CInetHost("2606:4700:4700::1111:1111").address().getAddress(), NLNET::CIPv6Address("2606:4700:4700::1111:1111"));
	EXPECT_EQ(NLNET::CInetHost("[2606:4700:4700::1111]").address().getAddress(), NLNET::CIPv6Address("2606:4700:4700::1111"));
	EXPECT_EQ(NLNET::CInetHost("[2606:4700:4700::1111]:53").address().getAddress(), NLNET::CIPv6Address("2606:4700:4700::1111"));
	EXPECT_EQ(NLNET::CInetHost("8.8.8.8").address().getAddress(), NLNET::CIPv6Address("8.8.8.8"));
	EXPECT_EQ(NLNET::CInetHost("8.8.8.8:53").address().getAddress(), NLNET::CIPv6Address("8.8.8.8"));
	EXPECT_EQ(NLNET::CInetHost("[8.8.8.8]").address().getAddress(), NLNET::CIPv6Address("8.8.8.8"));
	EXPECT_EQ(NLNET::CInetHost("[8.8.8.8]:53").address().getAddress(), NLNET::CIPv6Address("8.8.8.8"));
	EXPECT_EQ(NLNET::CInetHost("::ffff:8.8.8.8").address().getAddress(), NLNET::CIPv6Address("8.8.8.8"));
	EXPECT_EQ(NLNET::CInetHost("[::ffff:8.8.8.8]").address().getAddress(), NLNET::CIPv6Address("8.8.8.8"));
	EXPECT_EQ(NLNET::CInetHost("[::ffff:8.8.8.8]:53").address().getAddress(), NLNET::CIPv6Address("8.8.8.8"));
	EXPECT_ANY_THROW(NLNET::CInetHost("::ffff:8.8.8.8:53"));
}

TEST_F(CUTNetAddressTest, hostLookup)
{
	NLNET::CInetHost googleDns("dns.google");
	EXPECT_TRUE(!googleDns.isValid());
	EXPECT_TRUE(googleDns.isAddressValid());
	EXPECT_TRUE(googleDns.addresses().size() >= 2);
	EXPECT_TRUE((std::find(googleDns.addresses().begin(), googleDns.addresses().end(), NLNET::CInetAddress("8.8.8.8")) != googleDns.addresses().end())
	    || (std::find(googleDns.addresses().begin(), googleDns.addresses().end(), NLNET::CInetAddress("2001:4860:4860::8888")) != googleDns.addresses().end()));
	EXPECT_TRUE((std::find(googleDns.addresses().begin(), googleDns.addresses().end(), NLNET::CInetAddress("8.8.4.4")) != googleDns.addresses().end())
	    || (std::find(googleDns.addresses().begin(), googleDns.addresses().end(), NLNET::CInetAddress("2001:4860:4860::8844")) != googleDns.addresses().end()));

	NLNET::CInetHost oneDns("one.one.one.one:53");
	EXPECT_TRUE(oneDns.isValid());
	EXPECT_TRUE(oneDns.isAddressValid());
	EXPECT_TRUE(oneDns.addresses().size() >= 2);
	EXPECT_TRUE((std::find(oneDns.addresses().begin(), oneDns.addresses().end(), NLNET::CInetAddress("1.1.1.1:53")) != oneDns.addresses().end())
	    || (std::find(oneDns.addresses().begin(), oneDns.addresses().end(), NLNET::CInetAddress("[2606:4700:4700::1111]:53")) != oneDns.addresses().end()));
	EXPECT_TRUE((std::find(oneDns.addresses().begin(), oneDns.addresses().end(), NLNET::CInetAddress("1.0.0.1:53")) != oneDns.addresses().end())
	    || (std::find(oneDns.addresses().begin(), oneDns.addresses().end(), NLNET::CInetAddress("[2606:4700:4700::1001]:53")) != oneDns.addresses().end()));

	EXPECT_THROW(NLNET::CInetHost("invalid.invalid:80"), NLNET::ESocket);
}
