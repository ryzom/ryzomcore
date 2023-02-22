// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef NL_QUIC_SELFSIGN_H
#define NL_QUIC_SELFSIGN_H

#include "nel/misc/types_nl.h"

/// Find or create a self signed testing certificate
/// Returns a PCCERT_CONTEXT, writes a 20 bytes hash to certHash
extern void *FES_findOrCreateSelfSignedCertificate(uint8 *certHash);

/// Frees the certificate
extern void FES_freeSelfSignedCertificate(void *cert);

#endif /* NL_QUIC_SELFSIGN_H */

/* end of file */
