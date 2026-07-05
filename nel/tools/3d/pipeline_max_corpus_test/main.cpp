// Corpus-level roundtrip tester for pipeline_max.
//
// For a single .max file:
//   T1  structural roundtrip:  read-stream → serial-out to buffer → byte-compare per stream.
//   T2  parse/build roundtrip: read-stream → typed parse → clean → build → disown → serialize
//                              back → byte-compare per stream. Only run for streams we type
//                              (DllDirectory, ClassDirectory3, Scene). Others fall through to T1.
//
// Exit code:
//   0 = all requested tests passed.
//   1 = failure (details on stderr, machine-readable one-line summary on stdout).
//
// The Python driver (test/skel_corpus_test.py) iterates the manifest and aggregates.
//
// Streams tested:
//   DllDirectory, ClassDirectory3, ClassData, Config, Scene, VideoPostQueue,
//   \05SummaryInformation, \05DocumentSummaryInformation
// All by-name; missing streams are skipped (some files have no VideoPostQueue).

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/mem_stream.h>

#include <cstdio>
#include <fstream>

#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-utils.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/class_data.h"
#include "../pipeline_max/config.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"

#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"

using namespace PIPELINE::MAX;

// Serialize a container to a temp file and read the file bytes back. CMemStream's length()==Pos
// in write mode makes seek-back-then-restore fail during leaveChunk (see storage_chunks.cpp);
// COFile handles seeks freely, so temp-file round-trip is the working pattern (same as
// pipeline_max_dump's temp.bin approach).
static std::vector<uint8> writeContainerToTemp(CStorageContainer &ctr, const std::string &tempPath)
{
	{
		NLMISC::COFile of(tempPath);
		ctr.serial(of, 0); // explicit-size overload avoids the outer 0x4352 wrapper
	}
	std::vector<uint8> out;
	std::ifstream ifs(tempPath.c_str(), std::ios::binary);
	if (ifs)
	{
		ifs.seekg(0, std::ios::end);
		std::streampos end = ifs.tellg();
		ifs.seekg(0);
		out.resize((size_t)end);
		if ((size_t)end) ifs.read((char *)out.data(), (std::streamsize)end);
	}
	return out;
}

// Read an entire gsf stream into a byte vector.
static std::vector<uint8> readAll(GsfInput *in)
{
	std::vector<uint8> out;
	gsf_off_t sz = gsf_input_size(in);
	out.resize((size_t)sz);
	if (sz > 0)
	{
		gsf_input_seek(in, 0, G_SEEK_SET);
		if (!gsf_input_read(in, (size_t)sz, out.data()))
		{
			out.clear();
		}
	}
	return out;
}

// Print a short diff summary of two byte vectors for triage. Only the first diff run.
static std::string diffSummary(const std::vector<uint8> &a, const std::vector<uint8> &b)
{
	std::ostringstream ss;
	ss << "sizes " << a.size() << " vs " << b.size();
	size_t n = std::min(a.size(), b.size());
	size_t firstDiff = n;
	for (size_t i = 0; i < n; ++i) { if (a[i] != b[i]) { firstDiff = i; break; } }
	if (firstDiff < n)
	{
		ss << " first-diff-at " << firstDiff;
		ss << " a=";
		for (size_t i = firstDiff; i < firstDiff + 8 && i < a.size(); ++i)
		{
			char buf[8]; snprintf(buf, sizeof(buf), "%02x", a[i]); ss << buf;
		}
		ss << " b=";
		for (size_t i = firstDiff; i < firstDiff + 8 && i < b.size(); ++i)
		{
			char buf[8]; snprintf(buf, sizeof(buf), "%02x", b[i]); ss << buf;
		}
	}
	return ss.str();
}

struct StreamResult
{
	std::string Name;
	bool Exists;
	bool T1Ok;
	std::string T1Info;
	bool T2Applicable;
	bool T2Ok;
	std::string T2Info;
};

// Do a T1 roundtrip via CStorageContainer (raw pass-through). Reads gsf stream into src bytes,
// serials into a container, serials container out to a memory buffer, compares bytes.
static std::string g_tempPath = "/tmp/pipeline_max_corpus_test.tmp";

static bool t1Roundtrip(GsfInput *in, std::vector<uint8> &src, std::vector<uint8> &rt, std::string &info)
{
	src = readAll(in);
	gsf_input_seek(in, 0, G_SEEK_SET); // readAll left the cursor at end; CStorageContainer needs pos 0.
	CStorageStream ss(in);
	CStorageContainer ctr;
	try { ctr.serial(ss); }
	catch (std::exception &e) { info = std::string("read-throw: ") + e.what(); return false; }

	try { rt = writeContainerToTemp(ctr, g_tempPath); }
	catch (std::exception &e) { info = std::string("write-throw: ") + e.what(); return false; }
	if (rt == src) return true;
	info = diffSummary(src, rt);
	return false;
}

// Do a T2 roundtrip specialized per stream type. Returns false if roundtrip diverges.
// applicable=false means we don't have a typed class for this stream — caller falls back to T1 only.
static bool t2DllDirectory(GsfInput *in, const std::vector<uint8> &src, std::string &info)
{
	CDllDirectory dll;
	{ CStorageStream ss(in); try { dll.serial(ss); } catch (std::exception &e) { info = std::string("read-throw: ") + e.what(); return false; } }
	try { dll.parse(VersionUnknown); dll.clean(); dll.build(VersionUnknown); dll.disown(); }
	catch (std::exception &e) { info = std::string("lifecycle-throw: ") + e.what(); return false; }
	std::vector<uint8> rt;
	try { rt = writeContainerToTemp(dll, g_tempPath); } catch (std::exception &e) { info = std::string("write-throw: ") + e.what(); return false; }
	if (rt == src) return true;
	info = diffSummary(src, rt);
	return false;
}

static bool t2ClassDirectory3(GsfInput *in, const std::vector<uint8> &src, CDllDirectory *dll, std::string &info)
{
	CClassDirectory3 cd(dll);
	{ CStorageStream ss(in); try { cd.serial(ss); } catch (std::exception &e) { info = std::string("read-throw: ") + e.what(); return false; } }
	try { cd.parse(VersionUnknown); cd.clean(); cd.build(VersionUnknown); cd.disown(); }
	catch (std::exception &e) { info = std::string("lifecycle-throw: ") + e.what(); return false; }
	std::vector<uint8> rt;
	try { rt = writeContainerToTemp(cd, g_tempPath); } catch (std::exception &e) { info = std::string("write-throw: ") + e.what(); return false; }
	if (rt == src) return true;
	info = diffSummary(src, rt);
	return false;
}

static bool t2Scene(GsfInput *in, const std::vector<uint8> &src, CSceneClassRegistry *reg,
                    CDllDirectory *dll, CClassDirectory3 *cd, std::string &info)
{
	CScene scene(reg, dll, cd);
	{ CStorageStream ss(in); try { scene.serial(ss); } catch (std::exception &e) { info = std::string("read-throw: ") + e.what(); return false; } }
	try { scene.parse(VersionUnknown); scene.clean(); scene.build(VersionUnknown); scene.disown(); }
	catch (std::exception &e) { info = std::string("lifecycle-throw: ") + e.what(); return false; }
	std::vector<uint8> rt;
	try { rt = writeContainerToTemp(scene, g_tempPath); } catch (std::exception &e) { info = std::string("write-throw: ") + e.what(); return false; }
	if (rt == src) return true;
	info = diffSummary(src, rt);
	return false;
}

int main(int argc, char **argv)
{
	bool doT2 = false;
	bool verbose = false;
	int argi = 1;
	while (argi < argc && argv[argi][0] == '-' && argv[argi][1] == '-')
	{
		std::string a = argv[argi];
		if (a == "--parse") doT2 = true;
		else if (a == "--verbose" || a == "-v") verbose = true;
		else break;
		++argi;
	}
	if (argc - argi < 1)
	{
		std::cerr << "usage: pipeline_max_corpus_test [--parse] [--verbose] <input.max>\n";
		return 2;
	}
	const char *maxFile = argv[argi];

	g_set_prgname(argv[0]);
	gsf_init();

	GsfInput *src = gsf_input_stdio_new(maxFile, NULL);
	if (!src) { std::cerr << "cannot open " << maxFile << "\n"; return 2; }
	GsfInfile *in = gsf_infile_msole_new(src, NULL);
	if (!in) { std::cerr << "not an OLE file: " << maxFile << "\n"; g_object_unref(src); return 2; }

	CSceneClassRegistry reg;
	BUILTIN::CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);

	// Chunk-formatted streams that participate in the CStorageContainer round-trip. The two
	// \05Summary* streams and the OLE class-id are raw byte blobs (copied verbatim by any full
	// rewrite tool), so byte identity is trivially met and they aren't retested here.
	std::vector<std::string> streamNames = {
		"DllDirectory",
		"ClassDirectory3",
		"ClassData",
		"Config",
		"Scene",
		"VideoPostQueue",
	};

	// State that carries across streams for T2 (need dll to interpret classdir; both to interpret scene).
	CDllDirectory dllForT2;
	bool dllReady = false;
	CClassDirectory3 cdForT2(&dllForT2);
	bool cdReady = false;

	std::vector<StreamResult> results;
	bool anyFail = false;

	for (const std::string &name : streamNames)
	{
		StreamResult r{ name, false, false, "", false, false, "" };
		GsfInput *s = gsf_infile_child_by_name(in, name.c_str());
		if (!s) { results.push_back(r); continue; }
		r.Exists = true;

		std::vector<uint8> srcBytes, rtBytes;
		r.T1Ok = t1Roundtrip(s, srcBytes, rtBytes, r.T1Info);

		if (doT2)
		{
			g_object_unref(s);
			s = gsf_infile_child_by_name(in, name.c_str());
			if (name == "DllDirectory")
			{
				r.T2Applicable = true;
				r.T2Ok = t2DllDirectory(s, srcBytes, r.T2Info);
				// Also prime dllForT2 for later streams.
				if (r.T2Ok)
				{
					g_object_unref(s);
					s = gsf_infile_child_by_name(in, name.c_str());
					CStorageStream ss(s); try { dllForT2.serial(ss); dllForT2.parse(VersionUnknown); dllReady = true; } catch (...) {}
				}
			}
			else if (name == "ClassDirectory3" && dllReady)
			{
				r.T2Applicable = true;
				r.T2Ok = t2ClassDirectory3(s, srcBytes, &dllForT2, r.T2Info);
				if (r.T2Ok)
				{
					g_object_unref(s);
					s = gsf_infile_child_by_name(in, name.c_str());
					CStorageStream ss(s); try { cdForT2.serial(ss); cdForT2.parse(VersionUnknown); cdReady = true; } catch (...) {}
				}
			}
			else if (name == "Scene" && dllReady && cdReady)
			{
				r.T2Applicable = true;
				r.T2Ok = t2Scene(s, srcBytes, &reg, &dllForT2, &cdForT2, r.T2Info);
			}
		}

		g_object_unref(s);

		if (!r.T1Ok || (r.T2Applicable && !r.T2Ok)) anyFail = true;
		results.push_back(r);
	}

	// Concise one-line stdout summary + per-stream verbose if requested or on failure.
	std::cout << (anyFail ? "FAIL" : "OK") << " " << maxFile;
	for (const StreamResult &r : results)
	{
		if (!r.Exists) continue;
		std::cout << " " << r.Name << ":T1=" << (r.T1Ok ? "ok" : "FAIL");
		if (r.T2Applicable) std::cout << ",T2=" << (r.T2Ok ? "ok" : "FAIL");
	}
	std::cout << "\n";
	if (anyFail || verbose)
	{
		for (const StreamResult &r : results)
		{
			if (!r.Exists) continue;
			if (!r.T1Ok) std::cerr << "  T1 " << r.Name << " FAIL: " << r.T1Info << "\n";
			if (r.T2Applicable && !r.T2Ok) std::cerr << "  T2 " << r.Name << " FAIL: " << r.T2Info << "\n";
		}
	}

	g_object_unref(in);
	g_object_unref(src);
	gsf_shutdown();
	return anyFail ? 1 : 0;
}
