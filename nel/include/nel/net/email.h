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

#ifndef NL_EMAIL_H
#define NL_EMAIL_H

#include <string>

#include "nel/misc/report.h"


namespace NLNET {

/** Send an email
 * \param smtpServer must be a smtp email server.
 * \param from must be a valid email address. If empty, create a fake email address with anonymous@<ipaddress>.com
 * \param to must be a valid email address.
 * \param subject subject of the email. Can be empty.
 * \param body body of the email. Can be empty.
 * \param attachedFile a filename that will be send with the email. Can be empty.
 * \param onlyCheck If true, It'll not send the mail but only check if it could be send.
 */

bool sendEmail (const std::string &smtpServer, const std::string &from, const std::string &to, const std::string &subject, const std::string &body, const std::string &attachedFile = "", bool onlyCheck = false);

/**  If you call this function, the default from (when from is "") used in the sendEmail will be the one
 * you set by this function
 */
void setDefaultEmailParams (const std::string &smtpServer, const std::string &from, const std::string &to);

} // NLNET


#endif // NL_EMAIL_H

/* End of email.h */
