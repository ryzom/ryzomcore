/**
 * \file scene_impl.h
 * \brief CSceneImpl
 * \date 2012-08-24 12:33GMT
 * \author Jan Boon (Kaetemi)
 * CSceneImpl
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

#ifndef PIPELINE_SCENE_IMPL_H
#define PIPELINE_SCENE_IMPL_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "reference_maker.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CSceneImpl
 * \date 2012-08-22 08:53GMT
 * \author Jan Boon (Kaetemi)
 * Scene implementation
 */
class CSceneImpl : public CReferenceMaker
{
public:
	CSceneImpl(CScene *scene);
	virtual ~CSceneImpl();

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
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

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CSceneImpl */

typedef CSceneClassDesc<CSceneImpl> CSceneImplClassDesc;
extern const CSceneImplClassDesc SceneImplClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SCENE_IMPL_H */

/* end of file */
