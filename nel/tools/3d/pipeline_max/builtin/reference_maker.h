/**
 * \file reference_maker.h
 * \brief CReferenceMaker
 * \date 2012-08-22 08:52GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7
 * \author Claude Opus 4.8
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
 * \author Claude Opus 4.7
 * \author Claude Opus 4.8
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
	/// Storage method: false = 0x2034 (flat array), true = 0x2035 (sparse pairs).
	/// Only load-bearing when a reference chunk existed in the source or one is authored.
	bool m_ReferenceMap;
	/// Set when the source stream had a 0x2034 or 0x2035 chunk. build() only re-emits the
	/// reference chunk when this is set OR when references have been authored — otherwise
	/// classes that never had a reference chunk in the .max would get one on write, breaking
	/// byte-identity.
	bool m_HasReferencesChunk;

private:
	CStorageValue<uint8> *m_204B_Equals_2E;

	/// Default implementation, should preferably not use this, no direct read access will be provided
	std::vector<NLMISC::CRefPtr<CReferenceMaker> > m_References;
	/// Unknown value
	uint32 m_References2035Value0;
	/// Length of the source 0x2034 flat reference array (0 when none / not the 0x2034 form).
	/// build() re-emits at least this many entries so trailing empty (-1) slots survive a
	/// rebuild — subclasses that store references in their own vector (CTrackViewNode's
	/// m_Children, CSceneImpl's fixed slots) grow that vector only up to the last non-empty
	/// slot, so nbReferences() alone would silently drop the trailing -1 entries (a roundtrip
	/// defect: observed on a TVNode with two empty trailing slots in the sfx corpus).
	uint32 m_References2034Count;

	// Unknown chunks preserved verbatim. Types are IStorageObject* rather than CStorageRaw*
	// because the source can flag any of these as a container — createChunkById defers to the
	// default (CStorageContainer for container chunks, CStorageRaw for leaves) rather than
	// forcing a specific type.
	IStorageObject *m_Unknown2045;
	IStorageObject *m_Unknown2047;
	IStorageObject *m_Unknown21B0;

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
