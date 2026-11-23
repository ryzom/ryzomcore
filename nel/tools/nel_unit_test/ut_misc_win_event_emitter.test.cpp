// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <nel/misc/types_nl.h>

#ifdef NL_OS_WINDOWS

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <vector>

#include <nel/misc/events.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/event_server.h>
#include <nel/misc/win_event_emitter.h>

namespace {

// keeps the characters and counts the IME events an emitter posts
class CCollector : public NLMISC::IEventListener
{
public:
	CCollector()
	    : IMEEvents(0)
	{
	}

	virtual void operator()(const NLMISC::CEvent &event)
	{
		if (event == NLMISC::EventCharId)
			Chars.push_back(static_cast<const NLMISC::CEventChar &>(event).Char);
		else if (event == NLMISC::EventIME)
			++IMEEvents;
	}

	std::vector<u32char> Chars;
	uint IMEEvents;
};

}

// The messages are handed to the emitter directly: no window is created and no message queue is read.
class CUTMiscWinEventEmitter : public testing::Test
{
protected:
	virtual void SetUp()
	{
		m_Server.addListener(NLMISC::EventCharId, &m_Collector);
		m_Server.addListener(NLMISC::EventIME, &m_Collector);
	}

	virtual void TearDown()
	{
		m_Server.removeListener(NLMISC::EventCharId, &m_Collector);
		m_Server.removeListener(NLMISC::EventIME, &m_Collector);
	}

	// true when the emitter traps the message, so that the window procedure does not pass it to DefWindowProc
	bool send(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0)
	{
		bool trapped = m_Emitter.processMessage(NULL, msg, wParam, lParam, &m_Server);
		m_Server.pump();
		return trapped;
	}

	NLMISC::CWinEventEmitter m_Emitter;
	NLMISC::CEventServer m_Server;
	CCollector m_Collector;
};

TEST_F(CUTMiscWinEventEmitter, charOfTheBasicPlane)
{
	EXPECT_FALSE(send(WM_CHAR, 0x0416)); // CYRILLIC CAPITAL LETTER ZHE
	EXPECT_FALSE(send(WM_CHAR, 0x65E5)); // a CJK ideograph, as an IME delivers it
	ASSERT_EQ(2u, m_Collector.Chars.size());
	EXPECT_EQ((u32char)0x0416, m_Collector.Chars[0]);
	EXPECT_EQ((u32char)0x65E5, m_Collector.Chars[1]);
}

TEST_F(CUTMiscWinEventEmitter, surrogatePairIsOneChar)
{
	// U+1F600 comes as two WM_CHAR
	send(WM_CHAR, 0xD83D);
	EXPECT_TRUE(m_Collector.Chars.empty());
	send(WM_CHAR, 0xDE00);
	ASSERT_EQ(1u, m_Collector.Chars.size());
	EXPECT_EQ((u32char)0x1F600, m_Collector.Chars[0]);

	// U+20BB7, an ideograph outside the basic plane
	send(WM_CHAR, 0xD842);
	send(WM_CHAR, 0xDFB7);
	ASSERT_EQ(2u, m_Collector.Chars.size());
	EXPECT_EQ((u32char)0x20BB7, m_Collector.Chars[1]);
}

TEST_F(CUTMiscWinEventEmitter, halfAPairIsNoChar)
{
	// a second half alone
	send(WM_CHAR, 0xDE00);
	EXPECT_TRUE(m_Collector.Chars.empty());

	// a first half followed by an ordinary character: only that character
	send(WM_CHAR, 0xD83D);
	send(WM_CHAR, 'a');
	ASSERT_EQ(1u, m_Collector.Chars.size());
	EXPECT_EQ((u32char)'a', m_Collector.Chars[0]);

	// the dropped first half is not kept for a later second half
	send(WM_CHAR, 0xDE00);
	EXPECT_EQ(1u, m_Collector.Chars.size());

	// two first halves: the second one starts the pair
	send(WM_CHAR, 0xD83D);
	send(WM_CHAR, 0xD842);
	send(WM_CHAR, 0xDFB7);
	ASSERT_EQ(2u, m_Collector.Chars.size());
	EXPECT_EQ((u32char)0x20BB7, m_Collector.Chars[1]);
}

TEST_F(CUTMiscWinEventEmitter, imeMessagesAreLeftToTheSystemByDefault)
{
	EXPECT_FALSE(m_Emitter.areIMEEventsEnabled());

	const UINT messages[] = { WM_IME_SETCONTEXT, WM_IME_STARTCOMPOSITION, WM_IME_COMPOSITION, WM_IME_ENDCOMPOSITION, WM_IME_NOTIFY, WM_INPUTLANGCHANGE };
	for (size_t i = 0; i < sizeof(messages) / sizeof(messages[0]); ++i)
	{
		SCOPED_TRACE(messages[i]);
		EXPECT_FALSE(send(messages[i]));
	}
	EXPECT_EQ(0u, m_Collector.IMEEvents);
}

TEST_F(CUTMiscWinEventEmitter, imeMessagesAreTrappedOnRequest)
{
	m_Emitter.enableIMEEvents(true);

	const UINT messages[] = { WM_IME_SETCONTEXT, WM_IME_STARTCOMPOSITION, WM_IME_COMPOSITION, WM_IME_ENDCOMPOSITION, WM_IME_NOTIFY, WM_INPUTLANGCHANGE };
	const uint count = (uint)(sizeof(messages) / sizeof(messages[0]));
	for (uint i = 0; i < count; ++i)
	{
		SCOPED_TRACE(messages[i]);
		EXPECT_TRUE(send(messages[i]));
	}
	EXPECT_EQ(count, m_Collector.IMEEvents);

	// the characters still arrive
	EXPECT_FALSE(send(WM_CHAR, 0x65E5));
	EXPECT_EQ(1u, m_Collector.Chars.size());
}

TEST_F(CUTMiscWinEventEmitter, ignoredWheelIsNoIMEEvent)
{
	m_Emitter.enableIMEEvents(true);
	m_Emitter.enableMouseEvents(false);

	EXPECT_FALSE(send(WM_MOUSEWHEEL));
	EXPECT_EQ(0u, m_Collector.IMEEvents);
}

#endif // NL_OS_WINDOWS
