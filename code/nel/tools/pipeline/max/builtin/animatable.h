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
#include "../scene_class.h"
#include "../scene_class_unknown.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CAnimatable
 * \date 2012-08-22 08:52GMT
 * \author Jan Boon (Kaetemi)
 * This scene class owns the AppData chunk
 */
class CAnimatable : public CSceneClass
{
public:
	CAnimatable();
	virtual ~CAnimatable();

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
	virtual const ISceneClassDesc *classDesc();
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "") const;

}; /* class CAnimatable */

typedef CSceneClassDesc<CAnimatable> CAnimatableClassDesc;
extern CAnimatableClassDesc AnimatableClassDesc;
typedef CSuperClassDesc<CAnimatable, CSceneClassUnknown<CAnimatable> > CAnimatableSuperClassDesc;
extern CAnimatableSuperClassDesc AnimatableSuperClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_ANIMATABLE_H */

/* end of file */
