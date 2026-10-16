/**
 * \file biped.h
 * \brief CBiped
 * \date 2026-07-05
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
 * CBiped
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

#ifndef PIPELINE_BIPED_H
#define PIPELINE_BIPED_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../dll_plugin_desc.h"

namespace PIPELINE {
namespace MAX {

class CSceneClassRegistry;

namespace BIPED {

/**
 * \brief CBiped
 * \date 2026-07-05
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
 * Registers the typed classes for character studio's Biped controller (biped.dlc), a third-party
 * plugin DLL like UPDATE1/EPOLY — not part of 3ds Max's own builtin classes. Confirmed via corpus
 * inspection: every observed Biped-related ClassId (0x9155 Biped system, 0x9156 Vertical/
 * Horizontal/Turn, 0x9154 BipDriven/BipSlave Control, Biped SubAnim, 0x9125 Biped Object)
 * resolves through ClassDirectory3 to a DllDirectory entry named "biped.dlc", never to the
 * internal "Builtin" pseudo-entry.
 */
class CBiped
{
public:
	static void registerClasses(CSceneClassRegistry *registry);

public:
	CBiped();
	virtual ~CBiped();

}; /* class CBiped */

/**
 * \brief CDllPluginDescBiped
 * \date 2026-07-05
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
 * CDllPluginDescBiped
 */
class CDllPluginDescBiped : public IDllPluginDescInternal
{
public:
	virtual const ucchar *displayName() const NL_OVERRIDE;
	virtual const ucchar *internalName() const NL_OVERRIDE;
}; /* class CDllPluginDescBiped */

extern const CDllPluginDescBiped DllPluginDescBiped;

} /* namespace BIPED */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_BIPED_H */

/* end of file */
