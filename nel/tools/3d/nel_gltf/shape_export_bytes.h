/**
 * \file shape_export_bytes.h
 * \brief The one .shape serializer of the export toolchain: an in-memory IShape to the exact
 * bytes the reference-era pipeline writes for it. Previously four copies of this logic lived in
 * the direct shape exporter, the glTF writer, the special-shape codec and the glTF reader —
 * byte-parity between the direct and glTF routes hangs on this transformation, so it is defined
 * exactly once.
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

#ifndef NL_GLTF_SHAPE_EXPORT_BYTES_H
#define NL_GLTF_SHAPE_EXPORT_BYTES_H

#include <nel/misc/types_nl.h>

#include <string>
#include <vector>

namespace NL3D {
	class IShape;
}

namespace NLGLTF {

/// Serialize a shape to the exact bytes the export pipeline writes for it: export-era stream
/// flags (SerialOldPreferredMemory, saved/restored around the call), temp-file COFile route
/// (CMeshMRMGeom::save seeks back and forward to patch per-lod offsets — CMemStream rejects the
/// forward seek), then the export-era version patches (CMeshBase v10 -> v9 for the CMesh
/// family; CWaterShape v7 -> v4 + 14-byte truncation) — see the implementation for the full
/// rationale on each. Returns false with *err set on failure.
bool shapeToExportFileBytes(NL3D::IShape *shape, std::vector<uint8> &out, std::string *err);

/// shapeToExportFileBytes + write the bytes to outPath.
bool writeShapeExportFile(NL3D::IShape *shape, const std::string &outPath, std::string *err);

} /* namespace NLGLTF */

#endif /* NL_GLTF_SHAPE_EXPORT_BYTES_H */

/* end of file */
