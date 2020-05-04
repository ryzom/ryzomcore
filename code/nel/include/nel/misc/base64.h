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


#ifndef NL_BASE64_H
#define NL_BASE64_H

namespace NLMISC
{

/**
 * \brief base64 encode/decode
 * \date 2020-01-23 12:39GMT
 * \author Meelis Mägi (Nimetu)
 */

struct base64 {
	static std::string encode(const std::string &data)
	{
		/* Conversion table.  for base 64 */
		static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
								"abcdefghijklmnopqrstuvwxyz"
								"0123456789+/";
		static const char padChar = '=';

		size_t inLength = data.size();
		size_t outLength = 4 * ((inLength + 2) / 3);

		std::string out;
		out.resize(outLength);

		size_t iRead=0, oWrite=0;
		for (size_t iLoop = 0; iLoop < inLength/3; iLoop++)
		{
			out[oWrite+0] = tbl[ (data[iRead+0] >> 2) & 0x3F];
			out[oWrite+1] = tbl[((data[iRead+0] << 4) & 0x30) + ((data[iRead+1] >> 4) & 0x0F)];
			out[oWrite+2] = tbl[((data[iRead+1] << 2) & 0x3C) + ((data[iRead+2] >> 6) & 0x03)];
			out[oWrite+3] = tbl[  data[iRead+2] & 0x3F];
			iRead += 3;
			oWrite += 4;
		}

		// remaining bytes
		switch(inLength % 3)
		{
			case 2:
				out[oWrite+0] = tbl[ (data[iRead+0] >> 2) & 0x3F];
				out[oWrite+1] = tbl[((data[iRead+0] << 4) & 0x30) + ((data[iRead+1] >> 4) & 0x0F)];
				out[oWrite+2] = tbl[((data[iRead+1] << 2) & 0x3C)];
				out[oWrite+3] = padChar;
				break;
			case 1:
				out[oWrite+0] = tbl[ (data[iRead+0] >> 2) & 0x3F];
				out[oWrite+1] = tbl[((data[iRead+0] << 4) & 0x30)];
				out[oWrite+2] = padChar;
				out[oWrite+3] = padChar;
				break;
			default:
				break;
		}

		return out;
	}

	static std::string decode(const std::string &in)
	{
		static sint8 tbl[] = {
		//  00  01  02  03  04  05  06  07  08  09  0a  0b  0c  0d  0e  0f
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 00
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 10
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, // 20 +  /
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1,  0, -1, -1, // 30 0..9 =
			-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 40 A..
			15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, // 50 ..Z
			-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 60 a..
			41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, // 70 ..z
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 80
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 90
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // A0
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // B0
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // C0
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // D0
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // E0
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // F0
		};
		static char padChar = '=';

		size_t inLength = in.size();
		// add optional padding if its missing from input
		size_t outLength = (inLength + inLength % 4) / 4 * 3;

		std::string out;
		if (inLength > 0)
		{
			uint8 buf[4];
			size_t iBuf = 0;
			size_t iRead = 0;
			size_t oWrite = 0;

			out.resize(outLength);
			while(iRead < inLength && in[iRead] != padChar)
			{
				buf[iBuf] = (uint8)tbl[in[iRead]];
				// invalid byte in input
				if (buf[iBuf] == 0xFF)
					break;

				iRead++;
				iBuf++;
				if (iBuf == 4)
				{
					out[oWrite+0] = ((buf[0] << 2) & 0xFC) + ((buf[1] >> 4) & 0x0F);
					out[oWrite+1] = ((buf[1] << 4) & 0xF0) + ((buf[2] >> 2) & 0x0F);
					out[oWrite+2] = ((buf[2] << 6) & 0xC0) +  (buf[3] & 0x3F);
					oWrite += 3;
					iBuf = 0;
				}
			}

			if (iBuf > 0)
			{
				uint8 tmp[3];
				tmp[0] = ((buf[0] << 2) & 0xFC) + ((buf[1] >> 4) & 0x0F);
				tmp[1] = ((buf[1] << 4) & 0xF0) + ((buf[2] >> 2) & 0x0F);
				tmp[2] = ((buf[2] << 6) & 0xC0) +  (buf[3] & 0x3F);
				for(uint i = 0; i < iBuf-1; i++, oWrite++)
					out[oWrite] = tmp[i];
			}

			if (out.size() != oWrite)
				out.resize(oWrite);
		}

		return out;
	}
};

}//namespace NLMISC

#endif // NL_BASE64_H

