/**
 * \file reference_maker.h
 * \brief CReferenceMaker
 * \date 2012-08-22 08:52GMT
 * \author Jan Boon (Kaetemi)
 * CReferenceMaker
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

#ifndef PIPELINE_REFERENCE_MAKER_H
#define PIPELINE_REFERENCE_MAKER_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../storage_array.h"
#include "animatable.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CReferenceMaker
 * \date 2012-08-22 08:52GMT
 * \author Jan Boon (Kaetemi)
 * This class implements references
 */
class CReferenceMaker : public CAnimatable
{
public:
	CReferenceMaker(CScene *scene);
	virtual ~CReferenceMaker();

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

	// child classes should inherit, default implementation stores in a vector
	/// Get a reference
	virtual CReferenceMaker *getReference(uint index) const;
	virtual void setReference(uint index, CReferenceMaker *reference);
	virtual uint nbReferences() const;

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);
	/// Storage method
	bool m_ReferenceMap;

private:
	CStorageValue<uint8> *m_204B_Equals_2E;

	/// Default implementation, should preferably not use this, no direct read access will be provided
	std::vector<NLMISC::CRefPtr<CReferenceMaker> > m_References;
	/// Unknown value
	uint32 m_References2035Value0;

	CStorageRaw *m_Unknown2045;
	CStorageRaw *m_Unknown2047;
	CStorageRaw *m_Unknown21B0;

}; /* class CReferenceMaker */

typedef CSceneClassDesc<CReferenceMaker> CReferenceMakerClassDesc;
extern const CReferenceMakerClassDesc ReferenceMakerClassDesc;
typedef CSuperClassDesc<CReferenceMaker> CReferenceMakerSuperClassDesc;
extern const CReferenceMakerSuperClassDesc ReferenceMakerSuperClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_REFERENCE_MAKER_H */

/* end of file */
