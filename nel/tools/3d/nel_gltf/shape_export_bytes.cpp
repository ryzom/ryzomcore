/**
 * \file shape_export_bytes.cpp
 * \brief See shape_export_bytes.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
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
#include "shape_export_bytes.h"

#include <cstdio>

#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/3d/shape.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/index_buffer.h>

#ifdef NL_OS_WINDOWS
#include <process.h>
#define NLGLTF_SEB_GETPID _getpid
#else
#include <unistd.h>
#define NLGLTF_SEB_GETPID getpid
#endif

using namespace NLMISC;
using namespace NL3D;

namespace NLGLTF {

bool shapeToExportFileBytes(IShape *shape, std::vector<uint8> &out, std::string *err)
{
	// Write export-era stream versions: the reference plugin serialized against the 2004 NL3D
	// (vertex/index buffers in the "old preferred memory" form).
	bool oldVB = CVertexBuffer::SerialOldPreferredMemory;
	bool oldIB = CIndexBuffer::SerialOldPreferredMemory;
	CVertexBuffer::SerialOldPreferredMemory = true;
	CIndexBuffer::SerialOldPreferredMemory = true;
	char tmpPath[256];
	snprintf(tmpPath, sizeof(tmpPath), "/tmp/nel_gltf_shape_bytes.%d.tmp", (int)NLGLTF_SEB_GETPID());
	bool ok = false;
	try
	{
		// COFile-not-CMemStream: CMeshMRMGeom::save uses a seek-back-then-forward pattern to
		// patch its per-lod offsets after writing each lod (mesh_mrm.cpp:1942-1972). CMemStream
		// rejects the forward seek because its `length()` (checked in `seek(begin)`) returns the
		// current write position, not the max ever written — so the writer silently overwrites
		// its own lod-offset placeholders and the resulting file cannot be loaded (see
		// pipeline_max_design.md §9 T2 note for the same limitation on the corpus tester). Route
		// the serialize through a real file to sidestep the CMemStream constraint.
		{
			COFile ofile;
			if (!ofile.open(tmpPath))
			{
				if (err) *err = "cannot open temp file";
				throw NLMISC::Exception("temp open");
			}
			CShapeStream shapeStream(shape);
			shapeStream.serial(ofile);
			ofile.close();
		}
		{
			CIFile ifile;
			if (!ifile.open(tmpPath))
			{
				if (err) *err = "cannot reopen temp file";
				throw NLMISC::Exception("temp reopen");
			}
			out.resize(ifile.getFileSize());
			if (!out.empty())
				ifile.serialBuffer(&out[0], (uint)out.size());
			ifile.close();
		}
		CFile::deleteFile(tmpPath);
		std::string className = shape->getClassName();
		uint32 len = (uint32)out.size();
		// CMeshBase version byte 10 -> 9: current NL3D writes CMeshBase version 10 ("Ryzom Core
		// release check", 2024) which adds no fields over the export-era version 9 — same
		// pure-version-byte class as the zone v4/v5 byte (see pipeline_max_design.md §10h).
		// Stream layout: "SHAP" + u64 0x1 + u32 nameLen + name + classVersion byte + meshBase
		// version byte — CMesh/CMeshMRM/CMeshMRMSkinned write their OWN version byte first, THEN
		// CMeshBase's, so the "+1" skips the outer class version to reach CMeshBase's.
		uint32 meshBaseVerOff = 4 + 8 + 4 + (uint32)className.size() + 1;
		if ((className == "CMesh" || className == "CMeshMRM" || className == "CMeshMRMSkinned")
			&& meshBaseVerOff < len && out[meshBaseVerOff] == 10)
			out[meshBaseVerOff] = 9;
		// CWaterShape: reference-era exports are stream version 4 (2004 plugin build). Our v7
		// adds RealtimeReflection (1 byte) + fresnel bias/scale/power (12 bytes) +
		// EnvMapCalcReflectivity (1 byte) at the END of the serial, in that order. To emit a v4
		// stream from a v7 in-memory shape: patch the version byte + truncate the trailing 14
		// bytes. Safe because those 4 fields sit right at the end and aren't referenced by any
		// later field. For a plain-IShape-derived shape there's no inner class version, so the
		// version byte sits right at the end of the class name (no +1 offset).
		uint32 waterVerOff = 4 + 8 + 4 + (uint32)className.size();
		if (className == "CWaterShape" && waterVerOff < len && out[waterVerOff] == 7 && len > 14)
		{
			out[waterVerOff] = 4;
			out.resize(len - 14);
		}
		ok = true;
	}
	catch (const NLMISC::Exception &e)
	{
		if (err && err->empty()) *err = e.what();
	}
	CVertexBuffer::SerialOldPreferredMemory = oldVB;
	CIndexBuffer::SerialOldPreferredMemory = oldIB;
	return ok;
}

bool writeShapeExportFile(IShape *shape, const std::string &outPath, std::string *err)
{
	std::vector<uint8> bytes;
	if (!shapeToExportFileBytes(shape, bytes, err))
		return false;
	try
	{
		COFile file;
		if (!file.open(outPath))
		{
			if (err) *err = "cannot open " + outPath;
			return false;
		}
		if (!bytes.empty())
			file.serialBuffer(&bytes[0], (uint)bytes.size());
		file.close();
	}
	catch (const NLMISC::Exception &e)
	{
		if (err) *err = e.what();
		return false;
	}
	return true;
}

} /* namespace NLGLTF */

/* end of file */
