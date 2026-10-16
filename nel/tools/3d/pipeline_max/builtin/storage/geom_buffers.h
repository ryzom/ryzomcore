/**
 * \file geom_buffers.h
 * \brief CGeomBuffers
 * \date 2012-08-25 07:55GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 * \author Claude Fable 5
 * CGeomBuffers
 */

/*
 * Copyright (C) 2012  by authors
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

#ifndef PIPELINE_GEOM_BUFFERS_H
#define PIPELINE_GEOM_BUFFERS_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/vector.h>

// Project includes
#include "../../storage_object.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

struct CGeomTriIndex
{
	uint32 a;
	uint32 b;
	uint32 c;
	void serial(NLMISC::IStream &stream);
	std::string toString() const;
};

struct CGeomTriIndexInfo
{
	uint32 a;
	uint32 b;
	uint32 c;
	uint32 alwaysOne;
	uint32 smoothingGroups;
	void serial(NLMISC::IStream &stream);
	std::string toString() const;
};

struct CGeomPolyVertexInfo
{
	uint32 i1;
	NLMISC::CVector v;
	void serial(NLMISC::IStream &stream);
	std::string toString() const;
};

struct CGeomPolyEdgeInfo
{
	uint32 i1;
	uint32 a;
	uint32 b;
	void serial(NLMISC::IStream &stream);
	std::string toString() const;
};

struct CGeomPolyFaceInfo
{
	CGeomPolyFaceInfo();
	/// Vertex indices in the vertex buffer
	std::vector<uint32> Vertices;
	// Bitfield (implicitly stored)
	/// Unknown 01 00 01 00
	uint32 I1;
	// Unknown?
	// Unknown?
	/// Material index in multi-submat
	uint16 Material;
	/// Bitfield with smoothing groups
	uint32 SmoothingGroups;
	/// Cuts at local vertex index to local vertex index
	std::vector<std::pair<uint32, uint32> > Triangulation;
	void serial(NLMISC::IStream &stream);
	std::string toString() const;
};

/**
 * \brief CGeomBuffers
 * \date 2012-08-25 07:55GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 * CGeomBuffers
 */
class CGeomBuffers : public CStorageContainer
{
public:
	CGeomBuffers();
	virtual ~CGeomBuffers() NL_OVERRIDE;

	// inherited
	virtual std::string className() const NL_OVERRIDE;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const NL_OVERRIDE;
	virtual void parse(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void clean() NL_OVERRIDE;
	virtual void build(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void disown() NL_OVERRIDE;

	//! \name Typed geometry access (valid when the typed leaf serializers are enabled — the
	//! PMBS_GEOM_BUFFERS_PARSE default). NULL when the chunk is absent or rode through raw.
	//@{
	/// The tri-mesh vertex array (chunk 0x0914, count-prefixed CVector[]).
	const std::vector<NLMISC::CVector> *triVertices() const;
	/// The tri-mesh face array (chunk 0x0912, count-prefixed CGeomTriIndexInfo[]): a,b,c indices
	/// plus the two per-face dwords. The field names alwaysOne/smoothingGroups are historical
	/// mislabels; the corpus-validated meaning is smGroup at offset 12 and faceFlags (matID in
	/// the high word) at offset 16.
	const std::vector<CGeomTriIndexInfo> *triFaces() const;
	/// The poly-mesh vertex array (chunk 0x0100, count-prefixed CGeomPolyVertexInfo[]): carries
	/// the vertex position plus a per-vertex uint32 the format uses as an internal id. Used by
	/// the EditablePoly path in the shape exporter.
	const std::vector<CGeomPolyVertexInfo> *polyVertices() const;
	/// The poly-mesh face array (chunk 0x011a, CGeomPolyFaceInfo[]) — variable-size records with
	/// vertex list, optional matID / smoothing group / triangulation cuts. Use
	/// CGeomObject::triangulatePolyFace to convert each face to triangles.
	const std::vector<CGeomPolyFaceInfo> *polyFaces() const;
	//@}

	//! \name Typed map channels (tri mesh path)
	//! The map-channel chunk family, repeated per stored channel IN FILE ORDER inside this
	//! container (corpus-established): 0x0959 uint32 channel index (0 = vertex color, 1.. = UVW;
	//! 0..5 observed), 0x2398 uint32 support flag (1 on every corpus instance), 0x2394 count-
	//! prefixed CVector map vertices, 0x2396 count-prefixed CGeomTriIndex map-face corner
	//! triples (parallel to the 0x0912 mesh faces; count equality holds corpus-wide). The
	//! family occurs on EditableMesh objects only; the EditablePoly MNMesh channel storage is
	//! a different id set that stays raw. One Max 3 witness carries a group with no leading
	//! 0x0959 (Channel stays -1); consumers drop it, matching the historical raw read.
	//@{
	struct CMapChannelView
	{
		CMapChannelView() : Channel(-1), SupportFlag(0), HasSupportFlag(false), Verts(NULL), Faces(NULL) { }
		/// 0x0959 value; -1 when the group lacks the announce leaf (one Max 3 corpus witness).
		sint32 Channel;
		/// 0x2398 value (1 corpus-wide), valid when HasSupportFlag.
		uint32 SupportFlag;
		bool HasSupportFlag;
		/// 0x2394 map vertices (UVW as CVector; RGB for channel 0). NULL when the group lacks it.
		const std::vector<NLMISC::CVector> *Verts;
		/// 0x2396 per-face map corner indices into Verts. NULL when the group lacks it.
		const std::vector<CGeomTriIndex> *Faces;
	};
	/// Collect the stored map-channel groups in file order. Valid when the typed leaf
	/// serializers are enabled (the PMBS_GEOM_BUFFERS_PARSE default); raw-form chunks (a scene
	/// that rode through untyped) are not decoded here.
	void mapChannels(std::vector<CMapChannelView> &out) const;
	//@}

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container) NL_OVERRIDE;

}; /* class CGeomBuffers */

} /* namespace STORAGE */
} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_GEOM_BUFFERS_H */

/* end of file */
