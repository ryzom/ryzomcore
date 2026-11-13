// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// Runs one Ring scenario file through the editor's chain without the client, in the order the client
// does it: the file is parsed, brought to the versions of the editor as a table (the version update) and
// becomes the edited scenario; a test is started, which saves the scenario, reads it back and translates
// it to the run-time form; the run-time tree is checked as the scenario service checks an upload.
// Prints one "key: value" line per result.

#include "r2_headless_host.h"

#include "nel/misc/app_context.h"
#include "nel/misc/debug.h"
#include "nel/misc/displayer.h"
#include "nel/misc/file.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/string_common.h"
#include "nel/ligo/ligo_config.h"

#include "game_share/object.h"
#include "game_share/rt_scenario_check.h"
#include "game_share/scenario.h"
#include "game_share/scenario_entry_points.h"
#include "game_share/scenario_text_parser.h"

extern "C" {
#include <lua.h>
}

#include <algorithm>
#include <cstdio>
#include <set>

#ifdef NL_OS_WINDOWS
#include <crtdbg.h>
#include <cstdlib>
#endif

using namespace NLMISC;
using namespace R2;

namespace {

// What the program returns
enum TExitCode
{
	ExitOk = 0, // translated, and the run-time tree passed the check
	ExitUsage = 1,
	ExitParseFailed = 2,
	ExitLoadFailed = 3, // a script of the editor failed to load
	ExitUpdateFailed = 4,
	ExitReadBackFailed = 5,
	ExitTranslateFailed = 6,
	ExitCheckRefused = 8, // translated, and the run-time tree was refused
	ExitUpdateRepeats = 9, // the version update asks for an upload each time the scenario comes back
	ExitException = 10,
	ExitLuaPanic = 11, // printed by the host, see CHeadlessHost::luaPanic
	ExitFileMissing = 12,
	ExitExpectationFailed = 13 // the output lacks a line of the --expect-file
};

// A scenario that needs an update is uploaded and comes back, which runs the update again. Rounds after
// which the update is taken to repeat for ever.
const uint32 s_MaxUpdateRounds = 4;

struct CArgs
{
	std::string R2Dir;
	std::string Driver;
	std::string ScenarioFile;
	std::string RtOut;
	std::string HlOut;
	std::string Leveldesign;
	std::string LangDir;
	std::string Lang;
	std::string NelLog;
	std::vector<std::string> DataDirs;
	bool Verbose;
	bool CheckMd5;
	bool FromServer;
	bool ReturnBeforeTest;
	sint32 SelectedAct; // -1: the act the editor selects when it shows the scenario
	std::string EditFile;
	std::string ExpectFile;
	CArgs()
	    : Verbose(false)
	    , CheckMd5(false)
	    , FromServer(false)
	    , ReturnBeforeTest(false)
	    , SelectedAct(-1)
	{
	}
};

void usage()
{
	printf("ryzom_r2_scenario_chain [options] <scenario.r2>\n"
	       "  --r2-dir <dir>     directory of the editor's Lua (default: the source tree)\n"
	       "  --driver <file>    the host's own script, r2_headless_driver.lua (default: the source tree)\n"
	       "  --rt-out <file>    write the run-time tree of a translated scenario, in the text form\n"
	       "  --hl-out <file>    write the scenario as a started test saves it, in the text form\n"
	       "  --leveldesign <dir> leveldesign directory: creature sheets, sheet_id.bin and the interface\n"
	       "                     translations are read from it\n"
	       "  --data-dir <dir>   add a search path (sheet_id.bin and the island files are used when found)\n"
	       "  --lang-dir <dir>   directory of the interface translations, with --lang <code>\n"
	       "  --lang <code>      language of the interface translations (default: en)\n"
	       "  --check-md5        refuse a file whose header checksums do not match\n"
	       "  --from-server      the scenario does not come from a file: the editor gets it from the server\n"
	       "                     as it is, as for an edition session the server kept, and updates it then\n"
	       "  --selected-act <n> the author selects act n before the test starts (default: the act the editor\n"
	       "                     selects when it shows the scenario, act 1)\n"
	       "  --edit-file <file> the author adds a component to the first act before the test starts: the file\n"
	       "                     has it in the text form, as \"scenario = { ... }\"\n"
	       "  --return-before-test  the scenario comes back from the server before the test starts, as when\n"
	       "                     the editor connects again\n"
	       "  --expect-file <file>  what the run must come to, one expectation to a line (see\n"
	       "                     checkExpectations). The program returns 13 when one is not met.\n"
	       "  --nel-log <file>   write the NeL log to this file\n"
	       "  --verbose          print the script's messages as they come\n");
}

bool parseArgs(int argc, char **argv, CArgs &args)
{
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		bool hasValue = i + 1 < argc;
		if (arg == "--r2-dir" && hasValue) args.R2Dir = argv[++i];
		else if (arg == "--driver" && hasValue) args.Driver = argv[++i];
		else if (arg == "--rt-out" && hasValue) args.RtOut = argv[++i];
		else if (arg == "--hl-out" && hasValue) args.HlOut = argv[++i];
		else if (arg == "--data-dir" && hasValue) args.DataDirs.push_back(argv[++i]);
		else if (arg == "--leveldesign" && hasValue) args.Leveldesign = argv[++i];
		else if (arg == "--lang-dir" && hasValue) args.LangDir = argv[++i];
		else if (arg == "--lang" && hasValue) args.Lang = argv[++i];
		else if (arg == "--nel-log" && hasValue) args.NelLog = argv[++i];
		else if (arg == "--verbose") args.Verbose = true;
		else if (arg == "--check-md5") args.CheckMd5 = true;
		else if (arg == "--from-server") args.FromServer = true;
		else if (arg == "--return-before-test") args.ReturnBeforeTest = true;
		else if (arg == "--edit-file" && hasValue) args.EditFile = argv[++i];
		else if (arg == "--selected-act" && hasValue)
		{
			if (!fromString(std::string(argv[++i]), args.SelectedAct) || args.SelectedAct < 0) return false;
		}
		else if (arg == "--expect-file" && hasValue) args.ExpectFile = argv[++i];
		else if (!arg.empty() && arg[0] == '-') return false;
		else if (args.ScenarioFile.empty()) args.ScenarioFile = arg;
		else return false;
	}
	return !args.ScenarioFile.empty();
}

// the "key: value" lines printed so far, for the expectations
std::vector<std::string> s_ReportedLines;

// One line of output; line breaks inside a value would break the format
void report(const std::string &key, const std::string &value)
{
	std::string line = value;
	for (size_t i = 0; i < line.size(); ++i)
	{
		if (line[i] == '\n' || line[i] == '\r') line[i] = ' ';
	}
	s_ReportedLines.push_back(key + ": " + line);
	printf("%s: %s\n", key.c_str(), line.c_str());
	fflush(stdout);
}

// A value that spans lines (a traceback), between markers. Each line of the value starts with "| ", so
// that no text of an error can be taken for a marker or for a line of the report.
void reportBlock(const std::string &key, const std::string &value)
{
	std::string body = "| ";
	for (size_t i = 0; i < value.size(); ++i)
	{
		if (value[i] == '\r') continue;
		body += value[i];
		if (value[i] == '\n') body += "| ";
	}
	printf("%s <<<\n%s\n>>>\n", key.c_str(), body.c_str());
	fflush(stdout);
}

// The scenario as a started test saves it and the run-time scenario, as text and as trees, for the
// expectations
std::string s_SavedScenarioText;
std::string s_RtScenarioText;
CObject::TSmartPtr s_SavedScenario;
CObject::TSmartPtr s_RtScenario;
// The scenario the server has at the end of the run
CObject::TSmartPtr s_ServerScenario;

// The lines of a text form without their indentation and without the comma that ends an entry
void textFormLines(const std::string &text, std::set<std::string> &lines)
{
	size_t begin = 0;
	while (begin <= text.size())
	{
		size_t end = text.find('\n', begin);
		if (end == std::string::npos) end = text.size();
		size_t first = begin;
		while (first < end && (text[first] == ' ' || text[first] == '\t')) ++first;
		size_t last = end;
		while (last > first && (text[last - 1] == ' ' || text[last - 1] == '\t' || text[last - 1] == '\r')) --last;
		if (last > first && text[last - 1] == ',') --last;
		while (last > first && (text[last - 1] == ' ' || text[last - 1] == '\t')) --last;
		if (last > first) lines.insert(text.substr(first, last - first));
		begin = end + 1;
	}
}

// An expectation about the lines of a text form: "<key>_has: line" or "<key>_lacks: line". The line is a
// whole line of the text form, as textFormLines gives it. Returns false when the expectation is not one
// of key; met then tells whether the text form is as expected.
bool checkLineExpectation(const std::string &line, const std::string &key, const std::string &textForm, bool &met)
{
	const std::string has = key + "_has: ";
	const std::string lacks = key + "_lacks: ";
	bool isHas = line.compare(0, has.size(), has) == 0;
	bool isLacks = line.compare(0, lacks.size(), lacks) == 0;
	if (!isHas && !isLacks) return false;
	std::set<std::string> lines;
	textFormLines(textForm, lines);
	bool found = lines.find(line.substr(isHas ? has.size() : lacks.size())) != lines.end();
	// a run that did not get as far as the text form does not meet an expectation about it
	met = !textForm.empty() && (isHas ? found : !found);
	return true;
}

// The node of a tree at a path: attribute names and positions in a list, as "Acts[1].Features[0].Name".
// Positions start at 0. Returns nullptr when the tree has no such node.
const CObject *nodeAtPath(const CObject *root, const std::string &path)
{
	const CObject *node = root;
	size_t pos = 0;
	while (node && pos < path.size())
	{
		if (path[pos] == '.')
		{
			++pos;
		}
		else if (path[pos] == '[')
		{
			size_t close = path.find(']', pos);
			uint32 index = 0;
			if (close == std::string::npos || !fromString(path.substr(pos + 1, close - pos - 1), index)) return nullptr;
			node = (node->isTable() && index < node->getSize()) ? node->getValueAtPos(index) : nullptr;
			pos = close + 1;
		}
		else
		{
			size_t end = path.find_first_of(".[", pos);
			if (end == std::string::npos) end = path.size();
			node = node->isTable() ? node->getAttr(path.substr(pos, end - pos)) : nullptr;
			pos = end;
		}
	}
	return node;
}

// An expectation about one node of a tree: "<key>_at: path = value". The value is the text or the number
// of the node, "<empty>" for an empty text, "<none>" for a node the tree does not have, "<table>" for a
// table, "<size n>" for a table of n entries, and "@path" for the text of another node of the tree.
// Returns false when the expectation is not one of key.
bool checkPathExpectation(const std::string &line, const std::string &key, const CObject *root, bool &met)
{
	const std::string prefix = key + "_at: ";
	if (line.compare(0, prefix.size(), prefix) != 0) return false;
	met = false;
	size_t equals = line.find(" = ", prefix.size());
	if (!root || equals == std::string::npos) return true;
	std::string path = line.substr(prefix.size(), equals - prefix.size());
	std::string expected = line.substr(equals + 3);
	const CObject *node = nodeAtPath(root, path);
	if (!expected.empty() && expected[0] == '@')
	{
		const CObject *other = nodeAtPath(root, expected.substr(1));
		met = node && other && !node->isTable() && !other->isTable() && node->toString() == other->toString();
		return true;
	}
	if (expected == "<empty>") expected.clear();
	if (expected == "<none>")
		met = node == nullptr;
	else if (!node)
		met = false;
	else if (expected == "<table>")
		met = node->isTable();
	else if (expected.compare(0, 6, "<size ") == 0)
		met = node->isTable() && expected == toString("<size %u>", node->getSize());
	else if (node->isTable())
		met = false;
	else if (node->isNumber())
	{
		double value = 0;
		met = fromString(expected, value) && value == node->toNumber();
	}
	else
		met = node->toString() == expected;
	return true;
}

// Checks the run against the lines of an expectation file. Returns the number that are not met.
//   key: value                   a reported line: the whole value, or its start up to the end of a word
//   absent: key                  no reported line has this key
//   no_line: key: start          no reported line starts with this text: a key and the start of a value,
//                                as "version_update_note: uiR2EDVersionNoteIslandMissing("
//   scenario_has: line           a line of the scenario as the started test saves it, in the text form,
//   scenario_lacks: line         without its indentation and the comma at its end
//   scenario_at: path = value    a node of that scenario (see checkPathExpectation)
//   rt_has, rt_lacks, rt_at      the same for the run-time scenario
//   server_at: path = value      a node of the scenario the server has at the end of the run
// "result: n" is a reported line like the others, the value the run came to (see TExitCode);
// resultExpected tells whether the file has an expectation about it.
uint checkExpectations(const std::string &path, bool &resultExpected)
{
	resultExpected = false;
	CIFile file;
	std::string text;
	if (!file.open(path))
	{
		report("expect_failed", "can't read " + path);
		return 1;
	}
	text.resize(file.getFileSize());
	if (!text.empty())
		file.serialBuffer((uint8 *)&text[0], (uint)text.size());
	std::vector<std::string> expected;
	splitString(text, "\n", expected);
	uint missing = 0;
	std::vector<std::string> reported = s_ReportedLines;
	for (size_t i = 0; i < expected.size(); ++i)
	{
		std::string line = expected[i];
		while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == ' ')) line.resize(line.size() - 1);
		if (line.empty() || line[0] == '#') continue;
		if (line.compare(0, 8, "result: ") == 0) resultExpected = true;
		bool found = false;
		if (line.compare(0, 8, "absent: ") == 0)
		{
			const std::string key = line.substr(8) + ": ";
			found = true;
			for (size_t j = 0; j < reported.size() && found; ++j)
				found = reported[j].compare(0, key.size(), key) != 0;
		}
		else if (line.compare(0, 9, "no_line: ") == 0)
		{
			const std::string start = line.substr(9);
			found = true;
			for (size_t j = 0; j < reported.size() && found; ++j)
				found = reported[j].compare(0, start.size(), start) != 0;
		}
		else if (!checkLineExpectation(line, "scenario", s_SavedScenarioText, found)
		    && !checkLineExpectation(line, "rt", s_RtScenarioText, found)
		    && !checkPathExpectation(line, "scenario", s_SavedScenario, found)
		    && !checkPathExpectation(line, "rt", s_RtScenario, found)
		    && !checkPathExpectation(line, "server", s_ServerScenario, found))
		{
			for (size_t j = 0; j < reported.size() && !found; ++j)
			{
				found = reported[j].compare(0, line.size(), line) == 0
				    && (reported[j].size() == line.size() || reported[j][line.size()] == ' ');
			}
		}
		if (!found)
		{
			report("expect_failed", line);
			++missing;
		}
	}
	return missing;
}

// The text form of a scenario without what a save gives every scenario anew: the version name and the
// Versions table of the scenario itself
std::string withoutVersions(const std::string &text)
{
	std::string result;
	bool inVersions = false;
	size_t begin = 0;
	while (begin < text.size())
	{
		size_t end = text.find('\n', begin);
		if (end == std::string::npos) end = text.size();
		std::string line = text.substr(begin, end - begin);
		begin = end + 1;
		if (inVersions)
		{
			// the table ends at the indentation it began at
			if (line.compare(0, 3, "  }") == 0) inVersions = false;
			continue;
		}
		if (line.compare(0, 14, "  Versions = {") == 0)
		{
			inVersions = true;
			continue;
		}
		if (line.compare(0, 16, "  VersionName = ") == 0) continue;
		// the entry before the Versions table ends with a comma or not, whether the table follows
		while (!line.empty() && (line[line.size() - 1] == ' ' || line[line.size() - 1] == ',')) line.resize(line.size() - 1);
		result += line;
		result += '\n';
	}
	return result;
}

void countNodes(const CObject *object, uint32 &nodes, uint32 &instances)
{
	++nodes;
	if (!object->isTable()) return;
	if (object->getAttr("InstanceId")) ++instances;
	uint32 size = object->getSize();
	for (uint32 i = 0; i < size; ++i)
		countNodes(object->getValueAtPos(i), nodes, instances);
}

uint32 sizeOfAttr(const CObject *object, const std::string &name)
{
	const CObject *attr = object->getAttr(name);
	return (attr && attr->isTable()) ? attr->getSize() : 0;
}

bool writeFile(const std::string &path, const std::string &text)
{
	COFile out;
	if (!out.open(path)) return false;
	if (!text.empty())
		out.serialBuffer((uint8 *)&text[0], (uint)text.size());
	return true;
}

// "name@file:line" for each name, the place being the first use
std::string joinUses(const CHeadlessHost::TUseMap &uses)
{
	std::string result;
	for (CHeadlessHost::TUseMap::const_iterator it = uses.begin(); it != uses.end(); ++it)
	{
		if (!result.empty()) result += ", ";
		result += it->first;
		if (!it->second.empty()) result += "@" + it->second;
	}
	return result;
}

// Number of tables of a class in a tree
uint32 countClass(const CObject *object, const std::string &className)
{
	if (!object->isTable()) return 0;
	uint32 count = 0;
	const CObject *objectClass = object->getAttr("Class");
	if (objectClass && objectClass->isString() && objectClass->toString() == className) ++count;
	uint32 size = object->getSize();
	for (uint32 i = 0; i < size; ++i)
		count += countClass(object->getValueAtPos(i), className);
	return count;
}

// Longest string of a tree, in bytes
uint32 longestString(const CObject *object)
{
	if (object->isString()) return (uint32)object->toString().size();
	if (!object->isTable()) return 0;
	uint32 longest = 0;
	uint32 size = object->getSize();
	for (uint32 i = 0; i < size; ++i)
		longest = std::max(longest, longestString(object->getValueAtPos(i)));
	return longest;
}

std::string stackString(lua_State *state, int idx)
{
	const char *text = lua_tostring(state, idx);
	return text ? text : "";
}

// Log lines of one phase: the errors the script caught with their tracebacks, and its messages
void reportPhaseLog(const CHeadlessHost &host, const std::string &phase, size_t firstLine, bool withMessages)
{
	// The version update prints a line for each component it updates. Of those only the one before a
	// failure is printed: it names the class and the versions the failure is about.
	const std::vector<CHeadlessHost::CLogLine> &lines = host.getLog();
	std::string lastComponentLine;
	for (size_t i = firstLine; i < lines.size(); ++i)
	{
		const CHeadlessHost::CLogLine &line = lines[i];
		if (line.Phase != phase) continue;
		if (line.Kind == "luaError" || line.Kind == "loadError")
			reportBlock(phase + "_lua_error", line.Text);
		else if (line.Kind == "host")
			report(phase + "_host", line.Text);
		else if (line.Kind == "systemInfo")
			report(phase + "_note", line.Text); // what the scripts tell the author in the chat window
		else if (withMessages && (line.Kind == "debugInfo" || line.Kind == "messageBox"))
		{
			if (line.Text.find("Updating the component") != std::string::npos)
			{
				lastComponentLine = line.Text;
				continue;
			}
			if (line.Text.find("can not be updated") != std::string::npos && !lastComponentLine.empty())
			{
				report(phase + "_message", lastComponentLine);
				lastComponentLine.clear();
			}
			report(phase + "_message", line.Text);
		}
	}
}

// What the run used of the host: stubs and substituted values, undefined names, host functions, and
// what became of the node requests
void reportUses(const CHeadlessHost &host)
{
	const std::map<std::string, CHeadlessHost::TUseMap> &stubs = host.getCalledStubs();
	for (std::map<std::string, CHeadlessHost::TUseMap>::const_iterator it = stubs.begin(); it != stubs.end(); ++it)
		report("stubs_" + it->first, joinUses(it->second));
	const std::map<std::string, CHeadlessHost::TUseMap> &reads = host.getUndefinedReads();
	for (std::map<std::string, CHeadlessHost::TUseMap>::const_iterator it = reads.begin(); it != reads.end(); ++it)
		report("undefined_" + it->first, joinUses(it->second));
	const std::map<std::string, std::map<std::string, uint32>> &calls = host.getHostCalls();
	for (std::map<std::string, std::map<std::string, uint32>>::const_iterator it = calls.begin(); it != calls.end(); ++it)
	{
		std::string text;
		for (std::map<std::string, uint32>::const_iterator call = it->second.begin(); call != it->second.end(); ++call)
			text += toString("%s%s=%u", text.empty() ? "" : ", ", call->first.c_str(), call->second);
		report("host_" + it->first, text);
	}
	const std::map<std::string, CHeadlessHost::TUseMap> &dropped = host.getDroppedRequests();
	for (std::map<std::string, CHeadlessHost::TUseMap>::const_iterator it = dropped.begin(); it != dropped.end(); ++it)
		report("dropped_" + it->first, joinUses(it->second));
	const std::map<std::string, CHeadlessHost::TUseMap> &late = host.getLateRequests();
	for (std::map<std::string, CHeadlessHost::TUseMap>::const_iterator it = late.begin(); it != late.end(); ++it)
		report("late_" + it->first, joinUses(it->second));
	const std::map<std::string, uint32> &requests = host.getRequestCounts();
	std::string text;
	for (std::map<std::string, uint32>::const_iterator it = requests.begin(); it != requests.end(); ++it)
		text += toString("%s%s=%u", text.empty() ? "" : ", ", it->first.c_str(), it->second);
	report("requests", text);
}

// r2Headless.versionState(): prints the state and returns whether every class is at the current version
bool reportVersionState(CHeadlessHost &host, const std::string &key, bool &hasUndefined)
{
	std::string error;
	hasUndefined = false;
	if (!host.callDriver("versionState", std::vector<std::string>(), 5, error))
	{
		reportBlock(key + "_error", error);
		return false;
	}
	lua_State *state = host.getLua();
	bool ok = lua_toboolean(state, -5) != 0;
	std::string versionName = stackString(state, -4);
	std::string undef = stackString(state, -3);
	std::string older = stackString(state, -2);
	std::string newer = stackString(state, -1);
	lua_pop(state, 5);
	hasUndefined = !undef.empty();
	report(key, toString("current=%s name='%s'", ok ? "yes" : "no", versionName.c_str()));
	if (!undef.empty()) report(key + "_undefined", undef);
	if (!older.empty()) report(key + "_older", older);
	if (!newer.empty()) report(key + "_newer", newer);
	return ok;
}

// What the last version update came to (r2.Version.LastUpdate), under key: the status ("current",
// "unchanged", "changed" or "failed"), the steps that ran, the notes for the author, the classes the
// scenario has in a later version, and the failure. Returns the status, "" when no update ran.
// path says where the update ran: "file" for a scenario read from a file, "session" for one the editor
// got from the server. unknownClass tells whether a failure is about a class the editor does not have.
std::string reportLastUpdate(CHeadlessHost &host, const std::string &key, const std::string &path, bool &unknownClass)
{
	unknownClass = false;
	std::string error;
	if (!host.callDriver("lastUpdate", std::vector<std::string>(), 6, error))
	{
		reportBlock(key + "_error", error);
		return "";
	}
	lua_State *state = host.getLua();
	std::string status = stackString(state, -6);
	std::string steps = stackString(state, -5);
	std::string notes = stackString(state, -4);
	std::string newer = stackString(state, -3);
	std::string reason = stackString(state, -2);
	unknownClass = stackString(state, -1) == "class";
	lua_pop(state, 6);
	if (status.empty()) return status;
	report(key, status);
	report(key + "_path", path);
	if (!steps.empty()) report(key + "_steps", steps);
	std::vector<std::string> noteLines;
	splitString(notes, "\n", noteLines);
	for (size_t i = 0; i < noteLines.size(); ++i)
	{
		if (!noteLines[i].empty()) report(key + "_note", noteLines[i]);
	}
	if (!newer.empty()) report(key + "_newer", newer);
	if (status == "failed") report(key + "_failure", reason);
	return status;
}

// What one scenario update came to
struct CUpdateOutcome
{
	bool Attached; // the scenario became r2.Scenario
	bool WasCurrent; // every class was at the version of the editor
	bool UpdateError; // r2.Version.updateVersion raised an error
	bool UnknownClass; // the update failed on a class the editor does not have
	std::string Status; // of the update r2.Version.updateVersion ran, "" when it ran none
	CUpdateOutcome()
	    : Attached(false)
	    , WasCurrent(false)
	    , UpdateError(false)
	    , UnknownClass(false)
	{
	}
};

// One CEditor::scenarioUpdated: the tree becomes the edited scenario, r2.onScenarioUpdated attaches it,
// runs the version update and selects an act, the action historic is cleared, and the frame ends.
// The keys of the report start with prefix. fromServer tells whether the tree is the scenario the server
// has; the scenario a test reads back from its file is not.
CUpdateOutcome scenarioUpdated(CHeadlessHost &host, CObject *tree, uint32 startingActIndex, const std::string &prefix, bool fromServer)
{
	CUpdateOutcome outcome;
	std::string error;
	size_t firstLine = host.getLog().size();
	host.beginScenarioUpdated(tree, fromServer);

	std::vector<std::string> arguments;
	arguments.push_back(host.getScenarioInstanceId());
	if (!host.callDriver("attach", arguments, 1, error))
	{
		reportBlock(prefix + "attach_error", error);
		return outcome;
	}
	outcome.Attached = lua_toboolean(host.getLua(), -1) != 0;
	lua_pop(host.getLua(), 1);
	if (!outcome.Attached)
	{
		report(prefix + "attach_error", "the root of the scenario is not an instance");
		return outcome;
	}

	bool hasUndefined = false;
	outcome.WasCurrent = reportVersionState(host, prefix + "version_before", hasUndefined);
	// r2.Version.updateVersion: a scenario that is not at the versions of the editor is updated as a
	// table and sent in the place of this one; the scenario the editor has stays as it is
	if (host.callDriver("upgrade", std::vector<std::string>(), 1, error))
	{
		lua_pop(host.getLua(), 1);
		outcome.Status = reportLastUpdate(host, prefix + "version_update", "session", outcome.UnknownClass);
	}
	else
	{
		outcome.UpdateError = true;
		reportBlock(prefix + "upgrade_error", error);
	}

	// the rest of r2.onScenarioUpdated that is not interface: the act it selects
	arguments.clear();
	arguments.push_back(toString("%u", startingActIndex));
	if (!host.callDriver("selectAct", arguments, 0, error))
		reportBlock(prefix + "select_act_error", error);

	host.endScenarioUpdated();
	host.endFrame();
	reportPhaseLog(host, "update", firstLine, outcome.UpdateError || outcome.Status == "failed");
	return outcome;
}

// The scenario comes back from the server as the server has it: when a test ends, or when the editor
// connects again. The version update looks at it as at any scenario the editor is given, and has nothing
// to do when the server was given the versions of everything the editor did. The keys of the report start
// with prefix; "<prefix>upload" tells that the update sent a scenario to take the place of the server's.
void scenarioReturns(CHeadlessHost &host, const std::string &prefix)
{
	CObject *server = host.getServerHighLevel();
	if (!server) return;
	host.setPhase("update");
	CObject::TSmartPtr tree = server->clone();
	CUpdateOutcome outcome = scenarioUpdated(host, tree, 1, prefix, true);
	tree = nullptr;
	if (!outcome.Attached) return;
	CObject::TSmartPtr upload;
	if (host.takeUploadedScenario(upload))
		report(prefix + "upload", "the update sent a scenario to take the place of the one the server has");
}

// Reads a scenario file as CClientEditionModule::addToLoadList and CComLuaModule::parseScenario do. body
// gets the text of the file, which the client also keeps as save/r2_buffer.dat.
CObject::TSmartPtr parseScenarioFile(const std::string &filename, bool checkMd5, std::string &body, std::string &error)
{
	CScenarioValidator validator;
	CScenarioValidator::TValues values;
	std::string md5, signature;
	if (!validator.setScenarioToLoad(filename, values, md5, signature, checkMd5))
	{
		error = "header refused";
		return nullptr;
	}
	std::string loadedName;
	validator.applyLoad(loadedName, body, values);
	report("header", toString("values=%u signed=%s", (uint)values.size(), signature.empty() ? "no" : "yes"));
	return CScenarioTextParser::parse(body, nullptr, error);
}

// The limits CServerEditionModule::checkRtScenario gives the check, with the default quotas of the service
CRtScenarioLimits serviceLimits(std::string &description)
{
	CRtScenarioLimits limits;
	limits.setBotQuota(CRtScenarioLimits::DefaultMaxBotsPerAct);
	description = toString("bots_per_act=%u", limits.MaxBotsPerAct);

	// The aliases the primitive file of an act can give out. The service takes them from the ligo class
	// file of its data, which says how many bits of an alias are dynamic. That file is used when it is on
	// the search path and says so; the class file of the client data says nothing about aliases, and the
	// default of the check then stays.
	bool aliasesFromFile = false;
	if (CPath::exists("world_editor_classes.xml"))
	{
		NLLIGO::CLigoConfig ligoConfig;
		if (ligoConfig.readPrimitiveClass(CPath::lookup("world_editor_classes.xml").c_str(), false) && ligoConfig.getDynamicAliasMask() != 0xffffffff)
		{
			limits.MaxAliasesPerAct = ligoConfig.getDynamicAliasMask();
			aliasesFromFile = true;
		}
	}
	description += toString(" aliases_per_act=%u (%s)", limits.MaxAliasesPerAct, aliasesFromFile ? "ligo class file" : "default");

	// the rectangle that holds every island, when the island files are on the search path
	bool haveIslands = CPath::exists("r2_entry_points.txt");
	static const char *const s_IslandFiles[] = { "r2_desert_islands.xml", "r2_lakes_islands.xml", "r2_jungle_islands.xml", "r2_forest_islands.xml", "r2_roots_islands.xml" };
	for (size_t i = 0; i < sizeof(s_IslandFiles) / sizeof(s_IslandFiles[0]); ++i)
		haveIslands = haveIslands && CPath::exists(s_IslandFiles[i]);
	if (haveIslands)
	{
		const CScenarioEntryPoints::TCompleteIslands &islands = CScenarioEntryPoints::getInstance().getCompleteIslands();
		if (!islands.empty())
		{
			sint32 minX = 0, maxX = 0, minY = 0, maxY = 0;
			for (uint i = 0; i < islands.size(); ++i)
			{
				minX = std::min(minX, islands[i].XMin);
				maxX = std::max(maxX, islands[i].XMax);
				minY = std::min(minY, islands[i].YMin);
				maxY = std::max(maxY, islands[i].YMax);
			}
			limits.MinX = std::max(minX, limits.MinX + RtScenarioIslandMargin) - RtScenarioIslandMargin;
			limits.MaxX = std::min(maxX, limits.MaxX - RtScenarioIslandMargin) + RtScenarioIslandMargin;
			limits.MinY = std::max(minY, limits.MinY + RtScenarioIslandMargin) - RtScenarioIslandMargin;
			limits.MaxY = std::min(maxY, limits.MaxY - RtScenarioIslandMargin) + RtScenarioIslandMargin;
			description += toString(" islands=%u", (uint)islands.size());
			return limits;
		}
	}
	description += " islands=none (any position the AI service can hold)";
	return limits;
}

// Checks a run-time tree as the service checks an upload. Returns the verdict and prints it under key.
TRtScenarioVerdict reportRtCheck(const CObject *rt, const CRtScenarioLimits &limits, uint32 startingAct, const std::string &key)
{
	std::string description;
	TRtScenarioVerdict verdict = checkRtScenario(rt, limits, description);
	if (verdict == RtScenarioOk)
		verdict = checkRtScenarioStartingAct(rt, startingAct, description);
	switch (verdict)
	{
	case RtScenarioOk:
		report(key, "ok");
		break;
	case RtScenarioOverQuota:
		report(key, "over_quota: " + description);
		break;
	default:
		report(key, "invalid: " + description);
		break;
	}
	return verdict;
}

// The tree as it arrives at the service: through the serial form, compressed as the client sends it
CObject::TSmartPtr throughSerialForm(CObject *tree, std::string &error)
{
	CObjectSerializerServer out(tree);
	out.compress();
	CMemStream written;
	written.serial(out);
	CMemStream read(true);
	if (written.length() > 0)
		read.fill(written.buffer(), written.length());
	CObjectSerializerServer in;
	try
	{
		read.serial(in);
	}
	catch (const EStream &e)
	{
		error = e.what();
		return nullptr;
	}
	CObject::TSmartPtr copy = in.getData();
	if (!copy) error = "the serializer gave no tree";
	return copy;
}

int run(const CArgs &args)
{
	report("file", args.ScenarioFile);
	if (!CFile::fileExists(args.ScenarioFile))
	{
		report("file_error", "the file does not exist");
		return ExitFileMissing;
	}

	// 1. parse
	std::string error;
	std::string fileText;
	CObject::TSmartPtr scenario = parseScenarioFile(args.ScenarioFile, args.CheckMd5, fileText, error);
	if (!scenario)
	{
		report("parse", "fail");
		report("parse_error", error);
		return ExitParseFailed;
	}
	if (!scenario->isTable() || !scenario->isString("InstanceId") || !scenario->isString("Class"))
	{
		report("parse", "fail");
		report("parse_error", "the root table has no InstanceId or no Class");
		return ExitParseFailed;
	}
	uint32 nodes = 0, instances = 0;
	countNodes(scenario, nodes, instances);
	report("parse", "ok");
	report("tree", toString("nodes=%u instances=%u root_class=%s acts=%u", nodes, instances, scenario->toString("Class").c_str(), sizeOfAttr(scenario, "Acts")));

	// 2. the editor's scripts
	CHeadlessHost host;
	// the scenario the server has when the run ends, wherever it ends, for the expectations
	struct CKeepServerScenario
	{
		CHeadlessHost &Host;
		explicit CKeepServerScenario(CHeadlessHost &keptHost)
		    : Host(keptHost)
		{
		}
		~CKeepServerScenario()
		{
			CObject *server = Host.getServerHighLevel();
			s_ServerScenario = server ? server->clone() : nullptr;
		}
	} keepServerScenario(host);
	host.setVerbose(args.Verbose);
	if (!args.Leveldesign.empty())
		host.setSheetDirectory(args.Leveldesign);
	if (!args.LangDir.empty())
		host.loadTranslation(args.LangDir, args.Lang.empty() ? "en" : args.Lang);
	else if (!args.Leveldesign.empty() && CFile::isDirectory(args.Leveldesign + "/translation/translated"))
		host.loadTranslation(args.Leveldesign + "/translation/translated", args.Lang.empty() ? "en" : args.Lang);
	if (!host.init(args.R2Dir, args.Driver, error))
	{
		report("load", "fail");
		reportBlock("load_error", error);
		reportPhaseLog(host, "load", 0, true);
		reportUses(host);
		return ExitLoadFailed;
	}
	report("load", "ok");
	reportPhaseLog(host, "load", 0, false);

	// the scenario as the file has it, in the text form of a save
	std::string originalText = "scenario = ";
	scenario->serialize(originalText);

	// 3. The file is loaded. The client brings the scenario to the versions of the editor as a table,
	// before it uploads it (CClientEditionModule::loadScenarioSucceded). With --from-server the scenario
	// reaches the editor as it is, as one the server kept does.
	host.setPhase("update");
	std::string fileStatus;
	bool unknownClass = false;
	if (!args.FromServer)
	{
		size_t firstLine = host.getLog().size();
		bool untouched = true;
		bool updated = host.updateLoadedScenario(scenario, "File", untouched, error);
		fileStatus = reportLastUpdate(host, "version_update", "file", unknownClass);
		reportPhaseLog(host, "update", firstLine, !updated);
		if (!updated)
		{
			if (fileStatus != "failed") reportBlock("version_update_error", error);
			report("upgrade", fileStatus != "failed" ? "error" : (unknownClass ? "unknown_class" : "failed"));
			reportUses(host);
			return ExitUpdateFailed;
		}
		// A scenario the update had nothing to change in must have come through the table and the steps
		// as it was read, apart from the list of versions the update gives one of an earlier version
		if (fileStatus == "unchanged")
		{
			std::string afterText = "scenario = ";
			scenario->serialize(afterText);
			untouched = withoutVersions(afterText) == withoutVersions(originalText);
		}
		if (fileStatus != "changed")
			report("version_update_untouched", untouched ? "yes" : "no: the table of the scenario does not give the scenario as it was read");
	}

	// The scenario is uploaded and comes back as the edited scenario. One that is not at the versions of
	// the editor then (--from-server) is updated as a table and sent again, and comes back again.
	CUpdateOutcome first;
	CUpdateOutcome last;
	uint32 rounds = 0;
	bool repeats = false;
	bool sessionChanged = false;
	{
		CObject::TSmartPtr tree = scenario;
		scenario = nullptr;
		for (;;)
		{
			++rounds;
			last = scenarioUpdated(host, tree, 1, rounds == 1 ? "" : toString("round%u_", rounds), true);
			if (rounds == 1) first = last;
			if (last.Status == "changed") sessionChanged = true;
			tree = nullptr;
			if (!last.Attached || !host.takeUploadedScenario(tree))
				break;
			if (rounds == s_MaxUpdateRounds)
			{
				repeats = true;
				break;
			}
		}
	}
	report("update_rounds", toString("%u", rounds));
	if (!last.Attached || last.UpdateError)
	{
		report("upgrade", "error");
		reportUses(host);
		return ExitUpdateFailed;
	}
	if (repeats)
	{
		// every round sent the scenario again and found it in need of an update when it came back
		report("upgrade", "repeats");
		reportUses(host);
		return ExitUpdateRepeats;
	}
	if (last.Status == "failed")
	{
		report("upgrade", last.UnknownClass ? "unknown_class" : "failed");
		reportUses(host);
		return ExitUpdateFailed;
	}
	if (fileStatus == "changed" || sessionChanged) report("upgrade", "ok");
	else if (fileStatus == "unchanged" || first.Status == "unchanged") report("upgrade", "unchanged");
	else report("upgrade", "not_needed");

	// the author selects another act in the scenario window (r2.ScenarioWindow, r2_ui_event_handlers.lua)
	if (args.SelectedAct >= 0)
	{
		std::vector<std::string> arguments;
		arguments.push_back(toString("%d", args.SelectedAct));
		if (!host.callDriver("selectAct", arguments, 0, error))
			reportBlock("select_act_error", error);
		report("selected_act", toString("%d", args.SelectedAct));
	}

	// the author adds a component to the scenario
	if (!args.EditFile.empty())
	{
		host.setPhase("edit");
		size_t firstEditLine = host.getLog().size();
		std::string editText;
		CObject::TSmartPtr component;
		CIFile editFile;
		if (editFile.open(args.EditFile))
		{
			editText.resize(editFile.getFileSize());
			if (!editText.empty())
				editFile.serialBuffer((uint8 *)&editText[0], (uint)editText.size());
			component = CScenarioTextParser::parse(editText, nullptr, error);
		}
		else
		{
			error = "the file can not be read";
		}
		if (!component || !component->isTable() || !component->isString("InstanceId") || !component->isString("Class"))
		{
			report("edit", "fail");
			report("edit_error", component ? "the component has no InstanceId or no Class" : error);
			reportUses(host);
			return ExitFileMissing;
		}
		if (!host.addFeature(component, error))
		{
			report("edit", "fail");
			reportBlock("edit_error", error);
			reportPhaseLog(host, "edit", firstEditLine, true);
			reportUses(host);
			return ExitException;
		}
		host.endFrame();
		report("edit", "ok class=" + component->toString("Class"));
		reportPhaseLog(host, "edit", firstEditLine, false);
	}

	// the editor connects again, and gets the scenario the server has
	if (args.ReturnBeforeTest)
		scenarioReturns(host, "early_return_");

	// 4. A test is started. CClientEditionModule::requestStartScenario saves the scenario to
	// save/r2_buffer.dat with r2.Version.save. When that fails the file keeps what it had, which is the
	// scenario file as it was loaded (loadScenarioSucceded copies it there).
	host.setPhase("save");
	size_t firstLine = host.getLog().size();
	std::string buffer = fileText;
	if (host.callDriver("save", std::vector<std::string>(), 1, error))
	{
		std::string saved = stackString(host.getLua(), -1);
		lua_pop(host.getLua(), 1);
		if (saved == "saved")
		{
			buffer = "scenario = ";
			host.getHighLevel()->serialize(buffer);
			report("save", toString("ok bytes=%u", (uint)buffer.size()));
			// what the update and the save made of the file, apart from the versions the save gives
			report("scenario_vs_file", withoutVersions(buffer) == withoutVersions(originalText) ? "same" : "differs");
		}
		else
		{
			// r2.Version.save returned before it wrote the file
			report("save", "refused (" + saved + "): the test reads back the scenario file as it was loaded");
		}
	}
	else
	{
		report("save", "failed: the test reads back the scenario file as it was loaded");
		reportBlock("save_error", error);
	}
	reportPhaseLog(host, "save", firstLine, false);
	host.endFrame();
	host.endStepWithLateRequests();
	s_SavedScenarioText = buffer;
	s_SavedScenario = CScenarioTextParser::parse(buffer, nullptr, error);
	if (!args.HlOut.empty())
		report("hl_out", writeFile(args.HlOut, buffer) ? args.HlOut : "write failed");

	// CClientEditionModule::startingScenario: the starting act, then the saved scenario is read back, as
	// a file is, and becomes the edited scenario
	host.setPhase("update");
	if (!host.callDriver("initStartingActIndex", std::vector<std::string>(), 0, error))
		reportBlock("starting_act_error", error);
	uint32 lastActIndex = host.getStartingActIndex();
	CObject::TSmartPtr reread = CScenarioTextParser::parse(buffer, nullptr, error);
	if (!reread)
	{
		report("roundtrip", "fail");
		report("roundtrip_error", error);
		reportUses(host);
		return ExitReadBackFailed;
	}
	uint32 rereadNodes = 0, rereadInstances = 0;
	countNodes(reread, rereadNodes, rereadInstances);
	report("roundtrip", toString("ok nodes=%u instances=%u bytes=%u", rereadNodes, rereadInstances, (uint)buffer.size()));
	{
		firstLine = host.getLog().size();
		bool untouched = true;
		bool reloadUnknownClass = false;
		std::string rereadText = "scenario = ";
		reread->serialize(rereadText);
		bool updated = host.updateLoadedScenario(reread, "Test", untouched, error);
		std::string reloadStatus = reportLastUpdate(host, "reload_version_update", "file", reloadUnknownClass);
		reportPhaseLog(host, "update", firstLine, !updated);
		if (!updated)
		{
			// the client then keeps the scenario it has for the test
			if (reloadStatus != "failed") reportBlock("reload_version_update_error", error);
			reportUses(host);
			return ExitReadBackFailed;
		}
		if (reloadStatus == "unchanged")
		{
			std::string afterText = "scenario = ";
			reread->serialize(afterText);
			untouched = withoutVersions(afterText) == withoutVersions(rereadText);
		}
		if (reloadStatus != "changed" && !untouched)
			report("reload_version_update_untouched", "no: the table of the scenario does not give the scenario as it was read");
	}
	CUpdateOutcome reload = scenarioUpdated(host, reread, lastActIndex, "reload_", false);
	reread = nullptr;
	if (!reload.Attached)
	{
		report("roundtrip", "fail");
		reportUses(host);
		return ExitReadBackFailed;
	}
	{
		// an upload this update asked for comes back after the translation
		CObject::TSmartPtr late;
		if (host.takeUploadedScenario(late))
			report("reload_upload", "asked for during the start of the test; the scenario comes back after the translation");
	}

	// 5. translation
	host.setPhase("translate");
	host.resetRtIds();
	firstLine = host.getLog().size();
	if (!host.callDriver("translate", std::vector<std::string>(), 1, error))
	{
		host.endStepWithLateRequests();
		report("translate", "fail");
		reportBlock("translate_error", error);
		reportPhaseLog(host, "translate", firstLine, true);
		reportUses(host);
		return ExitTranslateFailed;
	}
	CObject::TSmartPtr rt = CHeadlessHost::getObjectFromLua(host.getLua(), -1);
	lua_pop(host.getLua(), 1);
	host.endStepWithLateRequests();
	if (!rt || !rt->isTable())
	{
		report("translate", "fail");
		report("translate_error", "the translator returned no table");
		reportPhaseLog(host, "translate", firstLine, true);
		reportUses(host);
		return ExitTranslateFailed;
	}
	report("translate", "ok");
	reportPhaseLog(host, "translate", firstLine, false);

	const CObject *acts = rt->getAttr("Acts");
	const CObject *texts = rt->getAttr("Texts");
	uint32 actCount = (acts && acts->isTable()) ? acts->getSize() : 0;
	std::string rtText;
	rt->serialize(rtText);
	s_RtScenarioText = rtText;
	s_RtScenario = rt;
	uint32 rtNodes = 0, rtInstances = 0;
	countNodes(rt, rtNodes, rtInstances);
	report("rt", toString("acts=%u texts=%u plot_items=%u locations=%u starting_act=%u nodes=%u bytes=%u longest_string=%u", actCount, (texts && texts->isTable()) ? sizeOfAttr(texts, "Texts") : 0, sizeOfAttr(rt, "PlotItems"), sizeOfAttr(rt, "Locations"), host.getStartingActIndex(), rtNodes, (uint)rtText.size(), longestString(rt)));
	for (uint32 i = 0; i < actCount; ++i)
	{
		// "actions" are the entries of the act's list; an action holds its sub actions, counted in "actions_all"
		const CObject *act = acts->getValueAtPos(i);
		report("rt_act", toString("index=%u npcs=%u groups=%u states=%u events=%u actions=%u actions_all=%u user_triggers=%u fauna_groups=%u", i, sizeOfAttr(act, "Npcs"), sizeOfAttr(act, "NpcGrps"), sizeOfAttr(act, "AiStates"), sizeOfAttr(act, "Events"), sizeOfAttr(act, "Actions"), countClass(act, "RtNpcEventHandlerAction"), sizeOfAttr(act, "UserTriggers"), sizeOfAttr(act, "FaunaGrps")));
	}
	if (!args.RtOut.empty())
	{
		// as the client writes scenario.rt.txt
		report("rt_out", writeFile(args.RtOut, rtText) ? args.RtOut : "write failed");
	}

	// 6. The check of the scenario service on an uploaded run-time scenario, on the tree and on the tree
	// as it arrives through the serial form
	std::string limitsDescription;
	CRtScenarioLimits limits = serviceLimits(limitsDescription);
	report("rt_check_limits", limitsDescription);
	TRtScenarioVerdict verdict = reportRtCheck(rt, limits, host.getStartingActIndex(), "rt_check");
	CObject::TSmartPtr copy = throughSerialForm(rt, error);
	TRtScenarioVerdict serialVerdict = RtScenarioInvalid;
	if (!copy)
	{
		report("rt_check_serial", "invalid: the serial form could not be read back: " + error);
	}
	else
	{
		// the serial form keeps a number as the smallest type that holds it, so the copy is compared by its text
		std::string copyText;
		copy->serialize(copyText);
		report("rt_serial_copy", copyText == rtText ? "same text form" : "the text form differs from the tree's");
		if (copyText != rtText && !args.RtOut.empty())
			writeFile(args.RtOut + ".serial.txt", copyText);
		serialVerdict = reportRtCheck(copy, limits, host.getStartingActIndex(), "rt_check_serial");
	}

	// 7. The test ends, and the scenario comes back from the server
	scenarioReturns(host, "return_");

	reportUses(host);
	if (verdict != RtScenarioOk || serialVerdict != RtScenarioOk)
		return ExitCheckRefused;
	return ExitOk;
}

} // namespace

int main(int argc, char **argv)
{
#ifdef NL_OS_WINDOWS
	// a failed assertion ends the process with a message on stderr, no dialog
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

	CArgs args;
#ifdef RYZOM_R2_LUA_DIR
	args.R2Dir = RYZOM_R2_LUA_DIR;
#endif
#ifdef RYZOM_R2_CHAIN_DRIVER
	args.Driver = RYZOM_R2_CHAIN_DRIVER;
#endif
	if (!parseArgs(argc, argv, args) || args.R2Dir.empty() || args.Driver.empty())
	{
		usage();
		return ExitUsage;
	}

	CApplicationContext applicationContext;
	createDebug(nullptr, false);
	// the NeL log goes to a file when asked for and nowhere else; errors and assertions stay on the console
	INelContext &context = INelContext::getInstance();
	context.getDebugLog()->removeDisplayer("DEFAULT_SD");
	context.getInfoLog()->removeDisplayer("DEFAULT_SD");
	context.getWarningLog()->removeDisplayer("DEFAULT_SD");
	context.getErrorLog()->removeDisplayer("DEFAULT_MBD");
	context.getAssertLog()->removeDisplayer("DEFAULT_MBD");
	if (!args.NelLog.empty())
	{
		CFileDisplayer *fileDisplayer = new CFileDisplayer(args.NelLog, true, "R2_CHAIN_FD");
		context.getDebugLog()->addDisplayer(fileDisplayer);
		context.getInfoLog()->addDisplayer(fileDisplayer);
		context.getWarningLog()->addDisplayer(fileDisplayer);
	}

	for (size_t i = 0; i < args.DataDirs.size(); ++i)
		CPath::addSearchPath(args.DataDirs[i], true, false);
	if (!args.Leveldesign.empty() && CFile::fileExists(args.Leveldesign + "/game_elem/sheet_id.bin"))
		CPath::addSearchFile(args.Leveldesign + "/game_elem/sheet_id.bin");
	if (CPath::exists("sheet_id.bin"))
	{
		CSheetId::init(false);
		report("sheet_id", "loaded");
	}
	else
	{
		CSheetId::initWithoutSheet();
		report("sheet_id", "none");
	}

	int result;
	try
	{
		result = run(args);
	}
	catch (const std::exception &e)
	{
		report("exception", e.what());
		result = ExitException;
	}
	// What the run came to, for the expectations. A run that is expected to end in a failure ("result: 4")
	// and does is a run that went as expected: the program returns 0 then.
	report("result", toString("%d", result));
	if (!args.ExpectFile.empty())
	{
		bool resultExpected = false;
		if (checkExpectations(args.ExpectFile, resultExpected) > 0)
			result = ExitExpectationFailed;
		else if (resultExpected)
			result = ExitOk;
	}
	report("exit", toString("%d", result));
	return result;
}
