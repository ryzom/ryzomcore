/**
 * \file wsm_derived_object.h
 * \brief CWSMDerivedObject
 * \date 2026-07-16 16:30GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CWSMDerivedObject
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

#ifndef PIPELINE_WSM_DERIVED_OBJECT_H
#define PIPELINE_WSM_DERIVED_OBJECT_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "derived_object.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CWSMDerivedObject
 * \date 2026-07-16 16:30GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 *
 * The WSM Derived object — ClassId (0x4ec13906, 0x5578130e), fixed container chunk id 0x2033 —
 * the world-space-modifier sibling of the OSM Derived wrapper. The corpus proves the storage
 * format identical to CDerivedObject's (the single corpus instance, sfx/meshtoparticle/
 * medusea.max, carries the same 0x2034 refs + 0x2500 ModApp + empty 0x2501 stream, its one
 * modifier under superclass 0x820), so the whole implementation is shared; consumers
 * dynamic_cast to CDerivedObject to handle both wrapper kinds.
 */
class CWSMDerivedObject : public CDerivedObject
{
public:
	CWSMDerivedObject(CScene *scene);
	virtual ~CWSMDerivedObject();

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

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CWSMDerivedObject */

typedef CSceneClassDesc<CWSMDerivedObject> CWSMDerivedObjectClassDesc;
extern const CWSMDerivedObjectClassDesc WSMDerivedObjectClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_WSM_DERIVED_OBJECT_H */

/* end of file */
