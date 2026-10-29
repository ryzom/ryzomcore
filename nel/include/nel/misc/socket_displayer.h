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

#ifndef NL_SOCKET_DISPLAYER_H
#define NL_SOCKET_DISPLAYER_H

#include "command.h"
#include "types_nl.h"
#include "time_nl.h"
#include "window_displayer.h"

#include <list>
#include <map>
#include <string>
#include <vector>

namespace NLMISC {

/**
 * Service console over a local socket, for a dashboard that runs in another process.
 *
 * The service connects to the path given on its command line (--ctrl=<path>): a named
 * pipe on Windows, a Unix domain socket elsewhere. The dashboard owns the window. This
 * displayer streams the log lines, the labels and the title bar to it, and executes the
 * commands it sends back, the same way CWinDisplayer does with its own controls. The
 * connection is retried while the dashboard is away, and the recent history is replayed
 * on every connection.
 *
 * Wire format: one message per line, fields separated by a tab, a backslash escaping
 * tab, newline, carriage return and backslash inside a field.
 *   service to dashboard: hello <version>, title <text>, label <index> <value>, log <rrggbb> <text>,
 *     command <name> <kind> <args> <help>, uncommand <name>, done <id> <1|0>
 *   dashboard to service: cmd <command line>, exec <id> <name> [<arg>...]
 * `exec` runs one command with its arguments as given, nothing is parsed, and `done` follows
 * with the id once it ran and its output is in the log, so a caller can take the lines in
 * between as the command's output. Version 2 of the protocol added exec and done.
 * The command table follows the command registry live: every entry on connection, then each
 * addition and removal. The kind is c for a command or variable, o for a named handler
 * instance, m for a member command of an instance (instance.command).
 * A label value follows CWindowDisplayer::createLabel: empty is a separator, "@Name|command"
 * is a button, anything else is a variable label.
 *
 * MT = main thread, DT = display thread, as in CWindowDisplayer.
 */
class CSocketDisplayer : public CWindowDisplayer, public ICommandRegistryListener
{
public:
	CSocketDisplayer(const std::string &path, const char *displayerName = "");
	virtual ~CSocketDisplayer() NL_OVERRIDE;

	// (MT) the command registry listener; queued for the display thread
	virtual void onCommandAdded(const std::string &name, TCommandEntryKind kind, const std::string &args, const std::string &help) NL_OVERRIDE;
	virtual void onCommandRemoved(const std::string &name) NL_OVERRIDE;

	// (MT) the text shown as the window title by the dashboard
	virtual void setTitleBar(const std::string &titleBar) NL_OVERRIDE;

protected:
	// (DT)
	virtual void open(std::string titleBar, bool iconified, sint x, sint y, sint w, sint h, sint hs, sint fs, const std::string &fn, bool ww, CLog *log) NL_OVERRIDE;
	// (DT)
	virtual void display_main() NL_OVERRIDE;
	// (MT) runs the exec commands and queues their done markers
	virtual void updateExtra() NL_OVERRIDE;

private:
	bool connectToDashboard();
	void disconnect();
	bool sendLine(const std::string &line);
	void pollIncoming();
	void handleLine(const std::string &line);
	void flushLog();
	void flushLabels(bool all);
	void flushTitle();
	void flushCommands();
	void flushDone();
	void replay();

	static std::string escapeField(const std::string &value);
	static std::string unescapeField(const std::string &value);

	struct CCommandEntry
	{
		std::string Name;
		TCommandEntryKind Kind;
		std::string Args;
		std::string Help;
		bool Removed;
	};

	bool sendCommand(const CCommandEntry &entry);

	/// An exec message: a command with its arguments already split, and the id its done marker carries
	struct CArgCommand
	{
		std::string Id;
		std::string Name;
		std::vector<std::string> Args;
	};

	std::string m_Path;
	CSynchronized<std::string> m_TitleBar;
	// (MT writes, DT reads) what the registry said since the last flush
	CSynchronized<std::vector<CCommandEntry>> m_CommandQueue;
	// (DT) the command table as the dashboard should have it
	std::map<std::string, CCommandEntry> m_Commands;
	// (DT writes, MT reads) exec messages waiting for the main thread
	CSynchronized<std::vector<CArgCommand>> m_ArgCommands;
	// (MT writes, DT reads) the done markers to send: id and success
	CSynchronized<std::vector<std::pair<std::string, bool>>> m_DoneQueue;
	std::string m_SentTitleBar;
	std::list<std::pair<uint32, std::string> > m_History;
	uint m_HistoryMax;
	std::string m_InBuffer;
	bool m_Connected;
	bool m_PathWarned;
	TTime m_LastConnectAttempt;
#ifdef NL_OS_WINDOWS
	void *m_Pipe;
	void *m_Event;
#else
	int m_Socket;
#endif
};

} // NLMISC

#endif // NL_SOCKET_DISPLAYER_H

/* End of socket_displayer.h */
