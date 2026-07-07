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
#include <sstream>
#include <string>
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

#include "../pipeline_max/builtin/param_block_2.h"
#include "../pipeline_max/builtin/mtl_base.h"
#include "../pipeline_max/builtin/multi_mtl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/control_keyframer.h"

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
static std::string g_tempPath = "/tmp/pipeline_max_corpus_test." + std::to_string((long)PMCT_GETPID()) + ".tmp";

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
static int modifySaveTest(CStorageOleIn &in, CSceneClassRegistry *reg, const std::string &tempMax, bool verbose)
{
	// All chunk streams read verbatim for byte-exact write-back of the unmodified ones + the
	// original-vs-rewritten comparison.
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
	int targetKind = 0; // 1 float, 2 int, 3 bool, 4 color
	float newF = 1234.5f; sint32 newI = 0x5eed1234; float newC[3] = { 0.125f, 0.5f, 0.875f };
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
	if (verbose)
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
			extra = " <KEYFRAMER keys=" + std::to_string(kf->keyCount()) + " range=[" + std::to_string(s) + ".." + std::to_string(e) + "]>";
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
						if (st->first == 0x0003 && rw && rw->Value.size() == 4) memcpy(&idx, rw->Value.data(), 4);
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
	bool doModifySave = false;
	bool doMtlDump = false;
	bool doUvgenDump = false;
	const char *dumpScene = NULL;
	const char *maxFile = NULL;
	// Accept flags in ANY position (the corpus drivers historically appended --parse after the
	// file path — with a leading-flags-only parser that silently turned T2 into a no-op).
	for (int i = 1; i < argc; ++i)
	{
		std::string a = argv[i];
		if (a == "--parse") doT2 = true;
		else if (a == "--verbose" || a == "-v") verbose = true;
		else if (a == "--pb2-selftest") doPb2SelfTest = true;
		else if (a == "--modify-save-test") doModifySave = true;
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
		std::cerr << "usage: pipeline_max_corpus_test [--parse] [--verbose] [--pb2-selftest] [--modify-save-test] <input.max>\n";
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

	if (doModifySave)
	{
		std::string tempMax = "/tmp/pipeline_max_modify_save." + std::to_string((long)PMCT_GETPID()) + ".max";
		int rc = modifySaveTest(in, &reg, tempMax, verbose);
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
		if (f) { if (!bytes.empty()) fwrite(bytes.data(), 1, bytes.size(), f); fclose(f); }
		std::cout << "wrote " << bytes.size() << " bytes to " << dumpScene << "\n";
		remove(g_tempPath.c_str());
		return 0;
	}

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

	remove(g_tempPath.c_str());
	return anyFail ? 1 : 0;
}
