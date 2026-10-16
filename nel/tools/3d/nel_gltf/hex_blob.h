/**
 * \file hex_blob.h
 * \brief Hex codec for the NeL glTF blob convention: binary process outputs (nel_igs, nel_anim,
 * nel_skel, ... "data" members) and packed material fields ride JSON strings as lowercase hex.
 * One shared codec — the writer (pipeline_max_export_gltf), the reader (mesh_utils) and the
 * material codec previously each carried their own copy.
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

#ifndef NL_GLTF_HEX_BLOB_H
#define NL_GLTF_HEX_BLOB_H

#include <nel/misc/types_nl.h>

#include <string>
#include <vector>

namespace NLGLTF {

/// Encode bytes as lowercase hex (the emission convention; the decoder accepts either case).
std::string bytesToHex(const uint8 *data, size_t len);
std::string bytesToHex(const std::vector<uint8> &bytes);

/// Decode a hex string (either case). Returns false on odd length or a non-hex character; an
/// empty input decodes to an empty vector (callers that require content check out.empty()).
bool hexToBytes(const std::string &hex, std::vector<uint8> &out);

} /* namespace NLGLTF */

#endif /* NL_GLTF_HEX_BLOB_H */

/* end of file */
