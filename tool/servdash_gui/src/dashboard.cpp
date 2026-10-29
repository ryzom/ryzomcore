// Ryzom Core - MMORPG Framework <https://wiki.ryzom.dev/>
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

#include "dashboard.h"

#include <QTimer>

#include <climits>

CDashboard::CDashboard(const CDashboardConfig &config, QObject *parent)
    : QObject(parent)
    , m_Title(config.Title)
{
	for (const CSectionConfig &section : config.Sections)
	{
		CProcessEntry *entry = new CProcessEntry(section, this);
		connect(entry, &CProcessEntry::launchWanted, this, &CDashboard::onLaunchWanted);
		connect(entry, &CProcessEntry::stopWanted, this, &CDashboard::onStopWanted);
		connect(entry, &CProcessEntry::stateChanged, this, &CDashboard::onStateChanged);
		connect(entry, &CProcessEntry::exited, this, &CDashboard::onExited);
		m_Entries.append(entry);
	}
	m_Timer = new QTimer(this);
	m_Timer->setInterval(250);
	connect(m_Timer, &QTimer::timeout, this, &CDashboard::onTick);
	m_Timer->start();
}

bool CDashboard::anyRunning() const
{
	for (CProcessEntry *entry : m_Entries)
		if (entry->running())
			return true;
	return false;
}

void CDashboard::start()
{
	for (CProcessEntry *entry : m_Entries)
		if (entry->autoLaunch())
			m_Queue.append(entry);
	autoStart();
}

void CDashboard::onTick()
{
	for (CProcessEntry *entry : m_Entries)
		entry->tick();
}

// *** the start order ***

void CDashboard::autoStart()
{
	if (m_Closing)
		return;
	if (m_AutoStarting)
	{
		// a launch raised state changes that call back here; run again when it is done
		m_AutoStartAgain = true;
		return;
	}
	m_AutoStarting = true;
	for (;;)
	{
		bool waitForCapture = false;
		for (CProcessEntry *entry : m_Entries)
			if (entry->state() == CProcessEntry::Launched && entry->config().Priority == INT_MAX)
				waitForCapture = true;
		if (waitForCapture)
			break;

		int lowest = INT_MAX;
		for (CProcessEntry *entry : m_Entries)
		{
			CProcessEntry::TState state = entry->state();
			bool waiting = (state == CProcessEntry::Stopped || state == CProcessEntry::New) && m_Queue.contains(entry);
			bool busy = state == CProcessEntry::Launched || state == CProcessEntry::Captured || state == CProcessEntry::Stopping;
			if ((waiting || busy) && entry->config().Priority < lowest)
				lowest = entry->config().Priority;
		}

		CProcessEntry *next = nullptr;
		for (CProcessEntry *entry : m_Queue)
		{
			CProcessEntry::TState state = entry->state();
			if ((state == CProcessEntry::Stopped || state == CProcessEntry::New) && entry->config().Priority <= lowest)
			{
				next = entry;
				break;
			}
		}
		if (next == nullptr)
			break;
		m_Queue.removeAll(next);
		next->launch();
	}
	m_AutoStarting = false;
	if (m_AutoStartAgain)
	{
		m_AutoStartAgain = false;
		autoStart();
	}
}

void CDashboard::onLaunchWanted(CProcessEntry *entry)
{
	if (m_Closing)
		return;
	if (!m_Queue.contains(entry))
		m_Queue.prepend(entry);
	autoStart();
}

void CDashboard::onStopWanted(CProcessEntry *entry)
{
	m_Queue.removeAll(entry);
	entry->shutdown();
}

void CDashboard::onStateChanged(CProcessEntry *entry)
{
	Q_UNUSED(entry);
	// launched, captured, ready and stopping all move the start order on
	autoStart();
}

void CDashboard::onExited(CProcessEntry *entry)
{
	Q_UNUSED(entry);
	if (m_Closing)
		checkAllStopped();
	else
		autoStart();
}

void CDashboard::launchAll()
{
	if (m_Closing)
		return;
	m_Queue.clear();
	for (CProcessEntry *entry : m_Entries)
		m_Queue.append(entry);
	autoStart();
}

void CDashboard::shutdownAll()
{
	m_Queue.clear();
	for (CProcessEntry *entry : m_Entries)
		entry->shutdown();
}

void CDashboard::beginClosing()
{
	m_Closing = true;
	for (CProcessEntry *entry : m_Entries)
		entry->setLaunchBlocked(true);
	shutdownAll();
	checkAllStopped();
}

void CDashboard::killAll()
{
	for (CProcessEntry *entry : m_Entries)
		entry->kill();
}

void CDashboard::checkAllStopped()
{
	if (m_Closing && !anyRunning())
		emit allStopped();
}
