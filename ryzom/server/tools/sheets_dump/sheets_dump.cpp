// NeL - MMORPG Framework <http://www.ryzomcore.org/>
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <nel/misc/types_nl.h>

// STL includes
#include <vector>
#include <string>
#include <map>

// NeL includes
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/sheet_id.h>
#include <nel/misc/stream.h>
#include <nel/georges/load_form.h>
#include <game_share/data_set_base.h>
#include <input_output_service/string_manager.h>
#include <gpm_service/sheets.h>
#include <server_share/continent_container.h>
#include <entities_game_service/egs_sheets/egs_sheets.h>
#include <game_share/time_weather_season/time_date_season_manager.h>
#include <ai_service/stdpch.h>
#include <ai_service/sheets.h>

// This tool converts a .packed_sheets file (a binary NLMISC::IStream serialized
// container) into a human readable, diffable plain text file.
//
// This works because .packed_sheets files are written using only generic
// NLMISC::IStream calls (serial/serialCont/serialPtrCont), never raw binary
// I/O. The reading side reuses the very same loadForm()/loadForm2() templates
// the real services use to load these files (with updatePackedSheet=false),
// so there is no risk of the reader silently drifting from the real format.
//
// The writing side used to go through NLMISC::COXml, but some sheet classes
// (e.g. CStaticOutpostBuilding, which conditionally serializes a whole extra
// sub-struct depending on a runtime Type value, see egs_static_outpost.cpp)
// trip up COXml's strict xmlPushBegin/xmlSetAttrib/xmlPushEnd protocol and
// abort with "the stream don't use XML streaming properly" (this is a real,
// documented limitation of COXml, not a bug in those classes: it is fine for
// a plain binary stream, which tolerates any data-dependent structure).
// CTextDumpStream below is a tiny custom IStream that renders the exact same
// serial()/serialCont() call graph as indented plain text instead, without
// COXml's rigid node protocol, so it works uniformly for every sheet type.

// egs_sheets.cpp (compiled below via the EGSSHEETS glob) is built with
// -DNO_EGS_VARS, which makes it expect these symbols to be defined
// externally instead of pulling in the real entities_game_service.
// None of the code paths that use them are actually exercised by this
// tool (we call loadForm()/loadForm2() directly instead of going through
// CSheets::init()/loadSheetSet()), but they still need to exist for the
// linker since the whole egs_sheets.cpp translation unit is linked in.
NLMISC::CVariable<bool> EGSLight("egs", "EGSLight", "Load EGS with a minimal set of feature loaded", false, 0, true);
NLMISC::CVariable<bool> LoadOutposts("egs", "LoadOutposts", "If false outposts won't be loaded", true, 0, true);
static std::string s_WriteDirectory;
std::string writeDirectory()
{
	return s_WriteDirectory;
}

namespace /* anonymous */
{

class CTextDumpStream : public NLMISC::IStream
{
public:
	// overriding some serial() overloads below hides ALL base class overloads
	// of that name, including IStream's generic "template<class T> void
	// serial(T&) { obj.serial(*this); }" fallback used by every custom class
	// (CSheetId, CStaticItem, ...): bring it (and the others) back into scope.
	using NLMISC::IStream::serial;

	CTextDumpStream()
		: NLMISC::IStream(false), _Depth(0), _NeedSeparator(false)
	{
		// route xmlPush*/xmlPop/xmlSetAttrib through the *Internal overrides below
		setXMLMode(true);
	}

	const std::string &text() const { return _Text; }

	// clear the buffer so it can be reused for the next entry: without this,
	// a single entry that produces an abnormal amount of text (e.g. a bad/
	// runaway loop count from corrupted data) keeps growing an ever-larger
	// shared buffer, making every following entry fail too as collateral
	// damage instead of just the one actually at fault.
	void reset()
	{
		_Text.clear();
		_Depth = 0;
		_NeedSeparator = false;
	}

	// explicit, greppable marker for the start of a top-level map entry,
	// with both the raw sheetid and its resolved name (so you don't have to
	// go look it up in sheet_id.bin separately to know what you're reading).
	void writeKey(const NLMISC::CSheetId &id)
	{
		_Text += "<KEY " + NLMISC::toString(id.asInt()) + " " + id.toString() + ">";
		_NeedSeparator = false;
	}

	// explicit entry separator, used between top-level dumped entries
	void newline()
	{
		_Text += "\n";
		_NeedSeparator = false;
	}

	virtual void serialBuffer(uint8 *buf, uint len)
	{
		for (uint i = 0; i < len; ++i)
			writeToken(NLMISC::toString((uint32)buf[i]));
	}

	virtual void serialBit(bool &bit) { serial(bit); }

	virtual void serial(uint8 &b) { writeToken(NLMISC::toString(b)); }
	virtual void serial(sint8 &b) { writeToken(NLMISC::toString(b)); }
	virtual void serial(uint16 &b) { writeToken(NLMISC::toString(b)); }
	virtual void serial(sint16 &b) { writeToken(NLMISC::toString(b)); }
	virtual void serial(uint32 &b) { writeToken(NLMISC::toString(b)); }
	virtual void serial(sint32 &b) { writeToken(NLMISC::toString(b)); }
	virtual void serial(uint64 &b) { writeToken(NLMISC::toString(b)); }
	virtual void serial(sint64 &b) { writeToken(NLMISC::toString(b)); }
	virtual void serial(float &b) { writeToken(NLMISC::toString(b)); }
	virtual void serial(double &b) { writeToken(NLMISC::toString(b)); }
	virtual void serial(bool &b) { writeToken(NLMISC::toString(b)); }
#ifndef NL_OS_CYGWIN
	virtual void serial(char &b) { writeToken(NLMISC::toString((sint32)b)); }
#endif
	virtual void serial(std::string &b) { writeQuoted(b); }
	virtual void serial(ucstring &b)
	{
		std::string u = b.toUtf8();
		writeQuoted(u);
	}

protected:
	virtual bool xmlPushBeginInternal(const std::string &name)
	{
		_Text += "\n" + std::string(_Depth * 2, ' ') + "<" + name + ">";
		++_Depth;
		_NeedSeparator = false;
		return true;
	}

	virtual bool xmlPushEndInternal() { return true; }

	virtual bool xmlPopInternal()
	{
		if (_Depth > 0)
			--_Depth;
		return true;
	}

	virtual bool xmlSetAttribInternal(const std::string & /* name */) { return true; }

private:
	std::string _Text;
	uint _Depth;
	bool _NeedSeparator;

	void writeToken(const std::string &s)
	{
		if (_NeedSeparator)
			_Text += " ";
		_Text += s;
		_NeedSeparator = true;
	}

	void writeQuoted(const std::string &s)
	{
		std::string escaped;
		escaped.reserve(s.size() + 2);
		escaped += '"';
		for (uint i = 0; i < s.size(); ++i)
		{
			if (s[i] == '"' || s[i] == '\\')
				escaped += '\\';
			escaped += s[i];
		}
		escaped += '"';
		writeToken(escaped);
	}
};

// Dump a container entry by entry, writing each entry's text out immediately
// and resetting the stream in between (rather than one serialCont() call
// accumulating everything in a single buffer for the whole map). This way:
// - a single corrupted/incompatible entry is reported with its exact sheet
//   name/id and skipped instead of aborting the whole dump;
// - an entry that produces an abnormal amount of text (e.g. a runaway loop
//   count from corrupted data) cannot poison every entry that follows it by
//   growing an ever-larger shared buffer.
template <class Iterator>
uint dumpEntriesPtr(Iterator first, Iterator last, CTextDumpStream &ts, NLMISC::COFile &of)
{
	uint nbFailed = 0;
	for (Iterator it = first; it != last; ++it)
	{
		ts.reset();
		try
		{
			NLMISC::CSheetId key = it->first;
			ts.writeKey(key);
			if (!it->second.isNull())
				it->second->serial(ts);
			ts.newline();
		}
		catch (const std::exception &e)
		{
			nlwarning("Entry '%s' (sheetid %u) failed to dump : %s",
				it->first.toString().c_str(), it->first.asInt(), e.what());
			++nbFailed;
			continue;
		}
		const std::string &text = ts.text();
		if (!text.empty())
			of.serialBuffer((uint8 *)text.data(), (uint)text.size());
	}
	return nbFailed;
}

template <class Iterator>
uint dumpEntries(Iterator first, Iterator last, CTextDumpStream &ts, NLMISC::COFile &of)
{
	uint nbFailed = 0;
	for (Iterator it = first; it != last; ++it)
	{
		ts.reset();
		try
		{
			NLMISC::CSheetId key = it->first;
			ts.writeKey(key);
			it->second.serial(ts);
			ts.newline();
		}
		catch (const std::exception &e)
		{
			nlwarning("Entry '%s' (sheetid %u) failed to dump : %s",
				it->first.toString().c_str(), it->first.asInt(), e.what());
			++nbFailed;
			continue;
		}
		const std::string &text = ts.text();
		if (!text.empty())
			of.serialBuffer((uint8 *)text.data(), (uint)text.size());
	}
	return nbFailed;
}

// Plain std::map<CSheetId, T> container, single sheet type filter.
template <class T>
bool dumpMap(const std::string &fileType, const std::string &inputFile, const std::string &outputFile)
{
	std::map<NLMISC::CSheetId, T> container;
	try
	{
		loadForm(fileType, inputFile, container, false, true);
	}
	catch (const std::exception &e)
	{
		nlwarning("Failed to load '%s' : %s", inputFile.c_str(), e.what());
		return false;
	}

	NLMISC::COFile of;
	if (!of.open(outputFile, false, true, false))
	{
		nlwarning("Can't open '%s' for writing", outputFile.c_str());
		return false;
	}
	CTextDumpStream ts;
	uint nbFailed = dumpEntries(container.begin(), container.end(), ts, of);
	of.close();

	if (nbFailed > 0)
		nlwarning("%u/%u entries failed to dump", nbFailed, (uint)container.size());
	nlinfo("Wrote %u entries to '%s'", (uint)container.size() - nbFailed, outputFile.c_str());
	return true;
}

// Plain std::map<CSheetId, T> container, several sheet type filters
// (used for the sheets that are stored in a hash_map at runtime but are
// still packed as a plain std::map on disk, e.g. items and creatures).
template <class T>
bool dumpMapMulti(const std::vector<std::string> &fileTypes, const std::string &inputFile, const std::string &outputFile)
{
	std::map<NLMISC::CSheetId, T> container;
	try
	{
		loadForm(fileTypes, inputFile, container, false, true);
	}
	catch (const std::exception &e)
	{
		nlwarning("Failed to load '%s' : %s", inputFile.c_str(), e.what());
		return false;
	}

	NLMISC::COFile of;
	if (!of.open(outputFile, false, true, false))
	{
		nlwarning("Can't open '%s' for writing", outputFile.c_str());
		return false;
	}
	CTextDumpStream ts;
	uint nbFailed = dumpEntries(container.begin(), container.end(), ts, of);
	of.close();

	if (nbFailed > 0)
		nlwarning("%u/%u entries failed to dump", nbFailed, (uint)container.size());
	nlinfo("Wrote %u entries to '%s'", (uint)container.size() - nbFailed, outputFile.c_str());
	return true;
}

// std::map<CSheetId, CSmartPtr<T> > container (e.g. AIS creature sheets).
template <class T>
bool dumpMapPtr(const std::string &fileType, const std::string &inputFile, const std::string &outputFile)
{
	std::map<NLMISC::CSheetId, NLMISC::CSmartPtr<T> > container;
	try
	{
		loadForm2(fileType, inputFile, container, false, true);
	}
	catch (const std::exception &e)
	{
		nlwarning("Failed to load '%s' : %s", inputFile.c_str(), e.what());
		return false;
	}

	NLMISC::COFile of;
	if (!of.open(outputFile, false, true, false))
	{
		nlwarning("Can't open '%s' for writing", outputFile.c_str());
		return false;
	}
	CTextDumpStream ts;
	uint nbFailed = dumpEntriesPtr(container.begin(), container.end(), ts, of);
	of.close();

	if (nbFailed > 0)
		nlwarning("%u/%u entries failed to dump", nbFailed, (uint)container.size());
	nlinfo("Wrote %u entries to '%s'", (uint)container.size() - nbFailed, outputFile.c_str());
	return true;
}

const char *KnownTypes[] = {
	"datasets", "ais", "egs_items", "egs_creatures",
	"egs_outpost_building", "egs_weather_setup"
};
const uint NbKnownTypes = sizeof(KnownTypes) / sizeof(KnownTypes[0]);

bool isKnownType(const std::string &s)
{
	for (uint i = 0; i < NbKnownTypes; ++i)
	{
		if (s == KnownTypes[i])
			return true;
	}
	return false;
}

void usage()
{
	nlinfo("USAGE : sheets_dump <input.packed_sheets> <output.txt> [type] [extra sheet_id.bin search dir]...");
	nlinfo("  <type> is optional, auto-detected from the input filename when omitted.");
	nlinfo("  Supported types (and the file name they are auto-detected from) :");
	nlinfo("    datasets              (datasets.packed_sheets)");
	nlinfo("    ais                   (ais.packed_sheets)");
	nlinfo("    egs_items             (egs_items.packed_sheets)");
	nlinfo("    egs_creatures         (egs_creatures.packed_sheets)");
	nlinfo("    egs_outpost_building  (egs_outpost_building.packed_sheets)");
	nlinfo("    egs_weather_setup     (egs_weather_setup.packed_sheets)");
	nlinfo("  The directory containing <input.packed_sheets> is always searched (recursively)");
	nlinfo("  for sheet_id.bin. Pass one or more extra directories (searched recursively) if");
	nlinfo("  it lives elsewhere, e.g. your live shard's leveldesign data directory.");
}

} /* anonymous namespace */

int main(int nNbArg, char **ppArgs)
{
	NLMISC::createDebug();

	if (nNbArg < 3)
	{
		usage();
		return EXIT_FAILURE;
	}

	std::string input = ppArgs[1];
	std::string output = ppArgs[2];

	std::string type;
	std::vector<std::string> extraSearchDirs;
	for (int i = 3; i < nNbArg; ++i)
	{
		std::string arg = ppArgs[i];
		if (isKnownType(arg))
			type = arg;
		else
			extraSearchDirs.push_back(arg);
	}

	if (type.empty())
		type = NLMISC::CFile::getFilenameWithoutExtension(NLMISC::CFile::getFilename(input));

	// sheet_id.bin is looked up via CPath, not next to the input file necessarily:
	// always search the input file's own directory, plus any extra directory given.
	NLMISC::CPath::addSearchPath(NLMISC::CFile::getPath(input), true, false);
	for (uint i = 0; i < extraSearchDirs.size(); ++i)
		NLMISC::CPath::addSearchPath(extraSearchDirs[i], true, false);

	// init sheet_id.bin, same as sheets_packer_shard does before any loadForm() call
	NLMISC::CSheetId::init(false);

	bool ok = false;
	try
	{
		if (type == "datasets")
		{
			ok = dumpMap<TDataSetSheet>("dataset", input, output);
		}
		else if (type == "ais")
		{
			ok = dumpMapPtr<AISHEETS::CCreature>("creature", input, output);
		}
		else if (type == "egs_items")
		{
			std::vector<std::string> fileTypes;
			fileTypes.push_back("sitem");
			ok = dumpMapMulti<CStaticItem>(fileTypes, input, output);
		}
		else if (type == "egs_creatures")
		{
			std::vector<std::string> fileTypes;
			fileTypes.push_back("creature");
			ok = dumpMapMulti<CStaticCreatures>(fileTypes, input, output);
		}
		else if (type == "egs_outpost_building")
		{
			ok = dumpMap<CStaticOutpostBuilding>("outpost_building", input, output);
		}
		else if (type == "egs_weather_setup")
		{
			ok = dumpMap<CWeatherSetupSheetBase>("weather_setup", input, output);
		}
		else
		{
			nlinfo("ERROR : unknown or undetected type '%s'", type.c_str());
			usage();
			return EXIT_FAILURE;
		}
	}
	catch (const std::exception &e)
	{
		// catches both NLMISC::Exception and plain std:: exceptions (e.g.
		// std::length_error from a corrupted/misaligned string read)
		nlwarning("Failed to convert '%s' : %s", input.c_str(), e.what());
		return EXIT_FAILURE;
	}

	return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* end of file */
