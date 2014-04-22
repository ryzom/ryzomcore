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

#include "stdnet.h"

#include "nel/misc/report.h"
#include "nel/misc/path.h"

#include "nel/net/tcp_sock.h"
#include "nel/net/email.h"

using namespace std;
using namespace NLMISC;


namespace NLNET {

static string DefaultSMTPServer, DefaultFrom, DefaultTo;

/* Conversion table.  for base 64 */
static char tbl[65] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/',
	'=' /* termination character */
};

/*
 * Encode the string S of length LENGTH to base64 format and place it
 * to STORE.  STORE will be 0-terminated, and must point to a writable
 * buffer of at least 1+BASE64_LENGTH(length) bytes.
 * where BASE64_LENGTH(len) = (4 * ((LENGTH + 2) / 3))
 */
static void uuencode (const char *s, const char *store, const int length)
{
	int i;
	unsigned char *p = (unsigned char *)store;
	unsigned char *us = (unsigned char *)s;

	/* Transform the 3x8 bits to 4x6 bits, as required by base64.  */
	for (i = 0; i < length; i += 3)
	{
		*p++ = tbl[us[0] >> 2];
		*p++ = tbl[((us[0] & 3) << 4) + (us[1] >> 4)];
		*p++ = tbl[((us[1] & 0xf) << 2) + (us[2] >> 6)];
		*p++ = tbl[us[2] & 0x3f];
		us += 3;
	}
	/* Pad the result if necessary...  */
	if (i == length + 1)
	{
		*(p - 1) = tbl[64];
	}
	else if (i == length + 2)
	{
		*(p - 1) = *(p - 2) = tbl[64];
	}
	/* ...and zero-terminate it.  */
	*p = '\0';
}

bool sendEMailCommand (CTcpSock &sock, const std::string &command, uint32 code = 250)
{
	string buffer = command + "\r\n";
	uint32 size = (uint32)buffer.size();
	if(!command.empty())
	{
		if (sock.send ((uint8 *)buffer.c_str(), size) != CSock::Ok)
		{
			nlwarning ("EMAIL: Can't send data to the server");
			return false;
		}
	}

	string res;
	char c;
	for(;;)
	{
		size = 1;

		if (sock.receive((uint8*)&c, size, false) == CSock::Ok)
		{
			res += c;
			if (c == '\n')
			{
				uint32 c;
				fromString(res, c);
				if (c != code)
				{
					nlwarning ("EMAIL: EMail command '%s' returned '%s' instead of code %d on sock %s", command.substr(0, 20).c_str(), res.substr(0, res.size()-2).c_str(), code, sock.remoteAddr().asString().c_str());
					return false;
				}
				return true;
			}
		}
		else
		{
			nlwarning ("EMAIL: EMail connection closed before end of line, command '%s' returned '%s' on sock %s (code %d)", command.substr(0, 20).c_str(), res.c_str(), sock.remoteAddr().asString().c_str(), code);
			return false;
		}
	}
}


bool sendEmail (const string &smtpServer, const string &from, const string &to, const string &subject, const string &body, const string &attachedFile, bool onlyCheck)
{
	bool ok  = false;
	CTcpSock sock;
	uint i;

	string formatedBody;
	string formatedFrom;
	string formatedTo;
	string formatedSMTPServer;

	try
	{

		if (smtpServer.empty())
		{
			if(DefaultSMTPServer.empty())
			{
				nlwarning ("EMAIL: Can't send email because no SMTPServer was provided");
				goto end;
			}
			else
			{
				formatedSMTPServer = DefaultSMTPServer;
			}
		}
		else
		{
			formatedSMTPServer = smtpServer;
		}

		sock.connect(CInetAddress(formatedSMTPServer, 25));

		if (!sock.connected())
		{
			nlwarning ("EMAIL: Can't connect to email server %s", formatedSMTPServer.c_str());
			goto end;
		}

		if (to.empty())
		{
			if(DefaultTo.empty())
			{
				nlwarning ("EMAIL: Can't send email because no To was provided");
				goto end;
			}
			else
			{
				formatedTo = DefaultTo;
			}
		}
		else
		{
			formatedTo = to;
		}

		if(from.empty())
		{
			if (DefaultFrom.empty())
			{
				formatedFrom = CInetAddress::localHost().hostName();
				formatedFrom += "@gnu.org";
			}
			else
			{
				formatedFrom = DefaultFrom;
			}
		}
		else
		{
			formatedFrom = from;
		}

		// we must skip the first line
		formatedBody = "\r\n";

		// replace \n with \r\n
		for (i = 0; i < body.size(); i++)
		{
			if (body[i] == '\n' && i > 0 && body[i-1] != '\r')
			{
				formatedBody += '\r';
			}
			formatedBody += body[i];
		}

		// add attachment if any
		if (!attachedFile.empty())
		{
			string ext = CFile::getExtension(attachedFile);

			string mimepart;

			// mime header and main mail text

			mimepart += "Mime-Version: 1.0\r\n";
			mimepart += "Content-Type: multipart/mixed;\r\n";
			mimepart += " boundary=\"Multipart_nel\"\r\n";
			mimepart += "\r\n";
			mimepart += "This is a multi-part message in MIME format.\r\n";
			mimepart += "\r\n";
			mimepart += "--Multipart_nel\r\n";
			mimepart += "Content-Type: text/plain; charset=us-ascii\r\n";
			mimepart += "Content-Transfer-Encoding: 7bit\r\n";

			formatedBody = mimepart + formatedBody;

			// mime attachment

			formatedBody += "--Multipart_nel\r\n";
			formatedBody += "Content-Disposition: attachment;\r\n";

			string lext = toLower(ext);
			if(lext == "tga")
			{
				formatedBody += "Content-Type: image/x-targa;\r\n";
			}
			else if(lext == "bmp")
			{
				formatedBody += "Content-Type: image/bmp;\r\n";
			}
			else if(lext == "png")
			{
				formatedBody += "Content-Type: image/png;\r\n";
			}
			else if(lext == "jpg" || lext == "jpeg")
			{
				formatedBody += "Content-Type: image/jpeg;\r\n";
			}
			else if(lext == "dmp")
			{
				formatedBody += "Content-Type: application/octet-stream;\r\n";
			}
			else
			{
				formatedBody += "Content-Type: text/plain; charset=us-ascii\r\n";
			}

			formatedBody += " name=\""+CFile::getFilename(attachedFile)+"\"\r\n";
			formatedBody += "Content-Transfer-Encoding: base64\r\n";
			formatedBody += " filename=\""+CFile::getFilename(attachedFile)+"\"\r\n";
			// empty line to say that it s the end of the header
			formatedBody += "\r\n";

			static const size_t src_buf_size = 45;// This *MUST* be a multiple of 3
			static const size_t dst_buf_size = 4 * ((src_buf_size + 2) / 3);
			size_t write_size = dst_buf_size;
			char src_buf[src_buf_size + 1];
			char dst_buf[dst_buf_size + 1];
			size_t size;

			FILE *src_stream = fopen (attachedFile.c_str(), "rb");
			if (src_stream == NULL)
			{
				nlwarning ("EMAIL: Can't attach file '%s' to the email because the file can't be open", attachedFile.c_str());
			}
			else
			{
				while ((size = fread(src_buf, 1, src_buf_size, src_stream)) > 0)
				{
					if (size != src_buf_size)
					{
						/* write_size is always 60 until the last line */
						write_size=(4 * ((size + 2) / 3));
						/* pad with 0s so we can just encode extra bits */
						memset(&src_buf[size], 0, src_buf_size - size);
					}
					/* Encode the buffer we just read in */
					uuencode(src_buf, dst_buf, (int)size);

					formatedBody += dst_buf;
					formatedBody += "\r\n";
				}
				fclose (src_stream);
			}
			formatedBody += "--Multipart_nel--";
		}

		// debug, display what we send into a file
		//	{	FILE *fp = fopen (CFile::findNewFile(getLogDirectory() + "mail.txt").c_str(), "wb");
		//	fwrite (formatedBody.c_str(), 1, formatedBody.size(), fp);
		//	fclose (fp); }

		if(!sendEMailCommand (sock, "", 220)) goto end;

		if(onlyCheck)
		{
			if(!sendEMailCommand (sock, "HELO localhost")) goto end;
			if(!sendEMailCommand (sock, "MAIL FROM: " + formatedFrom)) goto end;
			if(!sendEMailCommand (sock, "RCPT TO: " + formatedTo)) goto end;
			if(!sendEMailCommand (sock, "QUIT", 221)) goto end;

			ok = true;
		}
		else
		{
			if(!sendEMailCommand (sock, "HELO localhost")) goto end;
			if(!sendEMailCommand (sock, "MAIL FROM: " + formatedFrom)) goto end;
			if(!sendEMailCommand (sock, "RCPT TO: " + formatedTo)) goto end;
			if(!sendEMailCommand (sock, "DATA", 354)) goto end;

			string buffer =
				"From: " + formatedFrom + "\r\n"
				"To: " + formatedTo + "\r\n"
				"Subject: " + subject + "\r\n"
				+ formatedBody + "\r\n.";

			if(!sendEMailCommand (sock, buffer)) goto end;
			if(!sendEMailCommand (sock, "QUIT", 221)) goto end;

			ok = true;
		}
	}
	catch (const Exception &e)
	{
		nlwarning ("EMAIL: Can't send email: %s", e.what());
		goto end;
	}

end:
	if (sock.connected())
		sock.close ();

	return ok;
}

void setDefaultEmailParams (const std::string &smtpServer, const std::string &from, const std::string &to)
{
	DefaultSMTPServer = smtpServer;
	DefaultFrom = from;
	DefaultTo = to;
}

} // NLNET
