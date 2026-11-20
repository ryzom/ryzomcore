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

#include "stdmisc.h"

#include "nel/misc/ctrl_protocol.h"

#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/string_common.h"

#ifndef NL_OS_WINDOWS
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef NL_OS_MAC
#include <sys/sysctl.h>
#endif
#endif

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC {

// *** fields ***

std::string escapeCtrlField(const std::string &value)
{
	std::string result;
	result.reserve(value.size());
	for (std::string::size_type i = 0; i < value.size(); ++i)
	{
		char c = value[i];
		switch (c)
		{
		case '\\': result += "\\\\"; break;
		case '\t': result += "\\t"; break;
		case '\n': result += "\\n"; break;
		case '\r': result += "\\r"; break;
		default: result += c; break;
		}
	}
	return result;
}

std::string unescapeCtrlField(const std::string &value)
{
	std::string result;
	result.reserve(value.size());
	for (std::string::size_type i = 0; i < value.size(); ++i)
	{
		char c = value[i];
		if (c == '\\' && i + 1 < value.size())
		{
			++i;
			switch (value[i])
			{
			case 't': result += '\t'; break;
			case 'n': result += '\n'; break;
			case 'r': result += '\r'; break;
			default: result += value[i]; break;
			}
		}
		else
		{
			result += c;
		}
	}
	return result;
}

void splitCtrlLine(const std::string &line, std::vector<std::string> &fields)
{
	fields.clear();
	std::string::size_type start = 0;
	for (;;)
	{
		std::string::size_type end = line.find('\t', start);
		fields.push_back(unescapeCtrlField(line.substr(start, end == std::string::npos ? std::string::npos : end - start)));
		if (end == std::string::npos)
			break;
		start = end + 1;
	}
}

std::string joinCtrlLine(const std::vector<std::string> &fields)
{
	std::string line;
	for (uint i = 0; i < fields.size(); ++i)
	{
		if (i)
			line += '\t';
		line += escapeCtrlField(fields[i]);
	}
	return line;
}

// *** messages ***

CCtrlMessageReader::CCtrlMessageReader(size_t maxLine, size_t maxData)
    : m_MaxLine(maxLine)
    , m_MaxData(maxData)
    , m_InData(false)
    , m_DataLength(0)
{
}

void CCtrlMessageReader::clear()
{
	m_Buffer.clear();
	m_InData = false;
	m_DataLength = 0;
	m_DataFields.clear();
}

bool CCtrlMessageReader::next(CCtrlMessage &message, std::string &error)
{
	error.clear();
	if (m_InData)
	{
		if (m_Buffer.size() < m_DataLength)
			return false;
		message.Fields = m_DataFields;
		message.IsData = true;
		message.Data.assign(m_Buffer, 0, m_DataLength);
		m_Buffer.erase(0, m_DataLength);
		m_InData = false;
		m_DataFields.clear();
		return true;
	}

	std::string::size_type pos = m_Buffer.find('\n');
	if (pos == std::string::npos)
	{
		if (m_Buffer.size() > m_MaxLine)
			error = toString("a line longer than %u bytes", (uint)m_MaxLine);
		return false;
	}
	std::string line = m_Buffer.substr(0, pos);
	m_Buffer.erase(0, pos + 1);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	std::vector<std::string> fields;
	splitCtrlLine(line, fields);
	if (fields.size() >= 4 && fields[0] == "data")
	{
		uint64 length = 0;
		if (!fromString(fields[3], length) || length > m_MaxData)
		{
			error = toString("a data frame of '%s' bytes, %u at most", fields[3].c_str(), (uint)m_MaxData);
			return false;
		}
		m_InData = true;
		m_DataLength = (size_t)length;
		m_DataFields = fields;
		return next(message, error);
	}
	message.Fields.swap(fields);
	message.IsData = false;
	message.Data.clear();
	return true;
}

// *** registry ***

bool CCtrlRegistry::getDirectory(std::string &dir, bool create, std::string &error)
{
#ifdef NL_OS_WINDOWS
	const wchar_t *base = _wgetenv(L"LOCALAPPDATA");
	if (base == nullptr || base[0] == 0)
	{
		error = "LOCALAPPDATA is not set, so there is no registry of control endpoints (%LOCALAPPDATA%\\nel\\ctrl)";
		return false;
	}
	dir = CPath::standardizePath(wideToUtf8(base)) + "nel/ctrl";
	if (create && !CFile::isDirectory(dir) && !CFile::createDirectoryTree(dir))
	{
		error = toString("cannot create the registry of control endpoints '%s'", dir.c_str());
		return false;
	}
	return true;
#else
	const char *base = getenv("XDG_RUNTIME_DIR");
	if (base == nullptr || base[0] == 0)
	{
		error = "XDG_RUNTIME_DIR is not set, so there is no registry of control endpoints ($XDG_RUNTIME_DIR/nel.ctrl)";
		return false;
	}
	dir = CPath::standardizePath(base) + "nel.ctrl";
	struct stat st;
	if (lstat(dir.c_str(), &st) != 0)
	{
		if (!create)
		{
			error = toString("the registry of control endpoints '%s' does not exist", dir.c_str());
			return false;
		}
		if (mkdir(dir.c_str(), 0700) != 0 && errno != EEXIST)
		{
			error = toString("cannot create the registry of control endpoints '%s': %s", dir.c_str(), formatErrorMessage(getLastError()).c_str());
			return false;
		}
		if (lstat(dir.c_str(), &st) != 0)
		{
			error = toString("cannot read the registry of control endpoints '%s': %s", dir.c_str(), formatErrorMessage(getLastError()).c_str());
			return false;
		}
	}
	// the sockets live in it, so it must be ours and closed to everyone else
	if (!S_ISDIR(st.st_mode) || st.st_uid != geteuid() || (st.st_mode & 077) != 0)
	{
		error = toString("the registry of control endpoints '%s' must be a folder of this user with mode 0700", dir.c_str());
		return false;
	}
	return true;
#endif
}

std::string CCtrlRegistry::recordPath(const std::string &dir, const std::string &name, uint32 pid)
{
	return CPath::standardizePath(dir) + toString("%s.%u.json", name.c_str(), pid);
}

std::string CCtrlRegistry::endpointPath(const std::string &dir, const std::string &name, uint32 pid)
{
#ifdef NL_OS_WINDOWS
	(void)dir;
	return toString("\\\\.\\pipe\\nel.ctrl.%s.%u", name.c_str(), pid);
#else
	return CPath::standardizePath(dir) + toString("%s.%u.sock", name.c_str(), pid);
#endif
}

bool CCtrlRegistry::writeRecord(const std::string &path, const CJsonValue &record, std::string &error)
{
	std::string text = record.toString();
	text += '\n';
	std::string temp = path + toString(".%u.tmp", getCurrentProcessId());
	FILE *f = nlfopen(temp, "wb");
	if (f == nullptr)
	{
		error = toString("cannot write '%s': %s", temp.c_str(), formatErrorMessage(getLastError()).c_str());
		return false;
	}
	bool ok = fwrite(text.data(), 1, text.size(), f) == text.size();
	ok = (fclose(f) == 0) && ok;
	if (!ok)
	{
		error = toString("cannot write '%s'", temp.c_str());
		CFile::deleteFile(temp);
		return false;
	}
	if (!CFile::replaceFile(path, temp))
	{
		error = toString("cannot move '%s' to '%s'", temp.c_str(), path.c_str());
		CFile::deleteFile(temp);
		return false;
	}
	return true;
}

bool CCtrlRegistry::readRecord(const std::string &path, CJsonValue &record, std::string &error)
{
	FILE *f = nlfopen(path, "rb");
	if (f == nullptr)
	{
		error = toString("cannot read '%s': %s", path.c_str(), formatErrorMessage(getLastError()).c_str());
		return false;
	}
	std::string text;
	char buffer[4096];
	size_t n;
	while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0 && text.size() < 1024 * 1024)
		text.append(buffer, n);
	fclose(f);
	if (!record.parse(text, error))
	{
		error = toString("'%s' is not a record: %s", path.c_str(), error.c_str());
		return false;
	}
	if (!record.isObject())
	{
		error = toString("'%s' is not a record", path.c_str());
		return false;
	}
	return true;
}

void CCtrlRegistry::listRecords(const std::string &dir, std::vector<std::string> &paths)
{
	paths.clear();
	std::vector<std::string> files;
	CPath::getPathContent(dir, false, false, true, files);
	for (uint i = 0; i < files.size(); ++i)
	{
		const std::string &file = files[i];
		if (file.size() > 5 && file.compare(file.size() - 5, 5, ".json") == 0)
			paths.push_back(file);
	}
}

bool CCtrlRegistry::isValidName(const std::string &name)
{
	if (name.empty() || name.size() > 64)
		return false;
	for (uint i = 0; i < name.size(); ++i)
	{
		char c = name[i];
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'))
			return false;
	}
	return true;
}

uint32 CCtrlRegistry::getCurrentProcessId()
{
#ifdef NL_OS_WINDOWS
	return (uint32)::GetCurrentProcessId();
#else
	return (uint32)getpid();
#endif
}

uint64 CCtrlRegistry::getProcessStartTime(uint32 pid)
{
#ifdef NL_OS_WINDOWS
	HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)pid);
	if (process == NULL)
		return 0;
	// a handle can still be opened on a process that exited while others hold one
	DWORD exitCode = 0;
	FILETIME creation, exitTime, kernel, user;
	uint64 result = 0;
	if (GetExitCodeProcess(process, &exitCode) && exitCode == STILL_ACTIVE && GetProcessTimes(process, &creation, &exitTime, &kernel, &user))
	{
		uint64 ticks = ((uint64)creation.dwHighDateTime << 32) | creation.dwLowDateTime;
		// 100 ns since 1601 to ms since 1970
		result = (ticks - 116444736000000000ULL) / 10000;
	}
	CloseHandle(process);
	return result;
#elif defined(NL_OS_MAC)
	int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, (int)pid };
	struct kinfo_proc info;
	size_t size = sizeof(info);
	memset(&info, 0, sizeof(info));
	if (sysctl(mib, 4, &info, &size, NULL, 0) != 0 || size == 0)
		return 0;
	if (info.kp_proc.p_stat == SZOMB)
		return 0;
	return (uint64)info.kp_proc.p_starttime.tv_sec * 1000 + (uint64)info.kp_proc.p_starttime.tv_usec / 1000;
#else
	// field 22 of /proc/<pid>/stat is the start in clock ticks after the boot
	FILE *f = fopen(toString("/proc/%u/stat", pid).c_str(), "r");
	if (f == nullptr)
		return 0;
	char buffer[1024];
	size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
	fclose(f);
	buffer[n] = 0;
	// the command name is in parentheses and may hold anything, so the fields start after the last one
	char *p = strrchr(buffer, ')');
	if (p == nullptr)
		return 0;
	std::vector<std::string> fields;
	explode(std::string(p + 2), std::string(" "), fields, true);
	// fields[0] is field 3, the state
	if (fields.size() < 20 || fields[0] == "Z" || fields[0] == "X")
		return 0;
	uint64 startTicks = 0;
	if (!fromString(fields[19], startTicks))
		return 0;
	long ticksPerSecond = sysconf(_SC_CLK_TCK);
	if (ticksPerSecond <= 0)
		return 0;
	// in ms after the boot, not after the epoch: the boot time of /proc/stat moves when the clock is set (an NTP
	// step, a resumed virtual machine), and a running process would then look like another one. The registry is
	// in XDG_RUNTIME_DIR, which does not outlive the boot.
	return startTicks * 1000 / (uint64)ticksPerSecond + 1;
#endif
}

} // NLMISC

/* End of ctrl_protocol.cpp */
