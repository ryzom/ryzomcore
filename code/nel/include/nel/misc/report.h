// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef NL_REPORT_H
#define NL_REPORT_H

#include "types_nl.h"

namespace NLMISC {

/// Prepares the error report, writes it to disk and launches the error reporter
void report ( const std::string &body );

/** call this in the main of your appli to enable email: setReportEmailFunction (sendEmail);
 */
void setReportEmailFunction (void *emailFunction);

} // NLMISC

#endif // NL_REPORT_H

/* End of report.h */
