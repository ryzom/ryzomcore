/**
 * \file gltf_special_shape.h
 * \brief Structural glTF extras codec for the special shape classes (water / remanence /
 * flare) — full parity at the glTF-with-nel-extras level: every field the shape builders set
 * is decomposed into artist-visible nel_* keys, and the reader reconstructs the shape through
 * the same public setter sequence the builders use. Completeness is enforced by construction:
 * the writer rebuilds from its own extras and byte-compares the serialized shape against the
 * directly-built one before dropping the opaque blob (a mismatch falls back to nel_shape_blob
 * with a warning, never silent loss). Schema documented in wiki drafts/nel_gltf_extras.md.
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

#ifndef NL_GLTF_SPECIAL_SHAPE_H
#define NL_GLTF_SPECIAL_SHAPE_H

#include <nel/misc/types_nl.h>

#include <string>
#include <vector>

#include "json_value.h"

namespace NL3D {
	class IShape;
}

namespace NLGLTF {

/// Decompose a built special shape into structural extras. Returns true and sets *classOut
/// ("water" / "remanence" / "flare") when the shape is one of the structural classes; false
/// with *err empty when it is not (caller keeps its fallback), false with *err set on a
/// decompose failure (e.g. a texture class the codec doesn't carry).
bool specialShapeToExtras(NL3D::IShape *shape, CJsonValue &extras, std::string *classOut,
                          std::string *err);

/// Rebuild the shape from structural extras. Returns NULL with *err on failure. The caller
/// owns the returned shape.
NL3D::IShape *specialShapeFromExtras(const CJsonValue &extras, const std::string &shapeClass,
                                     std::string *err);

} /* namespace NLGLTF */

#endif /* NL_GLTF_SPECIAL_SHAPE_H */

/* end of file */
