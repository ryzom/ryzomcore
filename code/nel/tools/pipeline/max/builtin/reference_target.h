/**
 * \file reference_target.h
 * \brief CReferenceTarget
 * \date 2012-08-22 08:53GMT
 * \author Jan Boon (Kaetemi)
 * CReferenceTarget
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

#ifndef PIPELINE_REFERENCE_TARGET_H
#define PIPELINE_REFERENCE_TARGET_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/smart_ptr.h>

// Project includes
#include "reference_maker.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CReferenceTarget
 * \date 2012-08-22 08:53GMT
 * \author Jan Boon (Kaetemi)
 * This class counts the reference. It is recommended to use CRefPtr<T>
 * to refer to any pointers to classes inherited from this class.
 * NOTE: CRefPtr<T> does not delete the class when references go to
 * zero. When you remove a class from the scene, the class will be
 * deleted if the reference count is zero. Otherwise, you are
 * responsible for deleting it (for example, if you keep the class
 * backed up in an undo stack for undeletion). You may use CSmartPtr<T>
 * when the class is no longer owned by the scene container.
 */
class CReferenceTarget : public CReferenceMaker, public NLMISC::CRefCount
{
public:
	CReferenceTarget();
	virtual ~CReferenceTarget();

	// class desc
	static const ucchar *DisplayName;
	static const char *InternalName;
	static const char *InternalNameUnknown;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual void parse(uint16 version, TParseLevel level);
	virtual void clean();
	virtual void build(uint16 version);
	virtual void disown();
	virtual void init();
	virtual bool inherits(const NLMISC::CClassId classId) const;
	virtual const ISceneClassDesc *classDesc() const;
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "") const;

}; /* class CReferenceTarget */

typedef CSceneClassDesc<CReferenceTarget> CReferenceTargetClassDesc;
extern const CReferenceTargetClassDesc ReferenceTargetClassDesc;
typedef CSuperClassDesc<CReferenceTarget> CReferenceTargetSuperClassDesc;
extern const CReferenceTargetSuperClassDesc ReferenceTargetSuperClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_REFERENCE_TARGET_H */

/* end of file */
