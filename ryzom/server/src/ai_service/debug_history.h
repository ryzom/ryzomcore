// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef RYAI_DEBUG_HISTORY_H
#define RYAI_DEBUG_HISTORY_H

class CDebugHistory
{
private:
	typedef std::deque<std::string> THistoryContainer;
public:
	explicit CDebugHistory()
	: m_Recording(false)
	{
	}
	virtual	~CDebugHistory() { }
	void addHistory(std::string const& txt)
	{
		if (m_Recording)
			m_History.push_back(txt);
	}
	void addHistory(char const* txt)
	{
		if (m_Recording)
			addHistory(NLMISC::toString(txt));
	}
	
	template <class A>
	void addHistory(char const* txt, A a) { if (m_Recording) addHistory(NLMISC::toString(txt, a)); }
	template <class A, class B>
	void addHistory(char const* txt, A a, B b) { if (m_Recording) addHistory(NLMISC::toString(txt, a, b)); }
	template <class A, class B, class C>
	void addHistory(char const* txt, A a, B b, C c) { if (m_Recording) addHistory(NLMISC::toString(txt, a, b, c)); }
	template <class A, class B, class C, class D>
	void addHistory(char const* txt, A a, B b, C c, D d) { if (m_Recording) addHistory(NLMISC::toString(txt, a, b, c, d)); }
	
	void setRecording(bool val) { m_Recording = val; }
	inline bool isRecording() { return m_Recording; }
	
	void writeAsInfo()
	{
		int j = 0;
		FOREACH(itHistory, THistoryContainer, m_History)
		{
			nlinfo("HIST %3i: %s", j, itHistory->c_str());
			++j;
		}
	}
	
private:
	bool m_Recording;
	THistoryContainer m_History;
};

#endif
