/**
 * \file save_ops.cpp
 * \brief Whole-file .max save path: write-back, thumbnail, atomic copy.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * \author Claude Sonnet 5
 * \author Grok 4.5
 *
 * `saveWholeFile` (typed graph -> Scene stream -> OLE writer with other streams verbatim),
 * `captureTopDownThumbnail` + `prepareThumbnailOverride` (SI stream refresh), and the
 * `zpSaveTo` / `zpSaveOverwrite` / per-file overwrite+copy handlers that panel actions
 * and the script bridge share. Atomicity via saveCopyAtomic (temp+rename + one-time .bak).
 */

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif
#include <nel/misc/aabbox.h>
#include <nel/misc/app_context.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/cmd_args.h>
#include <nel/misc/common.h>
#include <nel/misc/config_file.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/event_server.h>
#include <nel/misc/events.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/path.h>
#include <nel/misc/time_nl.h>

#include <nel/3d/camera.h>
#include <nel/3d/driver_user.h>
#include <nel/3d/dru.h>
#include <nel/3d/event_mouse_listener.h>
#include <nel/3d/font_manager.h>
#include <nel/3d/landscape.h>
#include <nel/3d/landscape_model.h>
#include <nel/3d/register_3d.h>
#include <nel/3d/scene.h>
#include <nel/3d/scene_user.h>
#include <nel/3d/text_context.h>
#include <nel/3d/texture_mem.h>
#include <nel/3d/tile_bank.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/viewport.h>
#include <nel/3d/zone.h>
#include <nel/3d/zone_corner_smoother.h>
#include <nel/3d/zone_symmetrisation.h>

#include <nel/ligo/ligo_config.h>
#include <nel/ligo/ligo_error.h>
#include <nel/ligo/zone_template.h>
#include <nel/ligo/zone_region.h>
#include <nel/ligo/zone_bank.h>

#include "../pipeline_max/storage_ole.h"
#include "max_thumbnail.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef NL_OS_WINDOWS
#include <process.h>
#define ZP_GETPID _getpid
#else
#include <unistd.h>
#define ZP_GETPID getpid
#endif

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/storage_value.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"

#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"
#include "../pipeline_max/nelpatch/nelpatch.h"
#include "../pipeline_max/nelpatch/rkl_patch_object.h"

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/control_transform.h"

#include "../pipeline_max_export_common/max_math.h"
#include "../pipeline_max_export_common/max_scene.h"
#include "../pipeline_max_export_common/max_load.h"
#include "../pipeline_max_export_common/db_path.h"
#include "../pipeline_max_export_common/appdata_util.h"
#include "../pipeline_max_export_common/export_ids.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::NELPATCH;
using namespace MAXMATH;

#include "../pipeline_max_export_common/patch_eval.h"

#include "paint_core.h"
#include "context_display.h"
#include "editor_ui.h"

#include <nel/gui/ctrl_base.h>
#include <nel/gui/ctrl_base_button.h>
#include <nel/gui/ctrl_text_button.h>
#include <nel/gui/widget_manager.h>

#include "workspace_discovery.h"
#include "startup_ui.h"
#include "script_api.h"

#include "zp_state.h"

// ---------------------------------------------------------------------------------------------
// Whole-file save: rebuilt Scene stream + every other stream verbatim + OLE class id
// (modeled on the corpus harness' rpoModifySaveTest). The caller mutates the parsed scene
// (paint write-back) BEFORE calling; a null edit through this same path is byte-identical.

// Serialize a container to a temp file and read the file bytes back. CMemStream's write-mode
// seek-back fails during leaveChunk; COFile handles seeks freely, so temp-file roundtrip is
// the working pattern (same as the corpus harness).
std::vector<uint8> writeContainerToTemp(CStorageContainer &ctr, const std::string &tempPath)
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

/**
 * Whole-file save. Non-Scene streams come from `input` verbatim unless
 * summaryOverride is non-NULL, in which case SummaryInformation is replaced
 * (thumbnail write). --null-edit / plain --save pass NULL so SI is untouched.
 */
int saveWholeFile(const std::string &input, const std::string &output, CScene &scene,
                         bool verifyIdentical,
                         const std::vector<uint8> *summaryOverride)
{
	// The known .max stream set (same list as the corpus harness save tests).
	static const char *kStreams[] = {
		"VideoPostQueue", "Config", "ClassData", "DllDirectory", "ClassDirectory3", "Scene",
		"\05SummaryInformation", "\05DocumentSummaryInformation", NULL
	};
	std::vector<std::string> present;
	std::vector<std::vector<uint8> > rawOrig;
	uint8 classId[16];
	bool haveClassId;
	{
		CStorageOleIn in;
		if (!in.open(input)) { fprintf(stderr, "ERROR: not an OLE compound file: %s\n", input.c_str()); return 1; }
		for (const char **n = kStreams; *n; ++n)
		{
			std::vector<uint8> b;
			if (in.readStream(*n, b)) { present.push_back(*n); rawOrig.push_back(b); }
		}
		// Carry any root-level stream OUTSIDE the known .max set verbatim (review finding:
		// they were silently dropped, and the per-stream verify could not see the loss).
		// Appended AFTER the known list so the corpus files' data layout (which the
		// whole-file byte gate depends on) is untouched when no unknown streams exist.
		const std::vector<std::string> &allNames = in.streamNames();
		for (size_t i = 0; i < allNames.size(); ++i)
		{
			bool known = false;
			for (const char **n = kStreams; *n && !known; ++n)
				known = allNames[i] == *n;
			if (known)
				continue;
			std::vector<uint8> b;
			if (in.readStream(allNames[i], b))
			{
				fprintf(stderr, "WARNING: unknown stream '%s' (%u bytes) carried verbatim "
				        "(container layout may differ from the source)\n",
				        (allNames[i][0] == '\05' ? allNames[i].substr(1) : allNames[i]).c_str(),
				        (uint)b.size());
				present.push_back(allNames[i]);
				rawOrig.push_back(b);
			}
		}
		haveClassId = in.getClassId(classId);
	}

	// If we have a new SI and the stream was absent, append it.
	const std::string kSI = ZPTHUMB::kSummaryInformationStream;
	bool haveSI = false;
	for (size_t i = 0; i < present.size(); ++i)
		if (present[i] == kSI) { haveSI = true; break; }
	if (summaryOverride && !haveSI)
	{
		present.push_back(kSI);
		rawOrig.push_back(std::vector<uint8>()); // placeholder; overridden below
	}

	// Rebuild the Scene stream from the typed graph and write the whole file.
	// Temp path lives under the platform temp dir - CPath::getTemporaryDirectory
	// returns a trailing-slash form on both POSIX and Windows.
	std::string tempPath = NLMISC::CPath::getTemporaryDirectory()
	                       + NLMISC::toString("zone_painter.%d.tmp", (int)ZP_GETPID());
	std::vector<uint8> newScene;
	try
	{
		scene.clean();
		scene.build(VersionUnknown);
		scene.disown();
		newScene = writeContainerToTemp(scene, tempPath);
		// Put the LIVE scene back into its parsed state. The cycle above leaves it
		// disowned (the as-serialized form a fresh load reads), which clears every
		// parsed-side handle - the RklPatch claimed runs among them - so without this
		// the session's next carrier write-back fails ("setRPatch failed"): editing
		// after an in-session save was broken. Typed instances are created at serial
		// time and survive the cycle; parse only re-expands their state, so live node
		// and object pointers held by the session stay valid.
		scene.parse(VersionUnknown);
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "ERROR: scene rebuild: %s\n", e.what());
		remove(tempPath.c_str());
		return 1;
	}
	remove(tempPath.c_str());

	{
		CStorageOleOut out;
		for (size_t i = 0; i < present.size(); ++i)
		{
			if (present[i] == "Scene")
				out.addStream("Scene", newScene);
			else if (summaryOverride && present[i] == kSI)
				out.addStream(present[i], *summaryOverride);
			else
				out.addStream(present[i], rawOrig[i]);
		}
		if (haveClassId) out.setClassId(classId);
		if (!out.write(output)) { fprintf(stderr, "ERROR: cannot create %s\n", output.c_str()); return 1; }
	}

	uint diffs = 0;
	if (verifyIdentical)
	{
		CStorageOleIn in2;
		if (!in2.open(output)) { fprintf(stderr, "ERROR: cannot reopen %s\n", output.c_str()); return 1; }
		for (size_t i = 0; i < present.size(); ++i)
		{
			std::vector<uint8> b2;
			in2.readStream(present[i], b2);
			const std::vector<uint8> &expect =
				(summaryOverride && present[i] == kSI) ? *summaryOverride : rawOrig[i];
			if (b2 != expect)
			{
				fprintf(stderr, "ERROR: stream %s NOT byte-identical (%u -> %u bytes)\n",
				        (present[i][0] == '\05' ? present[i].substr(1) : present[i]).c_str(),
				        (uint)expect.size(), (uint)b2.size());
				++diffs;
			}
		}
	}
	if (verifyIdentical)
		printf("%s null-edit: %u stream diffs -> %s\n", diffs ? "FAIL" : "OK", diffs, output.c_str());
	else
		printf("OK save -> %s%s\n", output.c_str(), summaryOverride ? " (thumbnail updated)" : "");
	return diffs ? 1 : 0;
}



/**
 * Top-down orthographic capture of ONE file's zones into an OFFSCREEN render target
 * . Conventions follow the ligo plugin's MakeSnapShot - the zone-imaging
 * reference (nel/tools/3d/ligo/plugin_max/script.cpp; the ring pipeline's
 * screenshot_islands tiles the backbuffer for the same north-up ortho look):
 *   - ortho frustum over the CELL-ALIGNED rect of the zones (not the raw bbox), so
 *     thumbnails of adjacent bricks tile edge-to-edge like the ligo zone bitmaps;
 *   - north-up: X right, Y up on screen, camera looking straight down -Z;
 *   - repeated renders so landscape refine converges on the jumped camera.
 * Render target: the BACKBUFFER, used as scratch and never swapped - the next live
 * frame clears and fully redraws before any swap, and headless save flows never swap
 * at all. Capture resolution is therefore the window size (any CTextureMem "RT" on the
 * classic GL driver still rasterizes into the backbuffer, so an explicit FBO would only
 * add a wasted per-save VRAM upload). The readback is resampled to the cell rect's
 * aspect below and the SI encoder's 128px maxDim pass makes the final thumbnail.
 * zoneIds: the file's zone ids; NULL = every unfrozen zone (legacy single-file path -
 * NOTE: unfrozen includes instance CLONES, so NULL over-frames once instances exist;
 * per-file save paths must pass the file's ids).
 */
bool captureTopDownThumbnail(NLMISC::CBitmap &out, const std::vector<uint> *zoneIds)
{
	out.reset();
	if (!g_PaintCtx.UDriver || !g_PaintCtx.UScene || !g_PaintCtx.Land || !g_PaintCtx.Camera
	    || !g_PaintCtx.Zones || g_PaintCtx.Zones->empty())
		return false;

	// Frame the selected zones (a file's set), or every unfrozen zone. Two passes for
	// the id-selected form: frozen members are skipped first (--embedded-context puts
	// frozen embedded neighbor COPIES in a file's id range - framing them captured the
	// whole neighborhood); an all-frozen set (an RO-demoted file) falls back to
	// framing what it has.
	NLMISC::CAABBox bbox;
	bool init = false;
	for (int pass = 0; pass < 2 && !init; ++pass)
	{
		for (size_t i = 0; i < g_PaintCtx.Zones->size(); ++i)
		{
			const SPaintZone &z = (*g_PaintCtx.Zones)[i];
			if (zoneIds)
			{
				bool in = false;
				for (size_t k = 0; k < zoneIds->size() && !in; ++k)
					in = (*zoneIds)[k] == z.ZoneId;
				if (!in)
					continue;
				if (pass == 0 && z.Frozen)
					continue;
			}
			else if (z.Frozen)
				continue;
			for (size_t p = 0; p < z.Patches.size(); ++p)
			{
				const NL3D::CBezierPatch &bp = z.Patches[p].Patch;
				for (uint v = 0; v < 4; ++v)
				{
					if (!init) { bbox.setCenter(bp.Vertices[v]); bbox.setHalfSize(NLMISC::CVector::Null); init = true; }
					else bbox.extend(bp.Vertices[v]);
				}
				for (uint v = 0; v < 8; ++v) bbox.extend(bp.Tangents[v]);
				for (uint v = 0; v < 4; ++v) bbox.extend(bp.Interiors[v]);
			}
		}
		if (!zoneIds)
			break; // the unfiltered form never frames frozen zones
	}
	if (!init)
		return false;

	// Cell-aligned region (ligo: posX = CellSize*xmin, width = CellSize*cells)
	const float cs = g_SessionCellSize > 0.f ? g_SessionCellSize : 160.f;
	const float minX = (float)std::floor(bbox.getMin().x / cs + 1e-4) * cs;
	const float minY = (float)std::floor(bbox.getMin().y / cs + 1e-4) * cs;
	const float maxX = (float)std::ceil(bbox.getMax().x / cs - 1e-4) * cs;
	const float maxY = (float)std::ceil(bbox.getMax().y / cs - 1e-4) * cs;
	const int cellsW = std::max(1, (int)std::lround((maxX - minX) / cs));
	const int cellsH = std::max(1, (int)std::lround((maxY - minY) / cs));
	const float rectW = (float)cellsW * cs;
	const float rectH = (float)cellsH * cs;
	const float cx = minX + rectW * 0.5f;
	const float cy = minY + rectH * 0.5f;
	const float zTop = bbox.getMax().z + std::max(bbox.getHalfSize().z * 2.f, 50.f);

	// Output budget: ≈256 px per cell, longest side capped at 1024 (the SI encoder's
	// 128px maxDim pass makes the final thumb from this)
	const uint kPerCell = 256;

	NL3D::UDriver *udriver = g_PaintCtx.UDriver;
	NL3D::UScene *uscene = g_PaintCtx.UScene;
	NL3D::IDriver *driver = static_cast<NL3D::CDriverUser *>(udriver)->getDriver();

	NL3D::CCamera *camera = g_PaintCtx.Camera;
	const NLMISC::CMatrix oldMat = camera->getMatrix();
	const NL3D::CFrustum oldFrust = camera->getFrustum();

	// Ortho top-down, north-up (ligo: view.setRot(I, -K, J) - same basis)
	NLMISC::CMatrix camMat;
	camMat.identity();
	camMat.setRot(NLMISC::CVector(1, 0, 0), NLMISC::CVector(0, 0, -1), NLMISC::CVector(0, 1, 0), true);
	camMat.setPos(NLMISC::CVector(cx, cy, zTop));
	camera->setTransformMode(NL3D::ITransformable::DirectMatrix);
	camera->setMatrix(camMat);
	camera->setFrustum(-rectW * 0.5f, rectW * 0.5f, -rectH * 0.5f, rectH * 0.5f,
	                   0.1f, zTop - bbox.getMin().z + 100.f, /*perspective=*/false);

	// Render with refine convergence (ligo renders repeatedly for the same reason);
	// no swapBuffers anywhere - the scribbled backbuffer is redrawn before any swap.
	g_PaintCtx.Land->Landscape.setRefineMode(true);
	udriver->clearBuffers(NLMISC::CRGBA(40, 40, 40));
	uscene->render();
	g_PaintCtx.Land->Landscape.setRefineMode(false);
	g_PaintCtx.Land->Landscape.refineAll(camMat.getPos());
	for (int pass = 0; pass < 3; ++pass)
	{
		udriver->clearBuffers(NLMISC::CRGBA(40, 40, 40));
		uscene->render();
	}

	NLMISC::CBitmap full;
	driver->getBuffer(full); // window-sized backbuffer readback

	// Restore camera + live refine mode
	camera->setMatrix(oldMat);
	camera->setFrustum(oldFrust);
	g_PaintCtx.Land->Landscape.setRefineMode(true);

	if (full.getWidth() == 0 || full.getHeight() == 0)
		return false;
	if (full.getPixelFormat() != NLMISC::CBitmap::RGBA)
		full.convertToType(NLMISC::CBitmap::RGBA);
	// Undo the pow2 stretch: resample to the cell rect's true aspect at the oversampled
	// budget; the SI encoder's maxDim pass makes the final thumb.
	uint outW = kPerCell * (uint)cellsW, outH = kPerCell * (uint)cellsH;
	const uint maxSide = std::max(outW, outH);
	if (maxSide > 1024) { outW = outW * 1024 / maxSide; outH = outH * 1024 / maxSide; }
	if (outW && outH && (outW != full.getWidth() || outH != full.getHeight()))
		full.resample(outW, outH);
	out = full;
	return out.getWidth() > 0;
}

/** Legacy want-decision for the single-file/CLI paths: --thumbnail or the modal checkbox.
 *  Interactive board saves pass want=true explicitly (thumbnails always). */
bool zpLegacyWantThumbnail()
{
	if (g_PaintCtx.InteractiveSave)
	{
		if (ZPUI::SPaintUIBridge *b = ZPUI::getPaintUIBridge())
			g_PaintCtx.WantThumbnail = b->UpdateThumbnail;
	}
	return g_PaintCtx.WantThumbnail || g_CliWantThumbnail;
}

/** Build optional SI override; no override means leave SI alone.
 *  zoneIds: the saved FILE's zones - per-file framing ; NULL = legacy primary. */
bool prepareThumbnailOverride(const std::string &srcMax, std::vector<uint8> &siOut,
                                     bool &haveOverride, bool want,
                                     const std::vector<uint> *zoneIds)
{
	haveOverride = false;
	siOut.clear();
	if (g_NoThumbnailWrites)
		return true; // --no-thumbnail: byte-pure saves, SI stream always untouched
	if (!want)
		return true; // not requested - OK, no override

	NLMISC::CBitmap bmp;
	if (captureTopDownThumbnail(bmp, zoneIds))
	{
		if (!zoneIds)
		{
			// Legacy stash (screenshot-mode pre-capture pairing); COPY, not swap - a swap
			// would hand the STALE stash to the SI build below.
			g_CapturedThumb = bmp;
			g_HaveCapturedThumb = true;
		}
	}
	else if (g_HaveCapturedThumb && (!zoneIds || g_EditableFiles.size() <= 1))
	{
		// Headless fallback (GL context gone post-viewer): the --screenshot pre-capture
		// stash. Framing-safe for the NULL path and for single-file sessions (the stash
		// is captured with the same first-file framing); multi-file per-id saves must
		// not inherit a whole-scene stash, so they fail to "SI left unchanged" instead.
		bmp = g_CapturedThumb;
	}
	else
	{
		fprintf(stderr, "WARNING: thumbnail: capture failed (need a display - use xvfb-run and "
		        "--screenshot or interactive save); SI left unchanged\n");
		return true; // save continues without thumb update
	}
	std::string err;
	if (!ZPTHUMB::buildSummaryInformationWithThumbnail(srcMax, bmp, siOut, 128, true, &err))
	{
		fprintf(stderr, "WARNING: thumbnail: %s - SI left unchanged\n", err.c_str());
		return true;
	}
	haveOverride = true;
	printf("OK thumbnail: %ux%u -> SummaryInformation (%s)\n", bmp.getWidth(), bmp.getHeight(),
	       NLMISC::CFile::getFilenameWithoutExtension(srcMax).c_str());
	return true;
}

/** Scene for one editable file (primary uses g_PaintCtx.Scene; extras keep their Lm). */
PIPELINE::MAX::CScene *editableScene(const SEditableFileInfo &efi)
{
	if (efi.Lm)
		return efi.Lm->Scene;
	return g_PaintCtx.Scene;
}

/** Count dirty editable files (panel indicator). */
uint countDirtyEditableFiles()
{
	if (!g_PaintCtx.Core) return 0;
	uint n = 0;
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
		if (g_PaintCtx.Core->anyZoneDirty(g_EditableFiles[i].ZoneIds))
			++n;
	return n;
}

/**
 * Atomic overwrite of one path: temp → optional one-time .bak → rename.
 * Caller has already writeBack'd. Uses `src` for non-Scene OLE streams and `scene` for Scene.
 */
bool saveOneOverwrite(const std::string &orig, PIPELINE::MAX::CScene &scene, bool doThumb,
                             const std::vector<uint> *zoneIds)
{
	if (orig.empty() || !NLMISC::CFile::fileExists(orig))
	{
		fprintf(stderr, "ERROR: overwrite: original missing: %s\n", orig.c_str());
		return false;
	}
	std::string dir = NLMISC::CFile::getPath(orig);
	std::string base = NLMISC::CFile::getFilename(orig);
	std::string tempPath = dir + NLMISC::toString(".zone_painter_save_%d_%s.tmp",
	                                              (int)ZP_GETPID(), base.c_str());
	if (NLMISC::CFile::fileExists(tempPath))
		NLMISC::CFile::deleteFile(tempPath);

	std::vector<uint8> siOverride;
	bool haveSi = false;
	prepareThumbnailOverride(orig, siOverride, haveSi, doThumb, zoneIds);
	int saveRc = saveWholeFile(orig, tempPath, scene, false, haveSi ? &siOverride : NULL);
	if (saveRc != 0 || !NLMISC::CFile::fileExists(tempPath))
	{
		fprintf(stderr, "ERROR: overwrite: temp write failed for %s\n", orig.c_str());
		if (NLMISC::CFile::fileExists(tempPath))
			NLMISC::CFile::deleteFile(tempPath);
		return false;
	}
	std::string bakPath = orig + ".bak";
	if (!NLMISC::CFile::fileExists(bakPath))
	{
		if (!NLMISC::CFile::copyFile(bakPath, orig, /*failIfExists=*/true))
		{
			fprintf(stderr, "ERROR: overwrite: could not create %s\n", bakPath.c_str());
			NLMISC::CFile::deleteFile(tempPath);
			return false;
		}
		printf("OK backup -> %s\n", bakPath.c_str());
	}
	else
		printf("backup kept (already exists): %s\n", bakPath.c_str());

	if (!NLMISC::CFile::moveFile(orig, tempPath))
	{
		if (!NLMISC::CFile::copyFile(orig, tempPath, /*failIfExists=*/false)
		    || !NLMISC::CFile::deleteFile(tempPath))
		{
			fprintf(stderr, "ERROR: overwrite: rename/copy failed for %s (temp %s)\n",
			        orig.c_str(), tempPath.c_str());
			return false;
		}
	}
	if (NLMISC::CFile::fileExists(tempPath))
		NLMISC::CFile::deleteFile(tempPath);
	printf("OK save (overwrite) -> %s\n", orig.c_str());
	return true;
}

/** Absolute + standardized file path - the ONE identity form for path compares.
 *  standardizePath alone does not absolutize, so a file opened by relative path
 *  compared unequal to its own absolute path (dup-open false-refusals, copy-over-
 *  open-file guard bypass). */
std::string absFilePath(const std::string &path)
{
	std::string p = NLMISC::CPath::standardizePath(
		NLMISC::CPath::makePathAbsolute(path, NLMISC::CPath::getCurrentPath(), true), false);
	// makePathAbsolute internally standardizes WITH a final slash (it is directory-
	// oriented) and standardizePath(_, false) only declines to ADD one - strip it,
	// this identity form is for files.
	while (p.size() > 1 && p[p.size() - 1] == '/')
		p.resize(p.size() - 1);
	return p;
}

/** Open editable whose on-disk path equals `path` (absolute compare), else NULL.
 *  Copy-save targets must never silently land on an open file: the write would bypass
 *  the temp → .bak → rename discipline AND leave the in-memory scene stale vs disk. */
SEditableFileInfo *findEditableByPath(const std::string &path)
{
	const std::string t = absFilePath(path);
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
		if (absFilePath(g_EditableFiles[i].Path) == t)
			return &g_EditableFiles[i];
	return NULL;
}

/** Copy-save write: temp in the target's directory, then rename over target - a
 *  mid-write failure (disk full, crash) must not leave a destroyed target. The copy
 *  path has no .bak (the confirm click is the overwrite consent), so atomicity is the
 *  only protection an existing target gets. */
bool saveCopyAtomic(const std::string &src, const std::string &target,
                           PIPELINE::MAX::CScene &scene, const std::vector<uint8> *si)
{
	std::string dir = NLMISC::CFile::getPath(target);
	if (!dir.empty() && dir[dir.size() - 1] != '/' && dir[dir.size() - 1] != '\\')
		dir += "/";
	const std::string tempPath = dir + NLMISC::toString(".zone_painter_copy_%d_%s.tmp",
	                                                    (int)ZP_GETPID(),
	                                                    NLMISC::CFile::getFilename(target).c_str());
	if (saveWholeFile(src, tempPath, scene, false, si) != 0)
	{
		if (NLMISC::CFile::fileExists(tempPath))
			NLMISC::CFile::deleteFile(tempPath);
		return false;
	}
	if (!NLMISC::CFile::moveFile(target, tempPath))
	{
		if (!NLMISC::CFile::copyFile(target, tempPath, /*failIfExists=*/false))
		{
			fprintf(stderr, "ERROR: save: rename/copy failed for %s (temp %s)\n",
			        target.c_str(), tempPath.c_str());
			return false;
		}
		// Copy landed the full bytes - a leftover temp (locked-file corner) must not
		// turn a completed save into a reported failure.
		if (!NLMISC::CFile::deleteFile(tempPath))
			fprintf(stderr, "WARNING: save: temp file left behind: %s\n", tempPath.c_str());
	}
	if (NLMISC::CFile::fileExists(tempPath))
		NLMISC::CFile::deleteFile(tempPath);
	return true;
}

/**
 * Write-back + whole-file save to `target`. Single save implementation for panel modal,
 * --save, and --panel-save-test. Non-Scene OLE streams are read from InputPath (the opened
 * editable file); the Scene stream is rebuilt from the mutated Max scene.
 * Thumbnail write only when WantThumbnail/CLI --thumbnail is set .
 *
 * CLI multi-file : errors if more than one editable file is dirty (single-path --save).
 */
bool zpSaveTo(const std::string &target)
{
	g_LastSaveStatus.clear();
	if (!g_PaintCtx.Core || !g_PaintCtx.Scene)
	{
		g_LastSaveStatus = "save: no paint core/scene";
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	if (target.empty())
	{
		g_LastSaveStatus = "save: empty target path";
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	if (const SEditableFileInfo *open = findEditableByPath(target))
	{
		g_LastSaveStatus = "save: target is the open file '" + open->Basename
		                   + "' - use Overwrite (temp + .bak), not a copy over it";
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	// Multi-file: --save is single-path only
	if (g_EditableFiles.size() > 1)
	{
		uint dirty = countDirtyEditableFiles();
		if (dirty > 1)
		{
			g_LastSaveStatus = "save: multiple dirty editable files - use interactive save-all "
			                   "(Overwrite), not --save <one.path>";
			fprintf(stderr, "ERROR: %s (%u dirty of %u)\n",
			        g_LastSaveStatus.c_str(), dirty, (uint)g_EditableFiles.size());
			return false;
		}
	}
	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_LastSaveStatus = "write-back: " + err;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	// board-session saves stamp neighbor-hints appdata (legacy --save does not)
	writeNeighborHintsIfBoardSession();
	// Pick the (only) dirty file's scene when multi; else primary
	const SEditableFileInfo *srcFile = NULL;
	if (!g_EditableFiles.empty())
	{
		for (size_t i = 0; i < g_EditableFiles.size(); ++i)
		{
			if (g_PaintCtx.Core->anyZoneDirty(g_EditableFiles[i].ZoneIds)
			    || g_EditableFiles.size() == 1)
			{
				srcFile = &g_EditableFiles[i];
				if (g_EditableFiles.size() > 1)
					break; // first dirty
			}
		}
		if (!srcFile)
			srcFile = &g_EditableFiles[0];
	}
	const std::string &srcForStreams = srcFile ? srcFile->Path
		: (g_PaintCtx.InputPath.empty() ? target : g_PaintCtx.InputPath);
	PIPELINE::MAX::CScene *scene = srcFile ? editableScene(*srcFile) : g_PaintCtx.Scene;
	std::vector<uint8> siOverride;
	bool haveSi = false;
	// Per-file framing (never fold instance clones in); the headless stash fallback
	// inside prepareThumbnailOverride still applies to single-file sessions.
	prepareThumbnailOverride(srcForStreams, siOverride, haveSi, zpLegacyWantThumbnail(),
	                         srcFile ? &srcFile->ZoneIds : NULL);
	if (!saveCopyAtomic(srcForStreams, target, *scene, haveSi ? &siOverride : NULL))
	{
		g_LastSaveStatus = "save failed -> " + target;
		return false;
	}
	// A COPY save deliberately keeps the file dirty: the file's OWN path did not
	// receive the changes, so per-cell markers and close/quit confirms must still
	// fire. Only the Overwrite paths rebaseline (markZonesSaved).
	g_LastSaveStatus = "OK save -> " + target;
	printf("OK save (panel) -> %s\n", target.c_str());
	{
		SBoardOpScope boardOp;
		recordBoardOp(NLMISC::toString("painter.save(%s)", luaQuote(target).c_str()));
	}
	return true;
}


/** overwrite ONE editable file in place (the board cell "Save as…" dialog's
 *  Overwrite; same temp → .bak → rename as save-all, explicit thumbnail want). */
bool zpSaveFileOverwrite(const std::string &basename, bool wantThumb)
{
	g_LastSaveStatus.clear();
	SEditableFileInfo *efi = findEditableByBasename(basename);
	if (!efi || !g_PaintCtx.Core)
	{
		g_LastSaveStatus = "save: file not open: " + basename;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	if (!efi->Editable)
	{
		// Same guard as save-all and sessionSaveZone - a read-only (RO-toggled) file
		// must never be written in place, even via a stale-bound dialog or dev hook.
		g_LastSaveStatus = "save: read-only file: " + basename;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_LastSaveStatus = "write-back: " + err;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	writeNeighborHintsIfBoardSession();
	PIPELINE::MAX::CScene *scene = editableScene(*efi);
	if (!scene || !saveOneOverwrite(efi->Path, *scene, wantThumb, &efi->ZoneIds))
	{
		g_LastSaveStatus = "overwrite failed -> " + efi->Path;
		return false;
	}
	g_PaintCtx.Core->markZonesSaved(efi->ZoneIds);
	g_LastSaveStatus = "OK overwrite -> " + efi->Path;
	// the modal per-file Overwrite replays as painter.saveZone (the board-cell
	// save; thumbnail preference is not carried - irrelevant under the byte gates'
	// --no-thumbnail, and saveZone's always-thumb default matches interactive use).
	{
		SBoardOpScope boardOp;
		recordBoardOp(NLMISC::toString("painter.saveZone(%s)", luaQuote(basename).c_str()));
	}
	return true;
}

/** save ONE editable file as a copy. name: absolute, or relative to the FILE's
 *  own directory (bricks of a world share it, but resolve per file regardless). */
bool zpSaveFileCopy(const std::string &basename, const std::string &name, bool wantThumb)
{
	g_LastSaveStatus.clear();
	SEditableFileInfo *efi = findEditableByBasename(basename);
	if (!efi || !g_PaintCtx.Core)
	{
		g_LastSaveStatus = "save: file not open: " + basename;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	std::string target = name;
	if (target.empty())
	{
		g_LastSaveStatus = "save: empty target name";
		return false;
	}
	// isAbsolutePath covers Windows drive-letter paths; a bare '/' check only works on Unix
	// and re-prefixed C:/... under the file dir into a mangled relative path.
	if (!NLMISC::CPath::isAbsolutePath(target))
	{
		std::string dir = NLMISC::CFile::getPath(efi->Path);
		if (!dir.empty() && dir[dir.size() - 1] != '/' && dir[dir.size() - 1] != '\\')
			dir += "/";
		target = dir + target;
	}
	if (const SEditableFileInfo *open = findEditableByPath(target))
	{
		g_LastSaveStatus = "save: target is the open file '" + open->Basename
		                   + "' - use Overwrite (temp + .bak), not a copy over it";
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_LastSaveStatus = "write-back: " + err;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	writeNeighborHintsIfBoardSession();
	PIPELINE::MAX::CScene *scene = editableScene(*efi);
	std::vector<uint8> siOverride;
	bool haveSi = false;
	prepareThumbnailOverride(efi->Path, siOverride, haveSi, wantThumb, &efi->ZoneIds);
	if (!scene || !saveCopyAtomic(efi->Path, target, *scene, haveSi ? &siOverride : NULL))
	{
		g_LastSaveStatus = "save failed -> " + target;
		return false;
	}
	// A COPY save keeps the file dirty - the file's own path did not receive the
	// changes (see zpSaveTo); only Overwrite rebaselines.
	g_LastSaveStatus = "OK save -> " + target;
	printf("OK save (file copy) -> %s\n", target.c_str());
	return true;
}

/** Bridge: directory of one open editable's .max ("" if unknown) - the bound save
 *  dialog resolves relative copy names against this, matching zpSaveFileCopy. */
std::string zpFileDir(const std::string &basename)
{
	SEditableFileInfo *efi = findEditableByBasename(basename);
	return efi ? NLMISC::CFile::getPath(efi->Path) : std::string();
}

/**
 * In-place overwrite: for multi-select  this is save-all - each dirty editable file
 * gets temp → one-time .bak → rename. Single-file path unchanged.
 */

bool zpSaveOverwrite()
{
	SBoardOpScope boardOp;
	const bool ok = zpSaveOverwriteImpl();
	if (ok)
		recordBoardOp("painter.saveAll()");
	return ok;
}
bool zpSaveOverwriteImpl()
{
	g_LastSaveStatus.clear();
	if (!g_PaintCtx.Core || !g_PaintCtx.Scene)
	{
		g_LastSaveStatus = "overwrite: no paint core/scene";
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}

	std::string err;
	if (!g_PaintCtx.Core->writeBack(err))
	{
		g_LastSaveStatus = "write-back: " + err;
		fprintf(stderr, "ERROR: %s\n", g_LastSaveStatus.c_str());
		return false;
	}
	// board-session overwrite stamps neighbor-hints appdata
	writeNeighborHintsIfBoardSession();

	// interactive saves always refresh thumbnails; headless/CLI keeps the opt-in.
	const bool wantThumb = g_PaintCtx.InteractiveSave ? true : zpLegacyWantThumbnail();

	// Single-file legacy path when g_EditableFiles empty/one and only InputPath known
	if (g_EditableFiles.size() <= 1)
	{
		const std::string &orig = g_EditableFiles.empty() ? g_PaintCtx.InputPath
		                                                  : g_EditableFiles[0].Path;
		PIPELINE::MAX::CScene *scene = g_EditableFiles.empty() ? g_PaintCtx.Scene
		                                                       : editableScene(g_EditableFiles[0]);
		// Per-file ids even here: NULL frames every unfrozen zone, which includes
		// instance CLONES - a single open brick with placed instances embedded a
		// whole-board thumbnail. The stash fallback for headless flows still applies
		// (prepareThumbnailOverride allows it whenever the session is single-file).
		if (!saveOneOverwrite(orig, *scene, wantThumb,
		                      g_EditableFiles.empty() ? NULL : &g_EditableFiles[0].ZoneIds))
		{
			g_LastSaveStatus = "overwrite failed -> " + orig;
			return false;
		}
		if (!g_EditableFiles.empty())
			g_PaintCtx.Core->markZonesSaved(g_EditableFiles[0].ZoneIds);
		g_LastSaveStatus = "OK overwrite -> " + orig;
		return true;
	}

	// Multi: save each dirty file. Board sessions also rewrite clean editables when
	// neighbor hints changed (always write editables after stamp - paint dirty OR board).
	// DELIBERATE under --no-hint-stamp too: the byte gates save a clean session and
	// compare against null-edit output - skipping clean files would make them vacuous.
	uint saved = 0, skipped = 0;
	for (size_t i = 0; i < g_EditableFiles.size(); ++i)
	{
		SEditableFileInfo &efi = g_EditableFiles[i];
		const bool dirty = g_PaintCtx.Core->anyZoneDirty(efi.ZoneIds);
		if (!dirty && !g_BoardSession)
		{
			++skipped;
			continue;
		}
		if (!efi.Editable)
		{
			++skipped;
			continue;
		}
		PIPELINE::MAX::CScene *scene = editableScene(efi);
		// EVERY saved file gets its own per-zone thumbnail (was primary-only -
		// non-primary bricks edited on the board kept stale embedded thumbs forever).
		if (!saveOneOverwrite(efi.Path, *scene, wantThumb, &efi.ZoneIds))
		{
			g_LastSaveStatus = "overwrite failed -> " + efi.Path;
			return false;
		}
		g_PaintCtx.Core->markZonesSaved(efi.ZoneIds);
		++saved;
	}
	if (saved == 0)
	{
		g_LastSaveStatus = "overwrite: nothing dirty";
		printf("save-all: nothing dirty (%u files)\n", (uint)g_EditableFiles.size());
		return true;
	}
	g_LastSaveStatus = NLMISC::toString("OK save-all: %u file(s) (%u clean)", saved, skipped);
	printf("%s\n", g_LastSaveStatus.c_str());
	return true;
}


/** Panel Save when --save was given: one-click direct write to SavePath (no modal). */
void zpSaveDirect()
{
	if (!g_PaintCtx.Active || !g_PaintCtx.Core || !g_PaintCtx.Scene) return;
	if (g_PaintCtx.SavePath.empty())
	{
		fprintf(stderr, "WARNING: save: no --save path given\n");
		return;
	}
	zpSaveTo(g_PaintCtx.SavePath);
}


/** Save one editable file in place (writeBack already done by caller optional). */
bool sessionSaveOneFile(SEditableFileInfo &efi, std::string &err)
{
	if (!g_PaintCtx.Core)
	{
		err = "no paint core";
		return false;
	}
	// Board sessions always rewrite to stamp neighbor hints (even if paint clean).
	if (!g_PaintCtx.Core->anyZoneDirty(efi.ZoneIds) && !g_BoardSession)
	{
		err = "not dirty";
		return true; // no-op success
	}
	std::string wbErr;
	if (!g_PaintCtx.Core->writeBack(wbErr))
	{
		err = "write-back: " + wbErr;
		return false;
	}
	if (g_BoardSession)
		writeNeighborHintsIfBoardSession();
	PIPELINE::MAX::CScene *scene = editableScene(efi);
	if (!scene)
	{
		err = "no scene";
		return false;
	}
	// Per-cell board saves refresh the file's own thumbnail too (renders the
	// backbuffer between presented frames - never swapped, so nothing flickers).
	if (!saveOneOverwrite(efi.Path, *scene, /*doThumb=*/true, &efi.ZoneIds))
	{
		err = "overwrite failed";
		return false;
	}
	g_PaintCtx.Core->markZonesSaved(efi.ZoneIds);
	return true;
}


