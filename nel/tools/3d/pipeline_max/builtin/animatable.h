/**
 * \file animatable.h
 * \brief CAnimatable
 * \date 2012-08-22 08:52GMT
 * \author Jan Boon (Kaetemi)
 * CAnimatable
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

#ifndef PIPELINE_ANIMATABLE_H
#define PIPELINE_ANIMATABLE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../scene.h"
#include "../scene_class.h"
#include "../super_class_desc.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

class CAppData;

}

/**
 * \brief CAnimatable
 * \date 2012-08-22 08:52GMT
 * \author Jan Boon (Kaetemi)
 * This scene class owns the AppData chunk
 */
class CAnimatable : public CSceneClass
{
public:
	CAnimatable(CScene *scene);
	virtual ~CAnimatable();

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

	// public
	STORAGE::CAppData *appData();

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	IStorageObject *m_Unknown2140;
	STORAGE::CAppData *m_AppData;

}; /* class CAnimatable */

typedef CSceneClassDesc<CAnimatable> CAnimatableClassDesc;
extern const CAnimatableClassDesc AnimatableClassDesc;
typedef CSuperClassDesc<CAnimatable> CAnimatableSuperClassDesc;
extern const CAnimatableSuperClassDesc AnimatableSuperClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_ANIMATABLE_H */

/* end of file */
