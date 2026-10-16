/**
 * \file multi_mtl.h
 * \brief CMultiMtl
 * \date 2012-08-22 08:55GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 * CMultiMtl
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

#ifndef PIPELINE_MULTI_MTL_H
#define PIPELINE_MULTI_MTL_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "mtl_base.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CMultiMtl
 * \date 2012-08-22 08:55GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 *
 * The Multi/Sub-Object material, ClassId {0x200, 0}, superclass 0xc00. It holds N sub-materials:
 * reference 0 is the material's own ParamBlock2, references 1..N are the sub-materials, and the
 * sub-material count rides in chunk 0x4002 (see max_geometry_formats Part I.6). This typed class
 * exposes the sub-material list on top of the CMtlBase name; the raw chunks stay authoritative
 * (byte-exact roundtrip).
 */
class CMultiMtl : public CMtlBase
{
public:
	CMultiMtl(CScene *scene);
	virtual ~CMultiMtl() NL_OVERRIDE;

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual void parse(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void clean() NL_OVERRIDE;
	virtual void build(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void disown() NL_OVERRIDE;
	virtual void init() NL_OVERRIDE;
	virtual bool inherits(const NLMISC::CClassId classId) const NL_OVERRIDE;
	virtual const ISceneClassDesc *classDesc() const NL_OVERRIDE;
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const NL_OVERRIDE;

	//! \name Sub-material access (valid between parse and clean/disown)
	//@{
	/// Declared sub-material count (chunk 0x4002); 0 when absent.
	inline uint numSubMaterials() const { return m_NumSubMaterials; }
	/// Sub-material i (0-based; resolves to reference slot i+1), NULL when out of range.
	CMtlBase *subMaterial(uint i) const;
	//@}

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container) NL_OVERRIDE;

private:
	uint m_NumSubMaterials;

}; /* class CMultiMtl */

typedef CSceneClassDesc<CMultiMtl> CMultiMtlClassDesc;
extern const CMultiMtlClassDesc MultiMtlClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_MULTI_MTL_H */

/* end of file */
