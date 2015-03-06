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

#if FINAL_VERSION
#define NL_REPORT_SYNCHRONOUS false
#define NL_REPORT_DEFAULT NLMISC::ReportAbort
#else
#define NL_REPORT_SYNCHRONOUS true
#define NL_REPORT_DEFAULT NLMISC::ReportBreak
#endif

enum TReportResult
{
	// See also crash_report_widget.h EReturnValue
	ReportAlwaysIgnore = 21,
	ReportIgnore = 22,
	ReportAbort = 23,
	ReportBreak = 24
};

/** Display a crash report
 *
 * \param title set the title of the report. If empty, it'll display "NeL report"
 * \param subject extended title of the report
 * \param body message displayed in the edit text box. This string will be sent to the crash report tool
 * \param attachment binary file to attach. This is a filename
 * \param synchronous use system() and wait for the crash tool exit code, passes -dev flag; otherwise return defaultResult immediately
 * \param sendReport hide 'dont send' button, or auto enable 'send report' checkbox
 *
 * \return the button clicked or defaultResult
 */
TReportResult report(const std::string &title, const std::string &subject, const std::string &body, const std::string &attachment, bool synchronous, bool sendReport, TReportResult defaultResult);

/// Set the Url of the web service used to post crash reports to. String is copied
void setReportPostUrl(const char *postUrl);

} // NLMISC

#endif // NL_REPORT_H

/* End of report.h */
