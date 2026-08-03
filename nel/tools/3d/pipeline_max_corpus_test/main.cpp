/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7
 * \author Claude Sonnet 5
 * \author Claude Fable 5
 * \author Claude Opus 4.8
 * \author Claude Opus 4.8 (1M context)
 */
// Corpus-level roundtrip tester for pipeline_max.
//
// For a single .max file:
//   T1  structural roundtrip:  read-stream → serial-out to buffer → byte-compare per stream.
//   T2  parse/build roundtrip: read-stream → typed parse → clean → build → disown → serialize
//                              back → byte-compare per stream. Only run for streams we type
//                              (DllDirectory, ClassDirectory3, Scene). Others fall through to T1.
//
// The OLE2/CFB container is accessed through PIPELINE::MAX::CStorageOleIn/CStorageOleOut, which is
// backed either by the native reader/writer (default) or by libgsf (compile-time switch); this
// tool is backend-agnostic.
//
// Exit code:
//   0 = all requested tests passed.
//   1 = failure (details on stderr, machine-readable one-line summary on stdout).

#include <nel/misc/types_nl.h>

// MSVC 9.0 (VS2008) has no C99 snprintf; _snprintf is equivalent for our fixed-size formatting.
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/mem_stream.h>

#include <cstdio>
#include <fstream>
#ifdef NL_OS_WINDOWS
#include <process.h>
#define PMCT_GETPID _getpid
#else
#include <unistd.h>
#define PMCT_GETPID getpid
#endif

#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../pipeline_max/storage_ole.h"
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
#include "../pipeline_max/nelpatch/nelpatch.h"
#include "../pipeline_max/nelpatch/rkl_patch_object.h"

#include "../pipeline_max/builtin/animatable.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/storage/mesh_delta.h"
#include "../pipeline_max/builtin/storage/map_extender_cache.h"
#include "../pipeline_max/builtin/param_block.h"
#include "../pipeline_max/builtin/param_block_2.h"
#include "../pipeline_max/builtin/shape_object.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/wsm_derived_object.h"
#include "../pipeline_max/builtin/mtl_base.h"
#include "../pipeline_max/builtin/multi_mtl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/control_transform.h"
#include "../pipeline_max/builtin/geom_object.h"
#include "../pipeline_max/builtin/storage/geom_buffers.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/storage_array.h"

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
		if ((size_t)end) ifs.read((char *)nlVectorData(out), (std::streamsize)end);
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

// PID-suffixed so concurrent invocations (e.g. a parallelized corpus sweep, or two unrelated test
// runs overlapping) don't race on the same file and corrupt each other's round-trip.
static std::string g_tempPath = "/tmp/pipeline_max_corpus_test." + NLMISC::toString((sint32)PMCT_GETPID()) + ".tmp";

// Do a T1 roundtrip via CStorageContainer (raw pass-through). Serials the source bytes into a
// container, serials the container back out, compares bytes.
static bool t1Roundtrip(const std::vector<uint8> &src, std::vector<uint8> &rt, std::string &info)
{
	CStorageStream ss(src);
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
static bool t2DllDirectory(const std::vector<uint8> &src, std::string &info)
{
	CDllDirectory dll;
	{ CStorageStream ss(src); try { dll.serial(ss); } catch (std::exception &e) { info = std::string("read-throw: ") + e.what(); return false; } }
	try { dll.parse(VersionUnknown); dll.clean(); dll.build(VersionUnknown); dll.disown(); }
	catch (std::exception &e) { info = std::string("lifecycle-throw: ") + e.what(); return false; }
	std::vector<uint8> rt;
	try { rt = writeContainerToTemp(dll, g_tempPath); } catch (std::exception &e) { info = std::string("write-throw: ") + e.what(); return false; }
	if (rt == src) return true;
	info = diffSummary(src, rt);
	return false;
}

static bool t2ClassDirectory3(const std::vector<uint8> &src, CDllDirectory *dll, std::string &info)
{
	CClassDirectory3 cd(dll);
	{ CStorageStream ss(src); try { cd.serial(ss); } catch (std::exception &e) { info = std::string("read-throw: ") + e.what(); return false; } }
	try { cd.parse(VersionUnknown); cd.clean(); cd.build(VersionUnknown); cd.disown(); }
	catch (std::exception &e) { info = std::string("lifecycle-throw: ") + e.what(); return false; }
	std::vector<uint8> rt;
	try { rt = writeContainerToTemp(cd, g_tempPath); } catch (std::exception &e) { info = std::string("write-throw: ") + e.what(); return false; }
	if (rt == src) return true;
	info = diffSummary(src, rt);
	return false;
}

static bool t2Scene(const std::vector<uint8> &src, CSceneClassRegistry *reg,
                    CDllDirectory *dll, CClassDirectory3 *cd, std::string &info)
{
	CScene scene(reg, dll, cd);
	{ CStorageStream ss(src); try { scene.serial(ss); } catch (std::exception &e) { info = std::string("read-throw: ") + e.what(); return false; } }
	try { scene.parse(VersionUnknown); scene.clean(); scene.build(VersionUnknown); scene.disown(); }
	catch (std::exception &e) { info = std::string("lifecycle-throw: ") + e.what(); return false; }
	std::vector<uint8> rt;
	try { rt = writeContainerToTemp(scene, g_tempPath); } catch (std::exception &e) { info = std::string("write-throw: ") + e.what(); return false; }
	if (rt == src) return true;
	info = diffSummary(src, rt);
	return false;
}

// Parse a typed container out of a named stream's bytes. Returns false on absence or a read throw.
static bool loadContainer(const CStorageOleIn &in, const char *name, CStorageContainer &c)
{
	std::vector<uint8> b;
	if (!in.readStream(name, b)) return false;
	CStorageStream ss(b);
	c.serial(ss);
	return true;
}

// End-to-end parse&modify&save proof: load a .max, change one ParamBlock2 parameter through the
// typed CParamBlock2 modify API, write the whole .max back (Scene rebuilt from the typed scene
// graph, every other stream copied verbatim, OLE class id preserved), reload it, and verify (a)
// the modified parameter reads back the new value, (b) every non-Scene stream is byte-identical
// to the original, and (c) the Scene stream differs from the original ONLY in the modified
// parameter's payload bytes (a surgical, byte-localized edit — nothing else moved). This is the
// "programmatically adjust existing .max files" capability the material editor is built on.
// appDataMode: instead of a ParamBlock2 parameter, modify a script AppData entry through the
// typed CAppData::setScriptString (same-length one-byte toggle so the surgical byte-locality
// assertions below apply unchanged) — the end-to-end proof of programmatic export-flag editing.
static int modifySaveTest(CStorageOleIn &in, CSceneClassRegistry *reg, const std::string &tempMax, bool verbose, bool appDataMode)
{
	// All chunk streams read verbatim for byte-exact write-back of the unmodified ones + the
	// original-vs-rewritten comparison.
	static const char *kStreams[] = {
		"VideoPostQueue", "Config", "ClassData", "DllDirectory", "ClassDirectory3", "Scene",
		"\05SummaryInformation", "\05DocumentSummaryInformation", nullptr
	};
	std::vector<std::string> present;
	std::vector<std::vector<uint8> > rawOrig;
	for (const char **n = kStreams; *n; ++n)
	{
		std::vector<uint8> b;
		if (in.readStream(*n, b)) { present.push_back(*n); rawOrig.push_back(b); }
	}
	uint8 classId[16];
	bool haveClassId = in.getClassId(classId);

	// Typed load for the modification (dll/cd needed to resolve the scene class graph).
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cout << "SKIP modify-save: no DllDirectory\n"; return 0; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 1; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cout << "SKIP modify-save: no ClassDirectory3\n"; return 0; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 1; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cout << "SKIP modify-save: no Scene\n"; return 0; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 1; }
	}

	// Find a fixed-size scalar/color parameter to modify, and remember how to find it again.
	CSceneClassContainer *ssc = scene.container();
	sint32 targetIndex = -1;
	uint16 targetParam = 0;
	int targetKind = 0; // 1 float, 2 int, 3 bool, 4 color; appDataMode: 5 script string
	float newF = 1234.5f; sint32 newI = 0x5eed1234; float newC[3] = { 0.125f, 0.5f, 0.875f };
	uint32 targetSubId = 0; std::string newS;
	if (appDataMode)
	{
		// First script AppData entry with a non-empty string value; toggle its first byte with
		// ^0x01 (same length, and decimal-string digits stay digits: '0'<->'1', '2'<->'3', ...).
		sint32 idx = 0;
		for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end() && targetIndex < 0; ++it, ++idx)
		{
			BUILTIN::CAnimatable *anim = dynamic_cast<BUILTIN::CAnimatable *>(it->second);
			if (!anim) continue;
			BUILTIN::STORAGE::CAppData *ad = anim->existingAppData();
			if (!ad) continue;
			for (BUILTIN::STORAGE::CAppData::TMap::const_iterator eit = ad->entries().begin(); eit != ad->entries().end(); ++eit)
			{
				if (eit->first.ClassId != BUILTIN::STORAGE::CAppData::ScriptClassId
					|| eit->first.SuperClassId != BUILTIN::STORAGE::CAppData::ScriptSuperClassId) continue;
				std::string cur;
				if (!ad->getScriptString(eit->first.SubId, cur) || cur.empty()) continue;
				newS = cur;
				newS[0] = (char)(newS[0] ^ 0x01);
				if (!ad->setScriptString(eit->first.SubId, newS)) continue;
				targetIndex = idx;
				targetSubId = eit->first.SubId;
				targetKind = 5;
				break;
			}
		}
		if (targetIndex < 0)
		{
			std::cout << "SKIP appdata-modify-save: no non-empty script AppData entry found\n";
			return 0;
		}
	}
	else
	{
		sint32 idx = 0;
		for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end() && targetIndex < 0; ++it, ++idx)
		{
			BUILTIN::CParamBlock2 *pb2 = dynamic_cast<BUILTIN::CParamBlock2 *>(it->second);
			if (!pb2) continue;
			const std::vector<BUILTIN::CParamBlock2::SParam> &ps = pb2->params();
			for (uint p = 0; p < ps.size(); ++p)
			{
				const BUILTIN::CParamBlock2::SParam &sp = ps[p];
				if (sp.IsTab || !sp.HasConstant) continue;
				int kind = 0;
				switch (sp.baseType())
				{
				case BUILTIN::CParamBlock2::TYPE_FLOAT: case BUILTIN::CParamBlock2::TYPE_ANGLE:
				case BUILTIN::CParamBlock2::TYPE_PCNT_FRAC: case BUILTIN::CParamBlock2::TYPE_WORLD:
				case BUILTIN::CParamBlock2::TYPE_COLOR_CHANNEL: kind = 1; break;
				case BUILTIN::CParamBlock2::TYPE_INT: case BUILTIN::CParamBlock2::TYPE_TIMEVALUE:
				case BUILTIN::CParamBlock2::TYPE_RADIOBTN_INDEX: kind = 2; break;
				case BUILTIN::CParamBlock2::TYPE_BOOL: kind = 3; break;
				case BUILTIN::CParamBlock2::TYPE_RGBA: case BUILTIN::CParamBlock2::TYPE_POINT3:
				case BUILTIN::CParamBlock2::TYPE_HSV: kind = 4; break;
				default: kind = 0; break;
				}
				if (!kind) continue;
				// Read the current value and modify through the SAME first-by-id record the get*/
				// set* API keys on (avoids duplicate-id ambiguity), choosing a new value that
				// differs from the current one so the edit is observable.
				bool ok = false;
				if (kind == 1)
				{
					float cur = 0; if (!pb2->getFloat(sp.Id, cur)) continue;
					newF = (cur == 1234.5f) ? 6789.0f : 1234.5f;
					ok = pb2->setFloat(sp.Id, newF);
				}
				else if (kind == 2)
				{
					sint32 cur = 0; if (!pb2->getInt(sp.Id, cur)) continue;
					newI = cur ^ 0x00010001;
					ok = pb2->setInt(sp.Id, newI);
				}
				else if (kind == 3)
				{
					sint32 cur = 0; if (!pb2->getInt(sp.Id, cur)) continue;
					newI = cur ? 0 : 1;
					ok = pb2->setBool(sp.Id, newI != 0);
				}
				else
				{
					float cur[3] = { 0, 0, 0 }; if (!pb2->getColor(sp.Id, cur)) continue;
					newC[0] = cur[0] + 0.25f; newC[1] = cur[1]; newC[2] = cur[2];
					ok = pb2->setColor(sp.Id, newC);
				}
				if (ok) { targetIndex = idx; targetParam = sp.Id; targetKind = kind; break; }
			}
		}
	}
	if (targetIndex < 0)
	{
		std::cout << "SKIP modify-save: no modifiable ParamBlock2 parameter found\n";
		return 0;
	}
	if (verbose && !appDataMode)
	{
		// In-memory read-back right after the modify (isolates modify vs. serialize).
		BUILTIN::CParamBlock2 *pb2 = dynamic_cast<BUILTIN::CParamBlock2 *>(ssc->getByStorageIndex((uint32)targetIndex));
		if (pb2)
		{
			if (targetKind == 1) { float v = 0; pb2->getFloat(targetParam, v); std::cerr << "  in-mem float " << v << " (want " << newF << ")\n"; }
			else if (targetKind == 2 || targetKind == 3) { sint32 v = 0; pb2->getInt(targetParam, v); std::cerr << "  in-mem int " << v << " (want " << newI << ")\n"; }
			else { float c[3] = { 0, 0, 0 }; pb2->getColor(targetParam, c); std::cerr << "  in-mem color " << c[0] << "," << c[1] << "," << c[2] << "\n"; }
		}
	}

	// Rebuild the scene stream from the (now modified) typed graph.
	try { scene.clean(); scene.build(VersionUnknown); scene.disown(); }
	catch (std::exception &e) { std::cerr << "scene build: " << e.what() << "\n"; return 1; }
	std::vector<uint8> newScene;
	try { newScene = writeContainerToTemp(scene, g_tempPath); }
	catch (std::exception &e) { std::cerr << "scene write: " << e.what() << "\n"; return 1; }

	// Write the whole .max back: modified Scene from the graph, every other stream verbatim.
	{
		CStorageOleOut out;
		for (size_t i = 0; i < present.size(); ++i)
		{
			if (present[i] == "Scene") out.addStream("Scene", newScene);
			else out.addStream(present[i], rawOrig[i]);
		}
		if (haveClassId) out.setClassId(classId);
		if (!out.write(tempMax)) { std::cerr << "cannot create " << tempMax << "\n"; return 1; }
	}

	// Reload and verify the modification + the surgical byte-locality of the edit.
	int fails = 0;
	CStorageOleIn in2;
	if (!in2.open(tempMax)) { std::cerr << "cannot reopen rewritten .max\n"; return 1; }

	// (a) modified parameter reads back the new value
	{
		CDllDirectory dll2;
		CClassDirectory3 cd2(&dll2);
		CScene scene2(reg, &dll2, &cd2);
		bool ok = true;
		{ std::vector<uint8> b; if (in2.readStream("DllDirectory", b)) { CStorageStream ss(b); try { dll2.serial(ss); dll2.parse(VersionUnknown); } catch (...) { ok = false; } } else ok = false; }
		if (ok) { std::vector<uint8> b; if (in2.readStream("ClassDirectory3", b)) { CStorageStream ss(b); try { cd2.serial(ss); cd2.parse(VersionUnknown); } catch (...) { ok = false; } } else ok = false; }
		if (ok) { std::vector<uint8> b; if (in2.readStream("Scene", b)) { CStorageStream ss(b); try { scene2.serial(ss); scene2.parse(VersionUnknown); } catch (...) { ok = false; } } else ok = false; }
		if (!ok) { std::cerr << "reload parse failed\n"; ++fails; }
		else if (appDataMode)
		{
			BUILTIN::CAnimatable *anim = dynamic_cast<BUILTIN::CAnimatable *>(scene2.container()->getByStorageIndex((uint32)targetIndex));
			BUILTIN::STORAGE::CAppData *ad = anim ? anim->existingAppData() : NULL;
			std::string back;
			bool good = ad && ad->getScriptString(targetSubId, back) && back == newS;
			if (!good)
			{
				if (verbose) std::cerr << "  got script '" << back << "' want '" << newS << "'\n";
				std::cerr << "modified script AppData entry did not read back the new value\n"; ++fails;
			}
		}
		else
		{
			BUILTIN::CParamBlock2 *pb2 = dynamic_cast<BUILTIN::CParamBlock2 *>(scene2.container()->getByStorageIndex((uint32)targetIndex));
			bool good = false;
			if (pb2)
			{
				if (targetKind == 1) { float v = 0; good = pb2->getFloat(targetParam, v) && v == newF; if (!good && verbose) std::cerr << "  got float " << v << " want " << newF << "\n"; }
				else if (targetKind == 2 || targetKind == 3) { sint32 v = 0; good = pb2->getInt(targetParam, v) && v == newI; if (!good && verbose) std::cerr << "  got int " << v << " want " << newI << "\n"; }
				else { float c[3] = { 0, 0, 0 }; good = pb2->getColor(targetParam, c) && c[0] == newC[0] && c[1] == newC[1] && c[2] == newC[2]; if (!good && verbose) std::cerr << "  got color " << c[0] << "," << c[1] << "," << c[2] << " want " << newC[0] << "," << newC[1] << "," << newC[2] << "\n"; }
			}
			else if (verbose) std::cerr << "  reloaded storage " << targetIndex << " is not a ParamBlock2\n";
			if (!good) { std::cerr << "modified parameter did not read back the new value\n"; ++fails; }
		}
	}

	// (b) every non-Scene stream byte-identical; (c) Scene differs only in a small localized run
	for (size_t i = 0; i < present.size(); ++i)
	{
		std::vector<uint8> b2;
		in2.readStream(present[i], b2);
		if (present[i] != "Scene")
		{
			if (b2 != rawOrig[i]) { std::cerr << "stream " << present[i] << " changed unexpectedly\n"; ++fails; }
		}
		else
		{
			if (b2.size() != rawOrig[i].size()) { std::cerr << "Scene size changed " << rawOrig[i].size() << " -> " << b2.size() << "\n"; ++fails; }
			else
			{
				size_t nDiff = 0, first = b2.size(), last = 0;
				for (size_t k = 0; k < b2.size(); ++k)
					if (b2[k] != rawOrig[i][k]) { ++nDiff; if (k < first) first = k; last = k; }
				size_t span = nDiff ? (last - first + 1) : 0;
				if (nDiff == 0) { std::cerr << "Scene did not change (edit lost)\n"; ++fails; }
				else if (span > 12) { std::cerr << "Scene edit not localized: " << nDiff << " bytes over span " << span << "\n"; ++fails; }
				else if (verbose) std::cerr << "  Scene edit: " << nDiff << " bytes at 0x" << std::hex << first << std::dec << " (span " << span << ")\n";
			}
		}
	}

	if (appDataMode)
		std::cout << (fails ? "FAIL" : "OK") << " appdata-modify-save: subId " << targetSubId
		          << " @storage " << targetIndex << ", " << fails << " fail\n";
	else
		std::cout << (fails ? "FAIL" : "OK") << " modify-save: param 0x" << std::hex << targetParam << std::dec
		          << " kind " << targetKind << " @storage " << targetIndex << ", " << fails << " fail\n";
	return fails ? 1 : 0;
}

// Parse the Scene stream fully and run the CParamBlock2 write-direction self-check on every
// ParamBlock2 object.
static int pb2SelfTest(CStorageOleIn &in, CSceneClassRegistry *reg, bool verbose)
{
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cerr << "no DllDirectory\n"; return 2; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cerr << "no ClassDirectory3\n"; return 2; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cerr << "no Scene\n"; return 2; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 2; }
	}
	uint nPb2 = 0, nParams = 0, nFail = 0;
	uint nAnimFloat = 0, nAnimFloatResolved = 0;
	uint animTypeHist[32] = { 0 };
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		BUILTIN::CParamBlock2 *pb2 = dynamic_cast<BUILTIN::CParamBlock2 *>(it->second);
		if (!pb2) continue;
		++nPb2;
		nParams += (uint)pb2->params().size();
		std::string err;
		if (!pb2->selfTestReencode(err))
		{
			++nFail;
			std::cerr << "  PB2 selftest FAIL (block " << pb2->blockId() << "): " << err << "\n";
		}
		const std::vector<BUILTIN::CParamBlock2::SParam> &ps = pb2->params();
		for (uint i = 0; i < ps.size(); ++i)
		{
			if (ps[i].IsTab || ps[i].HasConstant || !ps[i].RefBacked) continue;
			uint16 bt = ps[i].baseType();
			if (BUILTIN::CParamBlock2::SParam::typeIsRefKind(ps[i].Type)) continue;
			animTypeHist[bt & 0x1f]++;
			if (bt == BUILTIN::CParamBlock2::TYPE_FLOAT)
			{
				++nAnimFloat;
				float v;
				if (pb2->getFloatAt0(ps[i].Id, v)) ++nAnimFloatResolved;
			}
		}
	}
	std::cout << (nFail ? "FAIL" : "OK") << " pb2-selftest: " << nPb2 << " blocks, " << nParams
	          << " params, " << nFail << " fail; anim-float " << nAnimFloatResolved << "/" << nAnimFloat
	          << " resolved at t=0";
	if (verbose)
	{
		std::cout << "; anim-by-type[";
		for (uint t = 0; t < 32; ++t) if (animTypeHist[t]) std::cout << " " << t << ":" << animTypeHist[t];
		std::cout << " ]";
	}
	std::cout << "\n";
	if (verbose && !nFail)
		std::cerr << "  (all " << nPb2 << " ParamBlock2 objects re-encode byte-exact)\n";
	return nFail ? 1 : 0;
}

// Parse the Scene stream fully and run the CParamBlock (old-style ParamBlock, superclass 0x8)
// write-direction self-check on every ParamBlock object: re-encode each decoded constant from
// its typed value and compare against the stored bytes. Also surfaces any entry-leaf chunk ids
// the typed decode did not recognize (must stay empty across the corpus) and counts animated
// (controller-backed) parameters and their t=0 resolution rate.
static int oldPbSelfTest(CStorageOleIn &in, CSceneClassRegistry *reg, bool verbose)
{
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cerr << "no DllDirectory\n"; return 2; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cerr << "no ClassDirectory3\n"; return 2; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cerr << "no Scene\n"; return 2; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 2; }
	}
	uint nPb = 0, nParams = 0, nFail = 0, nUnknownIds = 0;
	uint nAnim = 0, nAnimResolved = 0;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		BUILTIN::CParamBlock *pb = dynamic_cast<BUILTIN::CParamBlock *>(it->second);
		if (!pb) continue;
		++nPb;
		nParams += (uint)pb->params().size();
		std::string err;
		if (!pb->selfTestReencode(err))
		{
			++nFail;
			std::cerr << "  oldpb selftest FAIL: " << err << "\n";
		}
		if (!pb->unknownEntryChunkIds().empty())
		{
			nUnknownIds += (uint)pb->unknownEntryChunkIds().size();
			for (uint u = 0; u < pb->unknownEntryChunkIds().size(); ++u)
				std::cerr << "  oldpb UNKNOWN entry chunk id 0x" << std::hex
				          << pb->unknownEntryChunkIds()[u] << std::dec << "\n";
		}
		const std::vector<BUILTIN::CParamBlock::SParam> &ps = pb->params();
		for (uint i = 0; i < ps.size(); ++i)
		{
			if (!ps[i].Animated) continue;
			++nAnim;
			float v;
			if (pb->getFloatAt0(ps[i].Index, v)) ++nAnimResolved;
			if (verbose)
			{
				CSceneClass *ctrl = dynamic_cast<CSceneClass *>(pb->controllerForParam(ps[i].Index));
				std::cerr << "  oldpb anim idx " << ps[i].Index << " marker 0x" << std::hex
				          << ps[i].ValueChunkId << std::dec << " slot " << ps[i].RefSlot << " -> "
				          << (ctrl ? ctrl->classDesc()->classId().toString() : std::string("<null>"))
				          << " sc=0x" << std::hex << (ctrl ? ctrl->classDesc()->superClassId() : 0) << std::dec
				          << " '" << (ctrl ? ucstring(ctrl->classDesc()->displayName()).toUtf8() : std::string()) << "'\n";
			}
		}
	}
	std::cout << ((nFail || nUnknownIds) ? "FAIL" : "OK") << " oldpb-selftest: " << nPb << " blocks, "
	          << nParams << " params, " << nFail << " fail, " << nUnknownIds << " unknown-ids; anim "
	          << nAnimResolved << "/" << nAnim << " resolved at t=0\n";
	if (verbose && !nFail && !nUnknownIds)
		std::cerr << "  (all " << nPb << " ParamBlock objects re-encode byte-exact)\n";
	return (nFail || nUnknownIds) ? 1 : 0;
}

// Parse the Scene stream fully and run the CShapeObject (Shape superclass 0x40) decode
// consistency check on every shape object: re-encode every decoded knot record from its typed
// fields and compare against the stored 0x290a payload, verify every Spline3D container is
// structurally canonical, and surface any BezierShape/Spline3D sibling chunk ids the typed
// decode did not recognize (must stay empty across the corpus).
static int shapeSelfTest(CStorageOleIn &in, CSceneClassRegistry *reg, bool verbose)
{
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cerr << "no DllDirectory\n"; return 2; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cerr << "no ClassDirectory3\n"; return 2; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cerr << "no Scene\n"; return 2; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 2; }
	}
	uint nObj = 0, nBez = 0, nSplines = 0, nClosed = 0, nKnots = 0, nSteps = 0;
	uint nFail = 0, nUnknownIds = 0;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		BUILTIN::CShapeObject *so = dynamic_cast<BUILTIN::CShapeObject *>(it->second);
		if (!so) continue;
		++nObj;
		nBez += so->numBezierShapes();
		if (so->hasSteps()) ++nSteps;
		const std::vector<BUILTIN::CShapeObject::SSpline> &splines = so->splines();
		nSplines += (uint)splines.size();
		for (uint s = 0; s < splines.size(); ++s)
		{
			nKnots += (uint)splines[s].Knots.size();
			if (splines[s].closed()) ++nClosed;
		}
		std::string err;
		if (!so->selfTestReencode(err))
		{
			++nFail;
			std::cerr << "  shape selftest FAIL (" << so->classDesc()->classId().toString() << "): " << err << "\n";
		}
		if (!so->unknownSiblingIds().empty())
		{
			nUnknownIds += (uint)so->unknownSiblingIds().size();
			for (uint u = 0; u < so->unknownSiblingIds().size(); ++u)
				std::cerr << "  shape UNKNOWN sibling chunk id 0x" << std::hex
				          << so->unknownSiblingIds()[u] << std::dec << "\n";
		}
		if (verbose)
			std::cerr << "  shape " << so->classDesc()->classId().toString() << " '"
			          << ucstring(so->classDesc()->displayName()).toUtf8() << "': "
			          << so->numBezierShapes() << " beziershapes, " << splines.size()
			          << " splines, steps " << (so->hasSteps() ? NLMISC::toString(so->steps()) : std::string("-")) << "\n";
	}
	std::cout << ((nFail || nUnknownIds) ? "FAIL" : "OK") << " shape-selftest: " << nObj << " objects, "
	          << nBez << " beziershapes, " << nSplines << " splines (" << nClosed << " closed), "
	          << nKnots << " knots, " << nSteps << " with-steps, " << nFail << " fail, "
	          << nUnknownIds << " unknown-ids\n";
	return (nFail || nUnknownIds) ? 1 : 0;
}

// Parse the Scene stream fully and verify the CDerivedObject typed slot model on every OSM/WSM
// Derived wrapper: slot/mod-app count parity, contiguous 0x2500 run, single base at the last
// reference slot, canonical mod-app children (0x2510[52]/0x2511[24]/0x2512/0x2513[4]), no
// unknown orphan ids. Absent 0x2510/0x2512 are the norm (not misses); any structural anomaly
// dumps the file path.
static int derivedSelfTest(const char *maxFile, CStorageOleIn &in, CSceneClassRegistry *reg, bool verbose)
{
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cerr << "no DllDirectory\n"; return 2; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cerr << "no ClassDirectory3\n"; return 2; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cerr << "no Scene\n"; return 2; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 2; }
	}
	uint nOsm = 0, nWsm = 0, nSlots = 0, nApps = 0, nTM = 0, nData = 0, nBase = 0, nZeroMod = 0;
	uint nAnomaly = 0;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		BUILTIN::CDerivedObject *d = dynamic_cast<BUILTIN::CDerivedObject *>(it->second);
		if (!d) continue;
		if (dynamic_cast<BUILTIN::CWSMDerivedObject *>(it->second)) ++nWsm; else ++nOsm;
		nSlots += d->modifierCount();
		if (!d->modifierCount()) ++nZeroMod;
		if (d->hasBase()) ++nBase;
		for (uint i = 0; i < d->modifierCount(); ++i)
		{
			if (d->modApp(i)) ++nApps;
			float tm[12];
			if (d->modContextTM(i, tm)) ++nTM;
			if (d->localModData(i)) ++nData;
		}
		std::string err;
		if (!d->selfTest(err))
		{
			++nAnomaly;
			std::cerr << "  derived selftest ANOMALY (" << maxFile << ", "
			          << d->classDesc()->internalName() << "): " << err << "\n";
		}
		else if (verbose)
		{
			std::cerr << "  derived " << d->classDesc()->internalName() << ": "
			          << d->modifierCount() << " modifiers, base "
			          << (d->baseObject() ? d->baseObject()->classDesc()->classId().toString() : std::string("<none>")) << "\n";
		}
	}
	std::cout << (nAnomaly ? "FAIL" : "OK") << " derived-selftest: " << nOsm << " osm, " << nWsm
	          << " wsm, " << nSlots << " slots, " << nApps << " modapps, " << nTM << " tms, "
	          << nData << " localdata, " << nBase << " base, " << nZeroMod << " zero-mod, "
	          << nAnomaly << " anomaly\n";
	return nAnomaly ? 1 : 0;
}

// Per-chunk-id inventory accumulator for the mesh-delta / mapext selftests (§12.2: corpus
// inventory rides the same sweep as the validation). Keyed on (level, id); prints one
// machine-readable line per entry for the sweep driver to aggregate corpus-wide.
struct SChunkInvEntry
{
	uint Count;
	uint32 MinSize, MaxSize;
	uint Containers;
	SChunkInvEntry() : Count(0), MinSize(0xFFFFFFFFu), MaxSize(0), Containers(0) { }
};
typedef std::map<std::pair<std::string, uint16>, SChunkInvEntry> TChunkInv;

static void invAdd(TChunkInv &inv, const std::string &level, uint16 id, uint32 size, bool container)
{
	SChunkInvEntry &e = inv[std::make_pair(level, id)];
	++e.Count;
	if (size < e.MinSize) e.MinSize = size;
	if (size > e.MaxSize) e.MaxSize = size;
	if (container) ++e.Containers;
}

static void invPrint(const TChunkInv &inv, const char *tag)
{
	for (TChunkInv::const_iterator it = inv.begin(); it != inv.end(); ++it)
	{
		std::cout << tag << " level=" << it->first.first << " id=0x" << std::hex << it->first.second
		          << std::dec << " n=" << it->second.Count << " minsz=" << it->second.MinSize
		          << " maxsz=" << it->second.MaxSize << " cont=" << it->second.Containers << "\n";
	}
}

// Parse the Scene stream fully, and on every Edit Mesh modifier slot (ClassId (0x50, 0),
// superclass 0x810) of every OSM/WSM Derived wrapper decode the 0x2512 MeshDelta payload
// through the typed CMeshDelta overlay codec and verify the decode re-encodes every typed
// chunk bit-exactly (uninitialized 0x0210 corner words included). Prints the chunk-id/size
// inventory (MDINV lines) for the corpus histogram, delete-bitmap shape stats (MDBITS line),
// and a one-line summary. Unknown/irregular ids are structural findings (expected 0).
static int meshDeltaSelfTest(const char *maxFile, CStorageOleIn &in, CSceneClassRegistry *reg, bool verbose)
{
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cerr << "no DllDirectory\n"; return 2; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cerr << "no ClassDirectory3\n"; return 2; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cerr << "no Scene\n"; return 2; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 2; }
	}
	static const NLMISC::CClassId editMeshClassId(0x00000050, 0x00000000);
	uint nApps = 0, nNoData = 0, nNoDelta = 0, nDecoded = 0;
	uint nMoves = 0, nCVerts = 0, nCFaces = 0, nRemap = 0, nAttribs = 0, nDelV = 0, nDelF = 0;
	uint nFail = 0, nUnknown = 0, nIrregular = 0, nExtraDelta = 0;
	uint nBitsDword = 0, nBitsByte = 0, nBitsOther = 0, nBitsPadNonZero = 0;
	TChunkInv inv;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		BUILTIN::CDerivedObject *d = dynamic_cast<BUILTIN::CDerivedObject *>(it->second);
		if (!d) continue;
		for (uint i = 0; i < d->modifierCount(); ++i)
		{
			CSceneClass *mod = d->modifier(i);
			if (!mod || mod->classDesc()->classId() != editMeshClassId
				|| mod->classDesc()->superClassId() != 0x810) continue;
			++nApps;
			IStorageObject *lmd = d->localModData(i);
			if (!lmd) { ++nNoData; continue; }
			BUILTIN::STORAGE::CMeshDelta md;
			if (!md.decode(lmd)) { ++nNoDelta; continue; }
			++nDecoded;
			nMoves += (uint)md.moves().size();
			nCVerts += (uint)md.createdVerts().size();
			nCFaces += (uint)md.createdFaces().size();
			nRemap += (uint)md.faceRemap().size();
			nAttribs += (uint)md.faceAttribs().size();
			if (md.delVerts().Present) ++nDelV;
			if (md.delFaces().Present) ++nDelF;
			nExtraDelta += md.extraDeltaContainers();
			for (uint u = 0; u < md.unknownLocalDataIds().size(); ++u)
			{
				++nUnknown;
				std::cerr << "  meshdelta UNKNOWN 2512-level chunk id 0x" << std::hex
				          << md.unknownLocalDataIds()[u] << std::dec << " (" << maxFile << ")\n";
			}
			for (uint u = 0; u < md.unknownDeltaIds().size(); ++u)
			{
				++nUnknown;
				std::cerr << "  meshdelta UNKNOWN 4000-level chunk id 0x" << std::hex
				          << md.unknownDeltaIds()[u] << std::dec << " (" << maxFile << ")\n";
			}
			for (uint u = 0; u < md.irregularIds().size(); ++u)
			{
				++nIrregular;
				std::cerr << "  meshdelta IRREGULAR chunk id 0x" << std::hex
				          << md.irregularIds()[u] << std::dec << " (" << maxFile << ")\n";
			}
			std::string err;
			if (!md.selfTestReencode(err))
			{
				++nFail;
				std::cerr << "  meshdelta selftest FAIL (" << maxFile << "): " << err << "\n";
			}
			for (uint c = 0; c < md.localDataChildren().size(); ++c)
				invAdd(inv, "2512", md.localDataChildren()[c].Id, md.localDataChildren()[c].Size,
				       md.localDataChildren()[c].Container);
			for (uint c = 0; c < md.deltaChildren().size(); ++c)
				invAdd(inv, "4000", md.deltaChildren()[c].Id, md.deltaChildren()[c].Size,
				       md.deltaChildren()[c].Container);
			// Descend one level for the inventory: the delete/selection bitmap containers and
			// the recognized-untyped 0x0340/0x0430 containers (their children land at level
			// "sub<id>"), plus per-instance MDREC lines for the recognized-untyped record-table
			// leaves (size + leading dword — the sweep driver tests the count-prefix stride
			// hypotheses corpus-wide without typing them).
			{
				CStorageContainer *c2512 = dynamic_cast<CStorageContainer *>(lmd);
				CStorageContainer *c4000 = NULL;
				if (c2512)
					for (CStorageContainer::TStorageObjectConstIt jt = c2512->chunks().begin(); jt != c2512->chunks().end() && !c4000; ++jt)
						if (jt->first == 0x4000) c4000 = dynamic_cast<CStorageContainer *>(jt->second);
				if (c4000)
				{
					for (CStorageContainer::TStorageObjectConstIt jt = c4000->chunks().begin(); jt != c4000->chunks().end(); ++jt)
					{
						uint16 id = jt->first;
						if (id == 0x0170 || id == 0x0270 || id == 0x0340
							|| id == 0x0400 || id == 0x0410 || id == 0x0420 || id == 0x0430)
						{
							CStorageContainer *sub = dynamic_cast<CStorageContainer *>(jt->second);
							if (!sub) continue;
							char lvl[16];
							snprintf(lvl, sizeof(lvl), "sub%03x", id);
							for (CStorageContainer::TStorageObjectConstIt kt = sub->chunks().begin(); kt != sub->chunks().end(); ++kt)
							{
								CStorageRaw *raw = dynamic_cast<CStorageRaw *>(kt->second);
								invAdd(inv, lvl, kt->first, raw ? (uint32)raw->Value.size() : 0,
								       kt->second && kt->second->isContainer());
							}
						}
						else if (id == 0x0230 || id == 0x0330 || id == 0x0334 || id == 0x0338
							|| id == 0x033b || id == 0x0360 || id == 0x0120 || id == 0x0200)
						{
							CStorageRaw *raw = dynamic_cast<CStorageRaw *>(jt->second);
							if (!raw) continue;
							uint32 head = 0;
							if (raw->Value.size() >= 4) memcpy(&head, nlVectorData(raw->Value), 4);
							std::cout << "MDREC id=0x" << std::hex << id << std::dec
							          << " size=" << raw->Value.size() << " head=" << head << "\n";
						}
					}
				}
			}
			// Delete-bitmap shape stats: packed payload size vs the dword-padded / byte-padded
			// rule, and whether any pad bit beyond BitCount is set (uninit-tail witness).
			const BUILTIN::STORAGE::CMeshDelta::SBitArray *bas[2] = { &md.delVerts(), &md.delFaces() };
			for (uint b = 0; b < 2; ++b)
			{
				if (!bas[b]->Present) continue;
				size_t nBytes = ((size_t)bas[b]->BitCount + 7) / 8;
				size_t nDwordBytes = (((size_t)bas[b]->BitCount + 31) / 32) * 4;
				if (bas[b]->Packed.size() == nDwordBytes) ++nBitsDword;
				else if (bas[b]->Packed.size() == nBytes) ++nBitsByte;
				else ++nBitsOther;
				bool padNonZero = false;
				for (size_t k = bas[b]->BitCount; k < bas[b]->Packed.size() * 8; ++k)
					if ((bas[b]->Packed[k / 8] >> (k % 8)) & 1) { padNonZero = true; break; }
				if (padNonZero) ++nBitsPadNonZero;
			}
			if (verbose)
				std::cerr << "  meshdelta app " << nApps << ": " << md.moves().size() << " moves, "
				          << md.createdVerts().size() << " cverts, " << md.createdFaces().size()
				          << " cfaces, " << md.faceRemap().size() << " remap, "
				          << md.faceAttribs().size() << " attribs\n";
		}
	}
	invPrint(inv, "MDINV");
	if (nDelV || nDelF)
		std::cout << "MDBITS dword=" << nBitsDword << " byte=" << nBitsByte << " other=" << nBitsOther
		          << " padnonzero=" << nBitsPadNonZero << "\n";
	bool fail = nFail || nUnknown || nIrregular || nExtraDelta;
	std::cout << (fail ? "FAIL" : "OK") << " meshdelta-selftest: " << nApps << " apps, "
	          << nDecoded << " decoded, " << nNoData << " no-data, " << nNoDelta << " no-delta, "
	          << nMoves << " moves, " << nCVerts << " cverts, " << nCFaces << " cfaces, "
	          << nRemap << " remap, " << nAttribs << " attribs, " << nDelV << " delv, " << nDelF
	          << " delf, " << nFail << " fail, " << nUnknown << " unknown-ids, " << nIrregular
	          << " irregular, " << nExtraDelta << " extra-delta\n";
	return fail ? 1 : 0;
}

// Parse the Scene stream fully, and on every Map Extender modifier slot (ClassId
// (0x2ec82081, 0x045a6271), superclass 0x810) decode the 0x2512 cache through the typed
// CMapExtenderCache overlay codec (raw-leaf and container forms) and verify the functional
// chunks re-encode bit-exactly. Prints the chunk-id/size inventory (MXINV lines) and a
// one-line summary. Unknown ids are structural findings (expected 0).
static int mapExtSelfTest(const char *maxFile, CStorageOleIn &in, CSceneClassRegistry *reg, bool verbose)
{
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cerr << "no DllDirectory\n"; return 2; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cerr << "no ClassDirectory3\n"; return 2; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cerr << "no Scene\n"; return 2; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 2; }
	}
	uint nApps = 0, nNoData = 0, nEmpty = 0, nDecoded = 0, nDecodeFail = 0, nLeaf = 0, nContainer = 0;
	uint64 nVertsTotal = 0, nFacesTotal = 0;
	uint nChan1 = 0, nChan2 = 0, nChanOther = 0, nBadCorners = 0;
	uint nFail = 0, nUnknown = 0;
	TChunkInv inv;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		BUILTIN::CDerivedObject *d = dynamic_cast<BUILTIN::CDerivedObject *>(it->second);
		if (!d) continue;
		for (uint i = 0; i < d->modifierCount(); ++i)
		{
			CSceneClass *mod = d->modifier(i);
			if (!mod || mod->classDesc()->classId() != BUILTIN::STORAGE::CMapExtenderCache::ModifierClassId
				|| mod->classDesc()->superClassId() != 0x810) continue;
			++nApps;
			IStorageObject *lmd = d->localModData(i);
			if (!lmd) { ++nNoData; continue; }
			BUILTIN::STORAGE::CMapExtenderCache mx;
			bool ok = mx.decode(lmd);
			for (uint c = 0; c < mx.children().size(); ++c)
				invAdd(inv, "2512", mx.children()[c].Id, mx.children()[c].Size, mx.children()[c].Container);
			for (uint u = 0; u < mx.unknownIds().size(); ++u)
			{
				++nUnknown;
				std::cerr << "  mapext UNKNOWN cache chunk id 0x" << std::hex
				          << mx.unknownIds()[u] << std::dec << " (" << maxFile << ")\n";
			}
			if (!ok)
			{
				if (mx.emptyLeaf())
				{
					// Corpus-witnessed "no cache saved" state (never-evaluated modifier) —
					// counted, not a failure.
					++nEmpty;
					continue;
				}
				++nDecodeFail;
				std::cerr << "  mapext decode FAIL (" << maxFile << "): " << mx.lastError() << "\n";
				continue;
			}
			++nDecoded;
			if (mx.leafForm()) ++nLeaf; else ++nContainer;
			nVertsTotal += mx.numVerts();
			nFacesTotal += mx.numFaces();
			if (mx.channel() == 1) ++nChan1;
			else if (mx.channel() == 2) ++nChan2;
			else ++nChanOther;
			if (!mx.faceCornersValid())
			{
				++nBadCorners;
				std::cerr << "  mapext face corner out of range (" << maxFile << ")\n";
			}
			std::string err;
			if (!mx.selfTestReencode(err))
			{
				++nFail;
				std::cerr << "  mapext selftest FAIL (" << maxFile << "): " << err << "\n";
			}
			if (verbose)
				std::cerr << "  mapext cache " << nApps << ": " << mx.numVerts() << " uvw verts, "
				          << mx.numFaces() << " faces, channel " << mx.channel()
				          << (mx.leafForm() ? " (leaf)" : " (container)") << "\n";
		}
	}
	invPrint(inv, "MXINV");
	bool fail = nFail || nUnknown || nDecodeFail || nBadCorners;
	std::cout << (fail ? "FAIL" : "OK") << " mapext-selftest: " << nApps << " apps, "
	          << nDecoded << " decoded (" << nLeaf << " leaf, " << nContainer << " container), "
	          << nNoData << " no-data, " << nEmpty << " empty, " << nDecodeFail << " decode-fail, " << nVertsTotal
	          << " uvwverts, " << nFacesTotal << " faces, chan1 " << nChan1 << ", chan2 " << nChan2
	          << ", chanother " << nChanOther << ", " << nBadCorners << " bad-corners, " << nFail
	          << " fail, " << nUnknown << " unknown-ids\n";
	return fail ? 1 : 0;
}

// --- Map-channel selftest helpers (dual raw/typed readers: the inventory sweep runs BEFORE the
// typed leaf serializers are enabled, the validation sweep after — same code reads both forms).

// Read a 4-byte uint32 leaf that may be raw (pre-typing) or CStorageValue<uint32> (typed).
// Returns false when the chunk is neither form or the raw size is not exactly 4.
static bool gbReadU32(IStorageObject *so, uint32 &out)
{
	if (CStorageValue<uint32> *v = dynamic_cast<CStorageValue<uint32> *>(so)) { out = v->Value; return true; }
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(so);
	if (!raw || raw->Value.size() != 4) return false;
	memcpy(&out, nlVectorData(raw->Value), 4);
	return true;
}

// Element count of a count-prefixed stride-12 array leaf (raw or typed CStorageArraySizePre
// of CVector / CGeomTriIndex). Returns false on any shape violation (stride remainder, stored
// count != byte-derived count, size < 4).
static bool gbReadArray12Count(IStorageObject *so, uint32 &count)
{
	if (CStorageArraySizePre<NLMISC::CVector> *tv = dynamic_cast<CStorageArraySizePre<NLMISC::CVector> *>(so))
	{ count = (uint32)tv->Value.size(); return true; }
	if (CStorageArraySizePre<BUILTIN::STORAGE::CGeomTriIndex> *tf = dynamic_cast<CStorageArraySizePre<BUILTIN::STORAGE::CGeomTriIndex> *>(so))
	{ count = (uint32)tf->Value.size(); return true; }
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(so);
	if (!raw || raw->Value.size() < 4) return false;
	uint32 stored = 0;
	memcpy(&stored, nlVectorData(raw->Value), 4);
	if ((raw->Value.size() - 4) % 12) return false;
	if (stored != (raw->Value.size() - 4) / 12) return false;
	count = stored;
	return true;
}

// Parse the Scene stream fully; on every GeomObject's GeomBuffers (0x08fe) container, inventory
// every chunk id (GBINV lines, bucketed by the object's concrete class) and validate the
// map-channel chunk family — 0x0959 channel index (4 B), 0x2398 support flag (4 B), 0x2394
// count-prefixed Point3 map verts, 0x2396 count-prefixed uint32-triple map faces (parallel to
// the 0x0912 mesh faces) — the grammar mesh_eval's extractEditableMesh reads: 0x0959 announces
// the channel for the following 0x2394/0x2396 pair, in file order. Prints per-container family
// sequence signatures (GBSEQ lines) and a one-line summary. Shape violations (bad sizes, stride
// remainders, count mismatches, a family chunk with the container bit, 0x2394/0x2396 without a
// preceding 0x0959) are structural findings (expected 0 — they gate). Face-count disparity vs
// 0x0912 is counted informationally (mesh_eval warns + drops those channels). Once the typed
// leaf serializers are enabled, the same sweep counts the typed forms (typed=... in the
// summary) — the byte-exactness of the typed write path is T2's job, not this test's.
static int mapChannelSelfTest(const char *maxFile, CStorageOleIn &in, CSceneClassRegistry *reg, bool verbose)
{
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cerr << "no DllDirectory\n"; return 2; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cerr << "no ClassDirectory3\n"; return 2; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cerr << "no Scene\n"; return 2; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 2; }
	}
	uint nObj = 0, nWithMaps = 0, nChannels = 0, nViolations = 0, nOrphanFamily = 0;
	uint nFaceCountMismatch = 0, nTyped = 0, nRawForm = 0, nViewMismatch = 0;
	uint64 nMapVerts = 0, nMapFaces = 0;
	sint32 chanMin = 0x7FFFFFFF, chanMax = -0x7FFFFFFF;
	uint n2398One = 0, n2398Other = 0;
	TChunkInv inv;
	std::map<std::string, uint> seqHist;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		BUILTIN::CGeomObject *geom = dynamic_cast<BUILTIN::CGeomObject *>(it->second);
		if (!geom) continue;
		BUILTIN::STORAGE::CGeomBuffers *gb = geom->geomBuffers();
		if (!gb) continue;
		++nObj;
		std::string cls = geom->classDesc()->internalName();
		// Inventory every direct child of the GeomBuffers container.
		for (CStorageContainer::TStorageObjectConstIt ct = gb->chunks().begin(); ct != gb->chunks().end(); ++ct)
		{
			uint32 size = 0xFFFFFFFFu;
			sint32 sz = 0;
			if (CStorageRaw *raw = dynamic_cast<CStorageRaw *>(ct->second)) size = (uint32)raw->Value.size();
			else if (ct->second->getSize(sz)) size = (uint32)sz;
			invAdd(inv, cls, ct->first, size, ct->second->isContainer());
		}
		// Map-channel family grammar walk, file order (the mesh_eval read).
		uint32 triFaces = 0;
		bool haveTriFaces = false;
		{
			IStorageObject *ff = gb->findStorageObject(0x0912);
			if (ff) haveTriFaces = gbReadArray12Count(ff, triFaces); // 20-byte stride — raw only
			// typed form: CStorageArraySizePre<CGeomTriIndexInfo>
			if (!haveTriFaces && ff)
			{
				if (CStorageArraySizePre<BUILTIN::STORAGE::CGeomTriIndexInfo> *tf
					= dynamic_cast<CStorageArraySizePre<BUILTIN::STORAGE::CGeomTriIndexInfo> *>(ff))
				{ triFaces = (uint32)tf->Value.size(); haveTriFaces = true; }
				else if (CStorageRaw *raw = dynamic_cast<CStorageRaw *>(ff))
				{
					if (raw->Value.size() >= 4)
					{
						uint32 stored = 0;
						memcpy(&stored, nlVectorData(raw->Value), 4);
						if (raw->Value.size() == 4 + (size_t)stored * 20) { triFaces = stored; haveTriFaces = true; }
					}
				}
			}
		}
		bool anyFamily = false;
		sint32 currentChannel = -1;
		uint32 curMapVerts = 0;
		bool haveCurMapVerts = false;
		std::string seq;
		for (CStorageContainer::TStorageObjectConstIt ct = gb->chunks().begin(); ct != gb->chunks().end(); ++ct)
		{
			uint16 id = ct->first;
			if (id != 0x0959 && id != 0x2398 && id != 0x2394 && id != 0x2396) continue;
			anyFamily = true;
			if (dynamic_cast<CStorageRaw *>(ct->second)) ++nRawForm; else ++nTyped;
			if (ct->second->isContainer())
			{
				++nViolations;
				std::cerr << "  mapchannel VIOLATION: family id 0x" << std::hex << id << std::dec
				          << " with container bit (" << maxFile << ")\n";
				continue;
			}
			if (id == 0x0959)
			{
				uint32 chan = 0;
				if (!gbReadU32(ct->second, chan))
				{
					++nViolations;
					std::cerr << "  mapchannel VIOLATION: 0x0959 not a 4-byte value (" << maxFile << ")\n";
					continue;
				}
				currentChannel = (sint32)chan;
				haveCurMapVerts = false;
				++nChannels;
				if (currentChannel < chanMin) chanMin = currentChannel;
				if (currentChannel > chanMax) chanMax = currentChannel;
				char buf[16]; snprintf(buf, sizeof(buf), "59(%d) ", (int)currentChannel); seq += buf;
			}
			else if (id == 0x2398)
			{
				uint32 v = 0;
				if (!gbReadU32(ct->second, v))
				{
					++nViolations;
					std::cerr << "  mapchannel VIOLATION: 0x2398 not a 4-byte value (" << maxFile << ")\n";
					continue;
				}
				if (v == 1) ++n2398One; else ++n2398Other;
				if (v == 1) seq += "98 ";
				else { char buf[16]; snprintf(buf, sizeof(buf), "98(%u) ", v); seq += buf; }
				if (currentChannel < 0) ++nOrphanFamily; // informational, see below
			}
			else // 0x2394 / 0x2396
			{
				uint32 count = 0;
				if (!gbReadArray12Count(ct->second, count))
				{
					++nViolations;
					std::cerr << "  mapchannel VIOLATION: 0x" << std::hex << id << std::dec
					          << " bad count-prefixed stride-12 shape (" << maxFile << ")\n";
					continue;
				}
				if (currentChannel < 0)
				{
					// Corpus-witnessed once (Max 3 snowballs PI_PO_tree_A.max): a '2398 2394 2396'
					// group with no leading 0x0959 announce. Informational — mesh_eval drops such
					// a channel by design (currentChannel < 0), and the typed leaves don't care.
					++nOrphanFamily;
					std::cerr << "  mapchannel ORPHAN: 0x" << std::hex << id << std::dec
					          << " without preceding 0x0959 (" << maxFile << ")\n";
				}
				if (id == 0x2394) { nMapVerts += count; curMapVerts = count; haveCurMapVerts = true; seq += "94 "; }
				else
				{
					nMapFaces += count;
					seq += "96 ";
					if (haveTriFaces && count != triFaces) ++nFaceCountMismatch;
					(void)curMapVerts; (void)haveCurMapVerts;
				}
			}
		}
		if (anyFamily)
		{
			++nWithMaps;
			if (!seq.empty() && seq[seq.size() - 1] == ' ') seq.resize(seq.size() - 1);
			++seqHist[cls + ": " + seq];
			if (verbose)
				std::cerr << "  mapchannel " << cls << " seq: " << seq << "\n";
		}
		// Cross-check the typed CGeomBuffers::mapChannels() view against the chunk walk (only
		// meaningful once the typed leaf serializers are enabled; raw-form chunks yield an empty
		// view and nTyped == 0 above).
		{
			std::vector<BUILTIN::STORAGE::CGeomBuffers::CMapChannelView> view;
			gb->mapChannels(view);
			uint vChannels = 0, walkChannels = 0;
			uint64 vVerts = 0, vFaces = 0;
			for (uint c = 0; c < view.size(); ++c)
			{
				if (view[c].Channel >= 0) ++vChannels;
				if (view[c].Verts) vVerts += view[c].Verts->size();
				if (view[c].Faces) vFaces += view[c].Faces->size();
			}
			// Recompute the walk's totals for THIS container to compare.
			uint64 wVerts = 0, wFaces = 0;
			for (CStorageContainer::TStorageObjectConstIt ct = gb->chunks().begin(); ct != gb->chunks().end(); ++ct)
			{
				uint32 count = 0;
				if (ct->first == 0x0959) ++walkChannels;
				else if (ct->first == 0x2394 && gbReadArray12Count(ct->second, count)) wVerts += count;
				else if (ct->first == 0x2396 && gbReadArray12Count(ct->second, count)) wFaces += count;
			}
			// A raw-form (untyped) container legitimately yields an empty view; only compare when
			// the view saw anything or the container is fully typed.
			bool anyTypedHere = !view.empty();
			if (anyTypedHere && (vChannels != walkChannels || vVerts != wVerts || vFaces != wFaces))
			{
				++nViewMismatch;
				std::cerr << "  mapchannel VIEW MISMATCH: channels " << vChannels << "/" << walkChannels
				          << " verts " << vVerts << "/" << wVerts << " faces " << vFaces << "/" << wFaces
				          << " (" << maxFile << ")\n";
			}
		}
	}
	// Where else do the family ids sit? Sweep every scene object's orphaned chunks for the four
	// ids (the RPO patch object claims its object-level Mesh-cache copies internally; anything
	// showing up HERE would be an unclaimed occurrence outside GeomBuffers).
	uint nOrphanLevelFamily = 0;
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CSceneClass *sc = dynamic_cast<CSceneClass *>(it->second);
		if (!sc) continue;
		for (CStorageContainer::TStorageObjectConstIt ot = sc->orphanedChunks().begin(); ot != sc->orphanedChunks().end(); ++ot)
		{
			if (ot->first == 0x0959 || ot->first == 0x2398 || ot->first == 0x2394 || ot->first == 0x2396)
			{
				++nOrphanLevelFamily;
				CStorageRaw *raw = dynamic_cast<CStorageRaw *>(ot->second);
				std::cout << "GBOBJLVL cls=" << sc->classDesc()->internalName() << " id=0x" << std::hex
				          << ot->first << std::dec << " size=" << (raw ? (uint32)raw->Value.size() : 0xFFFFFFFFu) << "\n";
			}
		}
	}
	invPrint(inv, "GBINV");
	for (std::map<std::string, uint>::const_iterator st = seqHist.begin(); st != seqHist.end(); ++st)
		std::cout << "GBSEQ n=" << st->second << " " << st->first << "\n";
	bool fail = nViolations || nViewMismatch;
	std::cout << (fail ? "FAIL" : "OK") << " mapchannel-selftest: " << nObj << " geom-objects, "
	          << nWithMaps << " with-maps, " << nChannels << " channels (min "
	          << (nChannels ? chanMin : 0) << " max " << (nChannels ? chanMax : 0) << "), "
	          << nMapVerts << " mapverts, " << nMapFaces << " mapfaces, 2398 one/other "
	          << n2398One << "/" << n2398Other << ", " << nFaceCountMismatch << " facecount-mismatch, "
	          << nTyped << " typed, " << nRawForm << " raw, " << nViolations << " violations, "
	          << nViewMismatch << " view-mismatch, "
	          << nOrphanFamily << " orphan-family, " << nOrphanLevelFamily << " objlevel\n";
	return fail ? 1 : 0;
}

// Parse the Scene stream fully and inventory every transform controller (superclass 0x9008):
// class-id histogram (CTCLS lines), own-chunk inventory bucketed per class (CTINV lines), and,
// for the PRS (0x2005) / LookAt (0x2006) classes, the reference-slot layout — PRS refs 0/1/2 =
// pos/rot/scale sub-controllers, LookAt refs 0/1/2/3 = target-node/pos/roll/scale — with a
// per-slot histogram of the sub-controller classes (PRSSUB lines: class, superclass, typed-
// keyframer or not, key count, presence+size of the default-value chunk 0x2503/0x2504/0x2505/
// 0x2501 wherever a non-keyframer carries one). This doubles as the §12.2 corpus inventory for
// typing PRS/LookAt and as the standing validation that the typed accessors agree with the raw
// slot reads once CControlPRS/CControlLookAt exist.
static int prsSelfTest(const char *maxFile, CStorageOleIn &in, CSceneClassRegistry *reg, bool verbose)
{
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cerr << "no DllDirectory\n"; return 2; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cerr << "no ClassDirectory3\n"; return 2; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cerr << "no Scene\n"; return 2; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 2; }
	}
	static const NLMISC::CClassId prsClassId(0x00002005, 0x00000000);
	static const NLMISC::CClassId lookAtClassId(0x00002006, 0x00000000);
	uint nCtrl = 0, nPrs = 0, nLookAt = 0, nOther = 0;
	uint nMismatch = 0, nTypedPrs = 0, nTypedLookAt = 0, nForeignSc = 0;
	uint nValPos = 0, nValRot = 0, nValScale = 0;
	std::map<std::string, uint> clsHist;   // per transform-controller class
	std::map<std::string, uint> refHist;   // "cls nbrefs=N"
	std::map<std::string, uint> subHist;   // per (owner, slot, sub-class...) line
	std::map<std::string, uint> seqHist;   // own-chunk order signature per class
	TChunkInv inv;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CSceneClass *sc = dynamic_cast<CSceneClass *>(it->second);
		if (!sc || sc->classDesc()->superClassId() != 0x00009008) continue;
		++nCtrl;
		NLMISC::CClassId cid = sc->classDesc()->classId();
		++clsHist[cid.toString()];
		// Own-chunk inventory. After parse, m_Chunks still lists every original chunk (parse copies
		// the pointers onto the orphan list without clearing m_Chunks — ownership is the flag), so
		// chunks() alone is the complete, duplicate-free inventory for claimed AND unclaimed ids.
		char lvl[24];
		snprintf(lvl, sizeof(lvl), "%08x.%08x", (uint32)cid.a(), (uint32)cid.b());
		std::string seq;
		for (CStorageContainer::TStorageObjectConstIt ct = sc->chunks().begin(); ct != sc->chunks().end(); ++ct)
		{
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(ct->second);
			invAdd(inv, lvl, ct->first, raw ? (uint32)raw->Value.size() : 0xFFFFFFFFu, ct->second->isContainer());
			char tok[12];
			snprintf(tok, sizeof(tok), "%x ", ct->first);
			seq += tok;
		}
		BUILTIN::CReferenceMaker *rm = dynamic_cast<BUILTIN::CReferenceMaker *>(sc);
		uint nbRefs = rm ? rm->nbReferences() : 0;
		++refHist[std::string(lvl) + " nbrefs=" + NLMISC::toString(nbRefs)];
		bool isPrs = (cid == prsClassId);
		bool isLookAt = (cid == lookAtClassId);
		if (isPrs) ++nPrs;
		else if (isLookAt) ++nLookAt;
		else ++nOther;
		// Typed-class validation (once CControlPRS/CControlLookAt are registered): the exact-id
		// object must BE the typed class, its slot accessors must agree with the raw reference
		// reads, and its valueAt0 helpers must agree with the sub-keyframer's own eval.
		if (isPrs)
		{
			BUILTIN::CControlPRS *prs = dynamic_cast<BUILTIN::CControlPRS *>(sc);
			if (!prs)
			{
				++nMismatch;
				std::cerr << "  prs TYPED MISS: 0x2005 object is not CControlPRS (" << maxFile << ")\n";
			}
			else
			{
				++nTypedPrs;
				if (prs->positionController() != rm->getReference(0)
					|| prs->rotationController() != rm->getReference(1)
					|| prs->scaleController() != rm->getReference(2))
				{
					++nMismatch;
					std::cerr << "  prs SLOT MISMATCH (" << maxFile << ")\n";
				}
				float p[3], q[4], s7[7];
				bool tp = prs->posValueAt0(p), tr = prs->rotValueAt0(q), ts = prs->scaleValueAt0(s7);
				BUILTIN::CControlKeyFramerBase *kp = dynamic_cast<BUILTIN::CControlKeyFramerBase *>(rm->getReference(0));
				BUILTIN::CControlKeyFramerBase *kr = dynamic_cast<BUILTIN::CControlKeyFramerBase *>(rm->getReference(1));
				BUILTIN::CControlKeyFramerBase *ks = dynamic_cast<BUILTIN::CControlKeyFramerBase *>(rm->getReference(2));
				float p2[3], q2[4], s2[7];
				bool rp = kp && kp->posValueAt0(p2), rr = kr && kr->rotValueAt0(q2), rs = ks && ks->scaleValueAt0(s2);
				bool bad = (tp != rp) || (tr != rr) || (ts != rs)
					|| (tp && memcmp(p, p2, 12)) || (tr && memcmp(q, q2, 16)) || (ts && memcmp(s7, s2, 28));
				if (bad)
				{
					++nMismatch;
					std::cerr << "  prs VALUEAT0 MISMATCH (" << maxFile << ")\n";
				}
				if (tp) ++nValPos;
				if (tr) ++nValRot;
				if (ts) ++nValScale;
			}
		}
		else if (isLookAt)
		{
			BUILTIN::CControlLookAt *la = dynamic_cast<BUILTIN::CControlLookAt *>(sc);
			if (!la)
			{
				++nMismatch;
				std::cerr << "  lookat TYPED MISS: 0x2006 object is not CControlLookAt (" << maxFile << ")\n";
			}
			else
			{
				++nTypedLookAt;
				if (la->targetNode() != rm->getReference(0)
					|| la->positionController() != rm->getReference(1)
					|| la->rollController() != rm->getReference(2)
					|| la->scaleController() != rm->getReference(3))
				{
					++nMismatch;
					std::cerr << "  lookat SLOT MISMATCH (" << maxFile << ")\n";
				}
				float p[3];
				if (la->posValueAt0(p)) ++nValPos;
			}
		}
		if (isPrs || isLookAt)
		{
			if (!seq.empty() && seq[seq.size() - 1] == ' ') seq.resize(seq.size() - 1);
			++seqHist[std::string(isPrs ? "prs: " : "lookat: ") + seq];
		}
		if (rm)
		{
			// Slot inventory for EVERY transform-controller class (PRS/LookAt are the typed
			// targets; the biped classes 0x9154/0x9156/0x3011 ride along so the biped_rig
			// getLocalTransform path's possible inputs are inventoried too).
			for (uint slot = 0; slot < nbRefs && slot < 5; ++slot)
			{
				CSceneClass *sub = dynamic_cast<CSceneClass *>(rm->getReference(slot));
				std::ostringstream line;
				line << (isPrs ? "prs" : isLookAt ? "lookat" : lvl) << " slot=" << slot;
				if (!sub) line << " null";
				else
				{
					line << " cls=" << sub->classDesc()->classId().toString()
					     << " sc=0x" << std::hex << sub->classDesc()->superClassId() << std::dec;
					if (dynamic_cast<BUILTIN::INode *>(sub)) line << " node";
					BUILTIN::CControlKeyFramerBase *kf = dynamic_cast<BUILTIN::CControlKeyFramerBase *>(sub);
					if (kf)
					{
						line << " kf keys=" << (kf->keyCount() ? "y" : "0");
						uint dsz = 0;
						line << " def=" << (kf->defaultValue(dsz) ? NLMISC::toString(dsz) : std::string("-"));
					}
					else
					{
						// Non-keyframer: full own-chunk inventory (what would the raw default-value
						// fallback find on it?), bucketed under sub.<classid>.
						{
							char slvl[32];
							snprintf(slvl, sizeof(slvl), "sub.%08x.%08x",
							         (uint32)sub->classDesc()->classId().a(), (uint32)sub->classDesc()->classId().b());
							for (CStorageContainer::TStorageObjectConstIt ct = sub->chunks().begin(); ct != sub->chunks().end(); ++ct)
							{
								CStorageRaw *raw = dynamic_cast<CStorageRaw *>(ct->second);
								invAdd(inv, slvl, ct->first, raw ? (uint32)raw->Value.size() : 0xFFFFFFFFu, ct->second->isContainer());
							}
						}
						static const uint16 defIds[4] = { 0x2501, 0x2503, 0x2504, 0x2505 };
						for (uint d = 0; d < 4; ++d)
						{
							uint32 sz = 0xFFFFFFFFu;
							bool found = false;
							for (CStorageContainer::TStorageObjectConstIt ot = sub->orphanedChunks().begin(); ot != sub->orphanedChunks().end() && !found; ++ot)
								if (ot->first == defIds[d])
								{
									found = true;
									if (CStorageRaw *raw = dynamic_cast<CStorageRaw *>(ot->second)) sz = (uint32)raw->Value.size();
								}
							for (CStorageContainer::TStorageObjectConstIt ct = sub->chunks().begin(); ct != sub->chunks().end() && !found; ++ct)
								if (ct->first == defIds[d])
								{
									found = true;
									if (CStorageRaw *raw = dynamic_cast<CStorageRaw *>(ct->second)) sz = (uint32)raw->Value.size();
								}
							if (found)
								line << " def" << std::hex << defIds[d] << std::dec << "=" << sz;
						}
					}
				}
				++subHist[line.str()];
			}
		}
		if (verbose)
			std::cerr << "  ctrltm " << cid.toString() << " nbrefs=" << nbRefs << "\n";
	}
	for (std::map<std::string, uint>::const_iterator ht = clsHist.begin(); ht != clsHist.end(); ++ht)
		std::cout << "CTCLS n=" << ht->second << " cls=" << ht->first << "\n";
	for (std::map<std::string, uint>::const_iterator ht = refHist.begin(); ht != refHist.end(); ++ht)
		std::cout << "CTREF n=" << ht->second << " " << ht->first << "\n";
	for (std::map<std::string, uint>::const_iterator ht = subHist.begin(); ht != subHist.end(); ++ht)
		std::cout << "PRSSUB n=" << ht->second << " " << ht->first << "\n";
	for (std::map<std::string, uint>::const_iterator ht = seqHist.begin(); ht != seqHist.end(); ++ht)
		std::cout << "CTSEQ n=" << ht->second << " " << ht->first << "\n";
	invPrint(inv, "CTINV");
	// PRS/LookAt class ids under a FOREIGN superclass would pass a classid compare but not the
	// typed registration — prove there are none (the consumers moved from classid compares to
	// dynamic_cast on the typed classes).
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CSceneClass *sc = dynamic_cast<CSceneClass *>(it->second);
		if (!sc || sc->classDesc()->superClassId() == 0x00009008) continue;
		NLMISC::CClassId cid = sc->classDesc()->classId();
		if (cid == prsClassId || cid == lookAtClassId)
		{
			++nForeignSc;
			std::cerr << "  prs FOREIGN SUPERCLASS: " << cid.toString() << " sc=0x" << std::hex
			          << sc->classDesc()->superClassId() << std::dec << " (" << maxFile << ")\n";
		}
	}
	bool fail = nMismatch || nForeignSc;
	std::cout << (fail ? "FAIL" : "OK") << " prs-selftest: " << nCtrl << " transform-ctrls, "
	          << nPrs << " prs (" << nTypedPrs << " typed), " << nLookAt << " lookat ("
	          << nTypedLookAt << " typed), " << nOther << " other, valueat0 p/r/s "
	          << nValPos << "/" << nValRot << "/" << nValScale << ", "
	          << nMismatch << " mismatch, " << nForeignSc << " foreign-sc\n";
	return fail ? 1 : 0;
}

// Parse the Scene stream fully and run the CAppData script-entry write-path self-check: for
// every script AppData entry (the NEL3D_APPDATA_* MAXSCRIPT-keyed, null-terminated-string
// entries), read the string through the typed getScriptString, write the SAME value back
// through setScriptString, then rebuild the whole Scene stream and require byte-identity with
// the source — the idempotent-set proof that the write path reproduces the stored layout.
static int appDataSelfTest(CStorageOleIn &in, CSceneClassRegistry *reg, bool verbose)
{
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	std::vector<uint8> sceneBytes;
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cerr << "no DllDirectory\n"; return 2; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cerr << "no ClassDirectory3\n"; return 2; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 2; }
	}
	{
		if (!in.readStream("Scene", sceneBytes)) { std::cerr << "no Scene\n"; return 2; }
		CStorageStream ss(sceneBytes); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 2; }
	}
	uint nObj = 0, nEntries = 0, nNonString = 0, nSetFail = 0, nReadbackFail = 0;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		BUILTIN::CAnimatable *anim = dynamic_cast<BUILTIN::CAnimatable *>(it->second);
		if (!anim) continue;
		BUILTIN::STORAGE::CAppData *ad = anim->existingAppData();
		if (!ad) continue;
		bool counted = false;
		// Collect the script sub-ids first — setScriptString may not mutate the map during walk
		// (it doesn't for existing keys, but keep the walk clean).
		std::vector<uint32> subIds;
		for (BUILTIN::STORAGE::CAppData::TMap::const_iterator eit = ad->entries().begin(); eit != ad->entries().end(); ++eit)
		{
			if (eit->first.ClassId != BUILTIN::STORAGE::CAppData::ScriptClassId
				|| eit->first.SuperClassId != BUILTIN::STORAGE::CAppData::ScriptSuperClassId) continue;
			subIds.push_back(eit->first.SubId);
		}
		for (uint i = 0; i < subIds.size(); ++i)
		{
			if (!counted) { ++nObj; counted = true; }
			++nEntries;
			std::string value;
			if (!ad->getScriptString(subIds[i], value))
			{
				// Entry exists under the script key but is not a null-terminated string —
				// counted (a corpus-wide nonzero count would mean the convention is wrong).
				++nNonString;
				continue;
			}
			if (!ad->setScriptString(subIds[i], value)) { ++nSetFail; continue; }
			std::string back;
			if (!ad->getScriptString(subIds[i], back) || back != value) ++nReadbackFail;
		}
	}
	// Rebuild the Scene stream and require byte-identity with the source (idempotent set).
	bool byteIdentical = false;
	try
	{
		scene.clean(); scene.build(VersionUnknown); scene.disown();
		std::vector<uint8> rebuilt = writeContainerToTemp(scene, g_tempPath);
		byteIdentical = (rebuilt == sceneBytes);
	}
	catch (std::exception &e) { std::cerr << "rebuild: " << e.what() << "\n"; }
	bool fail = nSetFail || nReadbackFail || !byteIdentical;
	std::cout << (fail ? "FAIL" : "OK") << " appdata-selftest: " << nObj << " objects, "
	          << nEntries << " script entries, " << nNonString << " non-string, "
	          << nSetFail << " set-fail, " << nReadbackFail << " readback-fail, rebuild "
	          << (byteIdentical ? "byte-identical" : "DIFFERS") << "\n";
	if (verbose && !fail)
		std::cerr << "  (idempotent setScriptString over every script entry keeps the Scene stream byte-identical)\n";
	return fail ? 1 : 0;
}

// Parse the Scene stream fully and verify the RPatchMesh blob codec (nelpatch/rpo_data
// encodeRPatchMesh) is the byte-identity inverse of the decoder on every blob in the file:
// the 0x08FD chunk of every RklPatch object (base RPO state) and the 0x4001 RFINALPATCH leaf
// of every NeL Edit Patch / NeL Patch Painter modifier snapshot (per-node local data 0x2512 ->
// 0x1000). This is the write-direction proof the standalone zone painter's save path rides on
// (design-doc: overlay data, paired corpus-wide re-encode selftest). Prints an RPOV version
// histogram line per version seen, and flags 0x4001 leaves under unexpected modifier classes.
static int rpoSelfTest(const char *maxFile, CStorageOleIn &in, CSceneClassRegistry *reg, bool verbose)
{
	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cerr << "no DllDirectory\n"; return 2; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cerr << "no ClassDirectory3\n"; return 2; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 2; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cerr << "no Scene\n"; return 2; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 2; }
	}
	// The two modifier classes whose per-node local data carries an RFINALPATCH 0x4001 blob
	// (both save through RPatchMesh::Save; the zone exporter evaluates the same snapshots).
	static const NLMISC::CClassId nelEditPatchClassId(0x4dd14a3c, 0x4ac23c0c);
	static const NLMISC::CClassId nelPatchPaintClassId(0x0c49560f, 0x3c3d68e7);
	uint nRpo = 0, nSnap = 0, nDecodeFail = 0, nMismatch = 0, nOtherClass = 0;
	std::map<uint32, uint> versionInv;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		if (NELPATCH::CRklPatchObject *rpo = dynamic_cast<NELPATCH::CRklPatchObject *>(it->second))
		{
			const CStorageRaw *raw = rpo->rpoChunk();
			if (!raw) continue; // no 0x08FD chunk claimed (never observed; not an RPO state)
			++nRpo;
			NELPATCH::SRPatchMesh rp;
			std::string err;
			if (!NELPATCH::decodeRpoChunk(nlVectorData(raw->Value), raw->Value.size(), rp, err))
			{
				++nDecodeFail;
				std::cerr << "  rpo selftest DECODE FAIL 0x08fd (" << maxFile << "): " << err << "\n";
				continue;
			}
			++versionInv[rp.Version];
			std::vector<uint8> re;
			NELPATCH::encodeRpoChunk(rp, re);
			if (re != raw->Value)
			{
				++nMismatch;
				std::cerr << "  rpo selftest REENCODE MISMATCH 0x08fd (" << maxFile << "): "
				          << raw->Value.size() << " -> " << re.size() << " bytes\n";
			}
			continue;
		}
		BUILTIN::CDerivedObject *d = dynamic_cast<BUILTIN::CDerivedObject *>(it->second);
		if (!d) continue;
		for (uint i = 0; i < d->modifierCount(); ++i)
		{
			CStorageContainer *data = dynamic_cast<CStorageContainer *>(d->localModData(i));
			if (!data) continue;
			// local data wrapper 0x1000 -> RFINALPATCH 0x4001 raw leaf
			CStorageContainer *wrap = NULL;
			for (CStorageContainer::TStorageObjectConstIt jt = data->chunks().begin(); jt != data->chunks().end() && !wrap; ++jt)
				if (jt->first == 0x1000) wrap = dynamic_cast<CStorageContainer *>(jt->second);
			if (!wrap) continue;
			CStorageRaw *rfp = NULL;
			for (CStorageContainer::TStorageObjectConstIt jt = wrap->chunks().begin(); jt != wrap->chunks().end() && !rfp; ++jt)
				if (jt->first == 0x4001) rfp = dynamic_cast<CStorageRaw *>(jt->second);
			if (!rfp) continue;
			CSceneClass *mod = d->modifier(i);
			const NLMISC::CClassId modClass = mod ? mod->classDesc()->classId() : NLMISC::CClassId::Null;
			if (modClass != nelEditPatchClassId && modClass != nelPatchPaintClassId)
			{
				++nOtherClass;
				std::cerr << "  rpo selftest UNEXPECTED 0x4001 under modifier class "
				          << modClass.toString() << " (" << maxFile << ")\n";
			}
			++nSnap;
			NELPATCH::SRPatchMesh rp;
			std::string err;
			if (!NELPATCH::decodeRPatchMesh(nlVectorData(rfp->Value), rfp->Value.size(), rp, err))
			{
				++nDecodeFail;
				std::cerr << "  rpo selftest DECODE FAIL 0x4001 (" << maxFile << "): " << err << "\n";
				continue;
			}
			++versionInv[rp.Version];
			std::vector<uint8> re;
			NELPATCH::encodeRPatchMesh(rp, re);
			if (re != rfp->Value)
			{
				++nMismatch;
				std::cerr << "  rpo selftest REENCODE MISMATCH 0x4001 (" << maxFile << "): "
				          << rfp->Value.size() << " -> " << re.size() << " bytes\n";
			}
			if (verbose)
				std::cerr << "  rpo snapshot v" << rp.Version << " (" << rp.Patches.size()
				          << " ui patches) under " << modClass.toString() << "\n";
		}
	}
	for (std::map<uint32, uint>::const_iterator vt = versionInv.begin(); vt != versionInv.end(); ++vt)
		std::cout << "RPOV v=" << vt->first << " n=" << vt->second << "\n";
	bool fail = nDecodeFail || nMismatch || nOtherClass;
	std::cout << (fail ? "FAIL" : "OK") << " rpo-selftest: " << nRpo << " rpo, " << nSnap
	          << " snapshots, " << nDecodeFail << " decode-fail, " << nMismatch << " mismatch, "
	          << nOtherClass << " other-class\n";
	return fail ? 1 : 0;
}

// The whole-file NULL-EDIT proof for the zone painter's save path: parse the .max, push every
// RPatchMesh blob in the file through the decode->encode write path IN PLACE (every RklPatch
// 0x08FD via CRklPatchObject::setRPatch, every modifier-snapshot 0x4001 via encodeRPatchMesh
// into the raw leaf), rebuild the Scene stream from the typed graph, write the whole .max back
// (other streams verbatim, OLE class id preserved), and require EVERY stream byte-identical to
// the original — an unmodified decode written back through the painter's save path must be a
// no-op on the file. Skips (exit 0, "no-rpo") files without any blob.
static int rpoModifySaveTest(const char *maxFile, CStorageOleIn &in, CSceneClassRegistry *reg, const std::string &tempMax, bool verbose)
{
	static const char *kStreams[] = {
		"VideoPostQueue", "Config", "ClassData", "DllDirectory", "ClassDirectory3", "Scene",
		"\05SummaryInformation", "\05DocumentSummaryInformation", NULL
	};
	std::vector<std::string> present;
	std::vector<std::vector<uint8> > rawOrig;
	for (const char **n = kStreams; *n; ++n)
	{
		std::vector<uint8> b;
		if (in.readStream(*n, b)) { present.push_back(*n); rawOrig.push_back(b); }
	}
	uint8 classId[16];
	bool haveClassId = in.getClassId(classId);

	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cout << "SKIP rpo-modify-save: no DllDirectory\n"; return 0; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 1; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cout << "SKIP rpo-modify-save: no ClassDirectory3\n"; return 0; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 1; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cout << "SKIP rpo-modify-save: no Scene\n"; return 0; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 1; }
	}

	uint nRpo = 0, nSnap = 0, fails = 0;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		if (NELPATCH::CRklPatchObject *rpo = dynamic_cast<NELPATCH::CRklPatchObject *>(it->second))
		{
			const CStorageRaw *raw = rpo->rpoChunk();
			if (!raw) continue;
			NELPATCH::SRPatchMesh rp;
			std::string err;
			if (!NELPATCH::decodeRpoChunk(nlVectorData(raw->Value), raw->Value.size(), rp, err))
			{ std::cerr << "  rpo-modify-save DECODE FAIL 0x08fd (" << maxFile << "): " << err << "\n"; ++fails; continue; }
			if (!rpo->setRPatch(rp))
			{ std::cerr << "  rpo-modify-save setRPatch FAIL (" << maxFile << ")\n"; ++fails; continue; }
			++nRpo;
			continue;
		}
		BUILTIN::CDerivedObject *d = dynamic_cast<BUILTIN::CDerivedObject *>(it->second);
		if (!d) continue;
		for (uint i = 0; i < d->modifierCount(); ++i)
		{
			CStorageContainer *data = dynamic_cast<CStorageContainer *>(d->localModData(i));
			if (!data) continue;
			CStorageContainer *wrap = NULL;
			for (CStorageContainer::TStorageObjectConstIt jt = data->chunks().begin(); jt != data->chunks().end() && !wrap; ++jt)
				if (jt->first == 0x1000) wrap = dynamic_cast<CStorageContainer *>(jt->second);
			if (!wrap) continue;
			CStorageRaw *rfp = NULL;
			for (CStorageContainer::TStorageObjectConstIt jt = wrap->chunks().begin(); jt != wrap->chunks().end() && !rfp; ++jt)
				if (jt->first == 0x4001) rfp = dynamic_cast<CStorageRaw *>(jt->second);
			if (!rfp) continue;
			NELPATCH::SRPatchMesh rp;
			std::string err;
			if (!NELPATCH::decodeRPatchMesh(nlVectorData(rfp->Value), rfp->Value.size(), rp, err))
			{ std::cerr << "  rpo-modify-save DECODE FAIL 0x4001 (" << maxFile << "): " << err << "\n"; ++fails; continue; }
			NELPATCH::encodeRPatchMesh(rp, rfp->Value);
			++nSnap;
		}
	}
	if (!nRpo && !nSnap && !fails)
	{
		std::cout << "OK rpo-modify-save: no-rpo\n";
		return 0;
	}

	try { scene.clean(); scene.build(VersionUnknown); scene.disown(); }
	catch (std::exception &e) { std::cerr << "scene build: " << e.what() << "\n"; return 1; }
	std::vector<uint8> newScene;
	try { newScene = writeContainerToTemp(scene, g_tempPath); }
	catch (std::exception &e) { std::cerr << "scene write: " << e.what() << "\n"; return 1; }

	{
		CStorageOleOut out;
		for (size_t i = 0; i < present.size(); ++i)
		{
			if (present[i] == "Scene") out.addStream("Scene", newScene);
			else out.addStream(present[i], rawOrig[i]);
		}
		if (haveClassId) out.setClassId(classId);
		if (!out.write(tempMax)) { std::cerr << "cannot create " << tempMax << "\n"; return 1; }
	}

	CStorageOleIn in2;
	if (!in2.open(tempMax)) { std::cerr << "cannot reopen rewritten .max\n"; return 1; }
	for (size_t i = 0; i < present.size(); ++i)
	{
		std::vector<uint8> b2;
		in2.readStream(present[i], b2);
		if (b2 != rawOrig[i])
		{
			std::cerr << "  rpo-modify-save stream " << (present[i][0] == '\05' ? present[i].substr(1) : present[i])
			          << " NOT byte-identical (" << rawOrig[i].size() << " -> " << b2.size() << " bytes, " << maxFile << ")\n";
			++fails;
		}
	}

	std::cout << (fails ? "FAIL" : "OK") << " rpo-modify-save: " << nRpo << " rpo, " << nSnap
	          << " snapshots, " << fails << " fail\n";
	return fails ? 1 : 0;
}

// The whole-file NULL-EDIT proof for the PatchMesh chunk-stream encoder (Tier B): parse the
// .max, decode every PatchMesh in the file (the base stream of every RklPatch and the OUTPUT
// copy under every NeL Edit Patch / NeL Patch Painter modifier's 0x1140), write each straight
// back through encodePatchMesh IN PLACE (element streams, selection BitArrays, hooks, map
// channel; every other chunk untouched), rebuild the Scene stream, write the whole .max back
// and require EVERY stream byte-identical — decode -> encode must be the identity before any
// topological op is allowed to exist. Max 3 streams (reconstructed edge tables) are counted
// and skipped: their edge data is derived at decode and must never be written back.
static int pmModifySaveTest(const char *maxFile, CStorageOleIn &in, CSceneClassRegistry *reg, const std::string &tempMax, bool verbose)
{
	static const char *kStreams[] = {
		"VideoPostQueue", "Config", "ClassData", "DllDirectory", "ClassDirectory3", "Scene",
		"\05SummaryInformation", "\05DocumentSummaryInformation", NULL
	};
	std::vector<std::string> present;
	std::vector<std::vector<uint8> > rawOrig;
	for (const char **n = kStreams; *n; ++n)
	{
		std::vector<uint8> b;
		if (in.readStream(*n, b)) { present.push_back(*n); rawOrig.push_back(b); }
	}
	uint8 classId[16];
	bool haveClassId = in.getClassId(classId);

	CDllDirectory dll;
	CClassDirectory3 cd(&dll);
	CScene scene(reg, &dll, &cd);
	{
		std::vector<uint8> b;
		if (!in.readStream("DllDirectory", b)) { std::cout << "SKIP pm-modify-save: no DllDirectory\n"; return 0; }
		CStorageStream ss(b); try { dll.serial(ss); dll.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "dll: " << e.what() << "\n"; return 1; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("ClassDirectory3", b)) { std::cout << "SKIP pm-modify-save: no ClassDirectory3\n"; return 0; }
		CStorageStream ss(b); try { cd.serial(ss); cd.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "cd: " << e.what() << "\n"; return 1; }
	}
	{
		std::vector<uint8> b;
		if (!in.readStream("Scene", b)) { std::cout << "SKIP pm-modify-save: no Scene\n"; return 0; }
		CStorageStream ss(b); try { scene.serial(ss); scene.parse(VersionUnknown); } catch (std::exception &e) { std::cerr << "scene: " << e.what() << "\n"; return 1; }
	}

	static const NLMISC::CClassId nelEditPatchClassId(0x4dd14a3c, 0x4ac23c0c);
	static const NLMISC::CClassId nelPatchPaintClassId(0x0c49560f, 0x3c3d68e7);
	uint nBase = 0, nMod = 0, nMax3 = 0, fails = 0;
	CSceneClassContainer *ssc = scene.container();
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		if (NELPATCH::CRklPatchObject *rpo = dynamic_cast<NELPATCH::CRklPatchObject *>(it->second))
		{
			NELPATCH::SPatchMesh pm;
			std::string err;
			if (!rpo->decodePatch(pm, err))
			{ std::cerr << "  pm-modify-save DECODE FAIL base (" << maxFile << "): " << err << "\n"; ++fails; continue; }
			if (pm.EdgesReconstructed) { ++nMax3; continue; }
			if (!rpo->setPatchMesh(pm, err))
			{ std::cerr << "  pm-modify-save ENCODE FAIL base (" << maxFile << "): " << err << "\n"; ++fails; continue; }
			++nBase;
			continue;
		}
		BUILTIN::CDerivedObject *d = dynamic_cast<BUILTIN::CDerivedObject *>(it->second);
		if (!d) continue;
		for (uint i = 0; i < d->modifierCount(); ++i)
		{
			CSceneClass *mod = d->modifier(i);
			const NLMISC::CClassId modClass = mod ? mod->classDesc()->classId() : NLMISC::CClassId::Null;
			if (modClass != nelEditPatchClassId && modClass != nelPatchPaintClassId)
				continue;
			CStorageContainer *data = dynamic_cast<CStorageContainer *>(d->localModData(i));
			if (!data) continue;
			CStorageContainer *wrap = NULL;
			for (CStorageContainer::TStorageObjectConstIt jt = data->chunks().begin(); jt != data->chunks().end() && !wrap; ++jt)
				if (jt->first == 0x1000) wrap = dynamic_cast<CStorageContainer *>(jt->second);
			if (!wrap) continue;
			CStorageContainer *fp = NULL;
			for (CStorageContainer::TStorageObjectConstIt jt = wrap->chunks().begin(); jt != wrap->chunks().end() && !fp; ++jt)
				if (jt->first == 0x1140) fp = dynamic_cast<CStorageContainer *>(jt->second);
			if (!fp) continue;
			NELPATCH::SPatchMesh pm;
			std::string err;
			if (!NELPATCH::decodePatchMesh(fp->chunks(), pm, err))
			{ std::cerr << "  pm-modify-save DECODE FAIL 0x1140 (" << maxFile << "): " << err << "\n"; ++fails; continue; }
			if (pm.EdgesReconstructed) { ++nMax3; continue; }
			if (!NELPATCH::encodePatchMesh(pm, fp->chunksMut(), err))
			{ std::cerr << "  pm-modify-save ENCODE FAIL 0x1140 (" << maxFile << "): " << err << "\n"; ++fails; continue; }
			++nMod;
			if (verbose)
				std::cerr << "  pm 0x1140 " << pm.Verts.size() << "v/" << pm.Patches.size()
				          << "p under " << modClass.toString() << "\n";
		}
	}
	if (!nBase && !nMod && !nMax3 && !fails)
	{
		std::cout << "OK pm-modify-save: no-pm\n";
		return 0;
	}
	if (!nBase && !nMod && !fails)
	{
		// Max 3 only: nothing encodable in the file; skip rather than prove nothing.
		std::cout << "OK pm-modify-save: max3-only, " << nMax3 << " skipped\n";
		return 0;
	}

	try { scene.clean(); scene.build(VersionUnknown); scene.disown(); }
	catch (std::exception &e) { std::cerr << "scene build: " << e.what() << "\n"; return 1; }
	std::vector<uint8> newScene;
	try { newScene = writeContainerToTemp(scene, g_tempPath); }
	catch (std::exception &e) { std::cerr << "scene write: " << e.what() << "\n"; return 1; }

	{
		CStorageOleOut out;
		for (size_t i = 0; i < present.size(); ++i)
		{
			if (present[i] == "Scene") out.addStream("Scene", newScene);
			else out.addStream(present[i], rawOrig[i]);
		}
		if (haveClassId) out.setClassId(classId);
		if (!out.write(tempMax)) { std::cerr << "cannot create " << tempMax << "\n"; return 1; }
	}

	CStorageOleIn in2;
	if (!in2.open(tempMax)) { std::cerr << "cannot reopen rewritten .max\n"; return 1; }
	for (size_t i = 0; i < present.size(); ++i)
	{
		std::vector<uint8> b2;
		in2.readStream(present[i], b2);
		if (b2 != rawOrig[i])
		{
			std::cerr << "  pm-modify-save stream " << (present[i][0] == '\05' ? present[i].substr(1) : present[i])
			          << " NOT byte-identical (" << rawOrig[i].size() << " -> " << b2.size() << " bytes, " << maxFile << ")\n";
			++fails;
		}
	}

	std::cout << (fails ? "FAIL" : "OK") << " pm-modify-save: " << nBase << " base, " << nMod
	          << " mod, " << nMax3 << " max3-skip, " << fails << " fail\n";
	return fails ? 1 : 0;
}

// Recursive reference-tree dump (used by --uvgen-dump).
static void dumpRefTree(CSceneClass *obj, int depth, int maxDepth)
{
	BUILTIN::CReferenceMaker *rm = dynamic_cast<BUILTIN::CReferenceMaker *>(obj);
	if (!rm) return;
	std::string pad((size_t)(depth + 1) * 2, ' ');
	for (uint r = 0; r < rm->nbReferences(); ++r)
	{
		CSceneClass *ref = dynamic_cast<CSceneClass *>(rm->getReference(r));
		if (!ref) { std::cout << pad << "ref[" << r << "] = null\n"; continue; }
		std::string extra;
		if (BUILTIN::CControlKeyFramerBase *kf = dynamic_cast<BUILTIN::CControlKeyFramerBase *>(ref))
		{
			sint32 s = 0, e = 0; kf->range(s, e);
			extra = " <KEYFRAMER keys=" + NLMISC::toString(kf->keyCount()) + " range=[" + NLMISC::toString(s) + ".." + NLMISC::toString(e) + "]>";
		}
		std::cout << pad << "ref[" << r << "] " << ref->classDesc()->classId().toString()
		          << " sc=" << std::hex << ref->classDesc()->superClassId() << std::dec << extra;
		if (CStorageContainer *cont = dynamic_cast<CStorageContainer *>(ref))
		{
			std::cout << " chunks:";
			for (CStorageContainer::TStorageObjectConstIt ct = cont->chunks().begin(); ct != cont->chunks().end(); ++ct)
			{
				std::cout << " 0x" << std::hex << ct->first << std::dec;
				if (CStorageRaw *rw = dynamic_cast<CStorageRaw *>(ct->second)) std::cout << "(" << rw->Value.size() << ")";
			}
		}
		std::cout << "\n";
		if (ref->classDesc()->superClassId() == 0x8)
		{
			if (CStorageContainer *pb = dynamic_cast<CStorageContainer *>(ref))
				for (CStorageContainer::TStorageObjectConstIt et = pb->chunks().begin(); et != pb->chunks().end(); ++et)
				{
					if (et->first != 0x0002) continue;
					CStorageContainer *e = dynamic_cast<CStorageContainer *>(et->second);
					if (!e) continue;
					sint32 idx = -1; std::string subs;
					for (CStorageContainer::TStorageObjectConstIt st = e->chunks().begin(); st != e->chunks().end(); ++st)
					{
						char buf[16]; snprintf(buf, sizeof(buf), " 0x%x", st->first); subs += buf;
						CStorageRaw *rw = dynamic_cast<CStorageRaw *>(st->second);
						if (st->first == 0x0003 && rw && rw->Value.size() == 4) memcpy(&idx, nlVectorData(rw->Value), 4);
						if (rw) { char b2[16]; snprintf(b2, sizeof(b2), "(%zu)", rw->Value.size()); subs += b2; }
					}
					std::cout << pad << "  param idx=" << idx << " chunks:" << subs << "\n";
				}
		}
		if (depth + 1 < maxDepth) dumpRefTree(ref, depth + 1, maxDepth);
	}
}

int main(int argc, char **argv)
{
	bool doT2 = false;
	bool verbose = false;
	bool doPb2SelfTest = false;
	bool doOldPbSelfTest = false;
	bool doShapeSelfTest = false;
	bool doDerivedSelfTest = false;
	bool doMeshDeltaSelfTest = false;
	bool doMapExtSelfTest = false;
	bool doMapChannelSelfTest = false;
	bool doPrsSelfTest = false;
	bool doAppDataSelfTest = false;
	bool doRpoSelfTest = false;
	bool doRpoModifySave = false;
	bool doPmModifySave = false;
	bool doModifySave = false;
	bool doAppDataModifySave = false;
	bool doMtlDump = false;
	bool doUvgenDump = false;
	const char *dumpScene = nullptr;
	const char *maxFile = nullptr;
	// Accept flags in ANY position (the corpus drivers historically appended --parse after the
	// file path — with a leading-flags-only parser that silently turned T2 into a no-op).
	for (int i = 1; i < argc; ++i)
	{
		std::string a = argv[i];
		if (a == "--parse") doT2 = true;
		else if (a == "--verbose" || a == "-v") verbose = true;
		else if (a == "--pb2-selftest") doPb2SelfTest = true;
		else if (a == "--oldpb-selftest") doOldPbSelfTest = true;
		else if (a == "--shape-selftest") doShapeSelfTest = true;
		else if (a == "--derived-selftest") doDerivedSelfTest = true;
		else if (a == "--meshdelta-selftest") doMeshDeltaSelfTest = true;
		else if (a == "--mapext-selftest") doMapExtSelfTest = true;
		else if (a == "--mapchannel-selftest") doMapChannelSelfTest = true;
		else if (a == "--prs-selftest") doPrsSelfTest = true;
		else if (a == "--appdata-selftest") doAppDataSelfTest = true;
		else if (a == "--rpo-selftest") doRpoSelfTest = true;
		else if (a == "--rpo-modify-save-test") doRpoModifySave = true;
		else if (a == "--pm-modify-save-test") doPmModifySave = true;
		else if (a == "--modify-save-test") doModifySave = true;
		else if (a == "--appdata-modify-save-test") doAppDataModifySave = true;
		else if (a == "--mtl-dump") doMtlDump = true;
		else if (a == "--uvgen-dump") doUvgenDump = true;
		else if (a == "--dump-scene" && i + 1 < argc) dumpScene = argv[++i];
		else if (a.size() >= 2 && a[0] == '-' && a[1] == '-')
		{
			std::cerr << "unknown flag: " << a << "\n";
			return 2;
		}
		else if (!maxFile) maxFile = argv[i];
		else { std::cerr << "unexpected argument: " << a << "\n"; return 2; }
	}
	if (!maxFile)
	{
		std::cerr << "usage: pipeline_max_corpus_test [--parse] [--verbose] [--pb2-selftest] [--oldpb-selftest] [--shape-selftest] [--derived-selftest] [--meshdelta-selftest] [--mapext-selftest] [--mapchannel-selftest] [--prs-selftest] [--appdata-selftest] [--rpo-selftest] [--rpo-modify-save-test] [--pm-modify-save-test] [--modify-save-test] [--appdata-modify-save-test] <input.max>\n";
		return 2;
	}

	CStorageOleIn in;
	if (!in.open(maxFile)) { std::cerr << "not an OLE file: " << maxFile << "\n"; return 2; }

	CSceneClassRegistry reg;
	BUILTIN::CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);
	NELPATCH::CNelPatch::registerClasses(&reg);

	if (doPb2SelfTest)
	{
		int rc = pb2SelfTest(in, &reg, verbose);
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doOldPbSelfTest)
	{
		int rc = oldPbSelfTest(in, &reg, verbose);
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doShapeSelfTest)
	{
		int rc = shapeSelfTest(in, &reg, verbose);
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doDerivedSelfTest)
	{
		int rc = derivedSelfTest(maxFile, in, &reg, verbose);
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doMeshDeltaSelfTest)
	{
		int rc = meshDeltaSelfTest(maxFile, in, &reg, verbose);
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doMapExtSelfTest)
	{
		int rc = mapExtSelfTest(maxFile, in, &reg, verbose);
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doMapChannelSelfTest)
	{
		int rc = mapChannelSelfTest(maxFile, in, &reg, verbose);
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doPrsSelfTest)
	{
		int rc = prsSelfTest(maxFile, in, &reg, verbose);
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doAppDataSelfTest)
	{
		int rc = appDataSelfTest(in, &reg, verbose);
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doRpoSelfTest)
	{
		int rc = rpoSelfTest(maxFile, in, &reg, verbose);
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doRpoModifySave)
	{
		std::string tempMax = "/tmp/pipeline_max_rpo_modify_save." + NLMISC::toString((sint32)PMCT_GETPID()) + ".max";
		int rc = rpoModifySaveTest(maxFile, in, &reg, tempMax, verbose);
		remove(tempMax.c_str());
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doPmModifySave)
	{
		std::string tempMax = "/tmp/pipeline_max_pm_modify_save." + NLMISC::toString((sint32)PMCT_GETPID()) + ".max";
		int rc = pmModifySaveTest(maxFile, in, &reg, tempMax, verbose);
		remove(tempMax.c_str());
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doModifySave || doAppDataModifySave)
	{
		std::string tempMax = "/tmp/pipeline_max_modify_save." + NLMISC::toString((sint32)PMCT_GETPID()) + ".max";
		int rc = modifySaveTest(in, &reg, tempMax, verbose, doAppDataModifySave);
		remove(tempMax.c_str());
		remove(g_tempPath.c_str());
		return rc;
	}

	if (doMtlDump)
	{
		// Enumerate the typed material/texmap tree: name (CMtlBase), sub-materials (CMultiMtl).
		CDllDirectory dll; CClassDirectory3 cd(&dll); CScene scene(&reg, &dll, &cd);
		loadContainer(in, "DllDirectory", dll); dll.parse(VersionUnknown);
		loadContainer(in, "ClassDirectory3", cd); cd.parse(VersionUnknown);
		loadContainer(in, "Scene", scene); scene.parse(VersionUnknown);
		uint nMtl = 0, nTex = 0, nMulti = 0, nNamed = 0;
		CSceneClassContainer *ssc = scene.container();
		for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
		{
			BUILTIN::CMtlBase *mb = dynamic_cast<BUILTIN::CMtlBase *>(it->second);
			if (!mb) continue;
			TSClassId sc = mb->classDesc()->superClassId();
			if (sc == 0x00000c10) ++nTex; else ++nMtl;
			if (mb->hasName()) ++nNamed;
			BUILTIN::CMultiMtl *mm = dynamic_cast<BUILTIN::CMultiMtl *>(mb);
			if (mm) ++nMulti;
			if (verbose)
			{
				std::cout << "  " << (sc == 0x00000c10 ? "TEX " : "MTL ") << mb->classDesc()->classId().toString()
				          << " '" << mb->name() << "'";
				if (mm)
				{
					std::cout << " MULTI[" << mm->numSubMaterials() << "]:";
					for (uint s = 0; s < mm->numSubMaterials(); ++s)
					{
						BUILTIN::CMtlBase *sub = mm->subMaterial(s);
						std::cout << " '" << (sub ? sub->name() : std::string("<null>")) << "'";
					}
				}
				std::cout << "\n";
			}
		}
		std::cout << "OK mtl-dump: " << nMtl << " materials (" << nMulti << " multi), " << nTex
		          << " texmaps, " << nNamed << " named\n";
		remove(g_tempPath.c_str());
		return 0;
	}

	if (doUvgenDump)
	{
		CDllDirectory dll; CClassDirectory3 cd(&dll); CScene scene(&reg, &dll, &cd);
		loadContainer(in, "DllDirectory", dll); dll.parse(VersionUnknown);
		loadContainer(in, "ClassDirectory3", cd); cd.parse(VersionUnknown);
		loadContainer(in, "Scene", scene); scene.parse(VersionUnknown);
		CSceneClassContainer *ssc = scene.container();
		uint nTex = 0, nUvgen = 0;
		for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
		{
			CSceneClass *obj = dynamic_cast<CSceneClass *>(it->second);
			if (!obj) continue;
			TSClassId sc = obj->classDesc()->superClassId();
			NLMISC::CClassId cid = obj->classDesc()->classId();
			if (cid == NLMISC::CClassId(0x00000240, 0x00000000))
			{
				++nTex;
				std::string nm;
				if (BUILTIN::CMtlBase *mb = dynamic_cast<BUILTIN::CMtlBase *>(obj)) nm = mb->name();
				std::cout << "BitmapTex '" << nm << "'\n";
				dumpRefTree(obj, 0, 4);
			}
			else if (sc == 0x00000c20 || cid == NLMISC::CClassId(0x00000100, 0x00000000))
			{
				++nUvgen;
				std::cout << "UVGen " << cid.toString() << " sc=" << std::hex << sc << std::dec << " chunks:";
				if (CStorageContainer *cont = dynamic_cast<CStorageContainer *>(obj))
					for (CStorageContainer::TStorageObjectConstIt ct = cont->chunks().begin(); ct != cont->chunks().end(); ++ct)
						std::cout << " 0x" << std::hex << ct->first << std::dec;
				std::cout << "\n";
				dumpRefTree(obj, 0, 2);
			}
		}
		std::cout << "OK uvgen-dump: " << nTex << " bitmaptex, " << nUvgen << " uvgen\n";
		remove(g_tempPath.c_str());
		return 0;
	}

	if (dumpScene)
	{
		CDllDirectory dll; CClassDirectory3 cd(&dll); CScene scene(&reg, &dll, &cd);
		loadContainer(in, "DllDirectory", dll); dll.parse(VersionUnknown);
		loadContainer(in, "ClassDirectory3", cd); cd.parse(VersionUnknown);
		loadContainer(in, "Scene", scene); scene.parse(VersionUnknown);
		scene.clean(); scene.build(VersionUnknown); scene.disown();
		std::vector<uint8> bytes = writeContainerToTemp(scene, g_tempPath);
		FILE *f = fopen(dumpScene, "wb");
		if (f) { if (!bytes.empty()) fwrite(nlVectorData(bytes), 1, bytes.size(), f); fclose(f); }
		std::cout << "wrote " << bytes.size() << " bytes to " << dumpScene << "\n";
		remove(g_tempPath.c_str());
		return 0;
	}

	// Chunk-formatted streams that participate in the CStorageContainer round-trip. The two
	// \05Summary* streams and the OLE class-id are raw byte blobs (copied verbatim by any full
	// rewrite tool), so byte identity is trivially met and they aren't retested here.
	std::vector<std::string> streamNames;
	streamNames.push_back("DllDirectory");
	streamNames.push_back("ClassDirectory3");
	streamNames.push_back("ClassData");
	streamNames.push_back("Config");
	streamNames.push_back("Scene");
	streamNames.push_back("VideoPostQueue");

	// State that carries across streams for T2 (need dll to interpret classdir; both to interpret scene).
	CDllDirectory dllForT2;
	bool dllReady = false;
	CClassDirectory3 cdForT2(&dllForT2);
	bool cdReady = false;

	std::vector<StreamResult> results;
	bool anyFail = false;

	for (std::vector<std::string>::const_iterator ni = streamNames.begin(); ni != streamNames.end(); ++ni)
	{
		const std::string &name = *ni;
		StreamResult r = { name, false, false, "", false, false, "" };
		std::vector<uint8> srcBytes;
		if (!in.readStream(name, srcBytes)) { results.push_back(r); continue; }
		r.Exists = true;

		std::vector<uint8> rtBytes;
		r.T1Ok = t1Roundtrip(srcBytes, rtBytes, r.T1Info);

		if (doT2)
		{
			if (name == "DllDirectory")
			{
				r.T2Applicable = true;
				r.T2Ok = t2DllDirectory(srcBytes, r.T2Info);
				// Also prime dllForT2 for later streams.
				if (r.T2Ok)
				{
					CStorageStream ss(srcBytes); try { dllForT2.serial(ss); dllForT2.parse(VersionUnknown); dllReady = true; } catch (...) {}
				}
			}
			else if (name == "ClassDirectory3" && dllReady)
			{
				r.T2Applicable = true;
				r.T2Ok = t2ClassDirectory3(srcBytes, &dllForT2, r.T2Info);
				if (r.T2Ok)
				{
					CStorageStream ss(srcBytes); try { cdForT2.serial(ss); cdForT2.parse(VersionUnknown); cdReady = true; } catch (...) {}
				}
			}
			else if (name == "Scene" && dllReady && cdReady)
			{
				r.T2Applicable = true;
				r.T2Ok = t2Scene(srcBytes, &reg, &dllForT2, &cdForT2, r.T2Info);
			}
		}

		if (!r.T1Ok || (r.T2Applicable && !r.T2Ok)) anyFail = true;
		results.push_back(r);
	}

	// Concise one-line stdout summary + per-stream verbose if requested or on failure.
	std::cout << (anyFail ? "FAIL" : "OK") << " " << maxFile;
	for (std::vector<StreamResult>::const_iterator ri = results.begin(); ri != results.end(); ++ri)
	{
		const StreamResult &r = *ri;
		if (!r.Exists) continue;
		std::cout << " " << r.Name << ":T1=" << (r.T1Ok ? "ok" : "FAIL");
		if (r.T2Applicable) std::cout << ",T2=" << (r.T2Ok ? "ok" : "FAIL");
	}
	std::cout << "\n";
	if (anyFail || verbose)
	{
		for (std::vector<StreamResult>::const_iterator ri = results.begin(); ri != results.end(); ++ri)
		{
			const StreamResult &r = *ri;
			if (!r.Exists) continue;
			if (!r.T1Ok) std::cerr << "  T1 " << r.Name << " FAIL: " << r.T1Info << "\n";
			if (r.T2Applicable && !r.T2Ok) std::cerr << "  T2 " << r.Name << " FAIL: " << r.T2Info << "\n";
		}
	}

	remove(g_tempPath.c_str());
	return anyFail ? 1 : 0;
}
