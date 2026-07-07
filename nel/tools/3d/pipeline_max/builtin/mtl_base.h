/**
 * \file mtl_base.h
 * \brief CMtlBase
 * \date 2012-08-22 08:53GMT
 * \author Jan Boon (Kaetemi)
 * CMtlBase
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

#ifndef PIPELINE_MTL_BASE_H
#define PIPELINE_MTL_BASE_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>

// NeL includes

// Project includes
#include "reference_target.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CMtlBase
 * \date 2012-08-22 08:53GMT
 * \author Jan Boon (Kaetemi)
 *
 * The shared base of every material and texmap scene class (MtlBase). Its only universally
 * present payload is the material-base container: chunk 0x4000 wraps the base state, whose
 * 0x4001 child is the material/texmap NAME (UTF-16, sized by the chunk). Older files store the
 * 0x4001 name chunk bare on the object instead of nested in 0x4000. Everything else that
 * distinguishes a concrete material — the shader/maps/extended blocks of a Standard material,
 * the crop/bitmap of a BitmapTex, the NeL-material script flags — lives in ParamBlock2 objects
 * reached through the reference wiring (see CParamBlock2, max_geometry_formats Part I).
 *
 * This class keeps the raw chunks authoritative (parse decodes the name over the orphaned
 * chunks WITHOUT moving them, build re-emits verbatim — byte-exact roundtrip, the §5/§12.2
 * discipline). It gives a consumer (the exporter, a live material editor) a typed handle on
 * every material/texmap with its name, on top of the reference walk (sub-materials, textures)
 * and the typed CParamBlock2 parameters.
 */
class CMtlBase : public CReferenceTarget
{
public:
	CMtlBase(CScene *scene);
	virtual ~CMtlBase();

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const char *InternalNameUnknown;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();
	virtual void init();
	virtual bool inherits(const NLMISC::CClassId classId) const;
	virtual const ISceneClassDesc *classDesc() const;
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const;

	/// True when a name chunk (0x4001, bare or under 0x4000) was found.
	inline bool hasName() const { return m_NameChunk != NULL; }
	/// The material/texmap name (UTF-8), empty when absent. Valid between parse and clean/disown.
	std::string name() const;

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	void decodeName();

	/// The raw UTF-16 name chunk (0x4001), not owned (stays in the orphan list / 0x4000 base).
	CStorageRaw *m_NameChunk;

}; /* class CMtlBase */

typedef CSceneClassDesc<CMtlBase> CMtlBaseClassDesc;
extern const CMtlBaseClassDesc MtlBaseClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_MTL_BASE_H */

/* end of file */
