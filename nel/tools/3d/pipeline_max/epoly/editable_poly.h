/**
 * \file editable_poly.h
 * \brief CEditablePoly
 * \date 2012-08-26 12:02GMT
 * \author Jan Boon (Kaetemi)
 * CEditablePoly
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

#ifndef PIPELINE_EDITABLE_POLY_H
#define PIPELINE_EDITABLE_POLY_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../builtin/poly_object.h"

namespace PIPELINE {
namespace MAX {
namespace EPOLY {

/**
 * \brief CEditablePoly
 * \date 2012-08-26 12:02GMT
 * \author Jan Boon (Kaetemi)
 * CEditablePoly
 */
class CEditablePoly : public PIPELINE::MAX::BUILTIN::CPolyObject
{
public:
	CEditablePoly(CScene *scene);
	virtual ~CEditablePoly();

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
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

private:
	TStorageObjectContainer m_EditablePolyUnknown;

}; /* class CEditablePoly */

typedef CSceneClassDesc<CEditablePoly> CEditablePolyClassDesc;
extern const CEditablePolyClassDesc EditablePolyClassDesc;

} /* namespace EPOLY */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_EDITABLE_POLY_H */

/* end of file */
